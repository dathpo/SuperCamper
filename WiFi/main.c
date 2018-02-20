#include "msp430g2553.h"
#include <msp430.h>
#include <string.h>

#define TXLED BIT0
//#define RXLED BIT6
#define RXD BIT1
#define TXD BIT2

#define MCU_CLOCK           1000000
#define PWM_FREQUENCY       46      // In Hertz, ideally 50Hz.

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           650     // The minimum duty cycle for this servo
#define SERVO_MAX           2700    // The maximum duty cycle

unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;                            // %

//Server.
//const char * Server[]={
//"AT+CWMODE=3\r\n",
//"AT+CWJAP=\"meng-project\",\"ptzcamera\"\r\n",
//"AT+CIPMUX=1\r\n",
//"AT+CIPSERVER=1,100\r\n",
//};

//WiFi access point.
const char * AP[]={
		//"Z\r\n",
		"AT+RST\r\n",
		"AT+CWMODE=3\r\n",
		"AT+CWSAP=\"Test\",\"password\",3,0\r\n",
		"AT+CIFSR\r\n",
		"AT+CIPMUX=1\r\n",
		"AT+CIPSERVER=1,100\r\n",
		"AT+SLEEP=0\r\n",
};


//Creates TCP server. Port number is 333 by default.
//const char test[] = {"AT+CIPSERVER=1,100\r\n"};

//Default mode as access point. Configuration saved in flash.
//const char string2[] = {"AT+CWMODE_DEF=3\r\n"};

//Default connects to Wifi.
//const char string[] =

//Default connection mode is 1, multiple connections.
//const char string[] = { "AT+CIPMUX=1\r\n"};

unsigned int i; 	//Counter
int index=0;
int size;
int flag_left = 0;
int flag_right = 0;

int main(void)
{
	unsigned int servo_stepval, servo_stepnow;
	unsigned int servo_lut[ SERVO_STEPS+1 ];
	unsigned int j;
	size= sizeof(AP)/sizeof(AP[0]);
	// Calculate the step value and define the current step, defaults to minimum.
	servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
	servo_stepnow   = SERVO_MIN;

	// Fill up the LUT
	for (j = 0; j < SERVO_STEPS; j++) {
		servo_stepnow += servo_stepval;
		servo_lut[j] = servo_stepnow;
	}

	// Setup the PWM, etc.
	WDTCTL  = WDTPW + WDTHOLD;     // Kill watchdog timer
	TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
	TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
	TACCR0  = PWM_Period-1;        // PWM Period
	TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
	P1DIR   |= BIT6;               // P2.6 = output
	P1SEL   |= BIT6;               // P2.6 = TA1 output

	// WiFi Setup
	DCOCTL = 0; 			 // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;   // Set DCO
	DCOCTL = CALDCO_1MHZ;
	P2DIR |= 0xFF;			 // All P2.x outputs
	P2OUT &= 0x00; 			 // All P2.x reset
	P1SEL |= RXD + TXD ;     // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD ;    // P1.1 = RXD, P1.2=TXD
	P1DIR |= TXLED;
	P1OUT &= 0x00;
	UCA0CTL1 |= UCSSEL_2;        // SMCLK
	UCA0BR0 = 0x08;              // 1MHz 115200
	UCA0BR1 = 0x00;              // 1MHz 115200
	UCA0MCTL = UCBRS2 + UCBRS0;  // Modulation UCBRSx = 5
	UCA0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**
	UC0IE |= UCA0TXIE;          // Enable USCI_A0 TX interrupt
	UC0IE &= ~UCA0RXIE;         // Disable RX.

	//Comment this line out for servo use.
	__bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed

	while (1) {
		if(flag_right||flag_left){
			// Move forward toward the maximum step value
			if(flag_right){
				for (j = 45; j < 125; j++) {
					TACCR1 = servo_lut[j];
					__delay_cycles(20000);
				}}
			// Move backward toward the minimum step value
			if(flag_left){
				for (j = 125; j > 44; j--) {
					TACCR1 = servo_lut[j];
					__delay_cycles(20000);
				}
			}
			__bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed
			UC0IE &= ~UCA0TXIE;
			//UC0IE |= UCA0RXIE;
			flag_right = 0;
			flag_left = 0;
		}
	}
}
#pragma vector=USCIAB0TX_VECTOR
	__interrupt void USCI0TX_ISR(void)
	{
		P1OUT |= TXLED;
		UCA0TXBUF = AP[index][i++]; // TX next character
		if (i == strlen(AP[index])+1) // TX over?
		{
			UC0IE &= ~UCA0TXIE;
			UC0IE |= UCA0RXIE;
		}
		P1OUT &= ~TXLED;
	}

#pragma vector=USCIAB0RX_VECTOR
	__interrupt void USCI0RX_ISR(void)
	{
		//P1OUT |= RXLED;
		index++;

		if(index>=size){
			UC0IE &= ~UCA0TXIE; // Enable USCI_A0 TX interrupt
		}
		else
		{
			UC0IE |= UCA0TXIE;
			UC0IE &= ~UCA0RXIE;
			i=0;
		}
		if(UCA0RXBUF=='Q'){
			flag_right = 1;
		}
		if(UCA0RXBUF=='Z'){
			flag_left = 1;
		}
		if(flag_right||flag_left){
			__bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
		}
	}
		//Toggle LED.
		//P1OUT &= ~RXLED;

