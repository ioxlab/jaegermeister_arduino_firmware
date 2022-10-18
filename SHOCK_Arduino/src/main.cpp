#include <Arduino.h>

// Project defines
#define d_ID                    ("SHOCK\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")
#define d_COMMAND_DELIMITER     (":")
#define d_ARGUMENT_DELIMITER    (";")

enum ShockPad {
    DONT_SHOCK = 0,
    SHOCK = 1,
};

enum Level {
    InvalidLevel = 0,
    Level1 = 1,
    Level2 = 2,
    Level3 = 3,
    Level4 = 4,
};

ShockPad shock_pads[4] = {DONT_SHOCK, DONT_SHOCK, DONT_SHOCK, DONT_SHOCK};
Level shock_level = Level1;
bool parse_set(char *string);

// Parser defines
#define d_MAX_STRING_SIZE       (64)

// Communication defines
#define d_BAUD_RATE             (115200)

void setup ()
{
    Serial.begin (d_BAUD_RATE);
}

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
    } else if (strncmp("PLAY", data, 4) == 0) {
        char p1, p2, p3, p4;
        // TODO: Pads in shock_pads mit level shock_level schocken

        char response[21];
        sprintf(response, "P1=%d;P2=%d;P3=%d;P4=%d\n", p1, p2, p3, p4);
        Serial.print(response);
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

bool parse_set(char *data) {
    Level _shock_level = InvalidLevel;
    ShockPad _shock_pads[4] = { DONT_SHOCK, DONT_SHOCK, DONT_SHOCK, DONT_SHOCK };

    strtok(data, d_COMMAND_DELIMITER); // Skip command part
    char* argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    while (argument != nullptr) {
        _shock_level = InvalidLevel;

        if (strncmp(argument, "L", 1) == 0) {
            int num_level = 0;
            sscanf(argument, "L=%d", &num_level);
            if (num_level == 0) return false;
            _shock_level = (Level)num_level;
        } else if (strncmp(argument, "P1", 2) == 0) {
            _shock_pads[0] = SHOCK;
        } else if (strncmp(argument, "P2", 2) == 0) {
            _shock_pads[1] = SHOCK;
        } else if (strncmp(argument, "P3", 2) == 0) {
            _shock_pads[2] = SHOCK;
        } else if (strncmp(argument, "P4", 2) == 0) {
            _shock_pads[3] = SHOCK;
        } else {
            return false;
        }

        argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    }

    if (_shock_level == InvalidLevel) return false;

    shock_level = _shock_level;
    for (int i = 0; i < 4; ++i) {
        shock_pads[i] = _shock_pads[i];
    }

    return true;
}