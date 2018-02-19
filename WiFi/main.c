#include "msp430g2553.h"
#include <stdio.h>

void UARTSendArray(char *TxArray, int ArrayLength);
void UARTSendChar(char *TxChar);
void SwapChars(char *a, char *b);

int i = 0;
int j = 0;
int check = 1;

const char string1[] = { "AT\r\n" };
char string2[8];

void main(void)

{
 WDTCTL = WDTPW + WDTHOLD; // Stop WDT

 P1DIR |= BIT0;    // Set the LEDs on P1.0, P1.6  as outputs
 P1DIR |= BIT6;
 P1DIR &= ~(BIT1|BIT2);

 P1OUT &= ~BIT0;
 P1OUT &= ~BIT6;

 BCSCTL1 = CALBC1_16MHZ;   // Set DCO to 16MHz
 DCOCTL = CALDCO_16MHZ;    // Set DCO to 16MHz

 P1SEL = BIT1 + BIT2 ;    // P1.1 = RXD, P1.2=TXD
 P1SEL2 = BIT1 + BIT2 ;   // P1.1 = RXD, P1.2=TXD

 /* Configure hardware UART */
 UCA0CTL1 = UCSWRST; // hold module in reset
 UCA0CTL1 |= UCSSEL_2;    // Use SMCLK
 UCA0BR0 = 138;           // Set baud rate to 15200 with 16MHz clock (Data Sheet 15.3.13)
 UCA0BR1 = 0;           // Set baud rate to 15200 with 16MHz clock (Data Sheet 15.3.13)
 UCA0MCTL = UCBRS2+UCBRS1+UCBRS0;       // Second Stage Modulation UCBRSx = 7
 UCA0CTL1 &= ~UCSWRST;    // Initialize USCI state machine
 IE2 |= UCA0RXIE;         // Enable USCI_A0 RX interrupt

 while(1){

   if(check == 1)
   {
     P1OUT ^= BIT0;
     P1OUT &= ~BIT6;
     IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
     check = 0;
   }
 }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
 UCA0TXBUF = string1[i++];                 // TX next character

 if (i == sizeof string1 - 1)              // TX over?
 {
   P1OUT &= ~BIT0;
   IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
 }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
 string2[j++] = UCA0RXBUF;
 if(j<sizeof(string2)-1)
 {
   i = 0;
   j = 0;
   P1OUT |= BIT6;
   check = 1;
 }
}
