/*

Sterownik silnika BLDC.
Sterowanie silnikiem z wykorzystaniem Back-EMF.

Cz�stotliwo�� F_CPU: 8MHz (ustaw w opcjach projektu)

Szczeg�y: http://mikrokontrolery.blogspot.com/2011/03/silnik-bldc-sterownik-back-emf.html

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

	//zmienne potrzebne do przyk�adu zmiany pr�dko�ci
	//patrz p�tla g��wna while() poni�ej
	unsigned char 	PWM_akt;
	unsigned char 	PWM_kier = 1;   //kierunek zmiany pr�dko�ci
										//0 - zmniejszanie
										//1 - zwi�kszanie


	//diody testowe
	DDRC	|= (1<<PC5) | (1<<PC4);		//wyj�cie LED
	PORTC	|= (1<<PC5) | (1<<PC4);		//zga� diody, bo s� zapalane zerem


	//op�nienie na ustabilizowanie zasilania (mo�na usun��)
	_delay_ms(1000);

	//inicjujemy sterownik
	bldc_inicjuj_sterownik();


	//Przerwania globalne w��cz
	sei();

	//Start silnika
	bldc_start();

	//odczytaj aktualnie ustawione PWM (dla potrzeb przyk�adu poni�ej)
	PWM_akt = OCR1A;

	//p�tla g��wna
	while(1){


		//... tutaj dowolny Tw�j program


		//--- PRZYK�AD ------------------------
		//Przyk�ad funkcji zmieniaj�cej pr�dko�� obrot�w silnika
		//tylko wtedy, gdy sterownik ju� rozkr�ci� silnik i przeszed�
		//w tyb pracy

		//Aby przyk�ad dzia�a� usu� na pocz�tku main() komentarze
		//dla deklaracji zmiennych PWM_akt oraz PWM_kier

		if(bldc_tryb_pracy	== TRYB_PRACA){

			//PWM_akt += skok_PWM;

			if(PWM_kier) 	PWM_akt++;
			else	 		PWM_akt--;


			//sprawdzamy, czy osi�gn�li�my pr�g pr�dko�ci maksymalnej
			if(PWM_akt >254) PWM_kier = 0; //od teraz zmniejszamy obroty

			//sprawdzamy, czy osi�gn�li�my pr�g pr�dko�ci minimalnej
			if(PWM_akt <40)	PWM_kier = 1; //od teraz zwi�kszamy obroty

			//ustaw aktualne PWM w celu regulacji pr�dko�ci
			OCR1A	=	PWM_akt;
			OCR1B	=	PWM_akt;
			OCR2	=	PWM_akt;

			//dla test�w, op�nienie zmian pr�dko�ci
			_delay_ms(100);

		}
		//--- PRZYK�AD - Koniec -------------


		//... tutaj dowolny Tw�j program


	}
}






