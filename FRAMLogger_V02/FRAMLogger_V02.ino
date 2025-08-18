/*
*   FRAM-Logger   
* Data Logger into FRAM
*    V1      tr 01.08.2025
*    V2       - allow multiple recording sets
*
* ToDo
* - power saving  -> Sensor Modi
*                -> Sleep mode µC
*
*/

#include "setting.h"
#include <Wire.h>
#include "RTClib.h"            //V2.1.4  https://github.com/adafruit/RTClib
#include <ScioSense_ENS16x.h>  //V2.0.4  https://github.com/sciosense/ens16x-arduino
#include <ScioSense_ENS21x.h>  //V2.0.0  https://github.com/sciosense/ens21x-arduino
#include "custom_ens220_library/ens220_noSpi.h"
#include "Adafruit_FRAM_I2C.h"  //V2.0.3  https://github.com/adafruit/Adafruit_FRAM_I2C




RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };



ENS161 ens16x;
ENS210 ens210;
ENS220 ens220;


String inputString = "";
int year, month, day, hour, minute, second;
uint32_t unixTimeStart;
uint16_t datasetNo = 0;

bool newData = false;
bool loggingActive = false;
bool displayMeasurement = true;
Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();


/* ******************************************************************
*           S E T U P
*  ******************************************************************/
void setup() {
    Serial.begin(BAUDRATE);
    Wire.begin();
    fram.begin();

    initButtonLED();  //init button and LED

    LED(BLUE);
    ens16x.begin(&Wire, ENS16x_I2C_ADDRESS);
    ens210.begin();

    Serial.println(F("begin ens16x.."));
    while (ens16x.init() != true) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(F("success"));

    Serial.println(F("begin ens210.."));
    while (ens210.init() != true) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(F("success"));

    initENS220();


    ens16x.startStandardMeasure();

    LED(OFF);


    rtc.begin();
    WAIT_BETWEEN_MEASURE = fram.read(DATA_MEASUREMENT_TIME);  // get parameter from memory
    displayMenu();
}





/* ******************************************************************
*           L O O P
*  ******************************************************************/
void loop() {
    serialInput();  //  check if serial command has been sent
    checkButton();  //  check if button has been pressed (start/stop recording)


    DateTime now = rtc.now();

    if (((now.unixtime() - unixTimeStart) > WAIT_BETWEEN_MEASURE)) {
        measureSensor();  //read from sensor


        now = rtc.now();                 //update timestamp
        unixTimeStart = now.unixtime();  //store time of last measurement
        if (loggingActive) {             //logging active
            LED(GREEN);
            datasetNo++;
            if (DATA_FRAM_SIZE > (DATA_FRAM_START + DATA_PACKAGE_SIZE * datasetNo)) {  //cehck if memory is available
                myMeasurement.unixTime = now.unixtime();
                //copy struct byte by byte into FRAM
                unsigned char *ptr = (unsigned char *)&myMeasurement;
                size_t size = sizeof(struct measurements);



                for (size_t i = 0; i < size; i++) {
                    fram.write(DATA_FRAM_START + DATA_PACKAGE_SIZE * (datasetNo - 1) + i, ptr[i]);
                    // Serial.print(ptr[i], HEX);
                    // Serial.print(" ");
                }
                Serial.println("");
                fram.write(DATA_ADR_DATASET, highByte(datasetNo));
                fram.write(DATA_ADR_DATASET + 1, lowByte(datasetNo));
                LED(OFF);
            } else {
                // FRAM is filled up!
                LED(RED);
                Serial.print(F("Memory full!!"));
                loggingActive = false;
            }
        }
    }
}

