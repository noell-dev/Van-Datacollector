/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

#include <Adafruit_NeoPixel.h>

#define SEALEVELPRESSURE_HPA (1013.25)

#define PIN            0
#define NUMPIXELS      1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Replace the next variables with your SSID/Password combination
const char* ssid = "I come from a LAN down under";
const char* password = "MisteZuWenigeZeichen";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.0.0.219";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long delayTime = 10;
long lastMsg = 0;
char msg[50];
int value = 0;


Adafruit_MPU6050 mpu;
Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
float temperature = 0;
float humidity = 0;
float altitude = 0;
float accy = 0;
float accx = 0;
float accz = 0;

// LED Pin
int out1 = 25;
int out2 = 33;
int out3 = 32;

void setup() {
  pixels.begin();
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
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
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
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

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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

  Serial.println("");
  delay(100);

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
  
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

    pinMode(out1, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
    pinMode(out2, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
    pinMode(out3, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
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
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/out1") {
    Serial.print("Changing output 1 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out1, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out1, LOW);
    }
  } else if (String(topic) == "esp32/out2") {
    Serial.print("Changing output 2 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out2, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out2, LOW);
    }
  } else if (String(topic) == "esp32/out3") {
    Serial.print("Changing output 3 to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(out3, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(out3, LOW);
    }
  } else if (String(topic) == "esp32/lamp1") {
    Serial.println(messageTemp);
    pixels.setPixelColor(0, strtol(messageTemp.c_str(), NULL, 16)); // Moderately bright green color.
    pixels.show();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/out1");
      client.subscribe("esp32/out2");
      client.subscribe("esp32/out3");
      client.subscribe("esp32/lamp1");
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
  client.loop();

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    
    // Temperature in Celsius
    temperature = bme.readTemperature();   
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    humidity = bme.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);
  

    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    // Convert the value to a char array
    char altString[8];
    dtostrf(altitude, 1, 2, altString);
    Serial.print("Altitude: ");
    Serial.println(altString);
    client.publish("esp32/altitude", altString);

    /* Get new sensor events with the readings */
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);


    accx = a.acceleration.x;
    // Convert the value to a char array
    char accxString[8];
    dtostrf(accx, 1, 2, accxString);
    Serial.print("accx: ");
    Serial.println(accxString);
    client.publish("esp32/accx", accxString);


    accy = a.acceleration.y;
    // Convert the value to a char array
    char accyString[8];
    dtostrf(accy, 1, 2, accyString);
    Serial.print("accy: ");
    Serial.println(accyString);
    client.publish("esp32/accy", accyString);



    accz = a.acceleration.z;
    // Convert the value to a char array
    char acczString[8];
    dtostrf(accz, 1, 2, acczString);
    Serial.print("accz: ");
    Serial.println(acczString);
    client.publish("esp32/accz", acczString);
  }
}
