/*

Sterownik silnika BLDC.
Sterowanie silnikiem z wykorzystaniem Back-EMF.

Częstotliwość F_CPU: 8MHz (ustaw w opcjach projektu)

Szczegóły: http://mikrokontrolery.blogspot.com/2011/03/silnik-bldc-sterownik-back-emf.html

2013 Dondu

*/



//--- D E F I N I C J E   D O T.  B E Z P I E C Z N I K A  ------

//LED bezpiecznika
#define BEZP_LED_DDR		DDRD
#define BEZP_LED_PORT		PORTD
#define BEZP_LED_PIN		PD3

//--- D E F I N I C J E   D O T.  S I L N I K A  ------

//Faza U
//tranzystor górny
#define U_TR_G_PORTx		PORTB
#define U_TR_G_DDRx			DDRB
#define U_TR_G_PINx			PINB
#define U_TR_G_PIN			PB1
#define U_TR_G_USTAW_DDR	U_TR_G_DDRx  |=  (1<<U_TR_G_PIN); //ustaw port
#define U_TR_G_PIN_L		U_TR_G_PORTx &= ~(1<<U_TR_G_PIN); //ustaw niski
#define U_TR_G_ON 			TCCR1A		 |=	 (1<<COM1A1);		//włącz tranzystor
#define U_TR_G_OFF			TCCR1A		 &=	~(1<<COM1A1);		//wyłącz tranzystor
#define U_TR_G_SPRAW_STAN	(TCCR1A & (1<<COM1A1))				//warunek stanu 
//tranzystor dolny
#define U_TR_D_PORTx		PORTD
#define U_TR_D_DDRx			DDRD
#define U_TR_D_PINx			PIND
#define U_TR_D_PIN			PD4
#define U_TR_D_USTAW_DDR	U_TR_D_DDRx  |=  (1<<U_TR_D_PIN); //ustaw port
#define U_TR_D_PIN_L		U_TR_D_PORTx &= ~(1<<U_TR_D_PIN); //ustaw niski
#define U_TR_D_ON 			U_TR_D_PORTx |=  (1<<U_TR_D_PIN); //włącz tranz.
#define U_TR_D_OFF			U_TR_D_PORTx &= ~(1<<U_TR_D_PIN); //wyłącz tranz.
#define U_TR_D_SPRAW_STAN	(U_TR_D_PINx  &  (1<<U_TR_D_PIN)) //warunek stanu

//Faza V
//tranzystor górny
#define V_TR_G_PORTx		PORTB
#define V_TR_G_DDRx			DDRB
#define V_TR_G_PINx			PINB
#define V_TR_G_PIN			PB3
#define V_TR_G_USTAW_DDR	V_TR_G_DDRx  |=  (1<<V_TR_G_PIN); //ustaw port
#define V_TR_G_PIN_L		V_TR_G_PORTx &= ~(1<<V_TR_G_PIN); //ustaw niski
#define V_TR_G_ON 			TCCR2		 |=	  (1<<COM21);		//włącz tranzystor
#define V_TR_G_OFF			TCCR2		 &=	 ~(1<<COM21);		//wyłącz tranzystor
#define V_TR_G_SPRAW_STAN	(TCCR2 & (1<<COM21))				//warunek stanu 
//tranzystor dolny
#define V_TR_D_PORTx		PORTD
#define V_TR_D_DDRx			DDRD
#define V_TR_D_PINx			PIND
#define V_TR_D_PIN			PD5
#define V_TR_D_USTAW_DDR	V_TR_D_DDRx  |=  (1<<V_TR_D_PIN); //ustaw port
#define V_TR_D_PIN_L		V_TR_D_PORTx &= ~(1<<V_TR_D_PIN); //ustaw niski
#define V_TR_D_ON 			V_TR_D_PORTx |=  (1<<V_TR_D_PIN); //włącz tranz.
#define V_TR_D_OFF			V_TR_D_PORTx &= ~(1<<V_TR_D_PIN); //wyłącz tranz.
#define V_TR_D_SPRAW_STAN	(V_TR_D_PINx  &  (1<<V_TR_D_PIN)) //warunek stanu


//Faza W
//tranzystor górny
#define W_TR_G_PORTx		PORTB
#define W_TR_G_DDRx			DDRB
#define W_TR_G_PINx			PINB
#define W_TR_G_PIN			PB2
#define W_TR_G_USTAW_DDR	W_TR_G_DDRx  |=  (1<<W_TR_G_PIN); //ustaw port
#define W_TR_G_PIN_L		W_TR_G_PORTx &= ~(1<<W_TR_G_PIN); //ustaw niski
#define W_TR_G_ON 			TCCR1A		 |=	 (1<<COM1B1);		//włącz tranzystor
#define W_TR_G_OFF			TCCR1A		 &=	~(1<<COM1B1);		//wyłącz tranzystor
#define W_TR_G_SPRAW_STAN	(TCCR1A & (1<<COM1B1))				//warunek stanu 
//tranzystor dolny
#define W_TR_D_PORTx		PORTD
#define W_TR_D_DDRx			DDRD
#define W_TR_D_PINx			PIND
#define W_TR_D_PIN			PD7
#define W_TR_D_USTAW_DDR	W_TR_D_DDRx  |=  (1<<W_TR_D_PIN); //ustaw port
#define W_TR_D_PIN_L		W_TR_D_PORTx &= ~(1<<W_TR_D_PIN); //ustaw niski
#define W_TR_D_ON 			W_TR_D_PORTx |=  (1<<W_TR_D_PIN); //włącz tranz.
#define W_TR_D_OFF			W_TR_D_PORTx &= ~(1<<W_TR_D_PIN); //wyłącz tranz.
#define W_TR_D_SPRAW_STAN	(W_TR_D_PINx &   (1<<W_TR_D_PIN)) //warunek stanu


//Kąty położenia wirnika
#define KOMUT_1				1
#define KOMUT_2				2
#define KOMUT_3				3
#define KOMUT_4				4
#define KOMUT_5				5
#define KOMUT_6				6


//Wspólna definicja wyłączająca wszystkie tranzystory
#define WYLACZ_TRANZYSTORY	U_TR_G_OFF; U_TR_D_OFF; V_TR_G_OFF; V_TR_D_OFF; W_TR_G_OFF; W_TR_D_OFF; 			


//--- D E F I N I C J E   P W M ---
#define PWM_MIN   50				//początkowa wartość wypełnienia PWM


//Tryb pracy
#define	TRYB_START			0
#define	TRYB_PRACA			1


//--- D E F I N I C J E   K O M P A R A T O R A ----
#define KOMP_STAN_AKT		(ACSR & (1<<ACO))	//aktualny stan wyjścia 
												//komparatora
#define KOMP_KANAL_U		ADMUX = 0;			//kanał U do pomiaru BEMF
#define KOMP_KANAL_V		ADMUX = 1;			//kanał V do pomiaru BEMF
#define KOMP_KANAL_W		ADMUX = 2;			//kanał W do pomiaru BEMF


extern volatile unsigned char	bldc_polozenie_wirnika;


extern void bldc_bezpiecznik(void);
extern void bldc_bezpiecznik_stop(void);
extern void bldc_start_komutacja(void);
extern void bldc_inicjuj_sterownik(void);
extern void bldc_start_delay_ms(unsigned char il_ms);
extern void bldc_start(void);

extern volatile unsigned char bldc_tryb_pracy;   //aktualny tryb pracy
extern volatile unsigned int  bldc_komp_licznik; //licz. przerwań komparatora




