#include <msp430.h>				

#define MCU_CLOCK           1000000
#define PWM_FREQUENCY       30      // In Hertz, ideally 50Hz.
unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;                            // %
/**
 * blink.c
 */

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1OUT ^= BIT5;                            // P1.0 = toggle
  P1IFG &= ~BIT3;                           // P1.3 IFG cleared
}

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

    __enable_interrupt();

    P1IE |=  BIT3;                            // P1.3 interrupt enabled
    P1IES |= BIT3;                            // P1.3 Hi/lo edge
    P1REN |= BIT3;                            // Enable Pull Up on SW2 (P1.3)
    P1IFG &= ~BIT3;                           // P1.3 IFG cleared
                                              //BIT3 on Port 1 can be used as Switch
    TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
    TACCR0  = PWM_Period-1;        // PWM Period
    TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR |= BIT4;                            // Set P1.0 to output direction
    P1SEL |= BIT4;

    P1DIR |= BIT5;

    volatile unsigned int i;		// volatile to prevent optimization

    while(1)
    {
        __delay_cycles(50);
    }
}

