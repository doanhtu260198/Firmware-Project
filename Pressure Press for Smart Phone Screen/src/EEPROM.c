#include "SC92F732x_C.H"
#include "EEPROM.h"
#include "intrins.H"
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************

//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************

//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************

//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
unsigned char code *IapAddr = 0x00;
//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************

//----------------------------------------------------------------------------------------------
//******************************************FUNCTION********************************************
//----------------------------------------------------------------------------------------------
//write to eeprom
/*param: - ADddr: address to store data
		 - Value: value to store 
		 - IAPArea: 0x02 ==> write to EEPROM
		 			0x00 ==> write to ROM
*/
void IAPWrite(int Addr,char Value,char IAPArea)
{	
	EA = 0;
	IAPDAT = Value;      		 //register store data
	IAPADH = ((Addr>>8)&0X7F);   //address to store data
	IAPADL = Addr;               //address to store data
	
	IAPADE = IAPArea;  			 //IAPArea=0x00 write to ROM  IAPArea=0x02 write to EEPROM
	IAPKEY = 240;		  	     //operation time limit 
	IAPCTL = 0x06;     			 //CPU hold time 2ms
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	IAPADE = 0x00;     			 //something must be done
	EA = 1;
}

//----------------------------------------------------------------------------------------------
//Read from eeprom
char IAPRead(int Addr,char IAPArea)
{
	char ReadValue = 0x00;
	EA = 0;
	IAPADE = IAPArea;  			 //IAPArea=0x00 read from ROM  IAPArea=0x02 read from EEPROM
	ReadValue = *(IapAddr+Addr); //Read value from specific address
	IAPADE = 0x00;               //something must be done
	EA = 1;
	return ReadValue;
}


