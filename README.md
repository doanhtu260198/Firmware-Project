# Embedded-Project

- Pressure Press for Smart Phone Screen
  - A program to control pressure machine to press glasses for smart phone 
    + Main MCU is SC92F7323
    + Creating OS based on timer
    + read button to config time, heat and pressure 
    + display information about heat, pressure, running and setting time on 7 segment LED
    + save config to EEROM
  =================================================================================================================
- FM_receiver: arduino IDE
  - Main function:
    + Using 3 buttons to config volume, set RSSI threshold, backlight turn off time after config.
    + Display Volume, RSSI, configuration on LCD 16x2.
    + Save setting data on EEROM of MCU
    + After config, wait 5s for returning from CONFIG mode to RUNNING mode, the LCD backlight will turn off according to configuration time, frequency and volume can still be configed while in RUNNING mode, the program will check if the RSSI is above RSSI threshold so that it will allow the device to send analog datas to speaker
