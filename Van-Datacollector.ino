/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_MPU6050 mpu;
Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime = 10;

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
int base = 10; // decimal this time
char* behind; // will point behind the number

int out1 = 12;
int out2 = 11;
int out3 = 10;

bool out1_high = false;
bool out2_high = false;
bool out3_high = false;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  
    while(!Serial);    // time to get serial running
    Serial.println(F("Combined test"));

    unsigned status;

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
    pinMode(out1, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
    pinMode(out2, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
    pinMode(out3, OUTPUT); // Setzt den Digitalpin 13 als Outputpin
}



void loop() { 

  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    switch (int(strtol(inputString.c_str(), &behind, base)))
    {
    case 1:
      if(out1_high) {
        out1_high = false;
        digitalWrite(out1, LOW);
      } else
      {
        out1_high = true;
        digitalWrite(out1, HIGH);
      }
      break;
    case 2:
      if(out2_high) {
         out2_high = false;
        digitalWrite(out2, LOW);
      } else
      {
        out2_high = true;
        digitalWrite(out2, HIGH);
      }
      break;
    case 3:
      if(out3_high) {
        out3_high = false;
        digitalWrite(out3, LOW);
      } else
      {
        out3_high = true;
        digitalWrite(out3, HIGH);
      }
      break;
    default:
      break;
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  printOUT();
  printBME();
  printMPU();
  delay(delayTime);
}


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void printOUT() {
  Serial.print("out1_hi: ");
  Serial.println(out1_high);

  Serial.print("out2_hi: ");
  Serial.println(out2_high);

  Serial.print("out3_hi: ");
  Serial.println(out3_high);
}


void printBME() {
    Serial.print("temp: ");
    Serial.println(bme.readTemperature());

    Serial.print("pres: ");
    Serial.println(bme.readPressure() / 100.0F);

    Serial.print("aalt: ");
    Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA));

    Serial.print("humi: ");
    Serial.println(bme.readHumidity());
}

void printMPU() {
    /* Get new sensor events with the readings */
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  /* Print out the values */
  Serial.print("accx: ");
  Serial.println(a.acceleration.x);
  Serial.print("accy: ");
  Serial.println(a.acceleration.y);
  Serial.print("accz: ");
  Serial.println(a.acceleration.z);
  }
