#include <EEPROM.h>
#include "GravityTDS.h"

#define TdsSensorPin A0
GravityTDS gravityTds;

float temperature = 25,tdsValue = 0, tdsCalib = 0;

void setup()
{
    Serial.begin(115200);
    gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization
}

void loop()
{
    //temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    tdsCalib = 1.0012*tdsValue;
    Serial.print(tdsValue,2);
    Serial.println("ppm");
    Serial.print(tdsCalib,2);
    Serial.println("ppm");
    delay(1000);
}