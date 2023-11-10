#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

const char* ssid = "Orbit"; 
const char* password = "AELab123";
unsigned long myChannelNumber1 = 2157336;
unsigned long myChannelNumber2 = 2157346;
const char * WriteAPIKey1 = "0YUPKH6XGMYHAKUM";
const char * WriteAPIKey2 = "S50LBTXHFCMDNVLD";
int x1,x2;
 WiFiClient  client;
 void WifiConnect(){ //Prosedur buat konek ke wifi
   Serial.print("\n\nStatus Wifi: ");
   if(WiFi.status() != WL_CONNECTED){Serial.print("Menghubungkan\n"); WiFi.begin(ssid, password);}
   else{Serial.print("Terhubung!\n");}
   delay(10);
 }

String dataIn;
String dt[14];
int i;
boolean parsing = false;
float flophValue, floEcValue, flotdsValue, floCurahHujan, floTemperaturAir, flotempluar, flohumidluar, flotempdalam, flohumiddalam;
int intKondisiHeater, intKondisiCooler1, intKondisiCooler2, intNilaiHujan, intKondisiPenutup;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);
  dataIn = "";
}

void loop() { 
  if (Serial.available() > 0) {
    delay(12);
    char inChar = (char)Serial.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }
  if (parsing) {
    int j = 0;
    //kirim data yang telah diterima sebelumnya
    Serial.print("Data masuk : ");
    Serial.println(dataIn);

    //inisialisasi variabel, (reset isi variabel)
    dt[j] = "";
    //proses parsing data
    for (i = 1; i < dataIn.length(); i++) {
      //pengecekan tiap karakter dengan karakter (#) dan (,)
      if ((dataIn[i] == '#') || (dataIn[i] == ','))
      {
        //increment variabel j, digunakan untuk merubah index array penampung
        j++;
        dt[j] = "";     //inisialisasi variabel array dt[j]
      } else{
        //proses tampung data saat pengecekan karakter selesai.
        dt[j] = dt[j] + dataIn[i];
        //j++;
        }
    }
    
    //kirim data hasil parsing
    flophValue = dt[0].toFloat();
    floEcValue = dt[1].toFloat();
    flotdsValue = dt[2].toFloat();
    floCurahHujan = dt[3].toFloat();
    flotempluar = dt[4].toFloat();
    flohumidluar = dt[5].toFloat();
    flotempdalam = dt[6].toFloat();
    flohumiddalam = dt[7].toFloat();
    floTemperaturAir = dt[8].toFloat();
    intKondisiHeater = dt[9].toInt();
    intKondisiCooler1 = dt[10].toInt();
    intKondisiCooler2 = dt[11].toInt();
    intNilaiHujan = dt[12].toInt();
    intKondisiPenutup = dt[13].toInt();
        
    //SEND
    WifiConnect();
    sendThingSpeak1(flophValue, floEcValue, flotdsValue, floCurahHujan, flotempluar, flohumidluar, flotempdalam, flohumiddalam);
    sendThingSpeak2(floTemperaturAir, intKondisiHeater, intKondisiCooler1, intKondisiCooler2, intNilaiHujan, intKondisiPenutup);
    Serial.print("pH : "); Serial.println(flophValue);
    Serial.print("EC : "); Serial.println(floEcValue);
    Serial.print("TDS : "); Serial.println(flotdsValue);
    Serial.print("Curah Hujan : "); Serial.println(floCurahHujan);
    Serial.print("Temperatur Luar : "); Serial.println(flotempluar);
    Serial.print("Kelembaban Luar : "); Serial.println(flohumidluar);
    Serial.print("Temperatur Dalam : "); Serial.println(flotempdalam);
    Serial.print("Kelembaban Dalam : "); Serial.println(flohumiddalam);   
    Serial.print("Temperatur Air : "); Serial.println(floTemperaturAir);
    Serial.print("Heater : "); Serial.println(intKondisiHeater);
    Serial.print("Cooler 1 : "); Serial.println(intKondisiCooler1);
    Serial.print("Cooler 2 : "); Serial.println(intKondisiCooler2);
    Serial.print("Nilai Hujan : "); Serial.println(intNilaiHujan);
    Serial.print("Kondisi Penutup : "); Serial.println(intKondisiPenutup);

    parsing = false; 
    delay(200);
  }
}

void sendThingSpeak1(float phValue, float ecValue, float tdsValue, float curahHujan, float templuar, float humidluar, float tempdalam, float humiddalam)
{      
  ThingSpeak.setField(1, phValue);
  ThingSpeak.setField(2, ecValue);
  ThingSpeak.setField(3, tdsValue);
  ThingSpeak.setField(4, curahHujan);
  ThingSpeak.setField(5, templuar);
  ThingSpeak.setField(6, humidluar);
  ThingSpeak.setField(7, tempdalam);
  ThingSpeak.setField(8, humiddalam);
  x1 = ThingSpeak.writeFields(myChannelNumber1, WriteAPIKey1);
  if(x1 == 200){
    Serial.println("Channel 1 update successful.");
  }
  else{
    Serial.println("Problem updating channel 1. HTTP error code " + String(x1));
  }
}

void sendThingSpeak2(float temperature, int kondisiHeater, int kondisiCooler1, int kondisiCooler2, int nilaiHujan, int kondisiPenutup)
{ 
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, kondisiHeater);
  ThingSpeak.setField(3, kondisiCooler1);
  ThingSpeak.setField(4, kondisiCooler2);
  ThingSpeak.setField(5, nilaiHujan);
  ThingSpeak.setField(6, kondisiPenutup);    
  x2 = ThingSpeak.writeFields(myChannelNumber2, WriteAPIKey2);
  if(x2 == 200){
    Serial.println("Channel 2 update successful.");
  }
  else{
    Serial.println("Problem updating channel 2. HTTP error code " + String(x2));
  }
}