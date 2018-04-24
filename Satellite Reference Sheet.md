# Satellite Reference Sheet
**Newport High School 2017 - Last Revision: May 19th, 2017 ~ Version 2**

## Table of contents
* [Definitions](#definitions)
* [Status Codes](#statuscodes)
* [Launch Checklist](#launchchecklist)
* [Satellite Specifications](#satellitespecifications)
* [Data Collection](#datacollection)

## Definitions
* Computer (with an uppercase `C`) - the Adafruit Featherlight, the logic board
* LED - the single-color **L**ight **E**mitting **D**ioide located on the Computer. *This is different from the SD indicator light*
* SD card - the Micro-SD card that the Computer will flush its data to
* SD card slot - the physical slot on the Computer where the SD card is inserted

## Status Codes
Identifiable by LED
* `.`: Dot
* `-`: Dash

`---`: SD Card failed to initalize - check if SD card is missing or not plugged in all the way (`SD_CARD_FAIL_INIT`)

`.`: Waiting for `START_PIN` to be pulled (`WAITING_FOR_START_PIN`)

`-.-.-.-`: Mission is starting and data collection will begin shortly (`DATA_COLLECTION_STARTING`)

`-`: Flushed data to SD card (`DATA_FLUSH_TO_SD_CARD_SUCCESS`)

`BLINK`: *If and only if debug is activated*, the LED will continuously blink while the mission program is running. (`while (true)` loop in `void loop()`) (`DEBUG`)

## Launch Checklist
* Check battery levels, use fresh, unused batteries, and replace Geiger and Computer batteries as needed
* Format SD card to `FAT` and insert into Computer **before** the Computer has been turned on
* **Make sure all connections to the board are secure**
* Plug in power to the computer and wait for [`WAITING_FOR_START_PIN`](#statuscodes)
* Ensure [`WAITING_FOR_START_PIN`](#statuscodes) is flashing. This indicates that the SD card works and should collect data properly.
* (if applicable) Switch on `GEIGER`
* (if applicable) Test `GEIGER` sensor by pressing the ends of the tube, the `GEIGER_LED` should light up and emit a noise
* Take off `COMMIT_PIN` and wait for [`DATA_COLLECTION_STARTING`](#statuscodes) to finish

## Satellite Specifications (2017)
**Bolded** entries are unqiue to the satellite
### Ruder - <font color=red>RED</font>
* Light
* Acceleration
* Gyroscope
* **Real Time Clock**
* **Geiger**
* **Pressure & Temperature (`sensors.txt`)**

### Newport - <font color=black>BLACK</font>
* Light
* Acceleration
* Gyroscope
* **Humidity & Temperature (`humidity.txt`)**
* **Due to the absence of the Real Time Clock, time is omitted from data on this satellite**

## Data collection
The following points of data are collected by this program assuming all sensors are connected correctly:

`sensor.txt` - Non-specialized sensors
* Pressure (pHa)
* Temperature (Celcius)
* Light (lux)
* Infrared Light (counts)
* Visible Light (counts)

`geiger.txt` - Geiger Counter
* CPS
* CPM
* uSv/hr
* str

`gyro.txt` - Gyroscope
* x
* y
* z

`humidity.txt` - Humidity
* Humidity (%)
* Temperature (Celcius)

`accel.txt` - Accelerometer (appears as `Accelrometer` because of misspelling) (1 = `m/s^2`, 2 = `G-Force`)
* x1
* y1
* z1
* x2
* y2
* z2

All files contains the time (HH:MM:SS) the data was captured in index `0`. If `RTC == false`, the time will be omitted.
