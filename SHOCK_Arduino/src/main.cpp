#include <Arduino.h>
#include "FastLED.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Defines
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Parser defines
#define d_ID                    ("SHOCK\n")
#define d_OK                    ("OK\n")
#define d_BAD_SYNTAX            ("BAD_SYNTAX\n")
#define d_UNRECOGNIZED_COMMAND  ("UNRECOGNIZED_COMMAND\n")
#define d_COMMAND_DELIMITER     (":")
#define d_ARGUMENT_DELIMITER    (";")
#define d_MAX_STRING_SIZE       (64)

// Pin defines
#define d_SHOCKPAD_EN_CH1      (7)
#define d_SHOCKPAD_EN_CH2      (8)
#define d_SHOCKPAD_EN_CH3      (9)
#define d_SHOCKPAD_EN_CH4      (10)

#define d_FORCE_SENSE_CH1      (A0)
#define d_FORCE_SENSE_CH2      (A1)
#define d_FORCE_SENSE_CH3      (A2)
#define d_FORCE_SENSE_CH4      (A3)

#define d_LED_DATA_CH1         (6)
#define d_LED_DATA_CH2         (1)
#define d_LED_DATA_CH3         (2)
#define d_LED_DATA_CH4         (3)

// Led defines
#define d_NUM_LEDS_PER_RING    (32)
#define d_COLOR_ORDER          (GRB)
#define d_NUM_SHOCKPADS        (4)
#define d_STROBO_DELAY         (0.5)
#define d_STROBO_OFFSET_DELAY_MS         (1)

#define d_MAX_VOLTAGE_V        (5)
#define d_MAX_CURRENT_MA       (1000)
#define d_BRIGHTNESS_DIGITS    (255)

// Shock defines
#define d_SHOCK_TIME_LEVEL1_MS (10)
#define d_SHOCK_TIME_LEVEL2_MS (8)
#define d_SHOCK_TIME_LEVEL3_MS (4)
#define d_SHOCK_TIME_LEVEL4_MS (1)

#define d_SHOCK_TIME_OFFSET     (3000)


// Internal defines
#define d_LOWER_TIMER_LIMIT    (3)
#define d_UPPER_TIMER_LIMIT    (10)

// Communication defines
#define d_BAUD_RATE             (115200)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Declarations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

enum e_PadTouchStatus
{
    e_PadTouchStatusUntouched = 0,
    e_PadTouchStatusTouched,
    e_PadTouchStatusCount
};

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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Pattern led_strip_patterns[d_NUM_SHOCKPADS] =
{
    Pattern::REGENBOGEN_UMLAUFEND,
    Pattern::REGENBOGEN_UMLAUFEND,
    Pattern::REGENBOGEN_UMLAUFEND,
    Pattern::REGENBOGEN_UMLAUFEND,
};

CRGBArray<d_NUM_LEDS_PER_RING> msa_LedStripCh1;
CRGBArray<d_NUM_LEDS_PER_RING> msa_LedStripCh2;
CRGBArray<d_NUM_LEDS_PER_RING> msa_LedStripCh3;
CRGBArray<d_NUM_LEDS_PER_RING> msa_LedStripCh4;