/* ******************************************************************
*           serial communication and action
*  ******************************************************************/
void serialInput() {
    // input mask -< for communication terminate with CR+LF  (\n + \r)
    char c;

    while (Serial.available()) {
        c = Serial.read();

        inputString += c;
    }
    c = inputString.charAt(0);  //cehck for first characters, other ESC seqeunces are not relevant
    inputString = "";           //reset complete input String

    switch (c) {
        case '?':
            displayMenu();
            break;
        case 'd':  //enter time/date to set RTC
            enterTimeDate();
            break;
        case 's':  //start/stop logging
            controlLogging();
            break;
        case 'm':  //enable/disable measurement output to serial interface
            displayMeasurement = !displayMeasurement;
            Serial.println(F("**** HIDE/SHOW measurement ****"));
            break;
        case 'r':  //memory dump
            displayAllData();
            break;
        case 't':  //enter wait time
            enterWaitTime();
            break;
        case 'u':
            displayFRAM();
            break;
        default:
            break;
    }
}

/* ******************************************************************
*           date and time interpretation and handling
*  ******************************************************************/
bool parseDateTime(String str) {
    int firstDot = str.indexOf('.');
    int secondDot = str.indexOf('.', firstDot + 1);
    int space = str.indexOf(' ');
    int firstColon = str.indexOf(':', space);
    int secondColon = str.indexOf(':', firstColon + 1);

    if (firstDot == -1 || secondDot == -1 || space == -1 || firstColon == -1 || secondColon == -1) return false;

    String yy = str.substring(0, firstDot);
    String mm = str.substring(firstDot + 1, secondDot);
    String dd = str.substring(secondDot + 1, space);
    String hh = str.substring(space + 1, firstColon);
    String min = str.substring(firstColon + 1, secondColon);
    String ss = str.substring(secondColon + 1);

    year = yy.toInt();
    month = mm.toInt();
    day = dd.toInt();
    hour = hh.toInt();
    minute = min.toInt();
    second = ss.toInt();

    if (year < 0 || year > 99) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;
    if (second < 0 || second > 59) return false;

    return true;
}

/* ******************************************************************
*           format number to two digits always
*  ******************************************************************/
void print2digits(byte k) {
    if (k < 10) Serial.print('0');
    Serial.print(k);
}

/* ******************************************************************
*           format hex number to two digits always
*  ******************************************************************/
void print2digitsHex(byte k) {
    if (k < 0x10) Serial.print('0');
    Serial.print(k,HEX);
}


/* ******************************************************************
*           get data from the sensors
*  ******************************************************************/
void measureSensor() {

    // ens16x+ENS210
    if (ens210.singleShotMeasure() == RESULT_OK) {
        if (displayMeasurement) {
            Serial.print(F("Temp[°C]: "));
            Serial.print(ens210.getTempCelsius());
            Serial.print(F("\tHumidity[%RH]: "));
            Serial.print(ens210.getHumidityPercent());
#ifdef PARAMETER_TEMPERATURE
            myMeasurement.temperature = ens210.getDataT();
#endif
#ifdef PARAMETER_HUMIDITY
            myMeasurement.humidity = ens210.getDataH();
#endif
        }
        ens16x.writeCompensation(ens210.getDataT(), ens210.getDataH());
    }

    ens16x.wait();

    if (ens16x.update() == RESULT_OK) {
        if (ens16x.hasNewData()) {
            if (displayMeasurement) {
                //Serial.print(F("\tAQI ScioSense:"));
                //Serial.print(ens16x.getAirQualityIndex_ScioSense());
                Serial.print(F("\tAQI UBA:"));
                Serial.print(ens16x.getAirQualityIndex_UBA());
                Serial.print(F("\tTVOC:"));
                Serial.print(ens16x.getTvoc());
                Serial.print(F("\tECO2: "));
                Serial.print(ens16x.getEco2());
            }
        }

        if (ens16x.hasNewGeneralPurposeData()) {
            if (displayMeasurement) {
                // Serial.print("\tRS0:");
                // Serial.print(ens16x.getRs0());
                // Serial.print("\tRS1:");
                // Serial.print(ens16x.getRs1());
                // Serial.print("\tRS2:");
                // Serial.print(ens16x.getRs2());
                Serial.print(F("\tRS3:"));
                Serial.print(ens16x.getRs3());

                myMeasurement.resistanceHP3 = ens16x.getRs3();
#ifdef PARAMETER_ECO2
                myMeasurement.eCO2 = ens16x.getEco2();
#endif
#ifdef PARAMETER_TVOC
                myMeasurement.eCO2 = ens16x.getTvoc();
#endif
                myMeasurement.temperature = ens210.getDataT();
                Serial.print(F("\tT_raw:"));
                Serial.print(ens210.getDataT());
            }
        }
    }
    //ENS220

    // Start single shot measurement
    ens220.singleShotMeasure(ENS220_SENSOR_TEMPERATURE_AND_PRESSURE);

    // Wait until the measurement is ready
    ens220.waitSingleShot();

    // Check the DATA_STAT from the sensor. If data is available, it reads it
    auto result = ens220.update();

    if (result == RESULT_OK) {
        if (ens220.hasDataStatusFlag(ENS220_DATA_STATUS_PRESSURE_READY) && ens220.hasDataStatusFlag(ENS220_DATA_STATUS_TEMPERATURE_READY)) {
// Send the values that were collected during the ens220.update()
#ifdef PARAMETER_P
            if (displayMeasurement) {
                Serial.print("\tP[hPa]:");
                Serial.print(ens220.getPressureHectoPascal());

                if (displayMeasurement) Serial.println("");
                myMeasurement.pressure = ens220.getPressureRaw();
            }
#endif
        }
    }
}



