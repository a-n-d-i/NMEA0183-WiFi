#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "SoftwareSerial.h"
#include <WiFiUdp.h>
#include "credentials.h"
#include <Ticker.h>

/*
 * Some soft Watchdog functionality taken from 
 * https://github.com/esp8266/Arduino/issues/1532
 * 
 * The idea is: restart if you don't succeed in sending
 * a valid packet for more then 30 seconds.
 * 
 */
Ticker tickerOSWatch;

#define OSWATCH_RESET_TIME 30

static unsigned long last_loop;

void ICACHE_RAM_ATTR osWatch(void) {
    unsigned long t = millis();
    unsigned long last_run = abs(t - last_loop);
    if(last_run >= (OSWATCH_RESET_TIME * 1000)) {
      // save the hit here to eeprom or to rtc memory if needed
        ESP.restart();  // normal reboot 
        //ESP.reset();  // hard reset
    }
}

/* we have to use softserial because ttl level from out nmea source is inverted compared to 
the rs232 level of the esp8266 UART. Might be a good idea to use a level converter and the 
build in UART instead of a voltage divider and softserial. Already put in an order for parts...

get lib from here and extract to arduino libraries folder...
https://github.com/plerup/espsoftwareserial
*/
SoftwareSerial softserial(D1, 3, true); // RX, TX, inverted

const char* ssid = SSID;
const char* password = PASSWORD;

String currentSentence;

WiFiUDP Udp;

ESP8266WebServer server(80);

// handles http request to esp8266
void handleRoot() {
  server.send(200, "text/plain", "last sentence " + currentSentence);
}

void setup(void){
  last_loop = millis();
  
  tickerOSWatch.attach_ms(((OSWATCH_RESET_TIME / 3) * 1000), osWatch);

  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(4800);
  softserial.begin(4800);
  Serial.println("startup....");
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MDNS.begin("esp8266wifi");
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

char databuffer[300];

void loop(void){
  server.handleClient();
  digitalWrite(LED_BUILTIN, HIGH);
  softserial.setTimeout(30);

  // just listen on the serial and send every line which starts
  // with a "$" as an UDP Packet...
  if (softserial.available() > 0 ) {
    byte count = softserial.readBytesUntil(0x0A, databuffer, 300);
    if (databuffer[0] == '$') {
      // feed the watchdog
      last_loop = millis();
      currentSentence = String(databuffer);
      Serial.println(currentSentence);
      Udp.beginPacket(HOST, PORT);
      Udp.write(databuffer);
      Udp.endPacket();
      digitalWrite(LED_BUILTIN, LOW);
    }   
  }
}
