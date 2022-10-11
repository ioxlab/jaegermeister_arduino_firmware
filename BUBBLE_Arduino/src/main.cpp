#include <Arduino.h>

// Project defines
#define d_ID                    ("BUBBLE\n")
#define d_OK                    ("OK\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

enum Command {
    ID,
    TRIGGER,
    OFF,
    UNRECOGNIZED
};

Command get_command();

void cmd_id();

void cmd_trigger();

void cmd_off();

void cmd_unrecognized();

// Parser defines
#define d_MAX_STRING_SIZE       (64)

// Communication defines
#define d_BAUD_RATE             (115200)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0}; // Read bytes
int mc_ReadBytes = 0; // Amount of read bytes
int incomingByte = 0; // For incoming serial data

//Ultraschallsensor
#define PIN_ECHO    6
#define PIN_TRIGGER 7
unsigned long duration;
#define MIN_FILL_DISTANCE 2

// Gleichstrommotor
#define PIN_PWM     9
#define PIN_IN1     10
#define PIN_IN2     11
#define MOTOR_SPEED 100 // in range 0 - 255

// Seifenblasenmaschine
#define PIN_BUBBLE_RELAIS 2


void setup() {
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_PWM, OUTPUT);
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_BUBBLE_RELAIS, OUTPUT);

    Serial.begin(d_BAUD_RATE);
}

void fill_tank() {
    while (true) {
        // Measure fill level
        digitalWrite(PIN_TRIGGER, LOW);
        delayMicroseconds(2);
        digitalWrite(PIN_TRIGGER, HIGH);
        delayMicroseconds(10);
        duration = pulseIn(PIN_ECHO, HIGH);

        // If the fill level is below the threshold
        if (duration > (MIN_FILL_DISTANCE * 58)) {
            // Turn pump on
            analogWrite(PIN_PWM, MOTOR_SPEED);
            digitalWrite(PIN_IN1, HIGH);
            digitalWrite(PIN_IN2, LOW);
        } else {
            // Turn pump off and return
            digitalWrite(PIN_IN1, LOW);
            digitalWrite(PIN_IN2, LOW);
            return;
        }
    }
}

void empty_tank() {
    // Empty the tank
    analogWrite(PIN_PWM, MOTOR_SPEED);
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);

    delay(6000); // Warten bis ausgepump

    // Pump off
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
}

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
        case Command::ID:
            cmd_id();
        case Command::TRIGGER:
            cmd_trigger();
        case Command::OFF:
            cmd_off();
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
    } else if (strncmp("OFF", mca_StringBuffer, 3) == 0) {
        return Command::OFF;
    } else {
        return Command::UNRECOGNIZED;
    }
}

void cmd_id() {
    Serial.print(d_ID);
}

void cmd_trigger() {
    // Fill tank or top it off
    fill_tank();

    // Make bubbles
    digitalWrite(PIN_BUBBLE_RELAIS, HIGH);
    delay(10000);
    digitalWrite(PIN_BUBBLE_RELAIS, LOW);

    Serial.print(d_OK);
}

void cmd_off() {
    empty_tank();
    Serial.print(d_OK);
}

void cmd_unrecognized() {
    Serial.print(d_UNRECOGNIZED_COMMAND);
}