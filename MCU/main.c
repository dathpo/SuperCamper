#include "msp430g2553.h"
#include <msp430.h>
#include <string.h>

#define TXLED BIT0
//#define RXLED BIT6
#define RXD BIT1
#define TXD BIT2

#define TX_CLOCK 4096               // Corresponds to 1 second

#define MCU_CLOCK           1000000
#define PWM_FREQUENCY       46      // In Hertz, ideally 50Hz.

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           650     // The minimum duty cycle for this servo
#define SERVO_MAX           2700    // The maximum duty cycle

unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;
unsigned int MOTOR_PWM_Duty = 0;

char temp[50];

// AT commands used for ESP8266 configuration.
const char * AP[]={
        //Reset ESP8266 module.
        "AT+RST\r\n",
        //Configure ESP8266 module as SoftAP.
        "AT+CWMODE_CUR=2\r\n",
        //Configure access point.
        "AT+CWSAP_CUR=\"BTOpenzone\",\"Ciao1234567\",3,3\r\n",
        //Enable multiple connections.
        "AT+CIPMUX=1\r\n",
        //Setup TCP server on port 100.
        "AT+CIPSERVER=1,100\r\n",
        //Disable TCP server timeout.
        "AT+CIPSTO=0\r\n",
};

// Variables declaration.
unsigned int i;
int index=0;
int size;
int flag_left = 0;
int flag_right = 0;
int flag_center = 0;
int flag_on = 0;
int flag_off = 0;
int flag_forwards = 0;
int flag_backwards = 0;
int previous = 0;

int main(void)
{
    WDTCTL  = WDTPW + WDTHOLD; // Kill watchdog timer

    __enable_interrupt();

    unsigned int servo_stepval, servo_stepnow;
    unsigned int servo_lut[ SERVO_STEPS+1 ];
    unsigned int position;
    size= sizeof(AP)/sizeof(AP[0]);
    BCSCTL3 |= LFXT1S_2; // Initialise clock registers

    servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS ); // Calculate step value
    servo_stepnow   = SERVO_MIN;

    for (position = 0; position < SERVO_STEPS; position++) { // Fill look up table with positions
        servo_stepnow += servo_stepval;
        servo_lut[position] = servo_stepnow;
    }
    // Max value of is 117 (right side).
    // Min range is 78 (left side).

    // PWM setup.
    TACCTL1 = OUTMOD_7;               // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;        // SMCLK, upmode
    TACCR0  = PWM_Period-1;           // PWM Period
    TACCR1  = PWM_Duty;               // TACCR1 PWM Duty Cycle
    P1DIR   |= BIT6;                  // P2.6 = output
    P1SEL   |= BIT6;                  // P2.6 = TA1 output

    // Motor Control Setup
    TACCTL2 = OUTMOD_7;               // TACCR1 reset/set
    TACCR2  = MOTOR_PWM_Duty;         // TACCR1 PWM Duty Cycle
    P1DIR |= BIT4;                    // Set P1.0 to output direction
    P1SEL &= ~BIT4;                   // Turn Motor Off
    P1OUT &= ~BIT4;
    P1DIR |= BIT5;

    // WiFi Setup
    DCOCTL = 0;                       // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;            // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P2DIR |= 0xFF;                    // All P2.x outputs
    P2OUT &= 0x00;                    // All P2.x reset
    P1SEL |= RXD + TXD ;              // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ;             // P1.1 = RXD, P1.2=TXD
    P1DIR |= TXLED;
    P1OUT &= ~TXLED;

    UCA0CTL1 |= UCSSEL_2;             // SMCLK
    UCA0BR0 = 0x08;                   // 1MHz, 115200 baud rate
    UCA0BR1 = 0x00;                   // 1MHz, 115200 baud rate
    UCA0MCTL = UCBRS2 + UCBRS0;       // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;             // USCI state machine initialisation
    UC0IE |= UCA0TXIE;                // Enable USCI_A0 TX interrupt
    UC0IE &= ~UCA0RXIE;               // Disable RX

    position=117;
    flag_center=1;                    // Initialise at centre position

    // Transmission Timer Setup
    TA1CCTL0 |= CCIE;                 // Interrupt Enable
    TA1CTL |= TASSEL_1 + MC_1 + ID_3; // Use ACLK (32768 Hz), divide by 8 = 4096, divide by CCR0
    TA1CCR0 = TX_CLOCK;

    // Transmission Timer Setup
    TA1CCTL1 &= ~CCIE;                // Interrupt Disable - only to be used as a delay to servo rotation
    TA1CCR1 |= 256;                   // 0.125s


    __delay_cycles(200000);           // Delays for setting up the ESP8266

    TACCR1 = servo_lut[position];     // Start from centre position by default

    __delay_cycles(20000);

    // Main loop
    while (1) {
        if(flag_forwards){            // Car moves forwards
            P1OUT |= BIT5;
            TACCR2 |= 0;
            flag_forwards = 0;
        }
        else if(flag_backwards){      // Car moves backwards
            P1OUT &= ~BIT5;
            TACCR2 |= 0;
            flag_backwards = 0;
        }
        if(flag_on){                  // Car motor is on
            P1SEL |= BIT4;
            P1OUT |= BIT4;
            flag_on=0;
        }
        else if(flag_off){            // Car motor is off
            P1SEL &= ~BIT4;
            P1OUT &= ~BIT4;
            flag_off=0;
        }
        if(flag_right||flag_left){    // Change in direction required

            if(flag_right){           // Move right to maximum value
                position=117;
                TACCR1 = servo_lut[position];
                TA1CCTL1 |= CCIE;
                flag_right = 0;
            }

            if(flag_left){            // Move left to minimum value
                position=45;
                TACCR1 = servo_lut[position];
                TA1CCTL1 |= CCIE;
                flag_left = 0;
                previous = 1;         // Save in memory that the last servo command was left 
            }
        }
        if(flag_center){              // Change to centre
            if(previous){             // Last servo was left -> pick the correct value
                position = 95;
            }
            else{                     // Last servo was right
                position=86;
            }

            TACCR1 = servo_lut[position];
            TA1CCTL1 |= CCIE;
            flag_center=0;
            previous=0;
        }
        TA1CCTL1 |= CCIE;
        __bis_SR_register(CPUOFF + GIE); // Enter low power mode
    }
}

