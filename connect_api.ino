#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#define SECRET_SSID ""
#define SECRET_PASS ""
Adafruit_SGP30 sgp;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
char server[] = "https://espi-embarquee-back.herokuapp.com";    // name address for Google (using DNS)

WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connexion au réseau WPA/WPA2.
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }
  Serial.println("Connected to wifi");

  if (!sgp.begin()){
    Serial.println("Sensor not found !");
    return;
  }else{
     Serial.println("Sensor found !");
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}

void loop() {
  Serial.println("--- Début de la boucle ---");
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  delay(7500); // Toutes les 7.5 secondes
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  uint16_t TVOC_base, eCO2_base;
  //Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  //Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  //Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    char params[8192];
    // client.println("GET /api/co2/?format=json HTTP/1.1");
    snprintf(params, 1024, "GET /api/send-co2/?salle=218&value=%d HTTP/1.1", sgp.eCO2);
    Serial.println(params); 
    client.println(params);
  
    client.println("Host: espi-embarquee-back.herokuapp.com");
    client.println("Connection: close");
    char c = client.read();
    Serial.write(c);
    client.println();
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
  Serial.println(client.status());
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
    while (true);
  }
    Serial.println("--- Fin de la boucle ---");
}
