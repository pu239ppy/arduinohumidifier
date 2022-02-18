# arduinohumidifier
- Uses DHT11 to read humidity and temperature
- I did not have an LCD so I am using RED/YELLOW/GREEN LEDs to indicate humidity
- BLUE LED Lights up on BLE connections, you may use LightBlue app to connect an examine properties
- deep sleep is commented out because when in deep sleep BLE does not work.  I may want to start listening to broadcasts as opposed to reading.  But deep sleep should save some power.

## Future work
- Connect an atomizer to humidify
- Check BT status
