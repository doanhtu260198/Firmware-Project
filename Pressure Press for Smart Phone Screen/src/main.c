#include "SC92F732x_C.H"
#include "intrins.H"
#include "timer.h"
#include "LEDmultiplexing.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <exti.h>
//#include <pwm.h>
#include <adc.h>
#include <Math.H>
#include "EEPROM.h"
#include "uart.h"

//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************
//DEFINE OPERATION MODE 
#define START_MODE 		0												//press start button mode									
#define SETTING_MODE 	1												//press setting mode
#define RUN_MODE 			2												//press run mode
#define WAIT_MODE 		4												//press start button mode	

//DEFINE EEPROM ADDRESS 
#define TIME_AC_ADDR 	0												//eeprom address of time_AC		
#define TIME_DC_ADDR 	1												//eeprom address of time_DC		
#define TEMP_SET_ADDR 2												//eeprom address of temp_set
//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************
void delay_us(int time);
void delay_ms(int time);

//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
//DISPLAY LED VAR
extern unsigned char Display_number[8];				// array for storing display number
extern unsigned char codeLed[12]; 						// 0-9 code P00-P06
extern char multiplexCode[8]; 								// multilex code P20-P27
extern unsigned char waitLed[8];
//MODE VAR
char mode = WAIT_MODE;												// Operation mode

//ADC VAR
char temperature= 0;													// ADC read temperature 

//COUNTING TIME VAR
unsigned char time_run_AC = 0;								// Running time
unsigned char time_run_DC = 0;								// Running time


//SETTING VAR
char temp_set 	= 40;													//display set temperature
char temp_real	= 0;													//display real temperature
char time_AC 		= 30;													//display time AC
char time_DC 		= 20;													//display time DC


char new_set = 0;															//choosing number display when blink in setting mode
char lastData[8] = {8};												//array for storing old data to blink
char flag_set = 0;														//choosing led will be set when blink in setting mode
char flag_blink = 0;													//blink flag
char timeout = 0;															//timeout for setting mode blink maximum 5s


char test_uart = 10;
char *string_uart = "hello";
//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************


//==============================================================================================
//********************************************MAIN**********************************************
void main(){
	
	//--------------------------------------------------------------------------------------------
	//INPUT  = 0
	//OUTPUT = 1
	P0CON = 0xFF;  														//config port0 is OUTPUT
	P0PH  = 0x00;															//set output port0 is LOW
	
	P1CON = 0x00;  														//config port1 is INPUT 
	P1PH  = 0xFF;	 														//set output port1 is HIGH
	
	P2CON = 0xFF;  														//config port2 is OUTPUT
	P2PH  = 0x00;															//set output port2 is LOW
	
	P5CON = 0xFF;  														//config port3 is OUTPUT	
	P5PH  = 0x00;															//set output port3 is LOW
	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------READ EEPROM-------------------------------------------
	time_AC  = IAPRead(TIME_AC_ADDR,0x02);		//read time_AC from EEPROM at TIME_AC_ADDR address
	time_DC  = IAPRead(TIME_DC_ADDR,0x02);		//read time_DC from EEPROM at TIME_DC_ADDR address
	temp_set = IAPRead(TEMP_SET_ADDR,0x02);		//read temp_set from EEPROM at TEMP_SET_ADDR address
	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------SYSTEM INIT-------------------------------------------
	Timer0_Init();														//Timer init
	External_IT_configuration();							//external interrupt config
	ADC_configuration(8);											//ADC config
	Uart0_Init();															//uart 0 config
	//--------------------------------------------------------------------------------------------
	//----------------------------------------WDT INIT--------------------------------------------
	WDTCON |= 0x10;
	delay_ms(50);
	
	
	Uart0_Send(test_uart);
	Uart0_Send_String(string_uart);
	while(1){
		//------------------------------------------------------------------------------------------
		//-----------------------------------WAIT_MODE----------------------------------------------
		while(mode == WAIT_MODE){
				WDTCON |= 0x10;	
				Multiplex_Display(8,10,waitLed);					//display "- - - -"
				thread_run();
				P51 = 0;
		}
		
		//------------------------------------------------------------------------------------------
		//-----------------------------------START_MODE---------------------------------------------
		while(mode == START_MODE){
			WDTCON |= 0x10;	
			splitNumber();
			Multiplex_Display(8,10,Display_number);
			thread_run();
		}//end START_MODE
		
		//------------------------------------------------------------------------------------------
		//----------------------------------SETTING_MODE--------------------------------------------
		while(mode == SETTING_MODE){
			WDTCON |= 0x10;	
			//----------------------------------------------------------------------------------------
			//---------------------------------Blink led----------------------------------------------
			if(flag_blink == 0){
				Multiplex_Display(8,10,lastData);
			}
			else{
				Multiplex_Display(8,10,Display_number);
				if(flag_blink>=2){
					flag_blink = 0;
				}
			}
			
			thread_run();
			P51 = 0;
		}//end SETTING_MODE
		
		//------------------------------------------------------------------------------------------
		//------------------------------------RUN_MODE----------------------------------------------
		while(mode == RUN_MODE){
			WDTCON |= 0x10;	
			splitNumberRunTime();
			Multiplex_Display(8,10,Display_number);
			thread_run();
		}//end RUN_MODE
						
	}

}
//==============================================================================================
//==============================================================================================

//----------------------------------------------------------------------------------------------
//*******************************************FUNCTION*******************************************
//delay approximately us 
void delay_us(int time){
	while(time--){if(time<=0)break;}
}

//delay approximately ms 
void delay_ms(int time){
	while(time--){delay_us(150);if(time<=0)break;}
}

