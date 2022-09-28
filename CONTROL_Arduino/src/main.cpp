#include <Arduino.h>

// Project defines
#define d_ID                    ("CONTROL\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

#define d_INVALID_SWITCH_STATE  (-1)

enum Command {
    ID,
    SET,
    GET,
    APPLY,
    SHOT,
    UNRECOGNIZED
};

void cmd_id();
void cmd_set();
void cmd_get();
void cmd_apply();
void cmd_shot();
void cmd_unrecognized();

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_DELIMITER             (";")

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0};
char mc_ReadBytes = 0;

void setup() {
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
        case GET:
            cmd_get();
        case APPLY:
            cmd_apply();
        case SHOT:
            cmd_shot();
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
    } else if (strncmp("GET", mca_StringBuffer, 3) == 0) {
        return Command::GET;
    } else if (strncmp("APPLY", mca_StringBuffer, 5) == 0) {
        return Command::APPLY;
    } else if (strncmp("SHOT", mca_StringBuffer, 4) == 0) {
        return Command::SHOT;
    } else {
        return Command::UNRECOGNIZED;
    }
}

void cmd_id() {
    Serial.print(d_ID);
}

void cmd_set() {
    int switches[5];
    memset(switches, d_INVALID_SWITCH_STATE, 5);
    if (parse_set(switches)) {
        for (int i = 0; i < 5; i++) {
            if (switches[i] != d_INVALID_SWITCH_STATE) {
                // TODO: switches[i] in LED status schreiben

            }
        }
        Serial.print(d_OK);
    } else {
        Serial.print(d_BAD_SYNTAX);
    }
}

void cmd_get() {
    int l1, l2, l3, l4, l5, k, j;
    // TODO: LED, Schlüsselschalter und Ausgabetaste Status abfragen

    char response[33];
    sprintf(response, "L1=%d;L2=%d;L3=%d;L4=%d;L5=%d;K=%d;J=%d\n", l1, l2, l3, l4, l5, k, j);
    Serial.print(response);
}

void cmd_apply() {
    // TODO: LED Status auf Relais umsetzen

    Serial.print("OK\n");
}

void cmd_shot() {
    // TODO: Shot ausgeben

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
    char* cmd = strtok(mca_StringBuffer, ":"); // Skip command part
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