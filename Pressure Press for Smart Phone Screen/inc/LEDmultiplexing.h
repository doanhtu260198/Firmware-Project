#ifndef __LEDMULTIPLEXING_H__
#define __LEDMULTIPLEXING_H__

//-----------------------------------------------------------------------------------------
//****************************************INCLUDE******************************************

//-----------------------------------------------------------------------------------------
//*****************************************DEFINE******************************************

//-----------------------------------------------------------------------------------------
//****************************************PROTOTYPE****************************************
void splitNumber();//unsigned long Number,unsigned int Lednum,unsigned char displayNum[8]); //Split number to display 7segments-led
void splitNumberRunTime();																															//display led when in running operation mode
void Multiplex_Display(unsigned char Lednum, unsigned char Multiplex_time,char display[8]);	//Multiplexing to display 7segments-led
//void splitNumberTest(unsigned long Number,unsigned char Lednum,unsigned char displayNum[8],char i);
//-----------------------------------------------------------------------------------------

#endif 

