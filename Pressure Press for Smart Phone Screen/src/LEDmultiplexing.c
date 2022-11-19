#include "SC92F732x_C.H"
#include "LEDmultiplexing.h"
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************

//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************

//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************
extern void delay_us(int time);
extern void delay_ms(int time);
//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
extern unsigned char Display_number[8];
extern unsigned char codeLed[12]; // 0-9 code P00-P06
extern char multiplexCode[8]; 					// multilex code P20-P27

extern char temp_set 	;
extern char temp_real	;
extern char time_AC  ;
extern char time_DC 	;

extern unsigned char time_run_AC;								// Running time
extern unsigned char time_run_DC;								// Running time
//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************

//----------------------------------------------------------------------------------------------
//******************************************FUNCTION********************************************
//----------------------------------------------------------------------------------------------
//Split number to display 7segments-led
/*param: - Number: number to split
				 - Lednum: number of digit will split
				 - displayNum[8]: array to store splitting number
*/
void splitNumber(){//unsigned long Number,unsigned int Lednum,unsigned char displayNum[8]){
//	int i;
//	for(i=0;i<Lednum;i++){
//		displayNum[i]=Number%10;
//		Number=Number/10;
//	}
			Display_number[7]=temp_real/10;
			Display_number[6]=temp_real%10;
	
			Display_number[5]=time_AC/10;
			Display_number[4]=time_AC%10;
	
			Display_number[3]=temp_set/10;
			Display_number[2]=temp_set%10;
	
			Display_number[1]=time_DC/10;
			Display_number[0]=time_DC%10;
	
}


//Split number to display 7segments-led
/*param: - Number: number to split
				 - Lednum: number of digit will split
				 - displayNum[8]: array to store splitting number
*/
void splitNumberRunTime(){
			Display_number[7]=temp_real/10;
			Display_number[6]=temp_real%10;
	
			Display_number[5]=time_run_AC/10;
			Display_number[4]=time_run_AC%10;
	
			Display_number[3]=temp_set/10;
			Display_number[2]=temp_set%10;
	
			Display_number[1]=time_run_DC/10;
			Display_number[0]=time_run_DC%10;
	
}


//----------------------------------------------------------------------------------------------
//Multiplexing to display 7segments-led
/*param: - Lednum: number of led to display
				 - Multiplex_time: number of multiplexing time
*/
void Multiplex_Display(unsigned char Lednum, unsigned char Multiplex_time,char display[8]){
char i,j;
	for(i=0;i<Multiplex_time;i++){
		for(j=0;j<Lednum;j++){
			P2 = multiplexCode[7-j];							//bat led duoc phep hien thi
			P0 = codeLed[display[j]];		//mang chua con so can hien thi
			delay_ms(1);
			P2 = 0x00;
		}
	}
}


//void splitNumberTest(unsigned long Number,unsigned char Lednum,unsigned char displayNum[8],char i){
//	//int i;
//	for(i=0;i<Lednum;i++){
//		displayNum[i]=Number%10;
//		Number=Number/10;
//	}
//}