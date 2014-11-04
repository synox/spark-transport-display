#include "Transport.h"



// ------------- configuration ------------


Status Transport::getStatus(int diffSeconds) {
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

void Transport::updateLED(Status status) {
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



void Transport::init (Adafruit_CharacterOLED *lcd, String from, String to, HttpClient* client) {
	// /v1/connections?from=Wabern,Gurtenbahn&to=Bern&fields[]=connections/from/departure&limit=6
    this->lcd = lcd;
    // init connection array
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		connections[i] = 0;
	}
	String query ="/v1/connections?from=";
	query +=from + "&to=" + to +"&fields[]=connections/from/departureTimestamp&limit=6"; 
	this->query = query;
	httpClient = client;
}


void Transport::cleanupCache(unsigned long now) {
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


unsigned int Transport::getCacheSize() {
	unsigned int count = 0;
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		if(connections[i] != 0) {
			count++;
		}
	}
	return count;
}


void Transport::loadConnections(unsigned long now) {
    cleanupCache( now);
	if (getCacheSize() < 2 )  {
		Serial.println("loading connections");
        // refresh connections
		http_request_t request;
		request.path = this->query;
		request.body = "";
		request.forceIp = true;
		request.ip = IPAddress(178,209,54,56);
		request.hostname = "transport.opendata.ch";
		request.port = 80;

		http_response_t response;
		httpClient->get(request, response);

		if(response.status == 200) {
			Serial.println("Loaded transport data successfully: ");
			Serial.println(response.body);
			parseFahrplan(response.body);
		} else {
			Serial.println("Error while loading data:");
			Serial.println(response.status);
		}
	}
}




void Transport::parseFahrplan(String jsonData) {
	int offset = 0;
	do {
		offset = jsonData.indexOf("departureTimestamp\":", offset);
		if(TRANSPORT_DEBUG) Serial.print("offset: ");
		if(TRANSPORT_DEBUG) Serial.println(offset);

		if (offset == -1) {
			break;
		}
		//
		offset += 20; // move to timestamp
		String timestamp = jsonData.substring(offset, offset + 10); // timestamp has length 10
		if(TRANSPORT_DEBUG) Serial.print("ts: ");
		if(TRANSPORT_DEBUG) Serial.println(timestamp);
		if(timestamp.length() == 0) {
			continue;
		}
		addConnection(timestamp.toInt());
	} while (offset >= 0);
}



/**
 * Adds the given timestamp to the connection array. Multiple arrays might be possible.
 */
void Transport::addConnection(unsigned long ts) {
	if(TRANSPORT_DEBUG) Serial.print("fahrplan ts:");
	if(TRANSPORT_DEBUG) Serial.println(ts);

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


void Transport::printCache() {
	Serial.println("conn:");
	Serial.println("-----------");

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
			Serial.println(connections[i]);
	}
	Serial.println("-------");
}

void Transport::updateLed(unsigned long now) {
    Status status  = calculateStatus(now);
	updateLED(status);
}



Status Transport::calculateStatus(unsigned long now) {
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


/** print departure time right aligned. e.g.     23m */
bool Transport::displayDepartures(const int rowLength, const int rowCount) {
	unsigned long now = Time.now();
	int row = 0;

	if(rowCount == 0) {
		return false;
	}

	for (int i = 0; i < CONNECTION_CACHE_SIZE && row < rowCount; i++) {
		long diffSecs = connections[i] - now;
		if(diffSecs < 0 ) {
			// ignore past connections
			continue;
		} else if(diffSecs > 3 * 3600 ) {
			// ignore far future connections
		} else {
			String minutes(diffSecs / 60 ); // minutes
			String time = minutes + "m";

			// set cursor right aligned
			lcd->setCursor(rowLength-time.length(),row); //

			// Print duration
			lcd->print(time);
			row++;
		}
	}
	return true;
}


