/*

Sterownik silnika BLDC.
Sterowanie silnikiem z wykorzystaniem Back-EMF.

Częstotliwość F_CPU: 8MHz (ustaw w opcjach projektu)

Szczegóły: http://mikrokontrolery.blogspot.com/2011/03/silnik-bldc-sterownik-back-emf.html

2013 Dondu

*/


//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/interrupt.h>

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_tim.h>
#include "bldc.h"

volatile unsigned char	bldc_polozenie_wirnika = KOMUT_1;

volatile unsigned char bldc_tryb_pracy;   //aktualny tryb pracy
volatile unsigned int  bldc_komp_licznik; //licznik przerwań komparatora



//--------------------------------------------------------------------

void bldc_bezpiecznik_stop(void){

	//Funkcja wyłącza wszelkie tranzystory oraz przechodzi w stan sygnalizacji
	//błędu komutacji. Funkcja ta razem z funkcją bezpiecznik() pełni rolę
	//zabezpieczenia przeciwzwarciowego dla błędnie działającego algorytmu
	//komutacji w czasie pisania i testów programu.

	//wyłącz przerwania
	cli();

	//natychmiast wyłącz tranzystory
	WYLACZ_TRANZYSTORY

	//i ustaw stany niskie na pinach sterujących 
	U_TR_G_USTAW_DDR
	V_TR_G_USTAW_DDR
	W_TR_G_USTAW_DDR
	U_TR_D_USTAW_DDR
	V_TR_D_USTAW_DDR
	W_TR_D_USTAW_DDR
	U_TR_G_PIN_L
	V_TR_G_PIN_L
	W_TR_G_PIN_L
	U_TR_D_PIN_L
	V_TR_D_PIN_L
	W_TR_D_PIN_L

	//ustaw pin LED jako wyjście
	BEZP_LED_DDR	|=	(1<<BEZP_LED_PIN);

	//zatrzymaj program w pętli nieskończonej sygnalizując błąd 
	while(1){ 

		//zmień stan LED na przeciwny
		BEZP_LED_PORT  ^= (1<<BEZP_LED_PIN);		

		//co 100ms
		_delay_ms(100);

	}
}


//------------------------------------------------------------------

void bldc_bezpiecznik(void){

	//Sprawdzamy, czy nie występuje konflikt sterowania, powodujący
	//jednoczene otwarcie tranzystora górnego i dolnego w tej samej fazie,
	//co oznacza wystąpienie zwarcia !!!

	if(U_TR_G_SPRAW_STAN && U_TR_D_SPRAW_STAN){

		//Faza U - oba tranzystory są włączone - sytuacja niedopuszczalna!!!
		bldc_bezpiecznik_stop();


	}else if(V_TR_G_SPRAW_STAN && V_TR_D_SPRAW_STAN){

		//Faza V - oba tranzystory są włączone - sytuacja niedopuszczalna!!!
		bldc_bezpiecznik_stop();

		
	}else if(W_TR_G_SPRAW_STAN && W_TR_D_SPRAW_STAN){

		//Faza W - oba tranzystory są włączone - sytuacja niedopuszczalna!!!
		bldc_bezpiecznik_stop();

	}

}



//-----------------------------------------------------------