/* ******************************************************************
*           enter time and date to set RTC
*  ******************************************************************/
void enterTimeDate() {
    DateTime now = rtc.now();
    char buf2[] = "YY.MM.DD  hh:mm:ss";
    Serial.println(now.toString(buf2));

    Serial.println("Please enter date and time in format yy.mm.dd hh:mm:ss ");
    while (!Serial.available())
        ;
    while (!newData) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                newData = true;
            } else {
                inputString += c;
            }
        }
    }
    if (newData) {
        if (parseDateTime(inputString)) {
            Serial.print(F("Jahr: "));
            Serial.println(year);
            Serial.print(F("Monat: "));
            Serial.println(month);
            Serial.print(F("Tag: "));
            Serial.println(day);
            Serial.print(F("Stunden: "));
            Serial.println(hour);
            Serial.print(F("Minuten: "));
            Serial.println(minute);
            Serial.print(F("Sekunden: "));
            Serial.println(second);

            rtc.adjust(DateTime(year, month, day, hour, minute, second));
        } else {
            Serial.println(F("invalid fomat. please use yy.mm.dd hh:mm:ss"));
        }
        inputString = "";
        newData = false;
    }
}


/* ******************************************************************
*           enter time between measurement
*  ******************************************************************/
void enterWaitTime() {

    bool lockFunction = false;

    delay(200);
    while (Serial.available()) Serial.read();  //clear buffer

    Serial.print(F("enter waittime in sec (currently "));
    Serial.print(WAIT_BETWEEN_MEASURE);
    Serial.print(F(" sec):  "));

    while (!Serial.available())

        while (!lockFunction && Serial.available()) {
            int dataIn = Serial.parseInt();
            //Do something with the data - like print it
            //Serial.print(dataIn);
            WAIT_BETWEEN_MEASURE = dataIn;
            lockFunction = true;
        }
    Serial.print("\n-> set to: ");
    Serial.println(WAIT_BETWEEN_MEASURE);
    fram.write(DATA_MEASUREMENT_TIME, WAIT_BETWEEN_MEASURE);
}
/* ******************************************************************
*           read memory, process and display
*  ******************************************************************/
void displayAllData() {
    Serial.println(F("\n\t**** Display all data ****"));

    datasetNo = (fram.read(DATA_ADR_DATASET) << 8) + fram.read(DATA_ADR_DATASET + 1);
    Serial.print(F("number of stored datasets: "));
    Serial.println(datasetNo);

    uint8_t readData[DATA_PACKAGE_SIZE];
    size_t dataAddress = 0;  //varaible for addressing memory -< take care on size to be sufficient

    for (dataAddress = DATA_FRAM_START; dataAddress < ((DATA_FRAM_START + DATA_PACKAGE_SIZE * datasetNo) + 1); dataAddress++) {

        // Serial.print(i);Serial.print("\t");
        // Serial.print(DATA_PACKAGE_SIZE);
        // Serial.print("\t");
        // Serial.println((i-DATA_FRAM_START+1) % DATA_PACKAGE_SIZE);

        if (((dataAddress - DATA_FRAM_START + DATA_PACKAGE_SIZE) % DATA_PACKAGE_SIZE) == 0) {  //print full data set
                                                                                               //**if ((i % DATA_PACKAGE_SIZE) == 0) {
            if (dataAddress > DATA_FRAM_START) {
                memcpy(&myMeasurement, readData, DATA_PACKAGE_SIZE);  //copy bytes into structure to make it more handy
                                                                      //  for (int j=0;j<DATA_PACKAGE_SIZE;j++){
                                                                      //   Serial.print(readData[j],HEX);
                                                                      //  }
                                                                      //  Serial.print("\n");


               Serial.print(F("** "));
                DateTime dt(myMeasurement.unixTime);
                // Serial.print("\n");
                // Serial.print("Datum: ");
                print2digits(dt.day());
                Serial.print(".");
                print2digits(dt.month());
                Serial.print(".");
                Serial.print(dt.year());
                //Serial.print("  Uhrzeit: ");
                Serial.print("\t");
                print2digits(dt.hour());
                Serial.print(":");
                print2digits(dt.minute());
                Serial.print(":");
                print2digits(dt.second());

                //Serial.print(myMeasurement.unixTime);

#ifdef PARAMETER_HUMIDITY
                Serial.print("\t");
                Serial.print(myMeasurement.humidity);
#endif
#ifdef PARAMETER_ECO2
                Serial.print("\t");
                Serial.print(myMeasurement.eCO2);
#endif
#ifdef PARAMETER_TVOC
                Serial.print("\t");
                Serial.print(myMeasurement.TVOC);
#endif
                Serial.print("\t");
                Serial.print(myMeasurement.resistanceHP3);
#ifdef PARAMETER_TEMPERATURE
                Serial.print("\t");
                Serial.print((float)myMeasurement.temperature / 64 - 273.15);
#endif
#ifdef PARAMETER_P
                Serial.print("\t");
                Serial.print((float)myMeasurement.pressure / 64 / 100);
#endif
            }
            Serial.print("\n");
            if (dataAddress < ((DATA_FRAM_START + DATA_PACKAGE_SIZE * datasetNo))) {
                Serial.print(dataAddress / DATA_PACKAGE_SIZE - 1);
                Serial.print(":\t");
            }
        }

        uint8_t value = fram.read(dataAddress);
#ifdef DebugMode
       // Serial.print(value, HEX);
        print2digitsHex(value);
        Serial.print(" ");
#endif

        readData[(dataAddress - DATA_FRAM_START) % DATA_PACKAGE_SIZE] = value;
        //Serial.print("**");Serial.print(i);Serial.print(" ");Serial.print((i-DATA_FRAM_START)% DATA_PACKAGE_SIZE);Serial.print(" ");Serial.println(value);
        //Serial.print("\t");
        //Serial.print(value, HEX);Serial.print("\n");
    }

    Serial.print("\n");
}

