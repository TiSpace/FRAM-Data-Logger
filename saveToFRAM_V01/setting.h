/*
    µC Pinout
*   RED     ->  A1
*   GREEN   ->  A0
*   BLUE    ->  A2
*   I2C SCL ->  A5
*   I2C SDA ->  A4
*   Button  ->  D10
*/

#include "Arduino.h"
#include <stdint.h>

// LED connections (pin setup)
#define PIN_LEDGREEN A2
#define PIN_LEDRED A1
#define PIN_LEDBLUE A0

#define PIN_BUTTON 10

#define ENS161_I2C_ADDRESS 0x52



// structure definition for measurement values. Note: this structure will be written to the memory whatever it is defined!!
struct measurements {
    uint32_t unixTime;     // Unix timestamp
    uint16_t temperature;  //
    uint16_t resistanceHP3;
    uint16_t eCO2;  //little endian
} myMeasurement;


//Definitions for data storage in FRAM
uint8_t DATA_PACKAGE_SIZE = sizeof(myMeasurement);
#define DATA_FRAM_START 20   // start address for data storage, keep some bytes for vairious data
#define DATA_ADR_DATASET  DATA_FRAM_START-2  // address where number of stored datasets are kept
#define DATA_MEASUREMENT_TIME DATA_FRAM_START-3  // storage of time between measurements
#define DATA_FRAM_SIZE  32768   //available size in byte


// various definitions
#define BAUDRATE 9600
uint8_t WAIT_BETWEEN_MEASURE  =5;   // number of seconds between measurements


// structure for LED
typedef enum {
    OFF,
    BLUE = 1,
    RED = 2,
    GREEN = 4
} portLED;
portLED myLED;

// init button and LED pins
void initButtonLED() {
    pinMode(PIN_LEDGREEN, OUTPUT);
    pinMode(PIN_LEDRED, OUTPUT);
    pinMode(PIN_LEDBLUE, OUTPUT);
    pinMode(PIN_BUTTON,INPUT_PULLUP);
}

// control LED
void LED(uint8_t LED_colour) {
    // adress single colours or mixes
    digitalWrite(PIN_LEDGREEN, (~LED_colour & GREEN) >> 2);
    digitalWrite(PIN_LEDRED, (~LED_colour & RED) >> 1);
    digitalWrite(PIN_LEDBLUE, (~LED_colour & BLUE) >> 0);
}

// blink LED
void LED_blink(uint8_t LED_colour, uint8_t blinkNo, uint16_t delay_ms) {
    for (int i = 0; i < blinkNo; i++) {
        LED(LED_colour);
        delay(delay_ms / 2);
        LED(OFF);
        delay(delay_ms / 2);
    }
}
