all:
	spark  cloud flash 50ff68065067545627390287 .

compile: 
	spark compile .

monitor:
	spark serial monitor

update-dependencies:
	git clone https://github.com/bblanchon/ArduinoJson.git tmp/ArduinoJson
	cp -v tmp/ArduinoJson/JsonParser/*.cpp .
	cp -v tmp/ArduinoJson/JsonParser/*.h .
	rm -rf tmp/ArduinoJson
	
	git clone https://github.com/nmattisson/HttpClient.git tmp/HttpClient
	cp -v tmp/HttpClient/firmware/{*.cpp,*.h} .
	rm -rf tmp/HttpClient

clean:
	rm firmware_*.bin