//ShockPad shock_pads[d_NUM_SHOCKPADS] =                  {DONT_SHOCK, DONT_SHOCK, DONT_SHOCK, DONT_SHOCK};
ShockPad shock_pads[d_NUM_SHOCKPADS] =                  {SHOCK, SHOCK, SHOCK, SHOCK};
e_PadTouchStatus mea_TouchStatus [d_NUM_SHOCKPADS] =    {e_PadTouchStatusUntouched, e_PadTouchStatusUntouched, e_PadTouchStatusUntouched, e_PadTouchStatusUntouched, };
Level shock_level = Level2;
bool parse_set(char *string);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Function prototypes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Func_ShockPads(void);
void Func_Trigger(void);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Function implementations
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void setup ()
{
    // Serial setup
    Serial.begin (d_BAUD_RATE);
    // Setup ch1
    pinMode(d_FORCE_SENSE_CH1,  INPUT);
    pinMode(d_LED_DATA_CH1,     OUTPUT);
    pinMode(d_SHOCKPAD_EN_CH1,  OUTPUT);

    // Setup ch2
    pinMode(d_FORCE_SENSE_CH2,  INPUT);
    pinMode(d_LED_DATA_CH2,     OUTPUT);
    pinMode(d_SHOCKPAD_EN_CH2,  OUTPUT);

    // Setup ch3
    pinMode(d_FORCE_SENSE_CH2,  INPUT);
    pinMode(d_LED_DATA_CH2,     OUTPUT);
    pinMode(d_SHOCKPAD_EN_CH2,  OUTPUT);

    // Setup ch4
    pinMode(d_FORCE_SENSE_CH2,  INPUT);
    pinMode(d_LED_DATA_CH2,     OUTPUT);
    pinMode(d_SHOCKPAD_EN_CH2,  OUTPUT);

    // Led initialization
    FastLED.setMaxPowerInVoltsAndMilliamps(d_MAX_VOLTAGE_V, d_MAX_CURRENT_MA);

    FastLED.addLeds<NEOPIXEL, d_LED_DATA_CH1>(msa_LedStripCh1, d_NUM_LEDS_PER_RING);
    FastLED.addLeds<NEOPIXEL, d_LED_DATA_CH2>(msa_LedStripCh2, d_NUM_LEDS_PER_RING);
    FastLED.addLeds<NEOPIXEL, d_LED_DATA_CH3>(msa_LedStripCh3, d_NUM_LEDS_PER_RING);
    FastLED.addLeds<NEOPIXEL, d_LED_DATA_CH4>(msa_LedStripCh4, d_NUM_LEDS_PER_RING);

    FastLED.setBrightness(d_BRIGHTNESS_DIGITS);
    FastLED.clear();
    FastLED.show();
}

