

/*
 * 0    : 600
 * 90   : 1650
 * 180  : 2700
 */

#include <msp430.h>

#define ROTATE_RESET    0
#define ROTATE_0        1500
#define ROTATE_90       1650
#define ROTATE_180      1800

typedef unsigned short WORD;
typedef unsigned long DWORD;


void delay (DWORD);


void delay (DWORD ms) {
    unsigned int i;
    for (i=ms; i>0; i--) {
        __delay_cycles(1000);       // set for 16Mhz change it to 1000 for 1 Mhz
    }
}


#define servo_rotate(arg1) \
    for (i=0; i<30; i++) {  \
        P1OUT |= BIT2;      \
        __delay_cycles(arg1);   \
        P1OUT &= ~BIT2;         \
        __delay_cycles(20000);  \
    }


void servo_reset () {
    int i;
    servo_rotate(ROTATE_RESET);
}


void main (void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    P1DIR = BIT2 | BIT0 | BIT6;
    P1OUT |= BIT0;
    P1OUT &= ~BIT6;
    int i;

    for (;;) {
        servo_reset();
        servo_rotate(ROTATE_0);
        servo_rotate(ROTATE_90);
        servo_rotate(ROTATE_180);


        P1OUT ^= BIT0;
        P1OUT ^= BIT6;

        delay(5000);    // delay


    }

}

/*
void main (void) {
    unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
    unsigned int PWM_Duty       = 0;                            // %

    unsigned int servo_stepval, servo_stepnow;
    unsigned int servo_lut[ SERVO_STEPS+1 ];
    unsigned int i;

    // Calculate the step value and define the current step, defaults to minimum.
    servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
    servo_stepnow   = SERVO_MIN;

    // Fill up the LUT
    for (i = 0; i < SERVO_STEPS; i++) {
        servo_stepnow += servo_stepval;
        servo_lut[i] = servo_stepnow;
    }

    // Setup the PWM, etc.
    WDTCTL  = WDTPW + WDTHOLD;     // Kill watchdog timer
    TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
    TACCR0  = PWM_Period-1;        // PWM Period
    TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR   |= BIT2;               // P1.2 = output
    P1SEL   |= BIT2;               // P1.2 = TA1 output


    // DC power (NOT WORKING, perhaps to low voltage output ?)
    P1DIR   |= BIT3;               // VCC
    P1DIR   |= BIT4;               // GND
    P1OUT   = BIT3 | ~BIT4;



    // Main loop
    while (1){

        // Go to 0°
        TACCR1 = servo_lut[0];
        __delay_cycles(1000000);

        // Go to 45°
        TACCR1 = servo_lut[45];
        __delay_cycles(1000000);


        // Go to 90°
        TACCR1 = servo_lut[90];
        __delay_cycles(1000000);

        // Go to 180°
        TACCR1 = servo_lut[179];
        __delay_cycles(1000000);

        // Move forward toward the maximum step value

        for (i = 0; i < SERVO_STEPS; i++) {
            TACCR1 = servo_lut[i];
            __delay_cycles(20000);
        }               // Move backward toward the minimum step value      for (i = SERVO_STEPS; i &gt; 0; i--) {

        TACCR1 = servo_lut[i];
        __delay_cycles(20000);
    }

}
*/
