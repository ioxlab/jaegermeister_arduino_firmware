#include <Arduino.h>

// Project defines
#define d_ID                    ("BUBBLE\n")
#define d_OK                    ("OK\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

// Parser defines
#define d_MAX_STRING_SIZE       (64)

// Communication defines
#define d_BAUD_RATE             (115200)

//Ultraschallsensor
#define PIN_ECHO    6
#define PIN_TRIGGER 7
unsigned long duration;
#define MIN_FILL_DISTANCE 6

// Gleichstrommotor
#define PIN_PWM     9
#define PIN_IN1     10
#define PIN_IN2     11
#define MOTOR_SPEED 255 // in range 0 - 255

// Seifenblasenmaschine
#define PIN_BUBBLE_RELAIS 2


void setup() {
    // Set pinmodes
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_PWM, OUTPUT);
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_BUBBLE_RELAIS, OUTPUT);

    // Set output LOW
    digitalWrite(PIN_TRIGGER, LOW);
    digitalWrite(PIN_PWM, LOW);
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_BUBBLE_RELAIS, HIGH);

    Serial.begin(d_BAUD_RATE);
}

void fill_tank() {
    while (true) {
        // Measure fill level
        while (true) {
            delay(50);
            digitalWrite(PIN_TRIGGER, LOW);
            delayMicroseconds(2);
            digitalWrite(PIN_TRIGGER, HIGH);
            delayMicroseconds(10);
            duration = pulseIn(PIN_ECHO, HIGH);
            if (duration != 0)
                break;
        }
        Serial.print("Duration = ");
        Serial.print(duration);
        Serial.print("\n");
        // If the fill level is below the threshold
        if (duration > (MIN_FILL_DISTANCE * 58)) {
            // Turn pump on
            analogWrite(PIN_PWM, MOTOR_SPEED);
            digitalWrite(PIN_IN1, HIGH);
            digitalWrite(PIN_IN2, LOW);
            delay(100);
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

    delay(600000);

    // Pump off
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
}

// here to process incoming serial data after a terminator received
void process_data (const char * data)
{
    if (strncmp("?", data, 1) == 0) {
        Serial.print(d_ID);
    } else if (strncmp("TRIGGER", data, 7) == 0) {
        // Fill tank or top it off
        fill_tank();
        // Make bubbles
        digitalWrite(PIN_BUBBLE_RELAIS, LOW);
        delay(10000);
        digitalWrite(PIN_BUBBLE_RELAIS, HIGH);
        Serial.print(d_OK);
    } else if (strncmp("OFF", data, 3) == 0) {
        empty_tank();
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
            if (input_pos < (d_MAX_STRING_SIZE - 1))
                input_line [input_pos++] = inByte;
            break;

    }  // end of switch

} // end of processIncomingByte

void loop() {
    while (Serial.available() > 0)
        processIncomingByte(Serial.read());
}