/* ******************************************************************
*           display help menue
*  ******************************************************************/
void displayMenu() {

    Serial.println(F("\n\n****************************"));
    Serial.println(F("\tFRAM Data Logger"));

    Serial.println(__FILE__);
    Serial.print(__TIME__);
    Serial.print("  ");
    Serial.println(__DATE__);
    Serial.print(F("\nMenue:\n------\n"));
    Serial.println(F("\n?   this info"));
    Serial.println(F("d   set date and time"));
    Serial.println(F("s   START/STOP logging"));
    Serial.println(F("r   dump logging"));
    Serial.println(F("m   show/hide measurement"));
    Serial.println(F("t   set time between measurements (0-255sec)"));
    Serial.println(F("c   toggle continuous/rollover mode"));
    Serial.println(F("u   unified memory dump"));
    Serial.print("\n\n");
    Serial.print(F("Memory size (kB): "));
    Serial.println(DATA_FRAM_SIZE);
    Serial.print(F("possible dataset to store:     "));
    Serial.println((uint16_t)(DATA_FRAM_SIZE - DATA_FRAM_START) / DATA_PACKAGE_SIZE);
    Serial.print(F("time between measurements (s): "));
    Serial.println(WAIT_BETWEEN_MEASURE);
    datasetNo = (fram.read(DATA_ADR_DATASET) << 8) + fram.read(DATA_ADR_DATASET + 1);
    Serial.print(F("number of stored datasets:     "));
    Serial.println(datasetNo);
    Serial.print("\n");
}

