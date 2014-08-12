all: compile
	

flash:
	spark  cloud flash 50ff68065067545627390287 .

compile: clean
	spark compile .

monitor:
	spark serial monitor

update-dependencies:
	git clone https://github.com/bblanchon/ArduinoJson.git tmp/ArduinoJson
	cp -v tmp/ArduinoJson/JsonParser/*.cpp lib/
	cp -v tmp/ArduinoJson/JsonParser/*.h lib/
	rm -r tmp/ArduinoJson
	
	git clone https://github.com/nmattisson/HttpClient.git tmp/HttpClient
	cp -v tmp/HttpClient/firmware/{*.cpp,*.h} lib/
	rm -r tmp/HttpClient

clean:
	rm -f firmware_*.bin
	test -d tmp && rmdir tmp