void bldc_inicjuj_sterownik(void){

	//Funkcja inicjująca pracę sterownika.
	//-konfiguruje wszystkie wykorzystywane moduły mikrokontrolera
	//-ustawia piny mikrokontrolera tak, by wszystkie tranzystory
	// były wyłączone
	//-ustawia licznikia PWM na początkową wartość DutyCycle

	//ustaw stan początkowy wyjść sterujących tranzystorami
	U_TR_G_USTAW_DDR
	V_TR_G_USTAW_DDR
	W_TR_G_USTAW_DDR
	U_TR_D_USTAW_DDR
	V_TR_D_USTAW_DDR
	W_TR_D_USTAW_DDR
	U_TR_G_PIN_L
	V_TR_G_PIN_L
	W_TR_G_PIN_L
	U_TR_D_PIN_L
	V_TR_D_PIN_L
	W_TR_D_PIN_L

	

	//sprawdzamy, czy nie ma stanu zabronionego na tranzystorach
	//ZAWSZE WYWOŁUJ TĘ FUNKCJĘ, GDY ZMIENIASZ STAN TRANZYSTORÓW!!!
	bldc_bezpiecznik();



	//--- T I M E R Y    P W M   -------------------

	//Timer1
	//Obsługa PWM dla wyjść OC1A oraz OC1B
	//Mode 5 (Fast PWM, 8bit)  
	//Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM, 
	//(non-inverting mode)
	//preskaler 1
	TCCR1A	|=	(1<<WGM10) | (1<<COM1A1) | (1<<COM1B1);
	TCCR1B	|=	(1<<WGM12) | (1<<CS10);
	 
	
	//Timer2
	//Obsługa PWM  dla wyjścia OC2
	//Mode 3 (Fast PWM, 8bit)  
	//Clear OC2 on Compare Match, set OC2 at BOTTOM, (non-inverting mode)
	//preskaler 1
	TCCR2	|=	(1<<WGM21) | (1<<WGM20) | (1<<COM21) | (1<<CS20);


	//--- K O M P A R A T O R ---
	//Obsługuje wykrywanie położenia wirnika poprzez BEMF,
	//czyli ustalanie momentów przejścia przez zero sygnałów sterujących
	//przerwanie dla każdej zmiany wyjścia komparatora
	//Przerwanie jest włączane w funkcji startującej silnik.
	SFIOR	|= (1<<ACME);	//włącz multiplekser wejść ujemnych komparatora


	//ustaw wartość początkową PWM
	OCR1A 	= PWM_MIN;
	OCR1B 	= PWM_MIN;
	OCR2 	= PWM_MIN;

	//na wszelki wypadek
	WYLACZ_TRANZYSTORY
	bldc_bezpiecznik();

}



//------------------------------------------------------------------




void bldc_start_delay_ms(unsigned char il_ms){

	//Funkcja opóźnienia o zmiennej wartości czasu

	while(il_ms){
		_delay_ms(1);
		il_ms--;
	}
}


//-----------------------------------------------------------

void bldc_start_komutacja(void){

	//Funkcja komutacji w czasie rozruchu silnika.
	//nie zawiera obsługi BackEMF

	//zależnie od numeru aktualnej komutacji przełącza na następną
	switch (bldc_polozenie_wirnika++){

		case (KOMUT_1):	//komutacja 1
				W_TR_G_OFF
				U_TR_G_ON
			break;

		case (KOMUT_2):	//komutacja 2
				V_TR_D_OFF
				W_TR_D_ON
			break;

		case (KOMUT_3):	//komutacja 3
				U_TR_G_OFF
				V_TR_G_ON
			break;
	
		case (KOMUT_4):	//komutacja 4
				W_TR_D_OFF
				U_TR_D_ON
			break;

		case (KOMUT_5):	//komutacja 5
				V_TR_G_OFF
				W_TR_G_ON
			break;

		case (KOMUT_6):	//komutacja 6
				U_TR_D_OFF
				bldc_polozenie_wirnika = KOMUT_1; //komutacja od początku
				V_TR_D_ON
			break;

	}

	//sprawdzamy konflikty sterowania
	bldc_bezpiecznik();
}


//-----------------------------------------------------------

