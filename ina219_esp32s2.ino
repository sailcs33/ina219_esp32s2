//Val Vechnyak Apr 27, 2021

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <INA219.h>
#include <Adafruit_NeoPixel.h>
#include <~/esp/esp-idf/components/esp_system/include/esp_system.h>


// Onboard RGB LED.
#define PIN 18
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


INA219 monitor;

#define I2C_SDA 33 //SDA is on GPIO33
#define I2C_SCL 34 //SDL is on GPIO34

// Replace the next variables with your SSID/Password combination
const char* ssid = "<<YOUR_SSID>>";
const char* password = "<<YOUR_WIFI_PASSWORD>>";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "10.10.10.1";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define SHUNT_MAX_V 0.05  //rating for max of 50amp based on 100A/100mV - (100mV / 1000 = Volts) * 50AMP / 100AMP
/* Rated max for our shunt is 75mv for 50 A current:
   we will mesure only up to 20A so max is about (75 / 1000) * 20 / 50*/
#define BUS_MAX_V   16.0  /* with 12v lead acid battery this should be enough*/
#define MAX_CURRENT 50    /* In our case this is enaugh even shunt is capable to 50 A*/
#define SHUNT_R   0.001   /* Shunt resistor in ohm */


void setup() {
  Serial.begin(115200);

  setup_wifi(); //Call Wifi setup


  client.setServer(mqtt_server, 1883);

  setup_monitor(); //Call INA219 setup

  pixels.begin(); //LED
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    pixels.setPixelColor(0, pixels.Color(20, 0, 0)); //Turn on red LED while connecting to Wifi
    pixels.show();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  pixels.setPixelColor(0, pixels.Color(0, 0, 0));  //turn off red LED once Wifi is connected
  pixels.show();


}

void setup_monitor() {
  uint32_t currentFrequency;
  delay(10);

  Wire.begin(I2C_SDA, I2C_SCL);

  monitor.begin();
  // setting up our configuration
  // default values are RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  //**************The args are: (range, gain, bus_adc, shunt_adc, mode)
  monitor.configure(INA219::RANGE_16V, INA219::GAIN_1_40MV, INA219::ADC_128SAMP , INA219::ADC_128SAMP , INA219::CONT_SH_BUS);

  // calibrate with our values
  monitor.calibrate(SHUNT_R, SHUNT_MAX_V, BUS_MAX_V, MAX_CURRENT);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    pixels.setPixelColor(0, pixels.Color(0, 0, 20)); //Show blue LED while connecting to MQTT server
    pixels.show();
    delay(5);

    // Attempt to connect
    if (client.connect("HouseBattESP32Client")) {
      Serial.println("connected");

      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //turn off LED once connected
      pixels.show();

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      client.disconnect();
      esp_restart();
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = monitor.shuntVoltage();
  busvoltage = monitor.busVoltage();
  current_mA = monitor.shuntCurrent();


  //publish only if the current is worth it. .2 amp is only approx = to 2.5 watt @ 13volts
  if (abs(current_mA) >= 0.2) {
    char charAmps[15];
    dtostrf(current_mA, 6, 2, charAmps);
    client.publish("vessels/self/esp32/Amps", charAmps );
    Serial.print("Current: ");
    Serial.print(monitor.shuntCurrent(), 4);
    Serial.println(" A");
    pixels.setPixelColor(0, pixels.Color(0, 20, 0));  //Blink green LED when published
    pixels.show();
  }
  else {
    Serial.println("Current detected is too low to publish");
  }




  //publish only if voltage is over 10v.
  if (abs(busvoltage) >= 10.0) {
    char charVoltage[15];
    dtostrf(busvoltage, 6, 2, charVoltage);
    client.publish("vessels/self/esp32/Volts", charVoltage );
    Serial.print("Voltage: ");
    Serial.print(monitor.busVoltage(), 4);
    Serial.println(" V");
    pixels.setPixelColor(0, pixels.Color(0, 20, 0));  //Blink green LED when published
    pixels.show();
  }
  else {
    Serial.println("Voltage detected is too low to publish");
  }


  Serial.print("shunt voltage: ");
  Serial.print(monitor.shuntVoltage() * 1000, 4);
  Serial.println(" mV");

  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //Turn off LED
  pixels.show();

  delay(2000);


}
