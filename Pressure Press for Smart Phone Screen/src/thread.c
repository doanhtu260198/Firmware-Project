#include "SC92F732x_C.H"
#include "thread.h"
#include "intrins.H"
#include "timer.h"
#include "LEDmultiplexing.h"
#include <stdio.h>
#include <stdlib.h>
#include <exti.h>
#include <adc.h>
#include <Math.H>
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************
//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************
//THEADING TIMEOUT DEFINE
#define TIMEOUT_THREAD_1 620							//620ms
#define TIMEOUT_THREAD_2 50								//50ms
#define TIMEOUT_THREAD_3 483							//500ms
#define TIMEOUT_THREAD_4 955							//1000ms

//DEFINE OPERATION MODE 
#define START_MODE 		0										//press start button mode	
#define SETTING_MODE 	1										//press setting mode
#define RUN_MODE 			2										//press run mode
#define WAIT_MODE 		4										//press start button mode	
//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************

//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
// THREAD VAR
static int timer_thread_1 = 0;						//thread time counting 1
static int timer_thread_2 = 0;						//thread time counting 2
static int timer_thread_3 = 0;						//thread time counting 3
static int timer_thread_4 = 0;						//thread time counting 4

//DISPLAY LED VAR
unsigned char Display_number[8];
unsigned char codeLed[12]= {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xBF,0xFF}; 			// 0-9 code P00-P06
unsigned char multiplexCode[8]= {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80}; 											// multilex code P20-P27
unsigned char waitLed[8]= {10,10,10,10,10,10,10,10}; 																						// code "- - - -"

//OPERATION MODE VAR
extern char mode;																//Operation mode

//ADC VAR
extern char temperature;												// ADC read temperature 
																						
//COUNTING TIME VAR
extern unsigned char time_run_AC;								// Running time
extern unsigned char time_run_DC;								// Running time

//SETTING VAR
extern char temp_set 	;													//display set temperature
extern char temp_real	;													//display real temperature
extern char time_AC  	;													//display time AC
extern char time_DC 	;													//display time DC
extern char timeout 	;													//timeout for setting mode blink maximum 5s

extern char flag_blink;													//blink flag
extern char flag_set	;													//choosing led will be set when blink in setting mode

//extern char timeout,timout_int ;								
// extern bit flag_xa;

//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************

//*******************************************FUNCTION*******************************************
//----------------------------------------------------------------------------------------------
//------------------------------------------THREADING 1-----------------------------------------
//---------------------------------------------10ms---------------------------------------------
void thread1(){
	
}

//-----------------------------------------------------------------------------------------------
//------------------------------------------THREADING 2------------------------------------------
//--------------------------------------------100ms----------------------------------------------
void thread2(){
	//---------------------------------------------------------------------------------------------
	//Read ADC temperature value
	float tg;
	if(mode != WAIT_MODE){
		tg = ADC_read();
	}
	
	tg=tg*10;
	temperature=(char)(tg/10);
	temp_real=temperature+1;
	//temp_real=(char)tg%10;
	//if(temp_real<=5)temp_real=(char)(tg/10);
	//else	temp_real=(char)(tg/10+1);
	/*
	temperature=10*temp_set- (char)tg; if(temperature<=0)temperature=0;
	tg=10*temp_set-tg; if(tg<=0)tg=0;
	temperature=(char)tg;
	*/
	if((temp_real-temp_set)>0 &&(temp_real-temp_set)<=6 && (temp_set>35)) temp_real=temp_set;

}

//-----------------------------------------------------------------------------------------------
//------------------------------------------THREADING 3------------------------------------------
//--------------------------------------------500ms----------------------------------------------
void thread3(){
	//---------------------------------------------------------------------------------------------
	//------------------------------Blinking led when setting--------------------------------------
	if(mode == SETTING_MODE){
		flag_blink++;
	}
	//---------------------------------------------------------------------------------------------
	//---------TOGGLE MOC3063, OUTPUT Y10-220V, generate heat when temperature < temp set----------
	if(temperature<temp_set ){
		P51 = ~P51;
	}
	else if(temperature>=temp_set||mode == WAIT_MODE||mode == SETTING_MODE){
		P51 = 0;
	}	
}

//-----------------------------------------------------------------------------------------------
//------------------------------------------THREADING 4------------------------------------------
//--------------------------------------------1000ms---------------------------------------------
void thread4(){
	//P51 = ~P51;
	//---------------------------------------------------------------------------------------------
	//------------Auto set mode = START_MODE if not press any setting key after timeout------------
	if(mode == SETTING_MODE ){timeout++;}
	if(timeout>=5){
		mode = START_MODE;
		flag_set=0;
		timeout=0;
	}
	//---------------------------------------------------------------------------------------------
	//---------------------------------COUNTING TIME IN RUN MODE-----------------------------------
	if(mode == RUN_MODE){
		if(time_run_AC < time_AC){
			time_run_AC++;
			P50 = 1;	
		}
		else if(time_run_DC < time_DC){
			time_run_DC++;
			P16 = 1;
		}
		else if((time_run_AC>=time_AC) && (time_run_DC >= time_DC)){
			time_run_AC = 0;
			time_run_DC = 0;
			P50 = 0;
			P16 = 0;
			mode = START_MODE;
		}
	}

	
}

//-----------------------------------------------------------------------------------------------
//--------------------------------------CREATING THREADING---------------------------------------
void creating_thread_timer(){
	WDTCON |= 0x10;	
	//if(timer_thread_1 < TIMEOUT_THREAD_1){timer_thread_1++;} //thread 10ms
	if(timer_thread_2 < TIMEOUT_THREAD_2){timer_thread_2++;} //thread 100ms
	if(timer_thread_3 < TIMEOUT_THREAD_3){timer_thread_3++;} //thread 1000ms
	if(timer_thread_4 < TIMEOUT_THREAD_4){timer_thread_4++;} //thread 5000ms
}

//-----------------------------------------------------------------------------------------------
//--------------------------------------RUN ALL THREADING----------------------------------------
void thread_run(){

	//if(timer_thread_1 >= TIMEOUT_THREAD_1){thread1();timer_thread_1 = 0;}
	if(timer_thread_2 >= TIMEOUT_THREAD_2){thread2();timer_thread_2 = 0;}
	if(timer_thread_3 >= TIMEOUT_THREAD_3){thread3();timer_thread_3 = 0;}
	if(timer_thread_4 >= TIMEOUT_THREAD_4){thread4();timer_thread_4 = 0;}
}
