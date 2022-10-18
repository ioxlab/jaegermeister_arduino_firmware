#include <Arduino.h>

// Project defines
#define d_ID                    ("SHOCK\n")

// Parser defines
#define d_MAX_STRING_SIZE       (64)
#define d_DELIMITER             (";")

// Communication defines
#define d_BAUD_RATE             (115200)

// Pin defines
#define d_FEEDBACK_PIN          (A0)
#define d_ADJUST_PIN            (5)
#define d_SHOCK_ENABLE          (3)

char mca_StringBuffer[d_MAX_STRING_SIZE] = {0};
char mc_ReadedBytes = 0;



void setup ()
{
    Serial.begin (d_BAUD_RATE);
    pinMode(d_FEEDBACK_PIN, INPUT);
    pinMode(d_ADJUST_PIN,OUTPUT);
    pinMode(d_SHOCK_ENABLE,OUTPUT);

    digitalWrite(d_SHOCK_ENABLE, LOW);
}

int incomingByte = 0; // For incoming serial data

void loop ()
{

    // Read until \n
    while (true)
        {
            // Wait until bytes available
            while (Serial.available () == 0);
            // Read byte
            incomingByte = Serial.read ();

            // If it is a \n stop reading
            if (incomingByte == '\n')
                {
                    break;
                }
            else
                {
                    // Otherwise, add char to buffer
                    mca_StringBuffer[mc_ReadedBytes] = (char) incomingByte;
                    mc_ReadedBytes += 1;
                }
        }

    if (!strcmp (mca_StringBuffer, "?"))
        {
            Serial.print (d_ID);
            int i = 0;
            uint16_t lu16_FeedbackVoltageDigits = 0;
            uint16_t lu16_AdjustVoltageDigits = 0;

            analogWrite(d_ADJUST_PIN,lu16_AdjustVoltageDigits);

            while(i<10)
            {
                analogWrite(d_ADJUST_PIN,lu16_FeedbackVoltageDigits);
                lu16_FeedbackVoltageDigits+=5;
                Serial.println(analogRead(d_FEEDBACK_PIN));
                i++;
                delay(30);
            }
            analogWrite(d_ADJUST_PIN,0);
        }
    else
        {
            Serial.print ("UNRECOGNIZED COMMAND\n");
        }
    mc_ReadedBytes = 0;
    memset (&mca_StringBuffer,0,d_MAX_STRING_SIZE);
}