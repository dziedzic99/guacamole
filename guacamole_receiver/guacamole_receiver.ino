#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "font.h"
#define INCOMING_SIZE 14


SSD1306  display(0x3c, 4, 15);
String incomingParsed [INCOMING_SIZE];

void setup() {
  Serial.begin(9600);
  while (!Serial);
//  Serial.println("Serial on!");
//  Serial.println("SPI on!");
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433000000)) {
//    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
  display.flipScreenVertically();
  display.clear();

//  Serial.println("LoRa on!");
//  LoRa.setSpreadingFactor(12);
//  LoRa.setTxPower(17);
//  LoRa.setSignalBandwidth(500E3);
//  LoRa.setCodingRate4(8);
//  Serial.println("LoRa config on!");
}

void dspl(){
  display.clear();
  display.setFont(Monospaced_plain_9); 
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Num:   "+incomingParsed[0]);
  display.drawString(0, 9, "RSSI:  "+incomingParsed[INCOMING_SIZE-1]);

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, "GPS sats: "+incomingParsed[4]);
  display.drawString(128, 9, incomingParsed[2]);
  display.drawString(128, 18, incomingParsed[3]);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (incomingParsed[8 ]=="0") display.drawString(0, 18, "Beep:  Off");
  if (incomingParsed[8 ]=="1") display.drawString(0, 18, "Beep:  On");

  if (incomingParsed[9 ]=="0") display.drawString(0, 27, "Light: Off");
  if (incomingParsed[9 ]=="1") display.drawString(0, 27, "Light: On");

  if (incomingParsed[10]=="0") display.drawString(0, 36, "Clamp: Off");
  if (incomingParsed[10]=="1") display.drawString(0, 36, "Clamp: On");

  display.drawString(0, 45, "Temp:  "+incomingParsed[5].substring(0,incomingParsed[5].length()-1)+"C");
  display.drawString(0, 54, "Hum:   "+incomingParsed[6].substring(0,incomingParsed[6].length()-1)+"%");

  
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 36, "Altitude:");
  display.display();
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(Monospaced_plain_18);
  display.drawString(128, 45, "1250m");
  display.display();
}

void send(String string) {
  //Serial.println("Sending \"" + string + "\"");
  LoRa.beginPacket();
  LoRa.print(string);
  LoRa.endPacket();
//  Serial.println("sent \"" + string + "\"");
}

void loop() {
  // try to parse packet
  //Serial.println("loop");
  int packetSize = LoRa.parsePacket();
  char packet[packetSize];
  for (int i=0; i<packetSize; i++) {
      packet[i]=(char)LoRa.read();
  }
  if (packetSize) {
    String incoming = "";
    for (int i=0; i<packetSize; i++) {
//      Serial.print(packet[i]);
        incoming += packet[i];
    }
//    Serial.print(",");
//    Serial.println(LoRa.packetRssi());
    Serial.println(incoming);
      
      for (int i=0; i<INCOMING_SIZE-2; i++) {
        int p1 = incoming.indexOf(',');
        incomingParsed[i] = incoming.substring(0, p1);
        incoming = incoming.substring(p1+1, incoming.length());
      }
      Serial.println();
      incomingParsed[INCOMING_SIZE-2] = incoming;
      incomingParsed[INCOMING_SIZE-1] = LoRa.packetRssi();
      String outgoing = "";
      outgoing += incomingParsed[0];
      outgoing += ",";
      outgoing += "000";
      outgoing += ",";
      outgoing += "000";
      outgoing += ",";
      outgoing += "000";
      dspl();
      delay(50);
      send(outgoing);
    }
}

