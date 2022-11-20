#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <RDA5807M.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

//=======================================================================
//===============================CLASS===================================
RDA5807M radio;    // Create an instance of Class for RDA5807M Chip
LiquidCrystal lcd(10,9,8,7,6,5);

//=======================================================================
//==============================DEFINE===================================
#define NOT_ALLOW       0
#define ALLOW      1

//3 button
#define MODE    2
#define SELECT  3
#define INC     4

//back light LCD
#define BKL         A3
#define BLK_TIME    5

//mode operation
#define RUN         0
#define SET_FREQ    1
#define SET_VOL     2
#define SET_RSSI    3

//setup paramater
#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.

//eerom address 
#define FREQ_ADD_0     0
#define FREQ_ADD_1     2
#define FREQ_ADD_2     4
#define FREQ_ADD_3     6
#define FREQ_ADD_4     8

#define VOL_ADD_0      20
#define VOL_ADD_1      22

#define RSSI_ADD_0     30
#define RSSI_ADD_1     32

//led state rssi
#define LOW_RSSI_LED      13
#define MEDIUM_RSSI_LED   A2
#define HIGH_RSSI_LED     A1

//=======================================================================
//============================PROTOTYPE==================================


//=======================================================================
//=============================GLOBAL VAR================================
int mode = RUN;

unsigned int freq = 0;            //frequency runing
unsigned int vol =  0;            //volume runing
unsigned int rssi = 0;            //new rssi measure
unsigned int old_rssi = 0;        //last rssi measure
unsigned int RSSI_threshold =  0; //RSSI threshold

bool enb_tune = ALLOW;            //enable tuning frequency in RUN mode
bool enb_dispplay = ALLOW;        //enable display
int cursor_pos = 6;               //cursor positon of lcd
bool state_but = 1;               //state of INC button'
unsigned int blk_time = BLK_TIME; //back light will turn of after BLK_TIME second(s)
 
unsigned int freq_no[5] = {0};    //index frequency
unsigned int vol_no[2] = {0};     //index volume
unsigned int rssi_no[2] = {0};    //index rssi
//=======================================================================
//===============================SETUP===================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(10);
  //---------------------------------------------------------------------
  //initled state
  pinMode(LOW_RSSI_LED, OUTPUT);
  pinMode(MEDIUM_RSSI_LED, OUTPUT);
  pinMode(HIGH_RSSI_LED, OUTPUT);

  digitalWrite(LOW_RSSI_LED,HIGH);
  digitalWrite(MEDIUM_RSSI_LED,HIGH);
  digitalWrite(HIGH_RSSI_LED,HIGH);

  //---------------------------------------------------------------------
  //init 3 button
  pinMode(MODE, INPUT_PULLUP);
  pinMode(SELECT, INPUT_PULLUP);
  pinMode(INC, INPUT_PULLUP);

  attachInterrupt(0,switch_mode, FALLING);
  attachInterrupt(1,select_mode, LOW);
  // unlock this for first time load code then lock this  part and reload code
