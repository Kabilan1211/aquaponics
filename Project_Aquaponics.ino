#define BLYNK_TEMPLATE_ID "TMPLfdvyRyo0"
#define BLYNK_DEVICE_NAME "test"
#define BLYNK_AUTH_TOKEN "59RWk9lkhpe4n9r-V0YBk0zphSixSu7s"

#define PUMP1 D1̥
#define PUMP2 D2
#define SERVO D3
#define CLOUDCHECKER V4
#define TIMER V6 
#define BLYNK_PRINT Serial

// time trackers
unsigned long previousPhMillis = 0;
#define PHDELAY 2000

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include<Servo.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

Servo myservo;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "saravanan";
char pass[] = "1823912@v";
BlynkTimer timer;

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Blynk.setProperty(CLOUDCHECKER, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(CLOUDCHECKER, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(CLOUDCHECKER, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

void myTimerEvent()
{
  Blynk.virtualWrite(V5, millis() / 1000);
}

int phvalue;

int phsensor(){
   for(int i=1; i<=10; i++) {
  String phdata = Serial.readStringUntil(':');
  if(phdata !=NULL){
    String ph = Serial.readStringUntil('$');
    phvalue= (int)ph.toFloat();
    // Serial.println(phvalue);
    return phvalue;
  }
}
return -1;
}

double getConcentrationValue(int ph){
  if (ph > 10 || ph < 7){
    return -1;
  }
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    // Serial.print("[HTTPS] begin...\n");
    String phS = String(ph);
    if (https.begin(*client, "https://accs.vercel.app/api/ph?ph=" + phS)) {  // HTTPS
      // Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          StaticJsonDocument<256> doc;
          DeserializationError err = deserializeJson(doc, payload);
          if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.f_str());
          }
          return doc["data"]["ammonia"];
          
          // Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
    return 0;
}

BLYNK_WRITE(V2){
  if (param.asInt() == 1){
    myservo.write(140);
    delay(1000);
    myservo.write(0);
    delay(2000);
    Blynk.virtualWrite(V2, 0);
  }
}

// code for pumps
BLYNK_WRITE(V1){
  if(param.asInt()==1){
  digitalWrite(PUMP2, LOW);
  delay(5000);
  digitalWrite(PUMP1, LOW);
}
else{
  digitalWrite(PUMP1, HIGH);
  delay(10000);
  digitalWrite(PUMP2, HIGH);
}
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(SERVO, OUTPUT);
  myservo.attach(SERVO);
  
  timer.setInterval(1000L, myTimerEvent);
}

void loop(){
  Blynk.run();
  timer.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousPhMillis > PHDELAY){
      
    for(int i=1; i<=10; i++) {
        String phdata = Serial.readStringUntil(':');
        Serial.println(phdata); 
        if(phdata != ""){
            String ph = Serial.readStringUntil('$');
           
             Serial.println(ph);
            int phvalue=ph.toInt();
             Serial.println();
            Serial.println("PH Value");
            Serial.println(phvalue);
            if(ph <= 10 && ph >= 7){
              double concentration = getConcentrationValue(phValue);
              Blynk.virtualWrite(V7, concentration);
            }
    Blynk.virtualWrite(V6, phvalue);
                   }
    }
  }
}