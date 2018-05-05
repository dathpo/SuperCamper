#include "msp430g2553.h"
#include <msp430.h>
#include <string.h>

#define TXLED BIT0
//#define RXLED BIT6
#define RXD BIT1
#define TXD BIT2

#define TX_CLOCK 4096

#define MCU_CLOCK           1000000
#define PWM_FREQUENCY       46      // In Hertz, ideally 50Hz.

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           650     // The minimum duty cycle for this servo
#define SERVO_MAX           2700    // The maximum duty cycle

unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;                            // %
unsigned int MOTOR_PWM_Duty = 0;

char temp[50];
//WiFi access point.
const char * AP[]={
        //"Z\r\n",
        "AT+RST\r\n",
        //"AT+CWMODE=2\r\n",
        //"AT+CWSAP=\"Fuckthis\",\"Ciao1234567\",3,3\r\n",
        "AT+CWMODE_CUR=2\r\n",
        "AT+CWSAP_CUR=\"Fuckthis\",\"Ciao1234567\",3,3\r\n",
        //"AT+CIFSR\r\n",
        "AT+CIPMUX=1\r\n",
        "AT+CIPSERVER=1,100\r\n",
        //"AT+SLEEP=0\r\n",
};


//Creates TCP server. Port number is 333 by default.
//const char test[] = {"AT+CIPSERVER=1,100\r\n"};

//Default mode as access point. Configuration saved in flash.
//const char string2[] = {"AT+CWMODE_DEF=3\r\n"};

//Default connects to Wifi.
//const char string[] =

//Default connection mode is 1, multiple connections.
//const char string[] = { "AT+CIPMUX=1\r\n"};