//  EEPROM.write(FREQ_ADD_0, 0);
//  EEPROM.write(FREQ_ADD_1, 0);
//  EEPROM.write(FREQ_ADD_2, 0);
//  EEPROM.write(FREQ_ADD_3, 0);
//  EEPROM.write(FREQ_ADD_4, 0);
//  
//  EEPROM.write(VOL_ADD_0, 0);
//  EEPROM.write(VOL_ADD_1, 0);
//  
//  EEPROM.write(RSSI_ADD_0, 0);
//  EEPROM.write(RSSI_ADD_1, 0);
  //-----------------------------------------------------------------
  //read eerom to get freq,vol,rssi_threshold
  freq_no[0] = EEPROM.read(FREQ_ADD_0);
  freq_no[1] = EEPROM.read(FREQ_ADD_1);
  freq_no[2] = EEPROM.read(FREQ_ADD_2);
  freq_no[3] = EEPROM.read(FREQ_ADD_3);
  freq_no[4] = EEPROM.read(FREQ_ADD_4);
  
  vol_no[0] = EEPROM.read(VOL_ADD_0);
  vol_no[1] = EEPROM.read(VOL_ADD_1);

  rssi_no[0]= EEPROM.read(RSSI_ADD_0);
  rssi_no[1]= EEPROM.read(RSSI_ADD_1);

  freq = freq_no[0]*10000 + freq_no[1]*1000 + freq_no[2]*100 + freq_no[3]*10 + freq_no[4];
  vol = vol_no[0]*10 + vol_no[1];
  RSSI_threshold = rssi_no[0]*10 + rssi_no[1];
  
  Serial.print("FREQ: ");Serial.println(freq);
  Serial.print("VOL: ");Serial.println(vol);
  Serial.print("RSSI_threshold: ");Serial.println(RSSI_threshold);

  //---------------------------------------------------------------------
  //RDA5807 init
  radio.init();
  radio.debugEnable();
  radio.setBandFrequency(RADIO_BAND_FM, freq);
  radio.setVolume(vol);
  radio.setMono(true);
  radio.setMute(false);

  //---------------------------------------------------------------------
  //init back light lcd
  pinMode(BKL, OUTPUT);
  digitalWrite(BKL,HIGH);
  //---------------------------------------------------------------------
  //init lcd
  lcd.begin(16,2);
  lcd.setCursor(5,0);
  lcd.print("SMART");
  lcd.setCursor(2,1);
  lcd.print("FM RECEIVER");
  
  delay(1000);
  digitalWrite(LOW_RSSI_LED,LOW);
  digitalWrite(MEDIUM_RSSI_LED,LOW);
  digitalWrite(HIGH_RSSI_LED,LOW);
  
  lcd.clear();

  //---------------------------------------------------------------------
  //init timer 1s
  cli();                                  // tắt ngắt toàn cục
  
  /* Reset Timer/Counter1 */
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  
  /* Setup Timer/Counter1 */
  TCCR1B |= (1 << CS12);    // prescale = 256 
  TCNT1 = 3036;
  TIMSK1 = (1 << TOIE1);                  // Overflow interrupt enable 
  sei();                                  // cho phép ngắt toàn cục
  //---------------------------------------------------------------------
  //init watchdog 2s
  wdt_enable(WDTO_2S);

}
//=======================================================================
//=======================================================================

