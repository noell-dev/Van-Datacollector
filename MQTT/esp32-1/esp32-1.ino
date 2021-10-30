/*********
  © Tobiah Nöll
  https://noell.li
*********/

#include <WiFi.h>
// #include <SPI.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ADS1X15.h>
//#include <OneWire.h> 
//#include <DallasTemperature.h>

#include "credentials.h"


/* constants */
// #define ONE_WIRE_BUS 2 

#define I2C_SDA 23
#define I2C_SCL 22

/* not used currently
// Estimate Sealevel Pressure to set relative Pressure in abolute relation 
#define SEALEVELPRESSURE_HPA (1013.25)
*/

/* Variable Definitions */

// MQTT
long lastMsg = 0;
char msg[50];

// Sensors
float temperature = 0;
float humidity = 0;
/* not used currently
float altitude = 0;
float old_altitude = 0;
*/
float accy = 0;
float accx = 0;
float accz = 0;
float outTemp = 0;

int16_t shunt;
int16_t voltage;

//Sensor Char conversions
char tempString[8];
char humString[8];
char accxString[8];
char accyString[8];
char acczString[8];
char outTempString[8];
char shuntString[16];
char voltageString[16];

// Outputs
int out1 = 25;
int out2 = 33;
int out3 = 32;

// LEDs
int led1_pin = 0;
int led1_numpixels = 1;
int led1_color = 0xFFFFFF;
bool led1_on = false;

// instantiation from Libs:
TwoWire I2C = TwoWire(0);
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(led1_numpixels, led1_pin, NEO_GRB + NEO_KHZ800);
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme;
// OneWire oneWire(ONE_WIRE_BUS); 
// DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads1115; // Construct an ads1115 

void setup() {
  /* Set up Serial connection, mainly for debugging, might delete later */
  Serial.begin(115200);

  I2C.begin(I2C_SDA, I2C_SCL, 100000);

  /* Connect BME280 Sensor at Sensor ID76 */
  if (!bme.begin(0x76, &I2C)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) {
      delay(10);
      }
  }
  Serial.println("BME280 Connected!");

  /* Connect MPU6050Sensor */
  if (!mpu.begin(&I2C)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Connected!");

  /* Initialize Settings on MPU6050 Sensor and */
  // Accelerometer Range
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");

  /* output to Serial,  might delete Later: */
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  
  // Gyrometer Range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  
  /* output to Serial,  might delete Later: */
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }
  
  // Filter
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  /* output to Serial,  might delete Later: */
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV)");
  ads1115.begin(&I2C);  // Initialize ads1115 at address 0x49

  
/* Seems redundant, delete after Testing
    unsigned status;
    // BME
    status = bme.begin(0x76, &Wire);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
*/

  // Start WiFi Connection, 10s Delay
  setup_wifi();
  
  // initialize MQTT Client
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);

  // Initialize Outputs
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);

  // Start OneWire for Outdoor Sensor
  // sensors.begin();
}

void setup_wifi() {
  delay(10);
  /* output to Serial,  might delete Later: */
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  // Start connecting
  WiFi.begin(SSID, KEY);

  // Loop until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  /* output to Serial,  might delete Later: */
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  /* output to Serial,  might delete Later: */
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  String messageTemp;

  // Concatenate Chars to full message
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  /* Find if the Topic is relevant for us and act accordingly */
  if (String(topic) == "vc/out1") {
    Serial.print("Changing output 1 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out1, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out1, LOW);
    }
  } else if (String(topic) == "vc/out2") {
    Serial.print("Changing output 2 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out2, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out2, LOW);
    }
  } else if (String(topic) == "vc/out3") {
    Serial.print("Changing output 3 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out3, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out3, LOW);
    }
  } else if (String(topic) == "vc/lamp1-color") {
    Serial.println(messageTemp);
    led1_color = strtol(messageTemp.c_str(), NULL, 16);
    if (led1_on) {
      for (int i=0; i<led1_numpixels; i++){
        pixels1.setPixelColor(i, led1_color);
      }
      pixels1.show();
      }
  } else if (String(topic) == "vc/lamp1-out") {
    Serial.println(messageTemp);
    if (messageTemp == "on") {
      led1_on = true;
      for (int i=0; i<led1_numpixels; i++){
        pixels1.setPixelColor(i, led1_color);
      }
      pixels1.show();
    } else if (messageTemp == "off"){
      led1_on = false;
      pixels1.clear();
      pixels1.show();
    }
  }
}

/* Connect to MQTT */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe to our Output Topics
      client.subscribe("vc/out1");
      client.subscribe("vc/out2");
      client.subscribe("vc/out3");
      client.subscribe("vc/lamp1-color");
      client.subscribe("vc/lamp1-out");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  // Start MQTT Client Loop
  client.loop();
    
    // Temperature in Celsius
    temperature = bme.readTemperature();
    
    // Convert the value to a char array
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    humidity = bme.readHumidity();
    // Convert the value to a char arra
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);
  
/* not used currently
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    // Convert the value to a char array
    char altString[8];
    dtostrf(altitude, 1, 2, altString);
    Serial.print("Altitude: ");
    Serial.println(altString);
    client.publish("esp32/altitude", altString);
*/
    /* Get new sensor events with the readings */
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);


    accx = a.acceleration.x;
    // Convert the value to a char array
    dtostrf(accx, 1, 2, accxString);
    Serial.print("accx: ");
    Serial.println(accxString);
    client.publish("esp32/accx", accxString);


    accy = a.acceleration.y;
    // Convert the value to a char array
    dtostrf(accy, 1, 2, accyString);
    Serial.print("accy: ");
    Serial.println(accyString);
    client.publish("esp32/accy", accyString);



    accz = a.acceleration.z;
    // Convert the value to a char array
    dtostrf(accz, 1, 2, acczString);
    Serial.print("accz: ");
    Serial.println(acczString);
    client.publish("esp32/accz", acczString);

/*
    // Get Outdoor Temp 
    sensors.requestTemperatures();
    outTemp = sensors.getTempCByIndex(0);


    // Convert the value to a char array
    dtostrf(outTemp, 1, 2, outTempString);
    Serial.print("Outdoor: ");
    Serial.println(outTempString);
    client.publish("esp32/outdoorTemp", outTempString);
*/
/*    ads1115.setGain(GAIN_SIXTEEN);
    delay(100);
    shunt = ads1115.readADC_Differential_2_3();

    dtostrf(shunt, 1, 2, shuntString);
    Serial.print("Shunt: ");
    Serial.println(shuntString);
    client.publish("esp32/shunt", shuntString);

    ads1115.setGain(GAIN_TWOTHIRDS);
    delay(100);

    voltage = ads1115.readADC_Differential_0_1();
    dtostrf(voltage * 3, 1, 2, voltageString);
    Serial.print("Voltage: ");
    Serial.println(voltageString);
    client.publish("esp32/voltage", voltageString);
   */ 
    delay(5000);
}