unsigned int i;     //Counter
int index=0;
int size;
int flag_left = 0;
int flag_right = 0;
int flag_center = 0;
int flag_on = 0;
int flag_off = 0;
int flag_forwards = 0;
int flag_backwards = 0;
int main(void)
{
    WDTCTL  = WDTPW + WDTHOLD;     // Kill watchdog timer

    __enable_interrupt();

    unsigned int servo_stepval, servo_stepnow;
    unsigned int servo_lut[ SERVO_STEPS+1 ];
    unsigned int position;
    size= sizeof(AP)/sizeof(AP[0]);
    BCSCTL3 |= LFXT1S_2;    //initialise clock registers

    // Calculate the step value and define the current step, defaults to minimum.
    servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
    servo_stepnow   = SERVO_MIN;

    // Fill up the LUT
    for (position = 0; position < SERVO_STEPS; position++) {
        servo_stepnow += servo_stepval;
        servo_lut[position] = servo_stepnow;
    }

    // Setup the PWM, etc.
    TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
    TACCR0  = PWM_Period-1;        // PWM Period
    TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR   |= BIT6;               // P2.6 = output
    P1SEL   |= BIT6;               // P2.6 = TA1 output

//  // P1.3 Button Interrupt Config
//  P1IE |=  BIT3;                            // P1.3 interrupt enabled
//    P1IES |= BIT3;                            // P1.3 Hi/lo edge
//    P1REN |= BIT3;                            // Enable Pull Up on SW2 (P1.3)
//    P1IFG &= ~BIT3;                           // P1.3 IFG cleared
//                                              //BIT3 on Port 1 can be used as Switch
    // Motor Control Setup
    TACCTL2 = OUTMOD_7;            // TACCR1 reset/set
    TACCR2  = MOTOR_PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR |= BIT4;                            // Set P1.0 to output direction
    P1SEL &= ~BIT4;
    P1OUT &= ~BIT4;
    P1DIR |= BIT5;

    // WiFi Setup
    DCOCTL = 0;              // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;   // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P2DIR |= 0xFF;           // All P2.x outputs
    P2OUT &= 0x00;           // All P2.x reset
    P1SEL |= RXD + TXD ;     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ;    // P1.1 = RXD, P1.2=TXD
    P1DIR |= TXLED;
    P1OUT &= ~TXLED;


    //P1OUT &= 0x00;
    UCA0CTL1 |= UCSSEL_2;        // SMCLK
    UCA0BR0 = 0x08;              // 1MHz 115200
    UCA0BR1 = 0x00;              // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0;  // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**
    UC0IE |= UCA0TXIE;          // Enable USCI_A0 TX interrupt
    UC0IE &= ~UCA0RXIE;         // Disable RX.

    position=81;

    // Transmission Timer Setup
    TA1CCTL0 |= CCIE;  //Interrupt Enable
    TA1CTL |= TASSEL_1 + MC_1 + ID_3; // Use ACLK (32768 Hz), divide by 8 = 4096, divide by CCR0
    TA1CCR0 = TX_CLOCK;

    // Transmission Timer Setup
    TA1CCTL1 &= ~CCIE;  //Interrupt Disabe - only to be used as a delay to servo rotation
    TA1CCR1 |= 256; // 0.125s


    __delay_cycles(200000);
    // Transmission Timer Setup
    TA1CCTL0=CCIE;  //Interrupt Enable
    TA1CTL = TASSEL_1 + MC_1 + ID_3; // Use ACLK (32768 Hz), divide by 8 = 4096, divide by CCR0
    TA1CCR0 = TX_CLOCK;
    TACCR1 = servo_lut[position];
    //Center is position 95.
    __delay_cycles(20000);

    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed

    //Max range is 117 (right side).
    //Min range is 78. (left side).
    while (1) {
        if(flag_forwards){
            P1OUT |= BIT5;
            TACCR2 |= 0;
            flag_forwards = 0;
        }
        else if(flag_backwards){
            P1OUT &= ~BIT5;
            TACCR2 |= 0;
            flag_backwards = 0;
        }
        if(flag_on){
            P1SEL |= BIT4;
            P1OUT |= BIT4;
            flag_on=0;
        }
        else if(flag_off){
            P1SEL &= ~BIT4;
            P1OUT &= ~BIT4;
            flag_off=0;
        }
        if(flag_right||flag_left){
            // Move right toward the maximum step value
            if(flag_right){
                position=117;
                TACCR1 = servo_lut[position];
                TA1CCTL1 |= CCIE;
                flag_right = 0;
            }
            // Move left toward the minimum step value
            if(flag_left){
                position=45;
                TACCR1 = servo_lut[position];
                TA1CCTL1 |= CCIE;
                flag_left = 0;
            }
        }
        if(flag_center){
            position=81;
            TACCR1 = servo_lut[position];
            TA1CCTL1 |= CCIE;
            flag_center=0;
        }
        TA1CCTL1 |= CCIE;
        __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ int until Byte RXed
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{

    //P1OUT |= TXLED;
    if (index>=size){       //if whole message has been sent
        P1OUT &= ~TXLED;    //Turn off transmitter LED
        UC0IE &= ~UCA0TXIE; // Disable transmitter interrupts
        UC0IE |= UCA0RXIE; //Enable receiver interrupts
    }
    else{       //more commands to send
        temp[i] = AP[index][i];
        UCA0TXBUF = AP[index][i++]; // iterate next character
        if (i == strlen(AP[index])+1){ // message is over
            index++;    //go to next message
            i=0;
            UC0IE &= ~UCA0TXIE; // Disable transmitter interrupts
            TA1CCTL0 = CCIE;              // Timer 1 interrupt enable
            memset(&temp[0], 0, sizeof(temp));
        }
    }
}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer(void)
{
        TA1CCTL0 &= ~CCIE;            // Timer 1 interrupt disable
        P1OUT |= TXLED;
        UC0IE |= UCA0TXIE;          // Enable transmitter interrupts
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void timerServo(void)
{
        TA1CCTL1 &= ~CCIE;            // Timer 1 interrupt disable
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    P1OUT ^= TXLED;
    //On.
    if(UCA0RXBUF=='7'){
        flag_on = 1;
        //flag_forwards = 0;
        //flag_backwards = 0;

        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
    //Off.
    if(UCA0RXBUF=='9'){
        //P1SEL &= ~BIT4;
        //P1OUT &= ~BIT4;
        flag_off = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }

    if(UCA0RXBUF=='6'){
        flag_right = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
    if(UCA0RXBUF=='4'){
        flag_left = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
    if(UCA0RXBUF=='5'){
        flag_center = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
    if(UCA0RXBUF=='8'){
        flag_forwards = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
    if(UCA0RXBUF=='2'){
        flag_backwards = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Enter LPM0 w/ int until Byte RXed
    }
}


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  //turn the engine on and off
  P1SEL ^= BIT4;
  P1OUT ^= BIT4;

  //toggle direction
  P1OUT ^= BIT5;                            // P1.0 = toggle

  P1IFG &= ~BIT3;                           // P1.3 IFG cleared
}
