#include <Arduino.h>

// Project defines
#define d_ID                    ("FOG\n")
#define d_OK                    ("OK\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

enum Command {
    ID,
    TRIGGER,
    OPEN,
    CLOSE,
    UNRECOGNIZED
};

Command get_command();

void cmd_id();
void cmd_trigger();
void cmd_open();
void cmd_close();
void cmd_unrecognized();

// Parser defines
#define d_MAX_STRING_SIZE       (64)

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0}; // Read bytes
int mc_ReadBytes = 0; // Amount of read bytes
int incomingByte = 0; // For incoming serial data

void setup ()
{
    Serial.begin (d_BAUD_RATE);
}

void loop ()
{
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
            mca_StringBuffer[mc_ReadBytes] = (char) incomingByte;
            mc_ReadBytes += 1;
        }
    }

    // Handle command
    switch (get_command()) {
        case Command::ID:
            cmd_id();
        case Command::TRIGGER:
            cmd_trigger();
        case Command::OPEN:
            cmd_open();
        case Command::CLOSE:
            cmd_close();
        case Command::UNRECOGNIZED:
            cmd_unrecognized();
    }

    // Cleanup
    mc_ReadBytes = 0;
    memset(&mca_StringBuffer, 0, d_MAX_STRING_SIZE);
}

Command get_command() {
    if (strncmp("?", mca_StringBuffer, 1) == 0) {
        return Command::ID;
    } else if (strncmp("TRIGGER", mca_StringBuffer, 7) == 0) {
        return Command::TRIGGER;
    } else if (strncmp("OPEN", mca_StringBuffer, 4) == 0) {
        return Command::OPEN;
    } else if (strncmp("CLOSE", mca_StringBuffer, 5) == 0) {
        return Command::CLOSE;
    } else {
        return Command::UNRECOGNIZED;
    }
}

void cmd_id() {
    Serial.print(d_ID);
}

void cmd_trigger() {
    // TODO: Nebel verteilen

    Serial.print(d_OK);
}
void cmd_open() {
    // TODO: Klappe oeffnen

    Serial.print(d_OK);
}
void cmd_close() {
    // TODO: Klappe schliessen

    Serial.print(d_OK);
}

void cmd_unrecognized() {
    Serial.print(d_UNRECOGNIZED_COMMAND);
}