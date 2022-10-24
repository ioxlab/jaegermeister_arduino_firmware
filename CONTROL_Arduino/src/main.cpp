#include <Arduino.h>

// Project defines
#define d_ID                    ("CONTROL\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_IS_FREE               ("Y\n")
#define d_IS_NOT_FREE           ("N\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

#define d_INVALID_SWITCH_STATE  (-1)

// Relais
#define PIN_RELAIS_FOG 37
#define PIN_RELAIS_FIREWORK 35
#define PIN_RELAIS_BUBBLE 33
#define PIN_RELAIS_LED 31
#define PIN_RELAIS_DOOR 29
#define PIN_RELAIS_SHOT 27

#define NUM_RELAIS 6

const int RELAIS_PINS[NUM_RELAIS] = {
        PIN_RELAIS_LED,
        PIN_RELAIS_FIREWORK,
        PIN_RELAIS_FOG,
        PIN_RELAIS_BUBBLE,
        PIN_RELAIS_DOOR,
        PIN_RELAIS_SHOT,
};

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_ARGUMENT_DELIMITER             (";")
#define d_COMMAND_DELIMITER             (":")

// Communication defines
#define d_BAUD_RATE             (115200)

void setup() {
    // Set pinmodes
    pinMode(PIN_RELAIS_LED, OUTPUT);
    pinMode(PIN_RELAIS_FIREWORK, OUTPUT);
    pinMode(PIN_RELAIS_FOG, OUTPUT);
    pinMode(PIN_RELAIS_BUBBLE, OUTPUT);
    pinMode(PIN_RELAIS_DOOR, OUTPUT);
    pinMode(PIN_RELAIS_SHOT, OUTPUT);

    // Set output LOW
    digitalWrite(PIN_RELAIS_LED, HIGH);
    digitalWrite(PIN_RELAIS_FIREWORK, HIGH);
    digitalWrite(PIN_RELAIS_FOG, HIGH);
    digitalWrite(PIN_RELAIS_BUBBLE, HIGH);
    digitalWrite(PIN_RELAIS_DOOR, HIGH);
    digitalWrite(PIN_RELAIS_SHOT, HIGH);

    Serial.begin(d_BAUD_RATE);
}

bool parse_set(char *data);

// here to process incoming serial data after a terminator received
void process_data (char * data)
{
    if (strncmp("?", data, 1) == 0) {
        Serial.print(d_ID);
    } else if (strncmp("SET", data, 3) == 0) {
        if (parse_set(data)) {
            Serial.print(d_OK);
        } else {
            Serial.print(d_BAD_SYNTAX);
        }
    } else if (strncmp("SHOT", data, 4) == 0) {
        digitalWrite(PIN_RELAIS_SHOT, LOW);
        delay(500);
        digitalWrite(PIN_RELAIS_SHOT, HIGH);
        Serial.print(d_OK);
    } else if (strncmp("FREE", data, 4) == 0) {
        Serial.print(d_IS_FREE);
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
            if (input_pos < (d_MAX_STRING_SIZE - 1))
                input_line [input_pos++] = inByte;
            break;
    }  // end of switch
} // end of processIncomingByte

void loop() {
    while (Serial.available() > 0)
        processIncomingByte(Serial.read());
}

bool parse_set(char *data) {
    int switch_id;
    int switch_state;
    strtok(data, d_COMMAND_DELIMITER); // Skip command part
    char *argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    while (argument != nullptr) {
        switch_id = -1;
        switch_state = -1;

        sscanf(argument, "L%d=%d", &switch_id, &switch_state); // Parse argument
        if ((1 <= switch_id) && (switch_id <= NUM_RELAIS)) {
            if ((switch_state == 0) || (switch_state == 1)) {
                digitalWrite(RELAIS_PINS[switch_id - 1], !switch_state);
            } else {
                return false;
            }
        } else {
            return false;
        }

        argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    }

    return true;
}