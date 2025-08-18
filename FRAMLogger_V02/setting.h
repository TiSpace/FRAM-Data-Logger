/*
    µC Pinout
*   RED     ->  A1
*   GREEN   ->  A0
*   BLUE    ->  A2
*   I2C SCL ->  A5
*   I2C SDA ->  A4
*   Button  ->  D10
*/

// optional setup for parameter to record
#define PARAMETER_TEMPERATURE
#define PARAMETER_HUMIDITY
#define PARAMETER_TVOC  //ENS16x TVOC
#define PARAMETER_ECO2  //ENS16x eCO2
#define PARAMETER_AQIS  //ENS16x AQI-S
#define PARAMETER_AQIU  //ENS16x AQI-U
#define PARAMETER_P     // ENS220 pressure reading


#define PARAMETER_TEMPERATURE_FLAG (1 << 2)  //0
#define PARAMETER_HUMIDITY_FLAG (1 << 3)
#define PARAMETER_TVOC_FLAG (1 << 4)  //ENS16x TVOC
#define PARAMETER_ECO2_FLAG (1 << 5)  //ENS16x eCO2
#define PARAMETER_HP1_FLAG (1 << 6)   //
#define PARAMETER_HP2_FLAG (1 << 7)
#define PARAMETER_HP3_FLAG (1 << 8)
#define PARAMETER_AQIS_FLAG (1 << 9)   //ENS16x AQI-S
#define PARAMETER_AQIU_FLAG (1 << 10)  //ENS16x AQI-U
#define PARAMETER_P_FLAG (1 << 15)     // ENS220 pressure reading

//comment/uncomment parameter which are required
uint16_t PARAMETER_FLAG =  
  PARAMETER_TEMPERATURE_FLAG
  | PARAMETER_HUMIDITY_FLAG
  | PARAMETER_TVOC_FLAG
  | PARAMETER_ECO2_FLAG
  //| PARAMETER_HP1_FLAG
  //| PARAMETER_HP2_FLAG
  | PARAMETER_HP3_FLAG
  | PARAMETER_AQIS_FLAG
  | PARAMETER_AQIU_FLAG
  | PARAMETER_P_FLAG
  ;

#define DebugMode

#include "Arduino.h"
#include <stdint.h>

// LED connections (pin setup)
#define PIN_LEDGREEN A2
#define PIN_LEDRED A1
#define PIN_LEDBLUE A0

#define PIN_BUTTON 10

//#define I2C_ADDRESS
#define ENS16x_I2C_ADDRESS 0x52
#define ENS220_I2C_ADDRESS 0x20



  // structure definition for measurement values. Note: this structure will be written to the memory whatever it is defined!!
  struct measurements {
    uint32_t unixTime;  // Unix timestamp    little endian
#ifdef PARAMETER_TEMPERATURE
    uint16_t temperature;  //
#endif
#ifdef PARAMETER_HUMIDITY
    uint16_t humidity;
#endif
    uint16_t resistanceHP3;
#ifdef PARAMETER_ECO2
    uint16_t eCO2;  //little endian
#endif
#ifdef PARAMETER_ECO2
    uint16_t TVOC;  //little endian
#endif
#ifdef PARAMETER_P
    uint32_t pressure;
#endif
} myMeasurement;


//Definitions for data storage in FRAM
uint8_t DATA_PACKAGE_SIZE = sizeof(myMeasurement);
#define DATA_FRAM_START 20                         // start address for data storage, keep some bytes for vairious data
#define DATA_ADR_DATASET DATA_FRAM_START - 2       // address where number of stored datasets are kept
#define DATA_MEASUREMENT_TIME DATA_FRAM_START - 3  // storage of time between measurements
#define DATA_FRAM_SIZE 32768                       //available size in byte


// various definitions
#define BAUDRATE 115200
uint8_t WAIT_BETWEEN_MEASURE = 5;  // number of seconds between measurements


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
    pinMode(PIN_BUTTON, INPUT_PULLUP);
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