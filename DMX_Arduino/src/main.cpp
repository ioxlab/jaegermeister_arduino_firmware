#include <Arduino.h>

// Project defines
#define d_ID                    ("DMX\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

void cmd_id();

void cmd_trigger();

void cmd_unrecognized();

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_DELIMITER             (";")

// Communication defines
#define d_BAUD_RATE             (115200)

void setup() {
    Serial.begin(d_BAUD_RATE);
}

void process_data(char *data) {
    if (strncmp("?", data, 1) == 0) {
        Serial.print(d_ID);
    } else if (strncmp("TRIGGER", data, 7) == 0) {
        // TODO: Trigger firework here
        Serial.print(d_OK);
    } else {
        Serial.print(d_UNRECOGNIZED_COMMAND);
    }
}  // end of process_data

void processIncomingByte(const byte inByte) {
    static char input_line[d_MAX_STRING_SIZE];
    static unsigned int input_pos = 0;

    switch (inByte) {
        case '\n':   // end of text
            input_line[input_pos] = 0;  // terminating null byte
            process_data(input_line);
            input_pos = 0;
            break;
        case '\r':   // discard carriage return
            break;
        default:
            if (input_pos < (d_MAX_STRING_SIZE - 1))
                input_line[input_pos++] = inByte;
            break;
    }  // end of switch
} // end of processIncomingByte

void loop() {
    while (Serial.available() > 0)
        processIncomingByte(Serial.read());
}