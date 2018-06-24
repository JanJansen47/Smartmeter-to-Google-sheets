#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <ESP8266httpUpdate.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "CRC16.h"
#include <SoftwareSerial.h>

#define BAUD_RATE 115200

SoftwareSerial swSer(12, -1, false, 256);
// Button to update the firmware
const int updateButton = 2;

//Oled display
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>
#define  OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// temperatuur meting met Dallas
#define ONE_WIRE_BUS 14  //  gpio14 op de NodeMCU 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Tbinnen_val, Tbuit_val;

//Web update via  NAS

#define BOOT_AFTER_UPDATE true  // zorgt er voor dat de esp boot na remote image laden
//van het te uploaden bestand in SKETCH_BIN  Op de NAS staan deze bestanden in de web folder.
String url_upd = "http://sagittaweb.synology.me/ESP_bin/slimmemeter_piet.ino.bin";

int versie = 2;
// The ID below comes from Google Sheets.
const char *GScriptId = "---Your-Google-Script-ID--=";

// Push data on this interval
const int dataPostDelay = 60000;  // 1 minutes = 1 * 60 * 1000

const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";

const int httpsPort =  443;
HTTPSRedirect client(httpsPort);

// Prepare the url (without the varying data)
//String url = String("/macros/s/") + "AKfycbwBvLGsWUTdsBe-EcX9jYDtHi9jzSAW1Rtx5pYIX4OjRMOFkJ6Q" + "/exec?";
String url = String("/macros/s/") + "AKfycbwZUkMANJtR7cmIk8eGdT5VK3GDDC_AxFEd6BztkFE6QNpIDmM" + "/exec?";

//Fingerprint Command to retreive Fingerprint in Linux
//  echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout

const char* fingerprint = "97:E1:03:DA:DC:42:20:55:BE:8B:DA:F2:D3:B6:52:CE:4A:D1:B3:B8";


const bool outputOnSerial = true;

// Slimme meter variabelen
volatile long mEVLT = 123, EVLT; //Meter reading Electrics - consumption low tariff
volatile long mEVHT = 345, EVHT; //Meter reading Electrics - consumption high tariff
volatile long mEOLT = 0, EOLT; //Meter reading Electrics - return low tariff
volatile long mEOHT = 0, EOHT; //Meter reading Electrics - return high tariff
volatile long mEAV = 0;  //Meter reading Electrics - Actual consumption
volatile long mEAT = 0;  //Meter reading Electrics - Actual return
volatile long mGAS = 0, GAS;    //Meter reading Gas
volatile long prevGAS = 0;
#define MAXLINELENGTH 256 // longest normal line is 47 char (+3 for \r\n\0)
char telegram[MAXLINELENGTH];
unsigned int currentCRC = 0;
String  CRC2, CRC3;
long now, lastMsg , lastMsg1;


void setup() {

  Serial.begin(115200, SERIAL_8N1 );
  Serial.flush();
  swSer.begin(BAUD_RATE);
  pinMode(updateButton, INPUT_PULLUP);
  WiFiManager wifiManager;
  initdisplay();
  show(  "connect WiFi", "probeer");
  wifiManager.autoConnect("SlimmePeter", "Piet222");

  initdisplay();
  delay(10000);

  Serial.println(" IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print(String("Connecting to "));
  Serial.println(host);
  show("connecting to: ", host);

  // connect to Google host
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
    show(  "connection failed: ", "opnieuw");
  }

  // Connection Status, 1 = Connected, 0 is not.
  Serial.println("Connection Status: " + String(client.connected()));
  Serial.flush();

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    Serial.flush();
    return;
  }

  // Data will still be pushed even certification don't match.
  if (client.verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
    show(  "Certificate: ", "mis-match");
  }

  //init temperature sensors
  sensors.begin();  //Dallas temperatuur sensor
  show(  "Temp sensors: ", "start");
  //lees de temperatuur sensor uit
  sensors.requestTemperatures();// Dallas temperatuur sensor
  Tbinnen_val =   sensors.getTempCByIndex(0);
  show(  "Temp binnen: ", String(Tbinnen_val));
  Tbuit_val =  sensors.getTempCByIndex(1);
  show(  "Temp buiten: ", String(Tbuit_val));
}



