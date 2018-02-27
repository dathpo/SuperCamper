#include <msp430.h>

int state=1;

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
	state=!state;
    P1IFG &= ~BIT3;
}
//Voltage TEST.
int main(void)
{
	P1DIR |= BIT4;
	P1DIR|= BIT5;
	P1DIR &= ~BIT3;

	P1REN |= BIT3;
	P1IES |= BIT3;
	P1IE |= BIT3;
	P1IFG &= ~BIT3;
	ADC10CTL0 |= REF2_5V;
	ADC10CTL0 |= REFON;

	__enable_interrupt();
	while(1)
	{
		if(state==1)
		{
			P1OUT &= ~BIT4;
			P1OUT &= ~BIT5;
			__delay_cycles(20);
			P1OUT |= BIT5;
		}else{
			P1OUT &= ~BIT4;
			P1OUT &= ~BIT5;
			__delay_cycles(20);
			P1OUT |= BIT4;
		}
	}
}

