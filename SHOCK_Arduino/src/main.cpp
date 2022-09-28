#include <Arduino.h>

// Project defines
#define d_ID                    ("SHOCK\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

enum Command {
    ID,
    SET,
    PLAY,
    UNRECOGNIZED
};

Command get_command();

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
bool parse_set();

void cmd_id();
void cmd_set();
void cmd_play();
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
        case Command::SET:
            cmd_set();
        case Command::PLAY:
            cmd_play();
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
    } else if (strncmp("SET", mca_StringBuffer, 3) == 0) {
        return Command::SET;
    } else if (strncmp("PLAY", mca_StringBuffer, 4) == 0) {
        return Command::PLAY;
    } else {
        return Command::UNRECOGNIZED;
    }
}

void cmd_id() {
    Serial.print(d_ID);
}

void cmd_set() {
    if (parse_set()) {
        Serial.print(d_OK);
    } else {
        Serial.print(d_BAD_SYNTAX);
    }

}

void cmd_play() {
    char p1, p2, p3, p4;
    // TODO: Pads in shock_pads mit level shock_level schocken


    char response[21];
    sprintf(response, "P1=%d;P2=%d;P3=%d;P4=%d\n", p1, p2, p3, p4);
    Serial.print(response);
    Serial.print(d_OK);
}

void cmd_unrecognized() {
    Serial.print(d_UNRECOGNIZED_COMMAND);
}

/// Parses a SET command and saves the switch states in switches.
/// \param switches - Should be an int[5], initialized with -1;
/// \return True, when no parsing errors.
bool parse_set() {
    Level _shock_level = InvalidLevel;
    ShockPad _shock_pads[4] = { DONT_SHOCK, DONT_SHOCK, DONT_SHOCK, DONT_SHOCK };

    strtok(mca_StringBuffer, ":"); // Skip command part
    char* argument = strtok(nullptr, ";"); // Get next argument
    while (argument != nullptr) {
        _shock_level = InvalidLevel;

        if (strncmp(argument, "L", 1) == 0) {
            int num_level = 0;
            sscanf(argument, "L=%d", &num_level);
            if (num_level == 0) return false;
            _shock_level = (Level)num_level;
        } else if (strncmp(argument, "P1", 2) == 0) {
            _shock_pads[0] = SHOCK;
        } else if (strncmp(argument, "P1", 2) == 0) {
            _shock_pads[1] = SHOCK;
        } else if (strncmp(argument, "P1", 2) == 0) {
            _shock_pads[2] = SHOCK;
        } else if (strncmp(argument, "P1", 2) == 0) {
            _shock_pads[3] = SHOCK;
        } else {
            return false;
        }

        argument = strtok(nullptr, ";"); // Get next argument
    }

    if (_shock_level == InvalidLevel) return false;

    shock_level = _shock_level;
    for (int i = 0; i < 4; ++i) {
        shock_pads[i] = _shock_pads[i];
    }

    return true;
}