//=======================================================================
//===============================LOOP====================================
void loop() {
  // put your main code here, to run repeatedly:
    //-------------------------------------------------------------------
    //IN SET FREQUENCY MODE
    wdt_reset();
    lcd.clear();
    enb_dispplay = ALLOW;
    while(mode == SET_FREQ){
      wdt_reset();
      delay(1);
      //-----------------------------------------------------------------
      //set frequency
      if(enb_dispplay == ALLOW){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("FREQ: " + String(freq_no[0])+ String(freq_no[1]) + String(freq_no[2])+ String(freq_no[3])+ String(freq_no[4])  +"Mhz");
        lcd.setCursor(cursor_pos,0);
        lcd.blink();
        enb_dispplay = NOT_ALLOW;
      }
      //-----------------------------------------------------------------
      //read button INC
      state_but = digitalRead(INC);
      if(state_but == 0){
        while(digitalRead(INC) == 0);
        freq_no[cursor_pos - 6]++;
        if(freq_no[cursor_pos - 6] >= 10) freq_no[cursor_pos - 6] = 0;
        if(freq_no[0] >= 2) freq_no[0] = 0;
        freq = freq_no[0]*10000 + freq_no[1]*1000 + freq_no[2]*100 + freq_no[3]*10 + freq_no[4];
        radio.setFrequency(freq);
//        Serial.print("press freq: ");Serial.println(freq_no[cursor_pos - 6]);Serial.println(freq);
        enb_dispplay =ALLOW;
      }
      
    }
    //-------------------------------------------------------------------
    
    //-------------------------------------------------------------------
    //IN SET VOLUME MODE
    wdt_reset();
    lcd.clear();
    enb_dispplay = ALLOW;
    while(mode == SET_VOL){
      wdt_reset();
      delay(1);
//      vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for one second
      //-------------------------------------------------------------------
      //set volume
      if(enb_dispplay == ALLOW){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("VOL: " + String(vol_no[0]) + String(vol_no[1]));
        lcd.setCursor(cursor_pos,0);
        lcd.blink();
        enb_dispplay = NOT_ALLOW;
      }
      //-----------------------------------------------------------------
      //read button INC
      state_but = digitalRead(INC);
      if(state_but == 0){
        while(digitalRead(INC) == 0);
        vol_no[cursor_pos - 5]++;
        if(vol_no[0] >= 2)  vol_no[0] = 0;
        if(vol_no[1] >= 10) vol_no[1] = 0;
        vol = vol_no[0]*10 + vol_no[1];
        if(vol>15) {vol = 15; vol_no[0] = 1; vol_no[1] = 5;}
//        Serial.print("press vol: ");Serial.println(vol_no[cursor_pos - 5]);Serial.println(vol);
        radio.setVolume(vol);
        enb_dispplay =ALLOW;
      }
    }
    //-------------------------------------------------------------------
    
    //-------------------------------------------------------------------
    //IN SET RSSI MODE
    wdt_reset();
    lcd.clear();
    enb_dispplay = ALLOW;
    while(mode == SET_RSSI){
      wdt_reset();
      delay(1);
//      vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for one second
      //-----------------------------------------------------------------
      //set RSSI threshold
      if(enb_dispplay == ALLOW){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("RSSI: "+ String(rssi_no[0]) + String(rssi_no[1]));
        lcd.setCursor(cursor_pos,0);
        lcd.blink();
        enb_dispplay = NOT_ALLOW;
      }
      //-----------------------------------------------------------------
      //read button INC
      state_but = digitalRead(INC);
      if(state_but == 0){
        while(digitalRead(INC) == 0);
        rssi_no[cursor_pos - 6]++;
        if(rssi_no[cursor_pos - 6] >= 10)  rssi_no[cursor_pos - 6] = 0;
        RSSI_threshold = rssi_no[0]*10 + rssi_no[1];
//        Serial.print("press rssi: ");Serial.println(rssi_no[cursor_pos - 6]);Serial.println(RSSI_threshold);
        enb_dispplay =ALLOW;
        
      }
    }
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    //IN RUN MODE
    wdt_reset();
    lcd.clear(); 
    enb_dispplay = ALLOW;
    enb_tune = ALLOW;
    lcd.noBlink();
    if(freq<5000) {freq = 5000; freq_no[0] = 0; freq_no[1] = 5; freq_no[2] = 0; freq_no[3] = 0; freq_no[4] = 0; }
    else if(freq>11500) {freq = 11500; freq_no[0] = 1; freq_no[1] = 1; freq_no[2] = 5; freq_no[3] = 0; freq_no[4] = 0; }
    if(vol>15) {vol = 15; vol_no[0] = 1; vol_no[1] = 5;}
    if(RSSI_threshold>100) {RSSI_threshold = 20; rssi_no[0] = 2; rssi_no[1] = 0;}
    blk_time = BLK_TIME;
    while(mode == RUN){
      wdt_reset();
      delay(100);
      if(enb_dispplay == ALLOW){
//        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("RSSI: " + String(rssi));
        enb_dispplay=NOT_ALLOW;
      }
      if(enb_tune == ALLOW){
        if((freq_no[0] != EEPROM.read(FREQ_ADD_0)) ||(freq_no[1] != EEPROM.read(FREQ_ADD_1)) ||(freq_no[2] != EEPROM.read(FREQ_ADD_2)) ||(freq_no[3] != EEPROM.read(FREQ_ADD_3)) ||(freq_no[4] != EEPROM.read(FREQ_ADD_4)) || 
           (vol_no[0] != EEPROM.read(VOL_ADD_0)) || (vol_no[1] != EEPROM.read(VOL_ADD_1)) ||
           (rssi_no[0] != EEPROM.read(RSSI_ADD_0)) || (rssi_no[1] != EEPROM.read(RSSI_ADD_1))
        ){
          
          Serial.println(">>>>>>>>>>>>>>>>>>> SAVE DATA");
          EEPROM.write(FREQ_ADD_0, freq_no[0]);
          EEPROM.write(FREQ_ADD_1, freq_no[1]);
          EEPROM.write(FREQ_ADD_2, freq_no[2]);
          EEPROM.write(FREQ_ADD_3, freq_no[3]);
          EEPROM.write(FREQ_ADD_4, freq_no[4]);
          
          EEPROM.write(VOL_ADD_0, vol_no[0]);
          EEPROM.write(VOL_ADD_1, vol_no[1]);
          
          EEPROM.write(RSSI_ADD_0, rssi_no[0]);
          EEPROM.write(RSSI_ADD_1, rssi_no[1]);
        }
        lcd.setCursor(0,0);
        lcd.print("FREQ: " + String(freq)+"MHZ");
        radio.setFrequency(freq);
        radio.setVolume(vol);
        enb_tune = NOT_ALLOW;
      }
      if(rssi >= RSSI_threshold){
        radio.setMute(false);
        digitalWrite(LOW_RSSI_LED,HIGH);
        if(rssi >= RSSI_threshold + 20 ){
          digitalWrite(LOW_RSSI_LED,HIGH);
          digitalWrite(MEDIUM_RSSI_LED,HIGH);
          digitalWrite(HIGH_RSSI_LED,HIGH);
        }
        else if(rssi >= RSSI_threshold + 10 ){
          digitalWrite(LOW_RSSI_LED,HIGH);
          digitalWrite(MEDIUM_RSSI_LED,HIGH);
          digitalWrite(HIGH_RSSI_LED,LOW);
        }

        
      }
      else{
        radio.setMute(true);
        digitalWrite(LOW_RSSI_LED,LOW);
        digitalWrite(MEDIUM_RSSI_LED,LOW);
        digitalWrite(HIGH_RSSI_LED,LOW);
      }
      rssi = radio.get_RSSI();
//      Serial.println(rssi);
      if(rssi != old_rssi) enb_dispplay = ALLOW;
      old_rssi = rssi;

      
    }
    digitalWrite(BKL,HIGH);
}
//=======================================================================
//=======================================================================