/* ******************************************************************
*           check Button
*  ******************************************************************/
void checkButton() {

    if (digitalRead(PIN_BUTTON) == LOW) {
        LED(RED);
        while (digitalRead(PIN_BUTTON) == LOW) {
            delay(10);
        }
        controlLogging();
        LED(OFF);
    }
}


/* ******************************************************************
*           start/stop logging
*  ******************************************************************/
void controlLogging() {


    if (!loggingActive) {
        loggingActive = true;
        Serial.println(F("\t**** Logging started ****"));
        datasetNo = 0;  //start counting from begin
        DateTime now = rtc.now();
        unixTimeStart = now.unixtime();

    } else {
        loggingActive = false;
        Serial.print(F("\t**** Logging stopped ****  recorded measurements: "));
        Serial.println(datasetNo);
    }
}


/* ================================================================================
            ENS220 Routine: SingleShot Setup
   ================================================================================
*/
void initENS220() {

    // Start the communication, confirm the device PART_ID, and read the device UID
    ens220.begin(&Wire, ENS220_I2C_ADDRESS);

    while (ens220.init() != true) {
        Serial.println("Waiting for I2C to start");
        delay(1000);
    }

    Serial.print("Device ENS220 UID: ");
    Serial.println(ens220.getUID(), HEX);

    // Choose the desired configuration of the sensor. In this example we will use the Lowest Noise settings from the datasheet
    ens220.setDefaultConfiguration();
    // Set the Pressure ADC conversion time (MEAS_CFG register, field P_CONV)
    ens220.setPressureConversionTime(ENS220_PRESSURE_CONVERSION_TIME_T_16_4);
    // Set the Oversampling of pressure measurements (OVS_CFG register, field OVSP)
    ens220.setOversamplingOfPressure(ENS220_OVERSAMPLING_N_128);
    // Set the Oversampling of temperature measurements (OVS_CFG register, field OVST)
    ens220.setOversamplingOfTemperature(ENS220_OVERSAMPLING_N_128);
    // Set the ratio between P and T measurements as produced by the measurement engine (MEAS_CFG register, field PT_RATE)
    ens220.setPressureTemperatureRatio(ENS220_PRESSURE_TEMPERATURE_RATIO_PT_1);
    // Set the operation to One shot (STBY_CFG register, field STBY_T)
    ens220.setStandbyTime(ENS220_STANDBY_TIME_ONE_SHOT_OPERATION);
    // Set whether to use the FIFO buffer, a moving average, or none (MODE_CFG register, field FIFO_MODE)
    ens220.setPressureDataPath(ENS220_PRESSURE_DATA_PATH_DIRECT);

    // Write the desired configuration into the sensor
    ens220.writeConfiguration();
}


/* ******************************************************************
*           print used FRAM memory as HEX
*  ******************************************************************/

void displayFRAM() {
    // FRAM Speicher ausgeben
    datasetNo = (fram.read(DATA_ADR_DATASET) << 8) + fram.read(DATA_ADR_DATASET + 1);

    for (int i = DATA_FRAM_START; i < (DATA_FRAM_START + DATA_PACKAGE_SIZE * datasetNo); i++) {
        if ((i - DATA_FRAM_START + DATA_PACKAGE_SIZE) % DATA_PACKAGE_SIZE == 0) {
            Serial.print("\n");
            Serial.print(i);
            Serial.print(":\t");
        }
        print2digitsHex(fram.read(i));
        //Serial.print(fram.read(i), HEX);
        Serial.print(" ");
    }
    Serial.println();
}