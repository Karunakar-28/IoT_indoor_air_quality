#include "MQ135.h"
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

const char *apiKey = thinkspeakapi;
const char *ssid = WIFI_id ;
const char *pass = Wifi_pwd;
unsigned long myChannel = channel_id;

const int relayInput = 13;

WiFiClient client;

unsigned int output_pin = A0;
MQ135 gasSensor = MQ135(output_pin);
float ppm;
int offline = 0;
int alert = 0;
int fan = 0;
int first = 0;

void setup(){
  Serial.begin(115200);
  delay(10);
  preHeat();
  WiFi.mode(WIFI_STA); 
  internet();
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  pinMode(relayInput,OUTPUT);
  digitalWrite(relayInput,HIGH);
}
  
void loop(){
  get_value();
  if(!offline) upload();
  else delay(2000);
  Serial.println("-------------------------------------------------");
}

void internet(){
  Serial.print("Trying to connect with -> ");
  Serial.println(ssid);

  WiFi.begin(ssid,pass);

  unsigned long ElapsedTime;
  unsigned long StartTime = millis();
  
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(". ");
    ElapsedTime = millis() - StartTime;
    if(ElapsedTime >= 60000){
      break;
    }
  }
  Serial.println("");
  if(WiFi.status() != WL_CONNECTED){
    offline = 1;
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
    Serial.println("* Unable to connect to WiFi...                  *");
    Serial.println("* CO2 levels in PPM (offline mode)...           *");
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
  }
  else{
    Serial.println("WiFi connected");
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
  }
}

void preHeat(){
  Serial.println("Pre-Heating sensor"); 
  delay(60000);
  Serial.println("Pre-Heating done");
}

void get_value(){
  float rzero = gasSensor.getRZero();
  //Serial.print(rzero);
  //Serial.print("   ");
  Serial.print("PPM : ");
  ppm  = gasSensor.getPPM();
  Serial.println(ppm);
  if(alert and ppm<=800){
    alert = 0;
    //code here for making fan pin low
    digitalWrite(relayInput,HIGH);
    Serial.println("-------------------------------------------------");
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
    Serial.println("* Fan is switched off...                         *");
    fan = 0;
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
  }
  if(!alert and ppm>=1000){
    alert = 1;
    Serial.println("-------------------------------------------------");
    Serial.println("*-*-*-*-*-*-*::: Alert :::-*-*-*-*-*-*-*-*-*-*-**");
    Serial.println("* PPM greater than 1000                         *");
    // code here for making fan pin high
    digitalWrite(relayInput,LOW);
    Serial.println("* Fan is switched on...                         *");
    fan = 1;
    first = 1;
    Serial.println("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
  }
}

void upload(){
  ThingSpeak.setField(1, ppm);
  ThingSpeak.setField(2, fan);
  ThingSpeak.setField(3, first);
  int x = ThingSpeak.writeFields(myChannel, apiKey);
  if(first)first = 0;
  if(x == 200){
    Serial.println("Channel update successful.");
    Serial.println("-------------------------------------------------");
    for(int i=0;i<10;i++){
      get_value();
      Serial.println("-------------------------------------------------");
      delay(2000);
    }
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    internet();
  }
}