void process_data (char * data)
{
    if (strncmp("?", data, 1) == 0) {
            Serial.print(d_ID);
            Func_ShockPads();

        } else if (strncmp("SET", data, 4) == 0) {
            if (parse_set(data)) {
                    Serial.print(d_OK);
                } else {
                    Serial.print(d_BAD_SYNTAX);
                }
        } else if (strncmp("PLAY", data, 3) == 0) {
            char p1, p2, p3, p4;
            Func_ShockPads();
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
    ShockPad _shock_pads[4] = {DONT_SHOCK, DONT_SHOCK, DONT_SHOCK, DONT_SHOCK};

    strtok(data, d_COMMAND_DELIMITER); // Skip command part
    char *argument = strtok(nullptr, d_ARGUMENT_DELIMITER); // Get next argument
    while (argument != nullptr) {
        if (strncmp(argument, "L", 1) == 0) {
            int num_level = 0;
                    sscanf(argument, "L=%d", &num_level);
                    if (num_level == 0) return false;
                    _shock_level = (Level) num_level;
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

            analogWrite(d_ADJUST_PIN,lu16_AdjustVoltageDigits);

    shock_level = _shock_level;
    for (int i = 0; i < 4; ++i) {
            shock_pads[i] = _shock_pads[i];
        }

    return true;
}

void Func_ShockPads(void)
{
    uint16_t lu16_TimerValue =0;
    unsigned long lul_ActualTimestamp = 0;
    unsigned long lul_ShockTimestamp = 0;
    double ld16_StroboDelay_ms = d_STROBO_DELAY;

    // calculate a random number between 3000 and 10.000
    lu16_TimerValue= 1000*random(d_LOWER_TIMER_LIMIT, d_UPPER_TIMER_LIMIT);

    // calculate shock timestamp
    lul_ShockTimestamp = millis() + lu16_TimerValue;
    // loop while actual and normal time are different
    while (lul_ActualTimestamp < lul_ShockTimestamp)
    {
        // start led flickering until shock comes
        // check which leds should be light up
        for(uint8_t lu8_PadIndex = 0; lu8_PadIndex < d_NUM_SHOCKPADS; lu8_PadIndex++)
        {
            if (shock_pads[lu8_PadIndex] == SHOCK)
            {
                delay(ld16_StroboDelay_ms);
                    ld16_StroboDelay_ms += d_STROBO_OFFSET_DELAY_MS;
                // light up the led-rings in the shocker pads
                switch (lu8_PadIndex)
                    {
                        case 0:
                            msa_LedStripCh1.fill_solid (CRGB::Wheat);
                            break;
                        case 1:
                            msa_LedStripCh2.fill_solid (CRGB::Wheat);
                            break;
                        case 2:
                            msa_LedStripCh3.fill_solid (CRGB::Wheat);
                            break;
                        case 3:
                            msa_LedStripCh4.fill_solid (CRGB::Wheat);
                            break;
                        default:
                            break;
                    }
                FastLED.show();
                }
        }
        FastLED.clear ();
        FastLED.show();
        // get current timestamp
        lul_ActualTimestamp = millis();
    }
    // Shock the corresponding pads with the shock level
    Func_Trigger();


    // measure which pad is held

    // return message that corresponds to the pads that are pressed
    Serial.print("Ok\n");

}

void Func_Trigger()
{
    unsigned long lul_ActualTriggerTimestamp = millis();
    unsigned long lul_ShockTimestamp = lul_ActualTriggerTimestamp + d_SHOCK_TIME_OFFSET;

    while(lul_ActualTriggerTimestamp < lul_ShockTimestamp)
    {
        // check which leds should be light up
        for (uint8_t lu8_PadIndex = 0; lu8_PadIndex < d_NUM_SHOCKPADS; lu8_PadIndex++)
            {
                if ((shock_pads[lu8_PadIndex] == SHOCK) && shock_level == Level1)
                {
                    switch (lu8_PadIndex)
                    {
                        case 0:
                            digitalWrite (d_SHOCKPAD_EN_CH1, HIGH);
                        break;
                        case 1:
                            digitalWrite (d_SHOCKPAD_EN_CH2, HIGH);
                        break;
                        case 2:
                            digitalWrite (d_SHOCKPAD_EN_CH3, HIGH);
                        break;
                        case 3:
                            digitalWrite (d_SHOCKPAD_EN_CH4, HIGH);
                        break;
                        default:break;
                    }
                    delay (d_SHOCK_TIME_LEVEL1_MS);
                    switch (lu8_PadIndex)
                        {
                            case 0:
                                digitalWrite (d_SHOCKPAD_EN_CH1, LOW);
                            break;
                            case 1:
                                digitalWrite (d_SHOCKPAD_EN_CH2, LOW);
                            break;
                            case 2:
                                digitalWrite (d_SHOCKPAD_EN_CH3, LOW);
                            break;
                            case 3:
                                digitalWrite (d_SHOCKPAD_EN_CH4, LOW);
                            break;
                            default:break;
                        }
                    delay (d_SHOCK_TIME_LEVEL1_MS);
                }
                if ((shock_pads[lu8_PadIndex] == SHOCK) && shock_level == Level2)
                    {
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, HIGH);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, HIGH);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, HIGH);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, HIGH);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL2_MS);
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, LOW);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, LOW);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, LOW);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, LOW);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL2_MS);
                    }
                if ((shock_pads[lu8_PadIndex] == SHOCK) && shock_level == Level3)
                    {
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, HIGH);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, HIGH);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, HIGH);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, HIGH);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL3_MS);
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, LOW);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, LOW);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, LOW);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, LOW);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL3_MS);
                    }
                if ((shock_pads[lu8_PadIndex] == SHOCK) && shock_level == Level4)
                    {
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, HIGH);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, HIGH);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, HIGH);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, HIGH);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL4_MS);
                        switch (lu8_PadIndex)
                            {
                                case 0:
                                    digitalWrite (d_SHOCKPAD_EN_CH1, LOW);
                                break;
                                case 1:
                                    digitalWrite (d_SHOCKPAD_EN_CH2, LOW);
                                break;
                                case 2:
                                    digitalWrite (d_SHOCKPAD_EN_CH3, LOW);
                                break;
                                case 3:
                                    digitalWrite (d_SHOCKPAD_EN_CH4, LOW);
                                break;
                                default:break;
                            }
                        delay (d_SHOCK_TIME_LEVEL4_MS);
                    }
            }
        lul_ActualTriggerTimestamp = millis();
    }
}
