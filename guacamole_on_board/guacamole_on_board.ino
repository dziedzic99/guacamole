#include <NMEAGPS.h>
//#include <GPSport.h>
#include <Streamers.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Servo.h>

#include "Adafruit_BME280.h"


#define gpsPort Serial
#define INCOMING_SIZE 4
#define BEEP_PIN 15
#define SERVO_PIN 6
#define LIGHT_PIN 8


//###################################################
//GPS stuff
const unsigned char ubxRate10Hz[] PROGMEM =
{ 0x06, 0x08, 0x06, 0x00, 200, 0x00, 0x01, 0x00, 0x01, 0x00 };
static NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values
static void GPSisr( uint8_t c )
{
  gps.handle( c );
} // GPSisr
void sendUBX( const unsigned char *progmemBytes, size_t len )
{
  gpsPort.write( 0xB5 ); // SYNC1
  gpsPort.write( 0x62 ); // SYNC2

  uint8_t a = 0, b = 0;
  while (len-- > 0) {
    uint8_t c = pgm_read_byte( progmemBytes++ );
    a += c;
    b += a;
    gpsPort.write( c );
  }

  gpsPort.write( a ); // CHECKSUM A
  gpsPort.write( b ); // CHECKSUM B

}
//#####################################################

Adafruit_BME280 bme;
Servo servo;
unsigned long int lastTx = 1;
unsigned long int lastRx = 0;
bool beepOn = 0;
bool lightOn = 0;
bool releaseOn = 0;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Serial on");
  pinMode(BEEP_PIN, OUTPUT);
  pinMode(SERVO_PIN,OUTPUT);
  servo.attach(SERVO_PIN);
  pinMode(LIGHT_PIN,OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  bme.begin();
  #define SEALEVELPRESSURE_HPA ((float)bme.readPressure()/(float)100)
  bme.begin();
  Serial.println("BME on");
  LoRa.begin(433000000);
  Serial.println("LoRa on");
  //  LoRa.setSpreadingFactor(12);
  //  LoRa.setTxPower(17);
  //  LoRa.setSignalBandwidth(500E3);
  //  LoRa.setCodingRate4(8);
  Serial.println("LoRa config done");
  //gpsPort.attachInterrupt( GPSisr );
  gpsPort.begin(9600);
  Serial.println("GPS on");
  delay(500);
  //  sendUBX( ubxRate10Hz, sizeof(ubxRate10Hz) );
  //  Serial.println("GPS config on");
}


void send(String outgoing) {
  Serial.println("Sending \"" + outgoing + "\"");
  LoRa.beginPacket();
  LoRa.print(outgoing);
  LoRa.endPacket();
  Serial.println("sent \"" + outgoing + "\"");
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;   
  //check for reply
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  if (incoming != "") {
    Serial.print("received \""+incoming+"\"");
    String incomingParsed [INCOMING_SIZE];
    for (int i=0; i<INCOMING_SIZE-1; i++) {
      int p1 = incoming.indexOf(',');
      incomingParsed[i] = incoming.substring(0, p1);
      incoming = incoming.substring(p1+1, incoming.length()-1);
    }
    incomingParsed[INCOMING_SIZE-1] = incoming;
    if (incomingParsed[0].toInt() == lastTx) {
    lastRx=lastTx;
    if (incomingParsed[1] == "000") {
      beepOn = 0;
    }
    if (incomingParsed[1] == "111") {
      beepOn = 1;
    }
    if (incomingParsed[2] == "000") {
      lightOn = 0;
    }
    if (incomingParsed[2] == "111") {
      lightOn = 1;
    }
    if (incomingParsed[3] == "000") {
      releaseOn = 0;
    }
    if (incomingParsed[3] == "111") {
      releaseOn = 1;
    }
   
  }
 }
}


void loop() {
  while (gps.available( gpsPort )) {
    lastTx ++;
    if (lastTx>=100000) lastTx=0;
    if (lastTx-lastRx>10 && lastRx!=0){
      //in case of signal loss
      beepOn = 1;
      lightOn = 1;
    }
    if (beepOn) {
      digitalWrite(BEEP_PIN, HIGH);
    }
    if (releaseOn) {
      servo.write(90);
    }
    else {
      servo.write(0);
    }
    if (lightOn) {
      digitalWrite(LIGHT_PIN, HIGH);
    }
    fix = gps.read();
    //transmission packing
    String outgoing = "";
    for (int i=0; i<5-(String(lastTx)).length(); i++){
      outgoing += "0";
    }
    outgoing += String(lastTx);
    outgoing += ",";
    for (int i=0; i<5-(String(lastRx)).length(); i++){
      outgoing += "0";
    }
    outgoing += String(lastRx);
    outgoing += ",";
    outgoing += String(fix.latitude(), 6);
    outgoing += ",";
    outgoing += String(fix.longitude(), 6);
    outgoing += ",";
    outgoing += String(fix.satellites);
    outgoing += ",";
    outgoing += String(bme.readTemperature(), 2);
    outgoing += ",";
    outgoing += String(bme.readHumidity(), 2);
    outgoing += ",";
    outgoing += String(bme.readPressure(), 2);
    outgoing += ",";
    outgoing += String(beepOn);
    outgoing += ",";
    outgoing += String(lightOn);
    outgoing += ",";
    outgoing += String(releaseOn);
    int sum = 
    //send transmission
    send(outgoing);
    LoRa.receive();
    //kil the beeper and the LED
    digitalWrite(BEEP_PIN, LOW);
    digitalWrite(LIGHT_PIN, LOW);
  }
  onReceive(LoRa.parsePacket());
}
