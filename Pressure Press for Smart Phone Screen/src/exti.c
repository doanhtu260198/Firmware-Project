#include "SC92F732x_C.H"
#include "exti.h"
#include "LEDmultiplexing.h"
#include "EEPROM.h"
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************
extern char mode;
//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************
//DEFINE OPERATION MODE 
#define START_MODE 		0										//press start button mode	
#define SETTING_MODE 	1										//press setting mode
#define RUN_MODE 			2										//press run mode
#define WAIT_MODE 		4										//press start button mode	

//DEFINE EEPROM ADDRESS 
#define TIME_AC_ADDR 	0										//eeprom address of time_AC		
#define TIME_DC_ADDR 	1										//eeprom address of time_DC		
#define TEMP_SET_ADDR 2										//eeprom address of temp_set



//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************
extern void delay_us(int time);
extern void delay_ms(int time);
//extern void Multiplex_Display(unsigned int Lednum, unsigned int Multiplex_time,char display[8]);
//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
extern unsigned char Display_number[8];

//SETTING VAR
extern char lastData[8];
extern char flag_set;
extern char new_set;

extern char temp_set 	;
extern char temp_real	;
extern char time_AC  ;
extern char time_DC 	;

extern char timeout,timout_int ;

extern unsigned char time_run_AC;								// Running time
extern unsigned char time_run_DC;								// Running time

extern int timer_thread_1 ;
//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************

//----------------------------------------------------------------------------------------------
//******************************************FUNCTION********************************************
//----------------------------------------------------------------------------------------------
//External interrupt congfig
void External_IT_configuration(){
	P1CON = 0xCC;																					//enable INT00,01,10,11
	P1PH 	= 0xFF;																					//config as input pull-up
	
	//-------------------------------------------------------------------------------------------
	//config vector interrupt 0 
	INT0F = 0X03;     																		//config INT00,01 as FALLING
	INT0R = 0x00;																					//disable INT00 RISING
	
	//-------------------------------------------------------------------------------------------
	//config vector interrupt 1
	INT1F = 0X03;     																		//config INT00,01 as FALLING
	INT1R = 0x00;																					//disable INT00 RISING	
	
	//-------------------------------------------------------------------------------------------
	//enable vector interrupt
	IE  |= 0x05;																					//0000 0101 enable INT00 interrupt
	IP  |= 0X00;																					//priority of interrput
	EA = 1;						
}

//----------------------------------------------------------------------------------------------
//External interrupt 0 service
void EX0() interrupt	0
{
	
	char i = 0;														//for loop "store last data led to blink"
	timeout = 0;													//time out "Auto set mode = START_MODE if not press any setting key"
	WDTCON |= 0x10;	
	//----------------------------------------------------------------------------------------------
	//pressing set button
	if(P10 == 0 && mode != RUN_MODE && mode != WAIT_MODE){
		mode = SETTING_MODE;
		while(P10 == 0){Multiplex_Display(8,20,Display_number);}			////Anti bounce button
		//----------------------------------------------------------------------------------------------
		//choosing led will blink
		flag_set++; delay_ms(20);
		if(flag_set>=7){
				flag_set = 0;
				//mode = START_MODE;
				timout_int=0;
				//IE  |= 0x05;
		}
		//----------------------------------------------------------------------------------------------
		//store last data led to blink	
		for(i = 0;i<8;i++){
				lastData[i] = Display_number[i];
		}
		//----------------------------------------------------------------------------------------------
		//led blink case
		switch(flag_set){
			case 1:																//led time_AC_1
				lastData[4] = 11;
				new_set = Display_number[4];
				break;
			case 2:																//led time_AC_2
				lastData[5] = 11;
				new_set = Display_number[5];
				break;
			case 3:																//led time_DC_1
				lastData[0] = 11;
				new_set = Display_number[0];
				break;
			case 4:																//led time_DC_2
				lastData[1] = 11;
				new_set = Display_number[1];
				break;
			case 5:																//led temp_set_1
				lastData[2] = 11;
				new_set = Display_number[2];
				break;
			case 6:																//led temp_set_2
				lastData[3] = 11;
				new_set = Display_number[3];
				break;
		}					
	}//end pressing set button
	
	//----------------------------------------------------------------------------------------------
	//pressing up/down button
	else if(P11 == 0 && mode != WAIT_MODE){
		while(P11 == 0){Multiplex_Display(8,20,Display_number);}
		//if(timout_int==0){new_set++;timout_int=1;}
		new_set++; //delay_ms(20);
		if(new_set>=10)new_set=0;
		//----------------------------------------------------------------------------------------------
		//store new data
		switch(flag_set){
			case 1:
				//lastData[4] = 11;
				 Display_number[4]=new_set;
				 time_AC=Display_number[5]*10+Display_number[4];
				 IAPWrite(TIME_AC_ADDR,time_AC,0x02);							//write to EEPROM
				break;
			case 2:
				//lastData[5] = 11;
				//new_set = Display_number[5];
				Display_number[5]=new_set;
				time_AC=Display_number[5]*10+Display_number[4];
				IAPWrite(TIME_AC_ADDR,time_AC,0x02);
				break;
			case 3:
				//lastData[0] = 11;
				//new_set = Display_number[0];
				Display_number[0]=new_set;
				time_DC=Display_number[1]*10+Display_number[0];
				IAPWrite(TIME_DC_ADDR,time_DC,0x02);
				break;
			case 4:
				//lastData[1] = 11;
				//new_set = Display_number[1];
				Display_number[1]=new_set;
				time_DC=Display_number[1]*10+Display_number[0];
				IAPWrite(TIME_DC_ADDR,time_DC,0x02);
				break;
			case 5:
				//lastData[2] = 11;
				//new_set = Display_number[2];
				Display_number[2]=new_set;
				temp_set=Display_number[3]*10+Display_number[2];
				IAPWrite(TEMP_SET_ADDR,temp_set,0x02);
				break;
			case 6:
				//lastData[3] = 11;
				//new_set = Display_number[3];
				Display_number[3]=new_set;
				temp_set=Display_number[3]*10+Display_number[2];
				IAPWrite(TEMP_SET_ADDR,temp_set,0x02);
				break;
		}					
	}//end pressing up/down button
}

//----------------------------------------------------------------------------------------------
//External interrupt 1 service
void EX1() interrupt	2
{
	WDTCON |= 0x10;	
	flag_set=0;	
	time_run_AC = 0;
	time_run_DC = 0;
	//---------------------------------------------------------------------------------------------
	//PRESSING START BUTTON
	if(P14 == 0)
	{
	while(P14 == 0){Multiplex_Display(8,20,Display_number);}
		//---------------------------------------------------------------------------------------------
		//SWITCHING MODE
		if(mode == WAIT_MODE) {
			mode = START_MODE;
		}
		else{
			mode = WAIT_MODE;
			//IE  |= 0x05;
			P50 = 0;
			P16 = 0;
		}
	}
	//---------------------------------------------------------------------------------------------
	//PRESSING RUN BUTTON
	if(P15 == 0 && mode != WAIT_MODE)
	{
		while(P15 == 0){Multiplex_Display(8,20,Display_number);}			
		if((mode == START_MODE) || (mode == SETTING_MODE) ){
			mode = RUN_MODE;
		}
		else if(mode ==RUN_MODE){																//BUG BUG BUG
			time_run_AC = 0;
			time_run_DC = 0;
			mode = START_MODE;
			}
			P50 = 0;
			P16 = 0;

	}
}