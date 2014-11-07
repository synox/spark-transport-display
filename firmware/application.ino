
#include "Adafruit_CharacterOLED.h"
#include "Transport.h"
#include "openweathermap.h"
#include "HttpClient.h"


Adafruit_CharacterOLED *lcd = NULL;
// Assign special symbols
byte degreeSignArray[8] = { 8,20,8,0,3,4,4,3 };
byte degreeSignKey = 0;
byte iconRainArray[7] = {
		0b00100,
		0b01110,
		0b11111,
		0b00100,
		0b00100,
		0b10100,
		0b01100
};
byte iconRainKey = 1;

int activityLed  = D7;
unsigned int updateTimeout = 0;    // next time to contact the server


Transport transport;
Weather* weather;
HttpClient* httpClient;

void setup() {
	Time.zone(+1.0); // +1.0/+2.0 Winer / Sommerzeit // TODO: detect DST 
	Serial.begin(9600);
	pinMode(activityLed, OUTPUT);

	// use one httpClient for weather and transportation (save memory)
	httpClient = new HttpClient();

	
	// setup lcd                              rs, rw, enable, d4, d5, d6, d7
	lcd = new Adafruit_CharacterOLED(OLED_V1, D0, D1, D2,     D3, D4, D5, D6);
	lcd->createChar(degreeSignKey, degreeSignArray);
	lcd->createChar(iconRainKey, iconRainArray);

	lcd->clear();

    weather = new Weather("Bern,ch", httpClient, "ae22162b86e9ea6298b8dc79d2330691");

	// setup transport api
	transport.init(lcd, "Wabern,Gurtenbahn", 	"Bern", 	httpClient);

}

void loop() {
	// setup led
	RGB.control(true);
	RGB.brightness(100);


	if (updateTimeout > millis()) {
		// keep the same text & color while waiting
		return;
	}

	digitalWrite(activityLed, HIGH); // indicate activity

	transport.loadConnections(Time.now());
	transport.updateLed(Time.now());
	//transport.printCache();


	if (lcd != NULL) {
	// Display layout: (16x2 char)
	// +----------------+
	// |14:15 15/22`  5m|
	// |mod rain     12m|
	// +----------------+
		lcd->clear();
		displayCurrentTime(0, 0);

		// Transport
		if(!transport.displayDepartures(16, 2)) { // 16x2 Display
			lcd->setCursor(16-3,0); //
			lcd->print("---");
		}


		// Weather
		weather_response_t resp = weather->cachedUpdate();
		if ( resp.isSuccess) {
			// print temperature in first row after the time
			lcd->setCursor(6, 0); 
			lcd->print(resp.temp_low);
			lcd->print("/");
			lcd->print(resp.temp_high);
			lcd->write(byte(degreeSignKey));
			// print descr on second line
			lcd->setCursor(0, 1);
			int iconCode = getConditionIcon(resp.conditionCode);
			if(iconCode >= 0) {
				Serial.print("iconcode: ");
				Serial.println(byte(iconCode));
				lcd->write(byte(iconCode));
			}
			lcd->print(shortDescr(resp.descr).substring(0,12));
		} else {
			lcd->setCursor(0, 1);
			lcd->print("---");
		}
	}


	// check again in 5 seconds:
	updateTimeout = millis() + 5000;
	digitalWrite(activityLed, LOW);
}

int getConditionIcon(int conditionCode) {
	if ( conditionCode <= 199) {
		return -1;
	}
	if ( conditionCode <= 299) {
		//Thunderstorm
		return iconRainKey;
	}
	if ( conditionCode <= 399) {
		//Drizzle
		return iconRainKey;
	}
	if ( conditionCode <= 499) {
		// undefined
		return -1;
	}
	if ( conditionCode <= 599) {
		//Rain
		return iconRainKey;
	}
	if ( conditionCode <= 699) {
		//Snow
		return -1;
	}
	if ( conditionCode <= 799) {
		//Atmosphere
		return -1;
	}
	if ( conditionCode == 800) {
		// Sunny
		return -1;
	}
	if ( conditionCode <= 899) {
		// Clouds
		return -1;
	}

	// 900+ = Extreme condition
	return -1;
}

String shortDescr(String &descr) {
	if(descr.startsWith("moderate ")){
		 descr.remove(3,5);
		 return descr;
	}
	return descr;
}

void displayCurrentTime(int row, int col) {
	 int hour = Time.hour();
	 int min = Time.minute();
    lcd->setCursor(col, row);
    if(hour < 10) {
    	lcd->print(0);
    }
    lcd->print(hour);
    lcd->print(":");
    if(min < 10) {
    	lcd->print(0);
    }
    lcd->print(min);
}
