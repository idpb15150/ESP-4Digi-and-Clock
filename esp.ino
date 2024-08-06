//esp8266 v 2.7.0
//ardurino 1.8.6
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TM1637Display.h>
#define CLK D7  //0
#define DIO D8  //4
#define time_zone 3600*7
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht;
TM1637Display display(CLK, DIO);
char ssid[] = "Pumbrodin aisfibre_2.4G";  
char pass[] = "0990042557";    
int relay = D0; 
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;
const int ldrPin = A0;
/////////////////////////////////////////////////////////////////////
unsigned long epoch;
int hh;
int mm;
int ss;
int force_update = 1;
bool showdot = false;
uint32_t ts, ts1, ts2, ts3, ts4;
unsigned int localPort = 2390;

int ip = 0;
String IP_of_Server[5]    = {"122.155.169.213", "129.6.15.28", "158.108.212.149", "203.158.118.2", "129.250.35.250"};
IPAddress timeServer;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;

unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void NTP_get(void)
{
  sendNTPpacket(timeServer);
  delay(1000);
  int cb = udp.parsePacket();
  if (!cb) {
    force_update = 1;
  }
  else {
    force_update = 0;
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    epoch = secsSince1900 - seventyYears + time_zone;
  }
}
/////////////////////////////////////////////////////////////////////
void setup() {
pinMode(relay, OUTPUT);
lcd.begin();
dht.setup(D6);
lcd.backlight();
Serial.begin(9600);
pinMode(ldrPin, INPUT);
servo.attach(2); //D4
servo.write(0);
delay(2000);
/////////////////////////////////////////////////////////////////////
pinMode(7, OUTPUT);
  digitalWrite(7, LOW);  // ssid not connect
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  //Get package from NTP
  while (force_update == 1) {
    if (ip >= 5) ip = 0;
    timeServer.fromString(IP_of_Server[ip]);
    Serial.println(timeServer);
    ip += 1;
    NTP_get();
  }

  delay(100);

  hh = (epoch % 86400L) / 3600;
  mm = (epoch % 3600) / 60;
  ss = (epoch % 60);

  ts = ts1 = ts2 = ts3 = ts4 = millis();
  digitalWrite(2, HIGH);  //SSID connect ready

/////////////////////////////////////////////////////////////////////
}
void loop() {
int ldrStatus = analogRead(ldrPin);
float T = dht.getTemperature(); 
lcd.setCursor(2, 1);
lcd.println("Temp"+String(T,1)+" C"); 
////
////////////////////////////////////////////////////////////////////////
ts = millis();

  if (WiFi.status() == WL_CONNECTED) {

    /////////////////////////check time every 1 Hr.////////////////////////////////
    if ((WiFi.status() == WL_CONNECTED) && (ts - ts3 >= 3600000) && (force_update == 0)) {
      NTP_get();
      ts3 = millis();
    }

  }

  if ( ts - ts1 >= 1000 ) {

    epoch++; //Add a second

    hh = (epoch % 86400L) / 3600;
    mm = (epoch % 3600) / 60;
    ss = (epoch % 60);

    ts1 += 1000; // increment counter by 1 every 1sec

  }

  if ( ts - ts4 >= 500 ) {

    display.dotShow(showdot = !showdot);
    ts4 = millis();

  }

  if ( ts - ts2 >= 5 ) {

    display.write(hh / 10, 0);
    display.write(hh % 10, 1);
    display.write(mm / 10, 2);
    display.write(mm % 10, 3);

  }

  


///////////////////////////////////////////////////////////////////////
if (ldrStatus >= 200) {
servo.write(0);
 digitalWrite(relay, HIGH);
 delay(1000);
  lcd.begin();
  lcd.setCursor(5, 0); 
  lcd.print("Close"); 
}
else {
servo.write(90);
 digitalWrite(relay, LOW);
 delay(1000);
  lcd.begin();
  lcd.setCursor(5, 0); 
  lcd.print(""); 
  lcd.print("open"); 
}
}
