//Library dan inisialisasi MicroSD dan RTC
#include <SPI.h>                            //komunikasi SPI dengan MicroSD
#include <SD.h>                             //komunikasi SPI engan MicroSD
#include <Wire.h>                           //koneksi I2C dengan RTC
#include "RTClib.h"                         //koneksi I2C dengan RTC
#define SD_CS 53                            //koneksi MicroSD dengan Arduino
File myFile;
RTC_DS1307 rtc;

//Library dan inisialisasi sensor pH
#define SensorPin A1            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

//Library dan inisialisasi sensor EC
#include "DFRobot_EC.h"
#include <EEPROM.h>
#define EC_PIN A2
float voltage, ecValueSensor, ecVoltageCalib, ecConversion, ecValue;
DFRobot_EC ec;

//Library dan inisialisasi sensor TDS
#include <EEPROM.h>
#include "GravityTDS.h"
#define TdsSensorPin A3
GravityTDS gravityTds;
float tdsValueSensor=0, tdsVoltageCalib, tdsConversion, tdsValue=0;

//Library dan inisialisasi sensor curah hujan                
const int pin_interrupt = 3;
long int jumlah_tip = 0;
long int temp_jumlah_tip = 0;
float curah_hujan = 0.00;
float milimeter_per_tip = 0.00473334;
volatile boolean flag = false;

//Library dan inisialisasi sensor temperatur air
#include <OneWire.h>
int DS18S20_Pin = 5;   
OneWire ds(DS18S20_Pin); 

//Inisialisasi set Controller
const int IN1 = 9; //Cooler
const int IN2 = 10; //Cooler
const int IN3 = 11; //Heater
int setpoint = 25;
int chk;
int kondisiCooler1, kondisiCooler2, kondisiHeater;

//Library dan inisialisasi sensor suhu dan kelembaban udara
#include <DHT.h>
#define DHTLUAR_PIN 4
#define DHTDALAM_PIN 2
#define DHTTYPE DHT22 
DHT dhtLuar(DHTLUAR_PIN, DHTTYPE);
DHT dhtDalam(DHTDALAM_PIN, DHTTYPE);

//Library dan inisialisasi motor servo
#include <Servo.h>
Servo servoKu;
#define sensor_hujan 6
int nilaiHujan, kondisiPenutup;

//Inisialisasi variabel string untuk serial monitor dan parsing data
String strphValue, strecValue, strtdsValue, strCurahHujan,  strtempluar, strhumidluar, strtempdalam, strhumiddalam, strTemperaturAir, strKondisiHeater, strKondisiCooler1, strKondisiCooler2, strnilaiHujan, strKondisiPenutup;

//Method tambahan sensor curah hujan
void hitung_curah_hujan()
{
  flag = true;
}

void setup()
{
  Serial.begin(9600);
  
  //Inisialisasi microSD adapter dan RTC
  if(!SD.begin(SD_CS)) {
    Serial.println("Memori Error");
    return;
  }
  if(!SD.exists("GKU.csv")) {
    //membuat file
    myFile = SD.open("GKU.csv", FILE_WRITE);
    if(myFile) {
      myFile.println("Tanggal,pH,Konduktivitas,TDS,CurahHujan,TemperaturLuar,KelembabanLuar,TemperaturDalam,KelembabanDalam,TemperaturAir,Heater,Cooler1,Cooler2,NilaiHujan,KondisiPenutup");
      myFile.close();
    }
  }
  Wire.begin();
  rtc.begin();
  if(!rtc.isrunning()) {
  Serial.println("RTC Error");
  }
   
  //Inisialisasi sensor EC
  ec.begin();
  
  //Inisialisasi sensor TDS
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);
  gravityTds.begin();
  
  //Inisialisasi sensor curah hujan
  pinMode(pin_interrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_interrupt), hitung_curah_hujan, FALLING);

  //Inisialisasi Kontrol Temperatur
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  
  //Inisialisasi sensor temperatur dan kelembaban udara
  dhtLuar.begin();
  dhtDalam.begin(); 

  //Inisialisasi motor servo
  pinMode (sensor_hujan, INPUT);
  servoKu.attach(7);
}

