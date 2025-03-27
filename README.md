# Boulter Backplane

This is the egg packer backplane wiring harness tester controller software for Boulter Machine Works. 

It is of no use to you unless you are Colby, but there's nothing particularly proprietary in here so feel free to have a look.

This whip tester version is for the internal harness within the PLC/VFD box. The whip tester version is not as comprehensive, as it cannot be completely quick disconnected from a box for testing. The Whip tester connects to the circular 35 pin connector and to the 20 pin terminal strip that goes to the PLC I/O expansion module. 

It runs on an Adafruit Huzzah32 EPS32 Feather, your basic original ESP32 board with a USB to serial converter on the board. 4MB flash, no PSRAM, dual core CPU. Massive overkill for this job, but we wanted the WiFi interface for the web server.

It uses the SPIFFS filing system. It is important to load the file system image in addition to the firmware.
If you switch to an ESP32-S2 board you'll have to use FATFS, and edit the code slightly. 

This project is built using PlatformIO run from VS Code. It uses the Arduino framework and some Arduino libraries.

It uses the I2C bus to attach to a string of four (up to eight) PCF8575 GPIO extenders. This provides 64 pins which are used to energize and sense the connections on the wiring harness. Each IO extender has 16 GPIO pins in two banks of 8. I2C addresses can be from 0x20 to 0x27. We're using 0x20-0x23*. If we had a more complex wiring harness we could add another 64 wire endpoints. 

*the whip tester variant uses 0x24-0x26

The web based control interface appears on "http://htester.local" via mDNS on the local internal network. It uses port 80.

Note:

The datasheet numbers the GPIO pins 0-7 and 10-17, but the Arduino PCF8575 library uses 0-15 contiguous.

The pins are not truly bidirectional. They will sink current when you turn them ON (LED cathode to GPIO pin, LED anode to +3.3V) and are thus inverters. 

When reading the state, they are normally pulled high. Shorting the pin to ground will give a reading of 1, thus reads are also inverted.

# HTTP endpoints:

    / (backslash)

serves file index.html from SPIFFS partition

    /index.html

Main HTML file

    /index.js

Javascript file

    /index.css

Style sheet

    /logo.png

Logo for black on white (not presently used)

    /logoblack.png

Logo for white on black

    /favicon.ico
    
shows Boulter EGG logo in tab corner

    /setpin?pin= < pin > & val = < val >

Example:

Pins are numbered 0-127 across eight sequential GPIO boards.

Thus:
    
        htester.local/setpin?pin=20&val=1

will set pin number 4 on GPIO chip number two (I<sup>2</sup>C 0x21).

And:

        htester.local/setpin?pin=0&val=0

will clear pin zero on chip number one (I<sup>2</sup>C 0x20)

~~Bug fix~~ Feature needed: always returns OK, even if the GPIO fails. There is presently no way to see which GPIO chips are online.



    /getpin?pin= < pin >

Example:

    http://htester.local/getpin?pin=2

will return state of pin number two one GPIO chip number one (I2C 0x20). Returns 0 or 1

    /getmap

This returns a JSON file with the default KnownGood map of wires and their descriptions. 

    /setmap [not yet implemented]

This accepts a JSON file which overrides the built in wiring map and pin/connector descriptions 

    /scanmap

This runs a scan and returns a JSON file of success or any errors found

    /pairscan

*emph *with emph* in it*
**strong **with strong** in it**

This scans every wire on the first 80 GPIOs and finds matching pairs (or triples, etc.). This is how you build the **KnownGood[]** array map used for later comparisons. Each line lists a pin following by a comma separated list of pins that it is connected to. It requires some editing to massage into a **KnownGood[]** array. Mainly, the empty places must be replaced with -1 (negative one) to denote no connection. 

Note that **KnownGood[]** is presently four columns wide. If the wiring map changes and has more than four wires connected together, it will be necessary to widen the array. This will require changing

    #define GOOD_WIDTH <columns>

**KnownGood[]** is presently a (default) 32 bit integer array, which is a minor waste of memory, as we could use signed 8 bit values (char, int8_t) instead. 

Also, if you need more than three GPIO extenders you'll need to change

    #define PIN_COUNT <multiple of 16 GPIOs>

From 48 (or 64) to some even multiple of 16 up to 128. We do not exactly assume that the GPIO extenders will have contiguous addresses on the I<sup>2</sup>C bus, but scanning will be more efficient if we do.  

    /scanreport

This returns a JSON map of the current wiring harness in place, for comparison with the data returned by /getmap. 

    /refresh

This returns the result of the most recent scan, which could have been triggered in three different ways:
1. Clicking on the "scan" button on the index.html page
2. Doing a GET on /scanmap
3. Pressing the physical scan button on the black box.

# Black Box

The ESP32 is mounted in a black box with three LEDs and a push button.

The white LED is a power indicator. It will blink if no WiFi connection has been established. The scan button and red/green LEDS will still work in the absence of a WiFi connection.

The red LED means the most recent scan FAILED. There is a problem with the wiring harness or the test setup.

The green LED means the most recent scan PASSED. The wiring harness under test is good.

The pushbutton initiates a scan. The results are polled by the web interface (index.html) and will pop up in the browser after an average of 2 seconds. A typical scan takes ~1500 milliseconds to run, testing 4096-64=4032 possible connections, of which some 48 connections actually matter. 