// Interrupt service routine for UART transmission
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    if (index>=size){                   // Check if all instructions have sent
        P1OUT &= ~TXLED;                // Turn off transmitter LED
        UC0IE &= ~UCA0TXIE;             // Disable transmitter interrupt
        UC0IE |= UCA0RXIE;              // Enable receiver interrupt
    }
    else{
        temp[i] = AP[index][i];
        UCA0TXBUF = AP[index][i++];     // Send one character with TX buffer
        if (i == strlen(AP[index])+1){  // Check for end of single instruction
            index++;
            i=0;
            UC0IE &= ~UCA0TXIE;         // Disable transmitter interrupt
            TA1CCTL0 = CCIE;            // Enable timer 1 interrupt
            memset(&temp[0], 0, sizeof(temp));
        }
    }
}
// Interrupt service routine for timer used for ESP8266
#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer(void)
{
        TA1CCTL0 &= ~CCIE;              // Timer 1 interrupt disable
        P1OUT |= TXLED;
        UC0IE |= UCA0TXIE;              // Enable transmitter interrupts
}

// Interrupt service routine for timer used for servo
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timerServo(void)
{
        TA1CCTL1 &= ~CCIE;              // Timer 1 interrupt disable
}

// Interrupt service routine for UART receiver
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    // Motor is on
    if(UCA0RXBUF=='7'){
        flag_on = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE); // Exit low power mode
    }
    // Motor is off
    if(UCA0RXBUF=='9'){
        flag_off = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);
    }
    // Turn right
    if(UCA0RXBUF=='6'){
        flag_right = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);
    }
    // Turn left
    if(UCA0RXBUF=='4'){
        flag_left = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);
    }
    // Turn to centre
    if(UCA0RXBUF=='5'){
        flag_center = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);
    }
    // Move forwards
    if(UCA0RXBUF=='8'){
        flag_forwards = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);

    // Move backwards
    if(UCA0RXBUF=='2'){
        flag_backwards = 1;
        __bic_SR_register_on_exit(CPUOFF+GIE);
    }
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  // Turn the engine on and off
  P1SEL ^= BIT4;
  P1OUT ^= BIT4;

  // Toggle direction
  P1OUT ^= BIT5;                            // P1.0 = toggle
  P1IFG &= ~BIT3;                           // P1.3 IFG cleared
}