void loop(void){    
  //Sensor temperatur air
  float temperature = getTemp();

  //Kontrol Temperatur
  if(temperature > setpoint){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    kondisiCooler1 = 1;
    kondisiCooler2 = 1;
    kondisiHeater = 0;
  }
  else if(temperature < setpoint){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    kondisiCooler1 = 0;
    kondisiCooler2 = 0;
    kondisiHeater = 1;
  }
 
  //Sensor pH
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float phVoltage, phValueSensor, phVoltageCalib, phConversion, pHValue;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      phVoltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      phValueSensor = 3.5*phVoltage+Offset;
      //Persamaan Kalibrasi
      phVoltageCalib = (-0.369*phValueSensor)+4.866; //Kalibrasi tahap pertama
      phConversion = (phVoltageCalib/5.0)*14; //Konversi tegangan ke bentuk pH
      pHValue = (0.9964*phConversion)+0.0571; //Kalibrasi tahap kedua
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)
  {
    printTime=millis();
  }

  //Sensor EC
  static unsigned long timepoint = millis();
  if(millis()-timepoint>1000U)  //time interval: 1s
  {
    timepoint = millis();
    voltage = analogRead(EC_PIN)/1024.0*5000;   // read the voltage
    ecValueSensor =  ec.readEC(voltage,temperature);  // convert voltage to EC with temperature compensation
    //Persamaan Kalibrasi
    ecVoltageCalib = (0.2497*ecValueSensor)-0.0011; //Kalibrasi tahap pertama
    ecConversion = (ecVoltageCalib/5.0)*20.0; //Konversi tegangan ke bentuk EC
    ecValue = (0.999*ecConversion)+0.0012; //Kalibrasi tahap kedua
  }
  ec.calibration(voltage,temperature); 

  //Sensor TDS
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValueSensor = gravityTds.getTdsValue();  // then get the value
  //Persamaan Kalibrasi
  tdsVoltageCalib = (0.0049*tdsValueSensor)+0.0652; //Kalibrasi tahap pertama
  tdsConversion = (tdsVoltageCalib/1000)*5.0; //Konversi tegangan ke bentuk ppm
  tdsValue = (0.9987*tdsConversion)+4.6033; //Kalibrasi tahap kedua

  //Sensor curah hujan
  if (flag == true)
  {
    curah_hujan += milimeter_per_tip; 
    jumlah_tip++;
    delay(100);
    flag = false; // reset flag
  }
  curah_hujan = jumlah_tip * milimeter_per_tip;

  //Sensor temperatur dan kelembaban udara
  float templuar = dhtLuar.readTemperature();                     
  float humidluar = dhtLuar.readHumidity();
  float tempdalam = dhtDalam.readTemperature();                  
  float humiddalam = dhtDalam.readHumidity();

  //Inisialisasi sensor hujan sebagai digital input
  int kondisi_sensor = digitalRead(sensor_hujan);
  //Perulangan kondisi sensor saat adanya suatu masukan
  if(kondisi_sensor == LOW){
    nilaiHujan = 1;
  }
  else {
    nilaiHujan = 0;
  }
  //Perulangan kondisi motor servo saat adanya suatu masukan
  if(nilaiHujan == 1){
    servoKu.write (180);
    kondisiPenutup = 1;  
  }
  else{
    servoKu.write (0);
    kondisiPenutup = 0;
  }
              
  //Perubahan tipe data kedalam bentuk String
  strphValue = String(pHValue,2);
  strecValue = String(ecValue,2);
  strtdsValue = String(tdsValue);
  strCurahHujan = String(curah_hujan);
  strtempluar = String(templuar);
  strhumidluar = String(humidluar);
  strtempdalam = String(tempdalam);
  strhumiddalam = String(humiddalam);
  strTemperaturAir = String(temperature);
  strKondisiHeater = String(kondisiHeater);
  strKondisiCooler1 = String(kondisiCooler1);
  strKondisiCooler2 = String(kondisiCooler2);
  strnilaiHujan = String(nilaiHujan);
  strKondisiPenutup = String(kondisiPenutup);

  //Menampilkan data ke serial monitor
  Serial.println("*" + strphValue + "," + strecValue + "," + strtdsValue + "," + strCurahHujan + "," + strtempluar + "," + strhumidluar + "," + strtempdalam + "," + strhumiddalam + "," + strTemperaturAir + "," + strKondisiHeater + "," + strKondisiCooler1 + "," + strKondisiCooler2 + "," + strnilaiHujan + "," + strKondisiPenutup + "#");
  
  //Simpan data ke data logger
  save_datacsv(strphValue,strecValue,strtdsValue,strCurahHujan,strtempluar,strhumidluar,strtempdalam,strhumiddalam,strTemperaturAir,strKondisiHeater,strKondisiCooler1,strKondisiCooler2,strnilaiHujan,strKondisiPenutup);
  delay(20000);
}

//Method data logger (MicroSD Adapter & RTC)
void save_datacsv(String a, String b, String c, String d, String e, String f, String g, String h, String i, String j, String k, String l, String m, String n){
  //Set waktu dengan format YYYY-MM-DD HH:MM:SS
  DateTime now = rtc.now();
  String waktu = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
  String tgl = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
  String long_waktu = tgl+" "+waktu;
  String data = long_waktu+","+a+","+b+","+c+","+d+","+e+","+f+","+g+","+h+","+i+","+j+","+k+","+l+","+m+","+n;
  //Proses simpan data ke MicroSD
  myFile = SD.open("GKU.csv", FILE_WRITE);
  //Jika file bisa dibuka, simpan datanya
  if (myFile) {
    myFile.println(data);
    //tutup file:
    myFile.close();
  } else {
    //Jika file tidak bisa dibuka, print pesan error
    Serial.println("Writing Failed");
    Serial.println(" ");
  }
}

//Method tambahan sensor temperatur air
float getTemp(){
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }
  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperaturSum = tempRead / 16;

  return TemperaturSum;
}

//Method tambahan sensor pH
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
