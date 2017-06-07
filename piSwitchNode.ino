
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
#include <printf.h>

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

RF24 radio(9,10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setup() {
  Serial.begin(115200);
  printf_begin();
  
  // If this is a new node, the nodeID will return 0. Once the node is configured with an ID other than 0, this
  // bit will no longer run.
  while (!mesh.getNodeID()) {
    // Wait for the nodeID to be set via Serial
    if (Serial.available()) {
      mesh.setNodeID(Serial.read());
      Serial.print("Set NodeID: ");
      Serial.println(mesh.getNodeID());
    }
  }

  mesh.begin();
}

void loop() {
  mesh.update();
  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}