//=======================================================================
//===========================INT SERVICE=================================
void switch_mode(){
  wdt_reset();
  while(digitalRead(MODE) ==0);
  mode++;
  if(mode >= 4) mode = RUN;
  if(mode == 0){
    lcd.noBlink();

    
  }
  else if(mode == SET_FREQ){
    cursor_pos = 6;
  }
  else if(mode == SET_VOL){
    cursor_pos = 5;
  }
  else if(mode == SET_RSSI){
    cursor_pos = 6;
  }
//  delay(1);
//  enb_dispplay = ALLOW;
}
//=======================================================================
//=======================================================================

//=======================================================================
//===========================INT SERVICE=================================
void select_mode(){
  wdt_reset();
  while(digitalRead(SELECT) ==0);

  if(mode == SET_FREQ){
    //-------------------------------------------------------------------
    //set cursor for mode set frequency
    cursor_pos++;
    if(cursor_pos == 11) cursor_pos = 6;
  }
  else if(mode == SET_VOL){
    //-------------------------------------------------------------------
    //set cursor for mode set volume
    cursor_pos++;
    if(cursor_pos == 7)cursor_pos = 5;
  }

  else if(mode == SET_RSSI){
    //-------------------------------------------------------------------
    //set cursor for mode set RSSI
    cursor_pos++;
    if(cursor_pos == 8)cursor_pos = 6;
  }    
  enb_dispplay = ALLOW;
  digitalWrite(BKL,HIGH);
  blk_time = BLK_TIME;
}
//=======================================================================
//=======================================================================

//=======================================================================
//========================TIMER INT SERVICE==============================
ISR (TIMER1_OVF_vect) 
{
  wdt_reset();
  TCNT1 = 3036;
  if((mode == RUN) && (blk_time!= 0)){
    blk_time--;
//      Serial.print("blk_time: ");Serial.println(blk_time);
    if(blk_time == 0){
      digitalWrite(BKL,LOW);
    }
  }
    
}
//=======================================================================
//=======================================================================
