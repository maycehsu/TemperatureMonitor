#include <IRremote.h>
#include <IRremoteInt.h>
#define esp8266 Serial1   // use esp8266 to talk to esp8266
#define SSID "airport"    // put here the name of your wifi network
#define PASS "26885379"             // put here the password of your wifi network
#define IP "184.106.153.149" // thingspeak.com's IP

#define tempPIN 0 // soil moisture sensor connected to Analog Pin 0
#define redLED 12
#define greenLED 11
#define yellowLED 10
#define targetTemp 26
#define LOW_THRESH 25.5
#define HIGH_THRESH 28
#define SAMPLE_TIME 1000  //ms
#define AVERAGE_SAMPLES 10
#define TRIGGER_ONOFF_COUNTER 30 //30 consequence * 10s => 300 seconds


IRsend irsend; 

String GET = "GET /update?key=V77GYUTPMC8KK9B0&";    // put here your thingspeak key
String GET1 = "field1=";
String GET2 = "&field2=";

float tempVal = 0;                           // we send the soil moisture value
float avgTemp = 0;

float temperature (){
    
    //char temp_c[6];
    int sensorVal = analogRead(tempPIN);
    float volt = (sensorVal/1024.0)*5.0;
    float temp = (volt - 0.5)*100.0;
    
    
    if(temp < LOW_THRESH){
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, LOW);
        digitalWrite(yellowLED, HIGH);
    }
    else if(temp >= HIGH_THRESH){
        digitalWrite(redLED, HIGH);
        digitalWrite(greenLED, LOW);
        digitalWrite(yellowLED, LOW);
    }
    else{
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
    }
    //dtostrf(temp, 0, 1, temp_c); //converts floats to strings

    //return (String) temp_c;
    return temp;
}



void upadateValues(String temp, String temp_avg){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  esp8266.println(cmd);
  delay(2000);
  if(esp8266.find("Error")){
    Serial.print("Error1");
    return;
  }
  cmd = GET + GET1;
  cmd += temp;
  cmd += GET2;
  cmd += temp_avg;
  cmd += "\r\n";
  
  Serial.print(cmd);
  esp8266.print("AT+CIPSEND=");
  esp8266.println(cmd.length());
  if(esp8266.find(">")){
    esp8266.print(cmd);
  }else{
    esp8266.println("AT+CIPCLOSE");
  }
}
 
boolean connectWiFi(){
  esp8266.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  esp8266.println(cmd);
  delay(5000);
  if(esp8266.find("OK")){
    Serial.println("OK");
    return true;
  }else{
    Serial.println("KO");
    return false;
  }
}

void setup()
{
  pinMode(tempPIN, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, HIGH);
  digitalWrite(yellowLED, HIGH);
  Serial.begin(9600);
  esp8266.begin(115200);
  delay(5000);
  esp8266.println("AT");
  
  if(esp8266.find("OK")){
    Serial.println('Connect 8266 OK');
    connectWiFi();
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
  }
  else{
    Serial.println("Connect 8266 fail");
  }
}


int i=0;
float tempSum = 0; //temperature sum in 60 seconds
int lowCount = 0;
int highCount = 0;
int power = 1;
void loop(){
  //Heran air conditioner power on
  unsigned int power_on[110] = {750,3350,550,650,550,650,550,650,550,650,550,650,550,650,550,1750,550,1750,600,650,500,700,500,700,500,700,500,700,500,1800,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,600,600,550,700,500,700,500,700,500,700,500,700,500,700,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,1750,550,1750,550,700,500,700,500,1800,500,700,500,1800,500,700,500,1800,550,650,550,1750,550,650,550,650,550,1750,550,3400,700,};
  //Heran air conditioner powe off
  unsigned int power_off[110] = {750,3350,550,650,550,650,550,650,550,650,550,650,550,650,550,700,500,700,500,700,550,650,550,650,500,700,550,650,550,1750,550,650,550,650,550,650,550,650,550,650,550,650,550,650,600,600,550,700,550,650,550,650,500,700,500,700,500,700,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,650,550,1800,500,1800,500,700,500,700,500,1800,500,700,500,1800,550,650,550,1750,550,650,550,1750,550,1750,550,650,550,650,550,3450,650,};


  tempVal = temperature();
  
  
  
  if(i<AVERAGE_SAMPLES){
    Serial.print("Temperature: ");
    Serial.println(tempVal);
    tempSum += tempVal;
    i++;
  }
  else{
    avgTemp = tempSum/AVERAGE_SAMPLES;
    
    Serial.print("average Temperature :");
    Serial.println(avgTemp);
    
    
    if(avgTemp<LOW_THRESH){
      lowCount++;
      highCount = 0;
      Serial.print("low count :");
      Serial.println(lowCount);
      
      if(lowCount >=TRIGGER_ONOFF_COUNTER){
        irsend.sendRaw(power_off, 110, 38);
        Serial.println("Send Power Off");
        power = 0; 
        lowCount = 0;
      }
    }
    else if(avgTemp>=HIGH_THRESH){
      highCount++;
      lowCount = 0;
      Serial.print("high count :");
      Serial.println(highCount);
      
      if(highCount >=TRIGGER_ONOFF_COUNTER){
        irsend.sendRaw(power_on, 110, 38);
        Serial.println("Send Power On");
        power = 1;
        highCount = 0;
      }
      
    }
    else{
      lowCount = 0;
      highCount = 0;
      Serial.println('reset all counters, back to target temperature');
    }
    tempSum = 0;
    i=0;
    upadateValues((String)avgTemp, (String)power);
  }
  delay(SAMPLE_TIME);
  
}
