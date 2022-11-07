#include <Arduino.h>
#include "FastLED.h"

// LED configuration
#define LED_TYPE   WS2812B
#define COLOR_ORDER   GRB
#define VOLTS          5
#define MAX_MA          5000

#define BRIGHTNESS 255
#define NUM_STRIPS 4
#define NUM_LEDS_0 60 // Jaegermeister Logo
#define NUM_LEDS_1 62 // Fenster
#define NUM_LEDS_2 17 // Shot Schriftzug
#define NUM_LEDS_3 32 // Ausgabe

CRGBArray<NUM_LEDS_0> LEDs_0;
CRGBArray<NUM_LEDS_1> LEDs_1;
CRGBArray<NUM_LEDS_2> LEDs_2;
CRGBArray<NUM_LEDS_3> LEDs_3;

#define PIN_STRIP_0 5
#define PIN_STRIP_1 9
#define PIN_STRIP_2 3
#define PIN_STRIP_3 6

// Project defines
#define d_ID                    ("LED\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")

#define d_INVALID_LED_PATTERN   (-1)
#define d_INVALID_LED_STRIP     (-1)

void process_data(char *data);

bool parse_set(char *data);

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_COMMAND_DELIMITER     (":")
#define d_ARGUMENT_DELIMITER    (";")

// Communication defines
#define d_BAUD_RATE             (115200)

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

Pattern led_strip_patterns[NUM_STRIPS] = {
        Pattern::REGENBOGEN_UMLAUFEND,
        Pattern::REGENBOGEN_UMLAUFEND,
        Pattern::REGENBOGEN_UMLAUFEND,
        Pattern::REGENBOGEN_UMLAUFEND,
};

enum State {
    CheckForRX,
    RX,
    Displaying,
};

State state = CheckForRX;

void setup() {
    delay(3000); //safety startup delay
    FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_MA);

    FastLED.addLeds<LED_TYPE, PIN_STRIP_0, COLOR_ORDER>(LEDs_0, NUM_LEDS_0).getAdjustment(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, PIN_STRIP_1, COLOR_ORDER>(LEDs_1, NUM_LEDS_1).getAdjustment(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, PIN_STRIP_2, COLOR_ORDER>(LEDs_2, NUM_LEDS_2).getAdjustment(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, PIN_STRIP_3, COLOR_ORDER>(LEDs_3, NUM_LEDS_3).getAdjustment(TypicalLEDStrip);

    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    Serial.begin(d_BAUD_RATE);
}

void fill_strip_solid(int strip_id, CRGB color);

void fill_strip_rainbow(int strip_id);

int rainbow_offset = 0;
#define RAINBOW_STEP 10

bool processIncomingByte(const byte inByte) {
    static char input_line[d_MAX_STRING_SIZE];
    static unsigned int input_pos = 0;

//    Serial.print((char) inByte);
    switch (inByte) {
        case '\n':   // end of text
            input_line[input_pos] = 0;  // terminating null byte

            // terminator reached! process input_line here ...
//            Serial.print(input_line);
            process_data(input_line);

            // reset buffer for next time
            input_pos = 0;

            return true;

        case '\r':   // discard carriage return
            break;

        default:
            // keep adding if not full ... allow for terminating null byte
            if (input_pos < (d_MAX_STRING_SIZE - 1))
                input_line[input_pos++] = inByte;
            break;

    }  // end of switch
    return false;

} // end of processIncomingByte


bool strobo_state = true;

int check_rx_count = 0;

void loop() {
    switch (state) {
        case CheckForRX:
            delay(20);
            if (Serial.available() > 0)
                state = RX;
            else
                state = Displaying;
            break;
        case RX:
            while (Serial.available() > 0) {
                if (processIncomingByte(Serial.read())) {
                    delay(100);
                    state = CheckForRX;
                    break;
                }
            }
            break;
        case Displaying:
            rainbow_offset = (rainbow_offset + RAINBOW_STEP) % 256;
            for (int i = 0; i < NUM_STRIPS; i++) {
                switch (led_strip_patterns[i]) {
                    case JAEGERMEISTER_ORANGE:
                        fill_strip_solid(i, CRGB::DarkOrange);
                        break;
                    case JAEGERMEISTER_GRUEN:
                        fill_strip_solid(i, CRGB::Green);
                        break;
                    case BLAU:
                        fill_strip_solid(i, CRGB::Blue);
                        break;
                    case WEISS:
                        fill_strip_solid(i, CRGB::White);
                        break;
                    case ROT:
                        fill_strip_solid(i, CRGB::Red);
                        break;
                    case REGENBOGEN_UMLAUFEND:
                        fill_strip_rainbow(i);
                        break;
                    case REGENBOGEN_FLACKERND:
                        if (strobo_state) {
                            fill_strip_rainbow(i);
                        } else {
                            fill_strip_solid(i, CRGB::Black);
                        }
                        break;
                    case STROBOSKOP:
                        if (strobo_state) {

                            fill_strip_solid(i, CRGB::White);
                        } else {

                            fill_strip_solid(i, CRGB::Black);
                        }
                        break;
                    case AUS:
                        fill_strip_solid(i, CRGB::Black);
                        break;
                }
            }
            strobo_state = !strobo_state;
            FastLED.show();
            state = CheckForRX;
            break;
    }
}

void process_data(char *data) {
    if (strncmp("?", data, 1) == 0) {
        Serial.print(d_ID);
    } else if (strncmp("SET", data, 3) == 0) {
        if (parse_set(data)) {
            Serial.print(d_OK);
        } else {
            Serial.print(d_BAD_SYNTAX);
        }
    } else {
        Serial.print(d_UNRECOGNIZED_COMMAND);
    }
}

bool parse_set(char *data) {
    int strip_id;
    int pattern;
    strtok(data, d_COMMAND_DELIMITER); // Skip command part
    char *argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    while (argument != nullptr) {
        strip_id = d_INVALID_LED_STRIP;
        pattern = d_INVALID_LED_PATTERN;

        sscanf(argument, "S%d=%d", &strip_id, &pattern); // Parse argument
        if ((strip_id != d_INVALID_LED_STRIP) && (strip_id <= NUM_STRIPS)) {
            if ((pattern != d_INVALID_LED_PATTERN) && (pattern <= 9)) {
                led_strip_patterns[strip_id - 1] = (Pattern) pattern;
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

void fill_strip_solid(int strip_id, CRGB color) {
    switch (strip_id) {
        case 0:
            LEDs_0.fill_solid(color);
            break;
        case 1:
            LEDs_1.fill_solid(color);
            break;
        case 2:
            LEDs_2.fill_solid(color);
            break;
        case 3:
            LEDs_3.fill_solid(color);
            break;
    }
}

void fill_strip_rainbow(int strip_id) {
    switch (strip_id) {
        case 0:
            LEDs_0.fill_rainbow(rainbow_offset);
            break;
        case 1:
            LEDs_1.fill_rainbow(rainbow_offset);
            break;
        case 2:
            LEDs_2.fill_rainbow(rainbow_offset);
            break;
        case 3:
            LEDs_3.fill_rainbow(rainbow_offset);
            break;
    }
}