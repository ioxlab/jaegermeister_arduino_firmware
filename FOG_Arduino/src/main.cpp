#include <Arduino.h>

// Project defines
#define d_ID                    ("FOG\n")
#define d_OK                    ("OK\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

// Fog machine
#define PIN_FOG 13

// Parser defines
#define d_MAX_STRING_SIZE       (64)

// Communication defines
#define d_BAUD_RATE             (115200)

void setup ()
{
    // Set pinmode
    pinMode(PIN_FOG, OUTPUT);
    // Set output LOW
    digitalWrite(PIN_FOG, HIGH);
    Serial.begin (d_BAUD_RATE);
}

// here to process incoming serial data after a terminator received
void process_data (char * data)
{
    if (strncmp("?", data, 1) == 0) {
        Serial.print(d_ID);
    } else if (strncmp("TRIGGER", data, 7) == 0) {
        digitalWrite(PIN_FOG, LOW);
        delay(500);
        digitalWrite(PIN_FOG, HIGH);
        Serial.print(d_OK);
    } else {
        Serial.print(d_UNRECOGNIZED_COMMAND);
    }
}  // end of process_data

void processIncomingByte (const byte inByte)
{
    static char input_line [d_MAX_STRING_SIZE];
    static unsigned int input_pos = 0;

    switch (inByte)
    {
        case '\n':   // end of text
            input_line [input_pos] = 0;  // terminating null byte
            process_data (input_line);
            input_pos = 0;
            break;
        case '\r':   // discard carriage return
            break;
        default:
            // keep adding if not full ... allow for terminating null byte
            if (input_pos < (d_MAX_STRING_SIZE - 1))
                input_line [input_pos++] = inByte;
            break;
    }  // end of switch
} // end of processIncomingByte

void loop ()
{
    while (Serial.available () > 0)
        processIncomingByte (Serial.read ());
}