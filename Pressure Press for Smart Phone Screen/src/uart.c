#include "SC92F732x_C.H"
#include "uart.h"
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************

//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************

//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************

//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
bit UartSendFlag 		= 0; // interrupt send flag
bit UartReceiveFlag = 0; // interrupt receive flag

//----------------------------------------------------------------------------------------------
//*********************************************ENUM*********************************************

//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************

//----------------------------------------------------------------------------------------------
//******************************************FUNCTION********************************************
//----------------------------------------------------------------------------------------------
//-------------------------------config uart using timer 1 at 12Mhz ----------------------------
/* Baurate formula
	 Baudrate = ((2^SMOD)/16) * (fn1/((256-TH1)*2))
	 fn1 = fsys (if T1FD = 1)
	 fn1 = fsys/12 (if T1FD = 0)
	 SMOD bit in PCON register
	 TH1 in TH1 register
*/
void Uart0_Init(void)    
{
	P1CON &= 0XF3;
	P1PH 	|= 0X0C;	 	 //TX/RX pull up
	P13		 = 1;		 		 //TX set to 1
	SCON	 = 0X50;     /*mode 01: config full-duplex, 1 bit start, 1 bit stop, 8 bit data 
											 RI set upon receiving complete data frame ==> generate interrupt request*/
	PCON 	|= 0X80; 		 //operate under 1/32 system clock, using timer ==> not using system clock ==> parameter of baudrate
	T2CON  = 0x00;     /*TCLK and RCLK set to 00 ==> timer 1 is clock source of baudrate
											 TCLK or RCLK set to 1 ==> timer 2 is clock source of baudrate*/
	TMOD 	 = 0X20;     //timer 1 operating mode 2: 8bit auto-reload mode
	TMCON  = 0X02;     //bit T1FD = 1 ==> fn1 = Fsys, bit T1FD = 0 ==> fn1 = Fsys/12
	TL1 	 = 217;			 //parameter to calculate baudrate
	TH1 	 = 217;		 	 //parameter to calculate baudrate, TH1 and TL1 = 217 ==>baudrate ~= 19200,TH1 and TL1 = 178 ==>baudrate ~= 9600
	TR1 	 = 1;		 		 //Turn on Timer1 
	EUART  = 1;	     	 //Enable UART
	EA 		 = 1;		     //allow global interrupt
	
	
	
}

//------------------------------------------------------------------------------------------------
//----------------------------------------UART send data -----------------------------------------
// param: dat: data to send.
void Uart0_Send(unsigned char dat)
{
	SBUF = dat;
	while(!UartSendFlag);
	UartSendFlag = 0;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------UART send string data --------------------------------------
// param: *str: string data to send.
void Uart0_Send_String(unsigned char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)	// Send each char of string till the NULL 
	{
		Uart0_Send(str[i]);		// Call transmit data function 
	}
}

//------------------------------------------------------------------------------------------------
//----------------------------------ISR interrupt 4 for uart 0------------------------------------
void UartInt(void) interrupt 4
{
	if(TI)
	{
		TI = 0;	
		UartSendFlag = 1;		
	}
	if(RI)
	{
		RI = 0;	
		UartReceiveFlag = 1;
	}	
}
