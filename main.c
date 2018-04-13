#include <msp430.h>
#include<stdio.h>

#define POW1 10
#define POW01 1.258925412
#define POW001 1.023292992

/*
 * main.c
 */


float pow10( float exp)
{
    float result = 1;
    int la =0;
    if(exp<0)
    {
    	exp*=-1;
    	la =1;
    }
    while(1)
    {
    	if(exp >= 1)
    	{
    		result *=POW1;
    		exp--;
    	}
    	else
    	{
    		if(exp>=0.1)
    		{
    			result *= POW01;
    			exp -=0.1;
    		}
    		else
    		{
    			if(exp>=0.01)
    			{
    				result *=POW001;
    				exp -=0.01;
    			}
    			else
    				break;
    		}
    	}
    }
    if(la)
    	return 1/result;
    else
    	return result;
}
 float calculateDistance(float signalLevelInDb)
 {
    float exp = (-40.09754606 + (signalLevelInDb)) / 20.0;
    return pow10(exp);
}
 float getSignalLvl(char* rssi)
 {

	 return ((int)rssi[0]-48+((int)rssi[1]-48)*10);
 }

 int i1=4 ;int j1=4;int i2=9;int j2=7; int i3=9; int j3=1;
 float x,y;
 void  getTrilateration2D(float d1,float d2,float d3)
 {

	 x =

	 ( ( (d1*d1-d2*d2) + (i2*i2-i1*i1) + (j2*j2-j1*j1) ) * (2*j3-2*j2) - ( (d2*d2-d3*d3) + (i3*i3-i2*i2) + (j3*j3-j2*j2) ) *(2*j2-2*j1) ) / ( (2*i2-2*i3)*(2*j2-2*j1)-(2*i1-2*i2)*(2*j3-2*j2 )) ;

	 y = ( (d1*d1-d2*d2) + (i2*i2-i1*i1) + (j2*j2-j1*j1) + x*(2*i1-2*i2)) / (2*j2-2*j1);
 }

void Configure_Clock(void)
{
	DCOCTL  = 0;
	BCSCTL1 = CALBC1_12MHZ;
	DCOCTL = CALDCO_12MHZ;


}
void Configure_UART(void)
{
	P1SEL |= BIT1 + BIT2;
	P1SEL2 |= BIT1 + BIT2;

	UCA0CTL1 = UCSWRST;

	//UCA0CTL0 = 0X00;

	UCA0CTL1 |= UCSSEL_2;
	UCA0MCTL = UCBRS0;

	UCA0BR0 = 0xE2;
	UCA0BR1 = 0x04;
	//UCA0STAT |=  UCLISTEN;

	UCA0CTL1 &=~UCSWRST;

	IE2 |= UCA0RXIE;
	_BIS_SR(GIE);
}
int i = 0,h=-1,selected =-1;
char* command;
int len = 0;


void UARTSendString(char commandz[])
{
	i = 0;
	command = commandz;
	len = strlen(commandz);
	IE2 |= UCA0TXIE;
	while((IE2&UCA0TXIE)==UCA0TXIE)
	{
		_no_operation();
	}
	//__delay_cycles(60000000);
}
int j = 0;
char find[] = "302";
char find1[] ="TP-LINK_6FFF22";
char find2[]="ktx 201";
char RSSI[6];
char RSSI1[6];
char RSSI2[6];
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    Configure_Clock();
    Configure_UART();
    IE2 &= ~UCA0RXIE;
    UARTSendString("AT+RST\r\n");
    __delay_cycles(60000000);
    UARTSendString("AT+CWMODE=1\r\n");
    __delay_cycles(60000000);
    UARTSendString("AT+CWQAP\r\n");
    __delay_cycles(60000000);
    IE2 |= UCA0RXIE;
    UARTSendString("AT+CWLAP\r\n");
    IE2 &= ~UCA0TXIE;
    __delay_cycles(60000000);
    float a = getSignalLvl(RSSI);
    float b = getSignalLvl(RSSI1);
    float c = getSignalLvl(RSSI2);
    a = calculateDistance(a);
    b = calculateDistance(b);
    c = calculateDistance(c);

    getTrilateration2D(a,b,c);

    _bis_SR_register(CPUOFF + GIE);
    while(1)
    {
    	_no_operation();
    }

}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void TraUART(void)
{

	if(i >= len-1)
	{
		IE2 &= ~UCA0TXIE;
	}

	UCA0TXBUF = command[i++];
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void RecUART(void)
{

	IFG2 &= ~UCA0RXIFG;

	if(h >= 0)
	{
		if(selected==0)
		RSSI[h--] = UCA0RXBUF;
		if(selected==1)
		RSSI1[h--] = UCA0RXBUF;
		if(selected==2)
		RSSI2[h--] = UCA0RXBUF;
	}
	else
	{
		switch(selected)
		{
		case -1:
		case 0:if(UCA0RXBUF == find[j])
				{
					j++;

					selected = 0;
					goto Out;
				}
				if (selected == 0) goto skip;
		case 1:if(UCA0RXBUF == find1[j])
				{
					j++;

					selected = 1;
					goto Out;
				}
				if (selected == 1) goto skip;
		case 2:if(UCA0RXBUF == find2[j])
				{
					j++;

					selected = 2;
					goto Out;
				}
		skip:
		default: j = 0;
				selected=-1;
				break;
		}
		Out:

		if((j == strlen(find)&&selected==0)||(j ==strlen(find1)&&selected==1 )||(j==strlen(find2)&&selected==2))
		{
			h =4;
		}
	}
}
