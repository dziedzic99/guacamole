#include <SPI.h>
#include <LoRa.h>
#define INCOMING_SIZE 7

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
//  Serial.println("LoRa on!");
//  LoRa.setSpreadingFactor(12);
//  LoRa.setTxPower(17);
//  LoRa.setSignalBandwidth(500E3);
//  LoRa.setCodingRate4(8);
//  Serial.println("LoRa config on!");
}

void send(String string) {
  Serial.println("Sending \"" + string + "\"");
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
        Serial.print(packet[i]);
        incoming += packet[i];
    }
      Serial.print(",");
      Serial.println(LoRa.packetRssi());
//      Serial.println(incoming);
      String incomingParsed [INCOMING_SIZE];
      for (int i=0; i<INCOMING_SIZE-1; i++) {
        int p1 = incoming.indexOf(',');
        incomingParsed[i] = incoming.substring(0, p1);
        incoming = incoming.substring(p1+1, incoming.length()-1);
      }
      incomingParsed[INCOMING_SIZE-1] = incoming;
      String outgoing = "";
      outgoing += incomingParsed[0];
      outgoing += ",";
      outgoing += "000";
      outgoing += ",";
      outgoing += "000";
      outgoing += ",";
      outgoing += "000";
      //delay(10);
      send(outgoing);
    }
}

