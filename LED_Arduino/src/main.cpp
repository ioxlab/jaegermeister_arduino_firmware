#include <Arduino.h>

// Project defines
#define d_ID                    ("LED\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

#define d_INVALID_LED_PATTERN   (-1)
#define d_INVALID_LED_STRIP     (-1)

enum Command {
    ID,
    SET,
    PLAY,
    UNRECOGNIZED
};

Command get_command();

void cmd_id();
void cmd_set();
void cmd_play();
void cmd_unrecognized();

bool parse_set();

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_COMMAND_DELIMITER     (":")
#define d_ARGUMENT_DELIMITER    (";")
#define d_INVALID_KEY           (0)
#define d_KEY_STRIP_ID          ("S")
#define d_KEY_PATTERN_ID        ("P")
#define d_INVALID_VALUE         (-1)

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0};
int mc_ReadBytes = 0;

enum Pattern {
    JAEGERMEISTER_ORANGE = 1,
    JAEGERMEISTER_GRUEN = 2,
    BLAU = 3,
    WEISS = 4,
    ROT = 5,
    REGENBOGEN_UMLAUFEND = 6,
    REGENBOGEN_FLACKERND = 7,
    STROBOSKOP = 8,
    AUS = 9,
};

Pattern set_patterns[] = {
        Pattern::JAEGERMEISTER_GRUEN,
        Pattern::JAEGERMEISTER_ORANGE,
        Pattern::JAEGERMEISTER_ORANGE,
        Pattern::AUS,
        Pattern::JAEGERMEISTER_ORANGE
};

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
            mca_StringBuffer[mc_ReadBytes] = (char) incomingByte;
            mc_ReadBytes += 1;
        }
    }

    // Handle command
    switch (get_command()) {
        case ID:
            cmd_id();
        case SET:
            cmd_set();
        case PLAY:
            cmd_play();
        case UNRECOGNIZED:
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
    } else if (strncmp("PLAY", mca_StringBuffer, 3) == 0) {
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
    for (int strip_id = 0; strip_id < 5; strip_id++) {
        // TODO: LED pattern auf LED anzeigen
        switch (set_patterns[strip_id]) {
            case JAEGERMEISTER_ORANGE:
                break;
            case JAEGERMEISTER_GRUEN:
                break;
            case BLAU:
                break;
            case WEISS:
                break;
            case ROT:
                break;
            case REGENBOGEN_UMLAUFEND:
                break;
            case REGENBOGEN_FLACKERND:
                break;
            case STROBOSKOP:
                break;
            case AUS:
                break;
        }
    }
    Serial.print(d_OK);
}

void cmd_unrecognized() {
    Serial.print(d_UNRECOGNIZED_COMMAND);
}

/// Parses a SET command and saves the switch states in switches.
/// \param switches - Should be an int[5], initialized with -1;
/// \return True, when no parsing errors.
bool parse_set() {
    int strip_id;
    Pattern pattern;

    char key[1] = { d_INVALID_KEY };
    int value;
    strtok(mca_StringBuffer, d_COMMAND_DELIMITER); // Skip command part
    char* argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    while (argument != nullptr) {
        key[0] = d_INVALID_KEY;
        value = d_INVALID_VALUE;

        sscanf(argument, "%[A-Z]=%d", key, &value); // Parse argument
        if (strncmp(key, d_KEY_STRIP_ID, 1) == 0) {
            if ((1 <= strip_id) && (strip_id <= 5)) {
                strip_id = value;
            } else {
                return false;
            }
        } else if (strncmp(key, d_KEY_PATTERN_ID, 1) == 0) {
            if ((1 <= value) && (value <= 9)) {
                pattern = (Pattern)value;
            } else {
                return false;
            }
        } else {
            return false;
        }

        argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    }

    if (strip_id != d_INVALID_LED_STRIP) {
        set_patterns[strip_id - 1] = pattern;
        return true;
    } else {
        return false;
    }
}
