#include "Ampel.h"



// ------------- configuration ------------


Status Ampel::getStatus(int diffSeconds) {
	if (diffSeconds < -60) {
		// too late
		return off;
	} else if (diffSeconds < 90) {
		return missed;
	} else if (diffSeconds < 3 * 60) {
		return run;
	} else if (diffSeconds < 3.5 * 60 ) {
		return leave_now;
	} else if (diffSeconds < 5 * 60) {
		return walk;
	} else {
		// longer, then turn led off
		return off;
	}
}

void Ampel::updateLED(Status status) {
	RGB.control(true);

	switch(status) {
		case off:      RGB.color(0,0,0); break;
		case missed:   RGB.color(255,0,0); break;  // red
		case run:      RGB.color(255,50,0); break;// orange
		case leave_now:RGB.color(255,150,0); break;// yellow
		case walk:     RGB.color(0,255,0); break; // green
	}
}





// is a display connected?
#define DISPLAY true

// ------------- enf of configuration ------------

#define CONNECTION_CACHE_SIZE 10
unsigned long connections[CONNECTION_CACHE_SIZE];



void Ampel::init (Adafruit_CharacterOLED *lcd, const char* connName, const char* query, HttpClient* client) {
    this->lcd = lcd;
    // init connection array
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		connections[i] = 0;
	}
	this->query = query;
	this->connName = connName;

	 httpClient = client;

}


void Ampel::cleanupCache(unsigned long now) {
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		if(connections[i] == 0) {
			// empty
		} else if(connections[i]  < now - 60) {
			// delete old entries
			connections[i] = 0;
		}
		if(i > 0 && connections[i-1] == 0) {
			// if prev entry is empty: move current
			connections[i-1] = connections[i];
			connections[i] = 0;
		}
	}
}


unsigned int Ampel::getCacheSize() {
	unsigned int count = 0;
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		if(connections[i] != 0) {
			count++;
		}
	}
	return count;
}


void Ampel::loadConnections(unsigned long now) {
    cleanupCache( now);
	if (getCacheSize() < 2 )  {
		if(lcd != NULL) {
			lcd->setCursor(0,1);
			lcd->print("updating data...");
		}

		Serial.println("loading connections...");
        // refresh connections
		http_request_t request;
		request.path = this->query;
		request.body = "";
		request.hostname = "opentt.herokuapp.com";
		request.port = 80;

		http_response_t response;
		httpClient->get(request, response);

		if(response.status == 200) {
			Serial.println("Loaded data successfully: ");
			Serial.println(response.body);
			parseFahrplan(response.body);
		} else {
			Serial.println("Error while loading data:");
			Serial.println(response.status);
		}
	}
}




void Ampel::parseFahrplan(String jsonData) {
	int offset = 0;
	do {
		offset = jsonData.indexOf("departure\":", offset);
		if(AMPEL_DEBUG) Serial.print("offset: ");
		if(AMPEL_DEBUG) Serial.println(offset);

		if (offset == -1) {
			break;
		}
		//
		offset += 11; // move to timestamp
		String timestamp = jsonData.substring(offset, offset + 10); // timestamp has length 10
		if(AMPEL_DEBUG) Serial.print("ts: ");
		if(AMPEL_DEBUG) Serial.println(timestamp);
		if(timestamp.length() == 0) {
			continue;
		}
		addConnection(timestamp.toInt());
	} while (offset >= 0);
}



/**
 * Adds the given timestamp to the connection array. Multiple arrays might be possible.
 */
void Ampel::addConnection(unsigned long ts) {
	if(AMPEL_DEBUG) Serial.print("fahrplan ts:");
	if(AMPEL_DEBUG) Serial.println(ts);

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {

		if (connections[i] == ts) {
			// already in table, ignore
			return;
		} else if (connections[i] == 0) {
			// found empty slot, add ts. empty slots are in the end.
			connections[i] = ts;
			return;
		}
	}
}


void Ampel::printCache() {
	Serial.println("conn:");
	Serial.println("-----------");

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
			Serial.println(connections[i]);
	}
	Serial.println("-------");
}

void Ampel::updateLed(unsigned long now) {
    Status status  = calculateStatus(now);
	updateLED(status);
}



Status Ampel::calculateStatus(unsigned long now) {
	// Connections might be overlapping. In case you have every 2 minutes a connection,
	// you want to see green all the time. the enum Status is ordered by increasing priority.
	Status bestStatus = off;

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		unsigned long ts = connections[i];
		if(ts == 0) {
			continue;
		}
		int diff = ts - now;
		Status newColor = getStatus(diff);
		if (newColor > bestStatus) {
			bestStatus = newColor;
		}
		if(bestStatus == walk) {
			// it is as good as it gets, stop evaluating
			break;
		}
	}
	return bestStatus;
}

void Ampel::updateDisplay() {
	unsigned long now = Time.now();
	int row = 0;
	// assuming it is a 16 column, 2 row display
	for (int i = 0; i < CONNECTION_CACHE_SIZE && row < 2; i++) {
		long diffSecs = connections[i] - now;
		if(diffSecs < 0 ) {// || diffSecs > 3600 ) {
			continue;
		} else if(diffSecs > 3 * 3600 ) {
			// ignore far future connections
		} else {
			String minutes(diffSecs / 60 ); // minutes
			String time = minutes + "m";
			lcd->setCursor(16-time.length(),row); //

			// Print duration
			lcd->print(time);
			row++;
		}
	}
}


