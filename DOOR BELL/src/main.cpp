#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// D7 = LED že je aktivní zvonek
// D4 = jsem listener
// D5 = buzzer
// TLACITKO přepínání kdo bude poslouchat= D3
// TLAČITKO ZVONEK = D6

//#define MQTT_SERVER      "10.0.1.43"     //doma
#define MQTT_SERVER      "192.168.2.39"   //fcc ps
#define MQTT_SERVERPORT  1883
#define nazev           " Radka"  // role tohoto zařízení

#define bell_ton           1000
#define bell_delka         3000

#define nazev1           " Sklad"
#define nazev2           " Ucetni"
#define nazev3           " Bozek"
#define nazev4           " Obchod"
#define nazev5           " Radka"
#define nazev6           "Realizace"
#define nazev7           "  ALL"

#define OLED_RESET 0  // GPIO0

Adafruit_SSD1306 display(OLED_RESET);



WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER,MQTT_SERVERPORT, "", "");


Adafruit_MQTT_Publish p_doorbell = Adafruit_MQTT_Publish(&mqtt,"/fcc/doorbell");
Adafruit_MQTT_Publish p_alive = Adafruit_MQTT_Publish(&mqtt,"/fcc/alive");
Adafruit_MQTT_Publish p_online = Adafruit_MQTT_Publish(&mqtt,"/fcc/online");
Adafruit_MQTT_Publish p_listener = Adafruit_MQTT_Publish(&mqtt,"/fcc/listener");

Adafruit_MQTT_Subscribe s_listener = Adafruit_MQTT_Subscribe(&mqtt,"/fcc/listener");
Adafruit_MQTT_Subscribe s_doorbell = Adafruit_MQTT_Subscribe(&mqtt,"/fcc/doorbell");
Adafruit_MQTT_Subscribe s_alive = Adafruit_MQTT_Subscribe(&mqtt,"/fcc/alive");
Adafruit_MQTT_Subscribe s_online = Adafruit_MQTT_Subscribe(&mqtt,"/fcc/online");

void MQTT_connect();

void setup()   {
  pinMode(D3, INPUT_PULLUP);   // tlačítko SELECT
  pinMode(D6, INPUT_PULLUP);   // tlačítko ZVONEK
  pinMode(D7, OUTPUT);         // LED zvonek
  pinMode(D5, OUTPUT);         // výstup zvonek

  pinMode(D4,OUTPUT);          // LED jsem listener

  Serial.begin(115200);
  Serial.println();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  delay(1000);


    WiFiManager wifiManager;
     // wifiManager.resetSettings(); // testovaní portalu
    if (!wifiManager.autoConnect("AutoConnectAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)"); Serial.println("local ip"); Serial.println(WiFi.localIP());

    mqtt.subscribe(&s_listener);
    mqtt.subscribe(&s_doorbell);
    mqtt.subscribe(&s_alive);
    mqtt.subscribe(&s_online);
    MQTT_connect();

    if (nazev==nazev1) p_listener.publish(nazev1);   // pokud jsem master (sklad), pošli ze poslouchám já
    else p_alive.publish(nazev);   // pokud nejsem master - posli info že jsem nabootoval
    digitalWrite(D7,HIGH);  // vypni zvonek
  }

  void MQTT_connect() {
    int8_t ret;

    if (mqtt.connected()) {
      return;
    }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("Retrying MQTT connection in 5 seconds...");
         mqtt.disconnect();
         delay(5000);  // wait 5 seconds
         retries--;
         if (retries == 0) {
           // basically die and wait for WDT to reset me
           while (1);
         }
    }
    Serial.println("MQTT Connected!");
  }

  int i=1;
  boolean last_state=false; // stav tlačítka při posledním průchodu smyčky
  String message;

void loop() {

   MQTT_connect();

Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(50))) {
    if (subscription == &s_doorbell) {               // došla zpráva o změně stavu tlačítka zvonek
    display.setCursor(0,5);
    display.setTextColor(WHITE, BLACK);
    display.println("## BELL ##");
    display.display();

       message=(char *)s_doorbell.lastread;         // bylo zmáčknuté tlačítko zvonek
       if (message=="TRUE") {
       message=(char *)s_listener.lastread;        // jsem ten který by měl zonit?
        if ((message==nazev) or (message==nazev7) or (nazev==nazev1)) {
          tone(D5,bell_ton,bell_delka);}    // zazvoň
       }
         else {
           display.setCursor(0,5);
           display.setTextColor(WHITE, BLACK);
           display.println("          ");
           display.display();
          ;}
    };

     if (subscription == &s_listener) {               // je změna listenera?
       display.clearDisplay();
       display.setTextSize(1);
       display.setTextColor(WHITE);
       display.setCursor(5,20);
       display.println((char *)s_listener.lastread);
       display.display();

       message=(char *)s_listener.lastread;        // jsem ten který by měl zvonit?
       if ((message==nazev) or (message==nazev7) or (nazev==nazev1))  {
        tone(D5,1000,300);   // tak pípni
        p_online.publish(nazev); // a pošli zprávu že jsi online
       }
    };

       if (subscription == &s_alive) {               // bootoval někdo?
       if (nazev==nazev1) p_listener.publish((char *)s_listener.lastread); //pokud jsi master (Sklad) pošli mu kdo má zvonit (je listener)
       tone(D8,1000,100);   // a pípni
    };
       if (subscription == &s_online) {               // je zvolený listener online?
    display.setCursor(5,32);
    display.setTextColor(BLACK, WHITE);
    display.println("on-line");
    display.display();

       };

  }

if (!last_state)
 if (digitalRead(D6)!=1) {                           // bylo zmáčknuté  tlačítko?
   p_doorbell.publish("TRUE"); last_state= true;
  };

if (last_state)
if (digitalRead(D6)==1) {                            // bylo uvolněné tlačítko?
  p_doorbell.publish("FALSE"); last_state= false;
  };

 if (digitalRead(D3)!=1) {         // bylo zmáčknuto tlačítko SELECT?
 delay(200);

  i++;
  switch (i) {
  case 1:  p_listener.publish(nazev1);
   break;
  case 2:  p_listener.publish(nazev2);
   break;
  case 3:  p_listener.publish(nazev3);
   break;
  case 4: p_listener.publish(nazev4);
   break;
  case 5:  p_listener.publish(nazev5);
   break;
  case 6: p_listener.publish(nazev6);
   break;
  case 7: p_listener.publish(nazev7); i=0;
   break;
  }

 }
        message=(char *)s_listener.lastread;        // jsem ten který by měl zvonit?
        if ((message==nazev) or (message==nazev7) or (nazev==nazev1))  digitalWrite(D4,LOW);   // rozsviť LED
        else digitalWrite(D4,HIGH);

}
