
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
#include <printf.h>

// Time between ping to controller to update the switch status
#define PING_STATUS_TIME 60000

struct payload_t {
    unsigned long ms;
    unsigned long counter;
};

RF24 radio(9,10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

unsigned long statusTimer = 0;
bool status = false;

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

    // Send to the master node every second
    if (millis() - statusTimer >= PING_STATUS_TIME) {
        statusTimer = millis();

        // Send an 'M' type message containing the current millis()
        if (!mesh.write(&status, 65, sizeof(status))) {

            // If a write fails, check connectivity to the mesh network
            if (!mesh.checkConnection()) {
                //refresh the network address
                Serial.println("Renewing Address");
                mesh.renewAddress();
            } else {
                Serial.println("Send fail, Test OK");
            }
        } else {
            Serial.print("Send OK: "); Serial.println(statusTimer);
        }
    }
}

