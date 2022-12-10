#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "Etisalat 4G iModem-5669"
#define WIFI_PASSWORD "02712453"
#define API_KEY "AIzaSyBzOXJHsiKddSSGtLYOoVpGyU5mHiL3KSM"
#define DATABASE_URL "https://gr433-5068f-default-rtdb.firebaseio.com/"
int pH = 39;
int dataP;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

#define anInput     A0                        //analog feed from MQ135
#define co2Zero     55                        //calibrated CO2 0 level

const int sensorPin = A0; //Sensor AO pin to Arduino pin A0
int value;          //Variable to store the incomming data

#define SensorPin  A0
unsigned long int avgValue;
float b;
int buf[10], temp;

void setup() {
  pinMode(anInput, INPUT);                    //MQ135 analog feed set for input
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  pinMode(pH, INPUT);

}

void loop() {
  
}

void phReading()
{
  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6;
  phValue = 3.5 * phValue;
  Serial.print("    pH:");
  Serial.print(phValue, 2);
  Serial.println(" ");
  digitalWrite(2, HIGH);
  delay(800);
  digitalWrite(13, LOW);
}

void lvlReading()
{
  value = analogRead(sensorPin); //Read data from analog pin and store it to value variable

  if (value <= 480) {
    Serial.println("Water level: 0mm - Empty!");
  }
  else if (value > 480 && value <= 530) {
    Serial.println("Water level: 0mm to 5mm");
  }
  else if (value > 530 && value <= 615) {
    Serial.println("Water level: 5mm to 10mm");
  }
  else if (value > 615 && value <= 660) {
    Serial.println("Water level: 10mm to 15mm");
  }
  else if (value > 660 && value <= 680) {
    Serial.println("Water level: 15mm to 20mm");
  }
  else if (value > 680 && value <= 690) {
    Serial.println("Water level: 20mm to 25mm");
  }
  else if (value > 690 && value <= 700) {
    Serial.println("Water level: 25mm to 30mm");
  }
  else if (value > 700 && value <= 705) {
    Serial.println("Water level: 30mm to 35mm");
  }
  else if (value > 705) {
    Serial.println("Water level: 35mm to 40mm");
  }

  delay(3000); // Check for new value every 3 sec
}

void co2Reading()
{
  int co2now[10];                               //int array for co2 readings
  int co2raw = 0;                               //int for raw value of co2
  int co2ppm = 0;                               //int for calculated ppm
  int zzz = 0;                                  //int for averaging


  for (int x = 0; x < 10; x++) //samplpe co2 10x over 2 seconds
  {
    co2now[x] = analogRead(A0);
    delay(200);
  }

  for (int x = 0; x < 10; x++) //add samples together
  {
    zzz = zzz + co2now[x];
  }

  co2raw = zzz / 10;                          //divide samples by 10
  co2ppm = co2raw - co2Zero;                 //get calculated ppm

  Serial.print("Co2=");
  Serial.print(co2ppm);  // prints the value read
  Serial.println(" PPM");
  delay(50);

}
