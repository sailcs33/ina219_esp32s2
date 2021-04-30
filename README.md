# ina219_esp32s2
 Wireless Battery Meter

This allows basic battery monitoring data (current/amps and volts) to be sent to a MQTT broker over WiFi. It uses an INA219 breakout board connected to a big shunt (on the plus/high side). INA219 is, in turn, connected to the ESP32S2 controller. From the controller the data is published to two topics. In my case, the base topic is vessels/self/esp32.

This program uses examples and libraries from other open sources. I created this to allow me to monitor my battery bank and send it to the SignalK server for later usage analysis.

The program is made to work and tested with ESP32S2-DevKitM-1 with 100Amp 100mV shunt and a generic INA219 breakout board.

Please note that to use INA219 with an external shunt I would recommend desoldering the onboard 100R resistor.

I welcome updates and further improvements to this code!

