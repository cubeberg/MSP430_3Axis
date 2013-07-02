#include <msp430.h> 
#include "stepper_3AX.h"
#include "main.h"
/*
 * main.c
 */

#define BAUD115200 //will default to 9600 otherwise

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;
    if (CALBC1_16MHZ != 0xFF) {
		/* Adjust this accordingly to your VCC rise time */
		__delay_cycles(100000);

		/* Follow recommended flow. First, clear all DCOx and MODx bits. Then
		 * apply new RSELx values. Finally, apply new DCOx and MODx bit values.
		 */
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
		DCOCTL = CALDCO_16MHZ;
	}
    BCSCTL1 |= XT2OFF + DIVA_0;
    BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
    initUart();
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD






    IFG2 &= ~(UCA0TXIFG + UCA0RXIFG);
    IE2 |= UCA0TXIE + UCA0RXIE;
    //initButtons();
    StepperInit();
    StepperEnable();

	StepperGoTo(0,0,zHold);


	//testing this out to see if it reduces heat on steppers.  Don't want to
	//StepperDisable();

    __bis_SR_register(GIE);
	return 0;
}
void initUart()
{
    UCA0CTL1 |= UCSWRST;
    UCA0CTL1 = UCSSEL_2 + UCSWRST;
#ifdef BAUD115200
    UCA0MCTL = UCBRF_0 + UCBRS_7;
    UCA0BR0 = 138;
    UCA0BR1 = 0;
#else
    UCA0MCTL = UCBRF_0 + UCBRS_6;
    UCA0BR0 = 130;
	UCA0BR1 = 6;
#endif


    UCA0CTL1 &= ~UCSWRST;
}

int ConvertString(char startIndex, char stopIndex)
{
	int returnVal = 0;
	char i;
	int multiplier = 1;
	for(i = stopIndex; i >= startIndex;i--)
	{
		returnVal += ((RXBuffer[i] - 48) * multiplier);
		multiplier *= 10;
	}
	return returnVal;
}

struct Point ProcessBuffer()
{
	struct Point returnVal;
	int x,y,z, temp;
	x = -1;
	y = -1;
	z = -1;
	char startIndex,stopIndex;
	char mode = 0; //0=waiting,1=x,2=y,3=z

	if(bufIndex > 2)
	{
		//some sort of move command - ignoring everything else for now
		if(RXBuffer[0] == 'G' && RXBuffer[1] == '0' && (RXBuffer[2] == '0' | RXBuffer[2] == '1'))
		{
			char i;
			for(i = 3; i< bufIndex; i++)
			{
				if(mode == 0) //waiting for some sort of input
				{
					if (RXBuffer[i] == 'X')
					{
						mode = 1;
						startIndex = 0;
						stopIndex = 0;
					}
					else if (RXBuffer[i] == 'Y')
					{
						mode = 2;
						startIndex = 0;
						stopIndex = 0;
					}
					else if (RXBuffer[i] == 'Z')
					{
						mode = 3;
						startIndex = 0;
						stopIndex = 0;
					}
				}
				else if(mode > 0 && mode < 4)
				{
					if(startIndex >  0 && (RXBuffer[i] == '.' || RXBuffer[i] == ' ')) //we've got our first digit - this detects the end of a number
					{
						temp = ConvertString(startIndex, stopIndex);
						if(mode == 1)
							x = temp;
						else if (mode == 2)
							y = temp;
						else if (mode == 3)
							z = temp;
						mode = 0; //go back to waiting for the next command
					}
					else if (RXBuffer[i] >= 48 && RXBuffer[i] <= 57) //numeric character
					{
						if(startIndex == 0)
						{
							startIndex = i;
							stopIndex = i;
						}
						else
							stopIndex = i;
					}

				}
			}
			_nop();//for breakpoint
		}
	}
	returnVal.x = x;
	returnVal.y = y;
	returnVal.z = z;
	StepperGoToGCode(returnVal);
	SendSerial("d\r\n", 3);
	return returnVal;
}

void SendSerial(char * text, char len)
{
	char index;
	for(index = 0;index < len;index++)
	{
		char toDisp = *text++;
		TXBuffer[index] = toDisp;
	}
	txIndex = 0;
	txLen = len;
	SendSerialNext();//start first transmit
}
void SendSerialNext()
{
	if(txIndex < txLen)
	{
		IE2 |= UCA0TXIE;
		//while (!(IFG2&UCA0TXIFG));
		UCA0TXBUF = TXBuffer[txIndex];
		txIndex++;
	}
	else
	{
		IE2 &= ~UCA0TXIE;
		txLen = 0;
		txIndex = 0;
	}
}

//UART RX interrupt
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK(void)
{
	if(bufIndex >= 50)
		bufIndex = 0; //just in case we get a really long string


	RXBuffer[bufIndex] = UCA0RXBUF;
	bufIndex++;

	if(UCA0RXBUF == '\n')
	{
		ProcessBuffer();
		bufIndex = 0;
	}
}

//UART TX interrupt
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR_HOOK(void)
{
    SendSerialNext();
}

