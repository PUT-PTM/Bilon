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


//------------------------------------------------------------------

int main (void)
{

	//zmienne potrzebne do przykładu zmiany prędkości
	//patrz pętla główna while() poniżej
	unsigned char 	PWM_akt;
	unsigned char 	PWM_kier = 1;   //kierunek zmiany prędkości
										//0 - zmniejszanie
										//1 - zwiększanie


	//diody testowe
	DDRC	|= (1<<PC5) | (1<<PC4);		//wyjście LED
	PORTC	|= (1<<PC5) | (1<<PC4);		//zgaś diody, bo są zapalane zerem


	//opóźnienie na ustabilizowanie zasilania (można usunąć)
	_delay_ms(1000);

	//inicjujemy sterownik
	bldc_inicjuj_sterownik();


	//Przerwania globalne włącz
	sei();

	//Start silnika
	bldc_start();

	//odczytaj aktualnie ustawione PWM (dla potrzeb przykładu poniżej)
	PWM_akt = OCR1A;

	//pętla główna
	while(1){


		//... tutaj dowolny Twój program


		//--- PRZYKŁAD ------------------------
		//Przykład funkcji zmieniającej prędkość obrotów silnika
		//tylko wtedy, gdy sterownik już rozkręcił silnik i przeszedł
		//w tyb pracy

		//Aby przykład działał usuń na początku main() komentarze
		//dla deklaracji zmiennych PWM_akt oraz PWM_kier

		if(bldc_tryb_pracy	== TRYB_PRACA){

			//PWM_akt += skok_PWM;

			if(PWM_kier) 	PWM_akt++;
			else	 		PWM_akt--;


			//sprawdzamy, czy osiągnęliśmy próg prędkości maksymalnej
			if(PWM_akt >254) PWM_kier = 0; //od teraz zmniejszamy obroty

			//sprawdzamy, czy osiągnęliśmy próg prędkości minimalnej
			if(PWM_akt <40)	PWM_kier = 1; //od teraz zwiększamy obroty

			//ustaw aktualne PWM w celu regulacji prędkości
			OCR1A	=	PWM_akt;
			OCR1B	=	PWM_akt;
			OCR2	=	PWM_akt;

			//dla testów, opóźnienie zmian prędkości
			_delay_ms(100);

		}
		//--- PRZYKŁAD - Koniec -------------


		//... tutaj dowolny Twój program


	}
}






