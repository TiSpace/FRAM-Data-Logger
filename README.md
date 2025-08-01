# FRAM-Data-Logger
logging data into FRAM, utilizing RTC and ScioSense ENS16x and ENS210 sensors

<img src="./pic/FRAM_DATA_Logger_V01.png" width="800">
This is an easy example of using a FRAM memory along with RTC and ScioSense ENS16x EVKit to create a datalogger. FRAM is less power hungry and enablenearly unlimited write cycles
The used FRAM does support 32.768kB of memory and the currently used setup (V01) allows to store 3276 datasets (10bytes per dataset) including timestamp
