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
#define PIN_RELAIS_SPARK 35
#define PIN_RELAIS_BUBBLE 33
#define PIN_RELAIS_ENABLE_SHOT 31
#define PIN_RELAIS_DOOR 29
#define PIN_RELAIS_TRIGGER_SHOT 27
#define PIN_RELAIS_NC 25

enum Command {
    ID,
    SET,
    SHOT,
    FREE,
    UNRECOGNIZED
};

void cmd_id();
void cmd_set();
void cmd_shot();
void cmd_free();
void cmd_unrecognized();

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_DELIMITER             (";")

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0};
int mc_ReadBytes = 0;

void setup() {
    pinMode(PIN_RELAIS_FOG, OUTPUT);
    pinMode(PIN_RELAIS_SPARK, OUTPUT);
    pinMode(PIN_RELAIS_BUBBLE, OUTPUT);
    pinMode(PIN_RELAIS_ENABLE_SHOT, OUTPUT);
    pinMode(PIN_RELAIS_DOOR, OUTPUT);
    pinMode(PIN_RELAIS_TRIGGER_SHOT, OUTPUT);
    pinMode(PIN_RELAIS_NC, OUTPUT);

    Serial.begin(d_BAUD_RATE);
}

int incomingByte = 0; // For incoming serial data

bool parse_set(int switches[]);
Command get_command();


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
        case SHOT:
            cmd_shot();
        case FREE:
            cmd_free();
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
    } else if (strncmp("SHOT", mca_StringBuffer, 4) == 0) {
        return Command::SHOT;
    } else if (strncmp("FREE", mca_StringBuffer, 4) == 0) {
        return Command::FREE;
    } else {
        return Command::UNRECOGNIZED;
    }
}

void cmd_id() {
    Serial.print(d_ID);
}

void cmd_set() {
    int switches[5] = {
            d_INVALID_SWITCH_STATE,
            d_INVALID_SWITCH_STATE,
            d_INVALID_SWITCH_STATE,
            d_INVALID_SWITCH_STATE,
            d_INVALID_SWITCH_STATE,
    };
    if (parse_set(switches)) {
        for (int i = 0; i < 5; i++) {
            if (switches[i] != d_INVALID_SWITCH_STATE) {
                switch (i) {
                    case 0:
                        digitalWrite(PIN_RELAIS_FOG, switches[i]);
                        break;
                    case 1:
                        digitalWrite(PIN_RELAIS_SPARK, switches[i]);
                        break;
                    case 2:
                        digitalWrite(PIN_RELAIS_BUBBLE, switches[i]);
                        break;
                    case 3:
                        digitalWrite(PIN_RELAIS_ENABLE_SHOT, switches[i]);
                        break;
                    case 4:
                        digitalWrite(PIN_RELAIS_DOOR, switches[i]);
                        break;
                }
            }
        }
        Serial.print(d_OK);
    } else {
        Serial.print(d_BAD_SYNTAX);
    }
}

void cmd_free() {
    bool is_free = true;
    // TODO: Abfragen ob die Ausgabe frei is (ob der shot entnommen wurde) und in is_free speichern

    if (is_free) {
        Serial.print(d_IS_FREE);
    } else {
        Serial.print(d_IS_NOT_FREE);
    }
}

void cmd_shot() {
    digitalWrite(PIN_RELAIS_TRIGGER_SHOT, HIGH);
    delay(500);
    digitalWrite(PIN_RELAIS_TRIGGER_SHOT, LOW);
    Serial.print(d_OK);
}

void cmd_unrecognized() {
    Serial.print(d_UNRECOGNIZED_COMMAND);
}

/// Parses a SET command and saves the switch states in switches.
/// \param switches - Should be an int[5], initialized with -1;
/// \return True, when no parsing errors.
bool parse_set(int switches[]) {
    int switch_id;
    int switch_state;
    strtok(mca_StringBuffer, ":"); // Skip command part
    char* argument = strtok(nullptr, ";"); // Get next argument
    while (argument != nullptr) {
        switch_id = -1;
        switch_state = -1;

        sscanf(argument, "L%d=%d", &switch_id, &switch_state); // Parse argument
        if ((1 <= switch_id) && (switch_id <= 5)) {
            if ((switch_state == 0) || (switch_state == 1)) {
                switches[switch_id-1] = switch_state;
            } else {
                return false;
            }
        } else {
            return false;
        }

        argument = strtok(nullptr, ";"); // Get next argument
    }

    return true;
}