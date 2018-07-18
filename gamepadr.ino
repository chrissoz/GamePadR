//now has keepalive
//timer is just to report on serial every delay * second
//pin1 is lh switch
//pin2 is rh switch
//pin3 is vibration switch
//now GIT
#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiManager.h>
//#include <EthernetUdp.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiUDP.h>
WiFiUDP EthernetUdp;
#define PIN            15
#define NUMPIXELS      6

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned int localUdpPort = 58266;  // local port to listen on
byte incomingPacket[255];  // buffer for incoming packets
IPAddress remoteIPAddr = IPAddress(255, 255, 255, 255); // local subnet broadcast
int timer = 10; // overall timer
char  replyPacket[64];
char switchPos[4];
char oldswitchPos[4];
int pin1 = 5;
int pin2 = 4;
int pin3 = 13;
int padnumber;
long int keepalive = 0;
int maxkeepalive = 1000;
long int oldmillis;
boolean dead = false;
void setup() {
  // stuff for setup
  Serial.begin(115200);
  Serial.println("Booting");
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

  pixels.begin(); // This initializes the NeoPixel library.
  allpix(255, 0, 0);
  WiFi.begin("Game");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  allpix(255, 255, 255);
  delay (500);
  allpix(0, 0, 0);
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  padnumber = (WiFi.localIP())[3];
  EthernetUdp.begin(localUdpPort);
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);
  pinMode(pin3, INPUT_PULLUP);

  sprintf(oldswitchPos, "%d%d%d%d", padnumber, digitalRead(pin1), digitalRead(pin2), digitalRead(pin3));
}

void allpix(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, r, g, b);

  }
  pixels.show();
}
void loop() {
  ESP.wdtFeed();
  delay (1);
  if (millis() > (oldmillis + 20000)) {
    //report status ever 5s
    oldmillis=millis();
    Serial.print("Timer ");
    Serial.print(timer);
    Serial.print("  ");
    Serial.println(switchPos);
  }
  if (((millis()-keepalive) > maxkeepalive)  && !dead) {
    dead=true;
    allpix(255, 255, 255);
  }

  sprintf(switchPos, "%d%d%d%d", padnumber, digitalRead(pin1), digitalRead(pin2), digitalRead(pin3));
  if (memcmp_P(switchPos, oldswitchPos, 4 ) == 0) {} else {
    memcpy( oldswitchPos, switchPos,  4 );
    EthernetUdp.beginPacket(remoteIPAddr, localUdpPort);
    EthernetUdp.write(switchPos);
    EthernetUdp.endPacket();
    Serial.println(switchPos);
  }

  int packetSize = EthernetUdp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, EthernetUdp.remoteIP().toString().c_str(), EthernetUdp.remotePort());
    int len = EthernetUdp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    //Serial.println(len);
    if (len == 18) {
        for  (int i = 0; i < 19; i++) {
    Serial.printf(" %d", incomingPacket[i]);
  }
  Serial.println("");
  
      for (int i=0;   i < len; i = i + 3) {
        //Serial.print(incomingPacket[i]);
        pixels.setPixelColor(i / 3, pixels.Color(incomingPacket[i], incomingPacket[i + 1], incomingPacket[i+2]));
        pixels.show();
        Serial.printf("P %d r %d g %d b %d \n",i / 3, incomingPacket[i], incomingPacket[i + 1], incomingPacket[i+2]);
        

      }
    } else {
      if (len == 1) {
        if (dead) {
          allpix(0, 0, 0);
          dead=false;
        }
        keepalive = millis();
        //Serial.println("Keepalive recived");

      } else {
        Serial.println("Strange packet");
        for(int i = 0; i < len; i++){Serial.print(incomingPacket[i]);}
        Serial.println("");
      }
    }
  }
}