// Measure and push data at the given interval to the Google sheet
void loop() {
  // Update firmware ?
  if (!digitalRead(updateButton)) {
    show(  "Update software?", "druk update button meer dan 5 sec");
    if ( !digitalRead(updateButton)) {
      delay(5000) ;
    };
    if ( !digitalRead(updateButton)) {
      WebUpdate();
    };
  };
  now = millis();
  //Update temperature measurements
  if (now - lastMsg1 > 5000) {
    lastMsg1 = now;
    //lees de temperatuur sensor uit
    sensors.requestTemperatures();// Dallas temperatuur sensor
    Tbinnen_val =   sensors.getTempCByIndex(0);
    Tbuit_val =  sensors.getTempCByIndex(1);
    showslim();
  }

  //Read the smartmeter
  readTelegram();

  

}

// Deze procdure stuurt de data naar de Google sheet ...................................................

void postData() {

  if (!client.connected()) {
    Serial.println("Connecting to client again...");
    show(  "Connecting: ", "opnieuw");
    client.connect(host, httpsPort);
  }
  String urlFinal = url +   "&Tbinnen_val=" + String(Tbinnen_val) +  "&Tbuit=" + "Tbuit" + "&Tbuit_val=" + String(Tbuit_val) +  "&mEAT_val=" + String(mEAT)  + "&mEVLT_val=" + String(mEVLT) + "&mEVHT_val=" + String(mEVHT) + "&mEAV=" + "mEAV" + "&mEAV_val=" + String(mEAV) + "&mEOLT_val=" + String( mEOLT) + "&mEOHT_val=" + String( mEOHT) + "&mGAS_val=" + String(mGAS) + "&CRC2_val=" + String(CRC2) + "&CRC3_val=" + String(CRC3);
  client.printRedir(urlFinal, host, googleRedirHost);
}
//Webupdate van NAS  -----------------------------------------------------------------

void WebUpdate() {

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print( "URL: "); Serial.println(url_upd);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    show(  "setup wait", String(t));
    Serial.flush();
    delay(1000);
  }
  show(  "software update", "WACHTEN Piet");
  delay(1000);
  ESPhttpUpdate.rebootOnUpdate(BOOT_AFTER_UPDATE);  // true
  t_httpUpdate_return ret = ESPhttpUpdate.update(url_upd);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      show(  "http udate fout", ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}


//OLED Display 05-06-2018  -----------------------------------------------------------------
void initdisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x78 >> 1);          // init done
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Piet & Jet");
  display.setTextSize(1);
  display.setCursor(0, 25);  display.print("Versie:  "); display.println(versie);
  display.setCursor(0, 55);  display.print("RRSI:  ");   display.println(WiFi.RSSI());
  display.setCursor(0, 35);  display.print("SSID:  ");   display.println(WiFi.SSID());
  display.setCursor(0, 45); display.print("IP:  ");      display.println(WiFi.localIP());
  display.setCursor(0, 55);  display.print("RSSI:  ");   display.println(WiFi.RSSI());
  display.display();

}
//Show een regel (bericht)
void show( String txt, String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("--BERICHTEN--");
  display.setCursor(0, 35);
  display.print(txt);
  display.setCursor(0, 45);
  display.print(msg);
  display.display();
  delay(1000);
}

// display alle waarden van de slimme meter
void showslim() {
  Serial.println( "showslim");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);  display.print("EVLT: "); display.println(String(mEVLT));
  display.setCursor(0, 9); display.print("EVHT: "); display.println(String(mEVHT));
  display.setCursor(0, 18); display.print("EOLT: "); display.println(String(mEOLT));
  display.setCursor(0, 27); display.print("EOHT: "); display.println(String(mEOHT));
  display.setCursor(0, 36); display.print("EAV : "); display.println(String(mEAV));
  display.setCursor(0, 45); display.print("EAT : "); display.println(String(mEAT));
  display.setCursor(0, 54); display.print("GAS : "); display.println(String(mGAS));
  display.display();
}

// slimme meter 06-11-2017 ..................................................................

