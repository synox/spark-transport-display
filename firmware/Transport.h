#ifndef _TRANSPORT
#define _TRANSPORT

#include "Adafruit_CharacterOLED.h"
#include "HttpClient.h"
#include <time.h>

#define TRANSPORT_DEBUG 1
// possible status list:
enum Status {
	// increasing priority, if there is "walk" and "run", it shows "walk".
	off=1, missed=2, run=3, leave_now=4, walk=5
};

class Transport {
    public:
    void init(Adafruit_CharacterOLED *lcd, String from, String to, HttpClient* client);
    void cleanupCache(unsigned long now);
    unsigned int getCacheSize();
    void loadConnections(unsigned long now);
    void printCache();
    void updateLed(unsigned long now);
    bool displayDepartures(const int rowLength, const int rowCount);
    
    private: 
    String query;
    Adafruit_CharacterOLED *lcd;
	HttpClient* httpClient;

    void parseFahrplan(String jsonData);
    void addConnection(unsigned long ts);
    Status calculateStatus(unsigned long now);
    void updateLED(Status status);
    Status getStatus(int diffSeconds);

};

/**
 * parse a string of the form "2014-01-11T17:17:59+0200" and fixes the timezone offset
 */
long parseDateWithTimezone(String str);

/**
 * parse a string of the form "2014-01-11T17:17:59+0200"
 */
long parseDate(String str) ;


/**
 * can parse the timezone offset in the string "2014-01-11T17:17:59+0100"
 */
long parseTzOffset(String str) ;




#endif
