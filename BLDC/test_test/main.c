/*

Sterownik silnika BLDC.
Sterowanie silnikiem z wykorzystaniem Back-EMF.

Czêstotliwoœæ F_CPU: 8MHz (ustaw w opcjach projektu)

Szczegó³y: http://mikrokontrolery.blogspot.com/2011/03/silnik-bldc-sterownik-back-emf.html

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

	//zmienne potrzebne do przyk³adu zmiany prêdkoœci
	//patrz pêtla g³ówna while() poni¿ej
	unsigned char 	PWM_akt;
	unsigned char 	PWM_kier = 1;   //kierunek zmiany prêdkoœci
										//0 - zmniejszanie
										//1 - zwiêkszanie


	//diody testowe
	DDRC	|= (1<<PC5) | (1<<PC4);		//wyjœcie LED
	PORTC	|= (1<<PC5) | (1<<PC4);		//zgaœ diody, bo s¹ zapalane zerem


	//opóŸnienie na ustabilizowanie zasilania (mo¿na usun¹æ)
	_delay_ms(1000);

	//inicjujemy sterownik
	bldc_inicjuj_sterownik();


	//Przerwania globalne w³¹cz
	sei();

	//Start silnika
	bldc_start();

	//odczytaj aktualnie ustawione PWM (dla potrzeb przyk³adu poni¿ej)
	PWM_akt = OCR1A;

	//pêtla g³ówna
	while(1){


		//... tutaj dowolny Twój program


		//--- PRZYK£AD ------------------------
		//Przyk³ad funkcji zmieniaj¹cej prêdkoœæ obrotów silnika
		//tylko wtedy, gdy sterownik ju¿ rozkrêci³ silnik i przeszed³
		//w tyb pracy

		//Aby przyk³ad dzia³a³ usuñ na pocz¹tku main() komentarze
		//dla deklaracji zmiennych PWM_akt oraz PWM_kier

		if(bldc_tryb_pracy	== TRYB_PRACA){

			//PWM_akt += skok_PWM;

			if(PWM_kier) 	PWM_akt++;
			else	 		PWM_akt--;


			//sprawdzamy, czy osi¹gnêliœmy próg prêdkoœci maksymalnej
			if(PWM_akt >254) PWM_kier = 0; //od teraz zmniejszamy obroty

			//sprawdzamy, czy osi¹gnêliœmy próg prêdkoœci minimalnej
			if(PWM_akt <40)	PWM_kier = 1; //od teraz zwiêkszamy obroty

			//ustaw aktualne PWM w celu regulacji prêdkoœci
			OCR1A	=	PWM_akt;
			OCR1B	=	PWM_akt;
			OCR2	=	PWM_akt;

			//dla testów, opóŸnienie zmian prêdkoœci
			_delay_ms(100);

		}
		//--- PRZYK£AD - Koniec -------------


		//... tutaj dowolny Twój program


	}
}






