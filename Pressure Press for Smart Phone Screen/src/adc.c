#include "SC92F732x_C.H"
#include "adc.h"
#include <Math.H>
//----------------------------------------------------------------------------------------------
//********************************************EXTERN********************************************

//----------------------------------------------------------------------------------------------
//********************************************DEFINE********************************************
#define N 4094 
#define R1 10000 
#define R25 10000 
#define T25  298 
//----------------------------------------------------------------------------------------------
//******************************************PROTOTYPE*******************************************

//----------------------------------------------------------------------------------------------
//*******************************************VARIABLE*******************************************
//var for notify of completing ADC conversion
bit AdcFlag = 0;	
unsigned int ADCValue;
//int Channel;
//----------------------------------------------------------------------------------------------
//*******************************************STRUCT*********************************************
//enum Channel {AIN0=0,AIN1,AIN2,AIN3,AIN4,AIN5,AIN6,AIN7,AIN8,AIN9,VDD4=15};
/*
#if (IC_MODEL == SC92F7323)
	enum Channel {AIN0=0,AIN1,AIN2,AIN3,AIN4,AIN5,AIN6,AIN7,AIN8,AIN9,VDD4=15};
#endif

#if (IC_MODEL == SC92F7322)
	enum Channel {AIN0=0,AIN1,AIN4=4,AIN5,AIN6,AIN7,AIN8,AIN9,VDD4=15};
#endif
	
#if (IC_MODEL == SC92F7321)
	enum Channel {AIN0=0,AIN1,AIN2,AIN3,AIN8=8,AIN9,VDD4=15};
#endif
	
#if (IC_MODEL == SC92F7320)
	enum Channel {AIN0=0,AIN1,AIN6=6,AIN7,VDD4=15};
#endif
*/
//----------------------------------------------------------------------------------------------
//******************************************FUNCTION********************************************
//----------------------------------------------------------------------------------------------
//ADC congfiguration
void ADC_configuration(char Channel){
	//--------------------------------------------------------------------------------------------
	//Select and enable ADC channel
	ADCCON = 0X80|Channel;										//Select ADC channel
	if(Channel<8)
	{
		ADCCFG0 = 1<<Channel;  									//enable ADC channel 1-7
	}
	else
	{
		ADCCFG1 = 1<<(Channel-8);  						  //enable ADC channel 8-9
	}
	//--------------------------------------------------------------------------------------------
	//allow ADC interrupt
	IE |= 0X40;        												//enable ADC interrupt
	EA = 1;
}

//----------------------------------------------------------------------------------------------
//read ADC value
float ADC_read(){
		float	Term,Rntc;
		ADCCON |= 0X40;   											//start to read ADC value
		while(!AdcFlag);	         							//wait until ADC conversion completed
		AdcFlag = 0;
		ADCValue = (ADCVH<<4)+(ADCVL>>4);
		
		//Rntc=(2.345/10)*10000;
			Rntc=(N-ADCValue);
			Rntc=Rntc*R1/ADCValue;
		//Rntc=R1;
		//ADCValue = (unsigned int)Rntc;
		//ADCValue = 1000;
			Term= 100*(log10(R25)-log10(Rntc));
			Term = Term* (2.3026/39) ;
		Term = (10000/((float)(298)))-Term;
		Term= ((10000/Term) - 273) ;  
		return Term;
}
//----------------------------------------------------------------------------------------------
//caculateNTC

//void caculateNTC(unsigned int Dout,float Rntc,float	Term){
	
	// Vdd= Vrf --
	// Vout=(Vrf*dout)*2^N tinh dien ap ra; Rntc= R1(Vdd/Vout-) R noi tiep , rut gon 
  //	Rntc=R1*(N/Dout-1);
	// Rntc=R25*e^(B*(1/T-1/T25) lay log duoc
	// T=1/(1/T25 - (1/Bloge)* (logR25/Log RTC) 
	//Term= 100*(log10(R25)-log10(Rntc));
	//Term = Term* (2.3026/37) ;
	//Term = (10000/((float)(298)))-Term;
	//Term= ((10000/Term) - 273) ;  	
	//return Term;
//}

//----------------------------------------------------------------------------------------------
//ADC interrupt service
void ADC_Interrupt(void) interrupt 6
{
	ADCCON &= ~(0X10);  //清中断标志位
	AdcFlag = 1;
}

