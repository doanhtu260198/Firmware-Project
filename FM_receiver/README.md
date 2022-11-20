- FM_receiver
  - Main function:
    + Using 3 buttons to config volume, set RSSI threshold, backlight turn off time after config.
    + Display Volume, RSSI, configuration on LCD 16x2.
    + Save setting data on EEROM of MCU
    + After config, wait 5s for returning from CONFIG mode to RUNNING mode, the LCD backlight will turn off according to configuration time, frequency and volume can still be configed while in RUNNING mode, the program will check if the RSSI is above RSSI threshold so that it will allow the device to send analog datas to speaker