void bldc_start(void){

	//Funkcja odpowiedzialna za:
	//- ustawienie wirnika w pozycji początkowej 
	//- rozkręcenie silnika do momentu uzyskania odczytów BackMEF

	signed int i; 	//zmienna pomocnicza

	//wartości początkowe
	bldc_tryb_pracy			= TRYB_START;



	//RUSZAMY SILNIK
	//Poniżej przykład kolejnych kroków, mających na celu ruszenie silnika
	//z miejsca oraz rozkręcenie go do momentu w którym uzyskamy sygnał 
	//BackEMF. Po osiągnięciu odpowiedniej ilości przerwań z komparatora,
	//uznajemy, że nadszedł moment w którym należy się przełączyć na 
	//sterowanie komutacjami z wykorzystaniem przerwań BackEMF (komparatora).

	//UWAGA!
	//W zależności od silnika należy dobrać doświadczalnie procedurę startu

		

	//KROK 1
	//Ustaw wirnik w pozycji startowej
	bldc_polozenie_wirnika	= KOMUT_1;	
	W_TR_G_ON				//włączamy wybraną parę tranzystorów
	V_TR_D_ON
	bldc_bezpiecznik();		//sprawdzamy konflikty sterowania


	//odczekujemy oscylacje wirnika zmniejszając wypełnienie PWM
	//tutaj możesz dostosować doświadczalnie parametry pozycjonowania silnika
	//do bezwładności wirnika
	for(i=100; i>0; i-=5){

		//ustaw nowe PWM wykorzystując jawne rzutowanie http://mikrokontrolery.blogspot.com/2011/02/kurs-jezyka-c-rzutowanie-promocja-typow.html
		OCR1A = (unsigned char) i;
		OCR1B = (unsigned char) i;
		OCR2  = (unsigned char) i;

		//opóźnienie
		_delay_ms(300);
	}


	//KROK 2
	//ruszamy silnik z miejsca

	//początkowa wartość PWM dla startu silnika (regulacja prądu)
	OCR1A = 70;
	OCR1B = 70;
	OCR2  = 70;

	//rusz silnik z miejsca przełączając komutacje ze zmienjszającym się 
	//opóźnieniem, by osiągnąć początkową prędkość
	for(i=100; i>50; i-=10){
		bldc_start_komutacja();	//kolejna komutacja
		bldc_start_delay_ms(i);	//opóźnienie 
	}
	

	//KROK 3
	//zwiększaj dalej prędkość, ale mniejszymi krokami PWM
	for(i=60; i>=9; i-=1){
		bldc_start_komutacja();	//kolejna komutacja
		bldc_start_delay_ms(i);	//opóźnienie

	}
	

	//KROK 4
	//włącz przerwania z komparatora ponieważ już mogą być prawidłowe 
	//sygnały BEMF.
	ACSR	|= (1<<ACIE);

	//ustabilizuj obroty i czekaj na wykrycie odpowiedniej ilości przerwań
	for(i=0; i<200; i++){

		bldc_start_komutacja(); //kolejna komutacja
		bldc_start_delay_ms(9); //opóźnienie

		//Przerwij, gdy wykryto już wystarczającą ilość przerwań z komparatora
		//ilość impulsów zależy tylko od Ciebie - ja wybrałem 40
		if(bldc_komp_licznik > 40){
			
			//Wykryto odpowiednią ilość przerwań z sygnałów BackEMF
			//uznajemy więc, że możemy przejść na sterowanie za pomocą 
			//przerwań z sygnałów BackEMF

			//ustaw tryb pracy i wróć do main()
			bldc_tryb_pracy	= TRYB_PRACA;
			break;
		}

	}

	
	//KROK 5
	//sprawdzamy, czy sterownik przełączył się na tryb pracy
	if(bldc_tryb_pracy	!= TRYB_PRACA){

		//Sterownik nie przełączył się, ponieważ pomimo wielu obrotów wirnika
		//komparator nie wykrył założonej ilości impulsów sygnału BackEMF, 
		//stąd przerywamy procedurę startu.
		
		//Jeżeli sytuacja będzie się powtarzać, powinieneś zmienić parametry 
		//kroków 2, 3 i 4
		
		//wyłącz przerwania z komparatora
		ACSR	&= ~(1<<ACIE);	

		//wyłącz tranzystory
		WYLACZ_TRANZYSTORY
		bldc_bezpiecznik();

	}

	//wróć do main()

}



//-----------------------------------------------------------


