#include "arduino.h"
#include <ardumidi.h>
#include <Adafruit_NeoPixel.h>

/*
 * SN74HC165N_shift_reg
 *
 * Program to shift in the bit values from a SN74HC165N 8-bit
 * parallel-in/serial-out shift register.
 *
 * This sketch demonstrates reading in 16 digital states from a
 * pair of daisy-chained SN74HC165N shift registers while using
 * only 4 digital pins on the Arduino.
 *
 * You can daisy-chain these chips by connecting the serial-out
 * (Q7 pin) on one shift register to the serial-in (Ds pin) of
 * the other.
 *
 * Of course you can daisy chain as many as you like while still
 * using only 4 Arduino pins (though you would have to process
 * them 4 at a time into separate unsigned long variables).
 *
*/

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

int32_t magenta = strip.Color(20, 40, 20);
int32_t magenta_fuerte  = strip.Color(255, 0, 255);
/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   2

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

/* You will need to change the "int" to "long" If the
 * NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

int ploadPin        = 8;  // Connects to Parallel load pin the 165
int clockEnablePin  = 9;  // Connects to Clock Enable pin the 165
int dataPin         = 11; // Connects to the Q7 pin the 165
int clockPin        = 12; // Connects to the Clock pin the 165

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

int button_map[16] = {2,5,10,13,12,11,4,3,15,8,7,0,14,9,6,1};

/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(dataPin);

        /* Set the corresponding bit in bytesVal.
        */
        //bytesVal |= (bitVal << ((DATA_WIDTH-1) - i ));
        bytesVal |= (bitVal << button_map[((DATA_WIDTH-1) - i)]);

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}

/* Dump the list of zones along with their current status.
*/
void display_pin_values()
{
    //Serial.print("Pin States:\r\n");

    for(int i = 0; i < DATA_WIDTH; i++)
    {

        Serial.print("  Pin-");
        Serial.print(i);
        Serial.print(": ");

        if((pinValues >> i) & 1)
            Serial.print("HIGH");
            //midi_note_on(3, 12*5 + i, 127);
        else
            Serial.print("LOW");
            //midi_note_off(3, 12*5 + i, 127);

        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}

void send_midi_value()
{
    //Serial.print("Pin States:\r\n");

    for(int i = 0; i < DATA_WIDTH; i++)
    {

        if((pinValues >> i) != (oldPinValues >> i)){
          Serial.print("  Pin-");
          Serial.print(i);
          Serial.print(": ");

          if((pinValues >> i) & 1){
            Serial.print("HIGH");
            //midi_note_on(3, 12*5 + i, 127);
            strip.setPixelColor(i,magenta_fuerte);
          }

          else{
            Serial.print("LOW");
            strip.setPixelColor(i,magenta);
            //midi_note_off(3, 12*5 + i, 127);
          }

          strip.show();
          Serial.print("\r\n");
      }
    }

    Serial.print("\r\n");
}

void setup()
{
    Serial.begin(115200);

    strip.begin();

    for (uint16_t i = 0; i < 16; i++) {
      strip.setPixelColor(i,magenta);
    }
    strip.show(); // Initialize all pixels to 'off'

    /* Initialize our digital pins...
    */
    pinMode(ploadPin, OUTPUT);
    pinMode(clockEnablePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);

    digitalWrite(clockPin, LOW);
    digitalWrite(ploadPin, HIGH);

    /* Read in and display the pin states at startup.
    */
    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
}

void loop()
{
    /* Read the state of all zones.
    */
    pinValues = read_shift_regs();

    /* If there was a chage in state, display which ones changed.
    */
    if(pinValues != oldPinValues)
    {
        //Serial.print("*Pin value change detected*\r\n");
        //display_pin_values();
        send_midi_value();
        oldPinValues = pinValues;
    }

    delay(POLL_DELAY_MSEC);
}
