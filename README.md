About
===
This software enables you to display the weather, current time and the bus' next departure on a Display. 

This project is a compilation of different components: 
 * Weather from http://openweathermap.org/
 * Swiss Traffic Information from http://transport.opendata.ch/ (running a modified version at http://opentt.herokuapp.com/)
 * Time (provided by Spark Core firmware)

The required hardware is:
 *  Spark Core (https://www.spark.io/)
 *  16x2 OLED Display (http://www.adafruit.com/products/823)
 *  Some wires
 
The display
===

![image](doc/display.png)

The colored LED shows when it is the right time to leave and catch a bus. (Green=Good to go, Orange=Run!, Red=too late)

The display shows the time, weather, temperature and next bus departures. The position are configurable in the code. 

## Circuit Diagram
TODO (a real Circuit Diagram is still to be done)

see image: 
![image](doc/wires.jpg)

## Example Usage
see [spark-transport-display.cpp](firmware/spark-transport-display.cpp)

You have to change some location and API-Key information: 

Setup the display acording the your wiring: 

	lcd = new Adafruit_CharacterOLED(OLED_V1, D0, D1, D2, D3, D4, D5, D6);

Set your location (e.g. London,uk) and replace your api key from http://openweathermap.org

    weather = new Weather("Bern,ch", httpClient, "YOUR_OPENWEATHER_API_KEY");

Set your station and adjust the URL (from, to): 

	ampel.init(lcd, ">Bern",
			"/v1/connections?from=Wabern,Gurtenbahn&to=Bern&fields[]=connections/from/departure&limit=6",
			httpClient);



## Build and deploy
You must install [spark-cli](https://github.com/spark/spark-cli). 


	cd firmware
	spark  cloud flash 1234567 .

This will build and deploy the source in the spark cloud. (You must replace the device-id `1234567`. )


Additional Resources
----------------
* This project was inspired by Bastian Widmer (bastianwidmer.ch) and Christian Leu (leumund.ch)
* http://leumund.ch/d-i-y-busstop-lamp-arduino-0011112
* https://github.com/dasrecht/Arduino-Busstop-Light


Licence
----------------
      To the extent possible under law, the person who associated CC0 with
      "spark transport display" has waived all copyright and related or neighboring rights to "spark transport display"
      
      See http://creativecommons.org/publicdomain/zero/1.0/ for a copy of the CC0 legalcode.  