ISR (ANA_COMP_vect)
{
	
	//Funkcja obsługi przerwania z komparatora wykrywającego punkty ZC
	//funkcja zajmuje się wykryciem właściwego położenia wirnika
	//oraz przełączeniem następnej komutacji


	//DLA TESTÓW ZC możesz podłączyć do pinu PB0 oscyloskop i obserwować 
	//moment wykonania się przerwania. Wcześniej musisz ustawić 
	//pin PBO jako wyjście
	//PORTB	^=	(1<<PB0);	//zmień stan pinu na przeciwny

	//w zależności od trybu pracy sterownika
	switch (bldc_tryb_pracy){

		//-------------------------------------------------	

		case TRYB_START:
				//Podczas trybu startu jedynie zliczaj wykryte przerwania
				//z komparatora, czyli punkty ZC sygnału BackEMF
				bldc_komp_licznik++;
				break;

		//-------------------------------------------------

		case TRYB_PRACA: 

			//tryb normalnej pracy czyli kręcenia silnikiem na podstawie
			//wykrytych punktów ZC sygnałów BackEMF

			//w zależności od położenia wirnika (fakt wystąpienia przerwania 
			//oraz sprawdzenie stanu wyjścia komparatora)
			switch (bldc_polozenie_wirnika)
			{
				case (KOMUT_1):	//kąt 0 stopni
					if(!KOMP_STAN_AKT)	//gdy komparator = 0
					{
						W_TR_G_OFF		//wyłącz tranzystor górny W
						KOMP_KANAL_W;	//włącz kanał fazy W
						bldc_polozenie_wirnika = KOMUT_2; //następna komutacja
						U_TR_G_ON		//włącz tranzystor górny U
					}
					break;

				case (KOMUT_2):	//kąt 60 stopni
					if(KOMP_STAN_AKT)	//gdy komparator = 1
					{
						V_TR_D_OFF		//wyłącz tranzystor dolny V
						KOMP_KANAL_V;	//włącz kanał fazy V
						bldc_polozenie_wirnika = KOMUT_3; //następna komutacja
						W_TR_D_ON		//włącz tranzystor dolny W
					}
					break;

				case (KOMUT_3):	//kąt 120 stopni
					if(!KOMP_STAN_AKT)	//gdy komparator = 0
					{
						U_TR_G_OFF		//wyłącz tranzystor górny U
						KOMP_KANAL_U;	//włącz kanał fazy U
						bldc_polozenie_wirnika = KOMUT_4; //następna komutacja
						V_TR_G_ON		//włącz tranzystor górny V
					}
					break;
	
				case (KOMUT_4):	//kąt 180 stopni
					if(KOMP_STAN_AKT)	//gdy komparator = 1
					{
						W_TR_D_OFF		//wyłącz tranzystor dolny W
						KOMP_KANAL_W;	//włącz kanał fazy W
						bldc_polozenie_wirnika = KOMUT_5; //następna komutacja
						U_TR_D_ON		//włącz tranzystor dolny U
					}
					break;

				case (KOMUT_5):	//kąt 240 stopni
					if(!KOMP_STAN_AKT)	//gdy komparator = 0
					{
						V_TR_G_OFF		//wyłącz tranzystor górny V
						KOMP_KANAL_V;	//włącz kanał fazy V
						bldc_polozenie_wirnika = KOMUT_6; //następna komutacja
						W_TR_G_ON		//włącz tranzystor górny W
					}
					break;

				case (KOMUT_6):	//kąt 300 stopni
					if(KOMP_STAN_AKT)	//komparator = 1
					{
						U_TR_D_OFF	  	//wyłącz tranzystor dolny U
						KOMP_KANAL_U;	//włącz kanał fazy U
						bldc_polozenie_wirnika = KOMUT_1; //komut. od początku
						V_TR_D_ON		//włącz tranzystor dolny V
					}
					break;
			}

			//sprawdzamy konflikty sterowania
			bldc_bezpiecznik();

			break;

	}
}