void readTelegram() {
  // Serial.println("readTelegram");
  if (swSer.available()) {
    memset(telegram, 0, sizeof(telegram));
    while (swSer.available()) {
      int len = swSer.readBytesUntil('\n', telegram, MAXLINELENGTH);
      telegram[len] = '\n';
      telegram[len + 1] = 0;
      //yield();
      if (decodeTelegram(len + 1))  // alleen na valid CRC versturen
      {
      CRC3="PRIMA"; 
      if (now - lastMsg >  dataPostDelay) {
    yield();
    postData();
    lastMsg = now;
    }
      }
    }
  }
}
bool isNumber(char* res, int len) {
  for (int i = 0; i < len; i++) {
    if (((res[i] < '0') || (res[i] > '9'))  && (res[i] != '.' && res[i] != 0)) {
      return false;
    }
  }
  return true;
}

int FindCharInArrayRev(char array[], char c, int len) {
  for (int i = len - 1; i >= 0; i--) {
    if (array[i] == c) {
      return i;
    }
  }
  return -1;
}

long getValue(char* buffer, int maxlen) {
  // de waarde staat tussen ( en *
  int s = FindCharInArrayRev(buffer, '(', maxlen - 2);
  if (s < 8) return 0;
  if (s > 32) s = 32;
  int l = FindCharInArrayRev(buffer, '*', maxlen - 2) - s - 1;
  if (l < 4) return 0;
  if (l > 12) return 0;
  char res[16];
  memset(res, 0, sizeof(res));

  if (strncpy(res, buffer + s + 1, l)) {
    if (isNumber(res, l)) {
      return (1000 * atof(res));
    }
  }
  return 0;
}

bool decodeTelegram(int len) {
  //need to check for start
  int startChar = FindCharInArrayRev(telegram, '/', len);
  int endChar = FindCharInArrayRev(telegram, '!', len);
  bool validCRCFound = false;
  if(startChar>=0)
  {
    //start found. Reset CRC calculation
    currentCRC=CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);
    Serial.print("CRC1 HEX: ");Serial.println(currentCRC, HEX);
    if(outputOnSerial)
    {
         for(int cnt=startChar; cnt<len-startChar;cnt++)
         Serial.print(telegram[cnt]);
    }    
    
  }
  else if(endChar>=0)
  {
    //add to crc calc 
    currentCRC=CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);
    CRC2=currentCRC;   // dit is de "rest"

    // lees de rest uit het telegram
    char messageCRC[5];
    strncpy(messageCRC, telegram + endChar + 1, 4);
    messageCRC[4]=0; //thanks to HarmOtten (issue 5) 
   
    if(outputOnSerial)
    {
      Serial.print("2: ");
      for(int cnt=0; cnt<len;cnt++)
        Serial.print(telegram[cnt]);
    }    
    validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
    if(validCRCFound)
      Serial.println("\nVALID CRC FOUND!"); 
    else
      Serial.println("\n===INVALID CRC FOUND!===");
    currentCRC = 0;
  }
  else
  {
    currentCRC=CRC16(currentCRC, (unsigned char*)telegram, len);
    if(outputOnSerial)
    {
      Serial.print("3: ");
      for(int cnt=0; cnt<len;cnt++)
        Serial.print(telegram[cnt]);
    }
  }
CRC3= "NOK";

  if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0) 
 
    mEVLT =  getValue(telegram, len); 

  // 1-0:1.8.2(000560.157*kWh)
  // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0) 
 
    mEVHT = getValue(telegram, len);
   

  // 1-0:2.8.1(000348.890*kWh)
  // 1-0:2.8.1 = Elektra opbrengst laag tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0) 
 
    mEOLT = getValue(telegram, len);
  

  // 1-0:2.8.2(000859.885*kWh)
  // 1-0:2.8.2 = Elektra opbrengst hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0) 
  
    mEOHT = getValue(telegram, len);
   
    

  // 1-0:1.7.0(00.424*kW) Actueel verbruik
  // 1-0:2.7.0(00.000*kW) Actuele teruglevering
  // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0) 
    mEAV = getValue(telegram, len);
    
  if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
    mEAT = getValue(telegram, len);
   

  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
  if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0) 
     mGAS = getValue(telegram, len);

 
  return validCRCFound;
}

// eo slimme meter


