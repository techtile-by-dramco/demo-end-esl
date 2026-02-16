#include <Arduino.h>
#include "ADS1115.h"

ADS1115 adc0(ADS1115_DEFAULT_ADDRESS); 

//  Pins
#define RELAY_PIN           8

//  Commands
#define DISCHARGE_BUFFER    0x01
#define GET_DATA            0x02

void setup() {

  Wire.begin();  // join I2C bus

  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  adc0.initialize(); // initialize ADS1115 16 bit A/D chip

  // Serial.println("Testing device connections...");
  // Serial.println(adc0.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");

  // To get output from this method, you'll need to turn on the 
  //#define ADS1115_SERIAL_DEBUG // in the ADS1115.h file
  adc0.showConfigRegister();
  
  // We're going to do continuous sampling
  adc0.setMode(ADS1115_MODE_CONTINUOUS);

  adc0.setGain(ADS1115_PGA_4P096);

}

void loop() {

  if(Serial.available()){

    //  Read last command
    uint8_t command = Serial.read();

    //  Check command
    switch (command){
    case DISCHARGE_BUFFER:

      digitalWrite(RELAY_PIN, LOW);
      delay(30000);
      digitalWrite(RELAY_PIN, HIGH);

      // Serial.println("Done");

      break;

    case GET_DATA:
      int sensorOneCounts=adc0.getConversionP0N1();  // counts up to 16-bits  
      // Serial.println(sensorOneCounts);

      uint32_t microVolt = sensorOneCounts*adc0.getMvPerCount()*1000;

      for (int i = 0; i < 4; i++)
        Serial.write((microVolt >> (i * 8)) & 0xFF);

      break;
    
    default:
      break;
    }
  }

}
