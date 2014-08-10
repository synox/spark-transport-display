
#include "Adafruit_CharacterOLED.h"
#include "Ampel.h"
#include "openweathermap.h"
#include "HttpClient.h"

Adafruit_CharacterOLED *lcd = NULL;
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

int led = D7;
unsigned int nextTime = 0;    // next time to contact the server

Ampel ampel;

 Weather* weather;
HttpClient* httpClient;

void setup() {
	Time.zone(+2.0); // DST / Sommerzeit // TODO: detect DST 
	Serial.begin(9600);
	pinMode(led, OUTPUT);

	httpClient = new HttpClient();

	// setup led
	RGB.control(true);
	RGB.brightness(255);

	// setup lcd
	lcd = new Adafruit_CharacterOLED(OLED_V1, D0, D1, D2, D3, D4, D5, D6);
	lcd->createChar(degreeSignKey, degreeSignArray);
	lcd->createChar(iconRainKey, iconRainArray);

	lcd->clear();

    weather = new Weather("Bern,ch", httpClient, "ae22162b86e9ea6298b8dc79d2330691");

	// setup ampel
	ampel.init(lcd, ">Bern",
			"/v1/connections?from=Wabern,Gurtenbahn&to=Bern&fields[]=connections/from/departure&limit=6",
			httpClient);

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

void displayTime(int row, int col) {
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

void loop() {
	if (nextTime > millis()) {
		// keep the same color while waiting
		return;
	}

	digitalWrite(led, HIGH);


	ampel.loadConnections(Time.now());
	ampel.updateLed(Time.now());
	ampel.printCache();


	if (lcd != NULL) {
	// Display design: (16x2 char)
	// +----------------+
	// |14:15 5-12` 5min|
	// |mod rain   12min|
	// +----------------+
		lcd->clear();
		displayTime(0, 0);
		ampel.updateDisplay();

		// print weather

		weather_response_t resp = weather->cachedUpdate();
		if ( resp.isSuccess) {
			lcd->setCursor(7, 0);
			lcd->print(resp.temp_low);
			lcd->print("/");
			lcd->print(resp.temp_high);
			lcd->write(byte(degreeSignKey));
			// on new line
			lcd->setCursor(0, 1);
			int iconCode = getConditionIcon(resp.conditionCode);
			if(iconCode >= 0) {
				Serial.print("iconcode: ");
				Serial.println(byte(iconCode));
				lcd->write(byte(iconCode));
			}
			lcd->print(shortDescr(resp.descr).substring(0,12));
		}


	}


	// check again in 5 seconds:
	nextTime = millis() + 5000;
	digitalWrite(led, LOW);
}
