#include <Arduino.h>

// Project defines
#define d_ID                    ("CONTROL\n")

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_DELIMITER             (";")

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0};
char mc_ReadedBytes = 0;

void setup() {
    Serial.begin(d_BAUD_RATE);
}

int incomingByte = 0; // For incoming serial data

void loop() {

    // Read until \n
    while (true) {
        // Wait until bytes available
        while (Serial.available() == 0);
        // Read byte
        incomingByte = Serial.read();

        // If it is a \n stop reading
        if (incomingByte == '\n') {
            break;
        } else {
            // Otherwise, add char to buffer
            mca_StringBuffer[mc_ReadedBytes] = (char) incomingByte;
            mc_ReadedBytes += 1;
        }
    }

    if (!strcmp(mca_StringBuffer, "?")) {
        Serial.print(d_ID);
    } else {
        Serial.print("UNRECOGNIZED COMMAND\n");
    }
    mc_ReadedBytes = 0;
    memset(&mca_StringBuffer, 0, d_MAX_STRING_SIZE);
}