DEVICE_ID=50ff68065067545627390287

all: compile

compile: clean
	spark compile .
	mv $$(ls -t1 firmware_*.bin | head -1) firmware-latest.bin

flash: compile
	spark  cloud flash $(DEVICE_ID) firmware-latest.bin
	
flash_usb: compile
	spark flash $(DEVICE_ID) --usb firmware-latest.bin

monitor:
	spark serial monitor

update-dependencies: clean
	git clone https://github.com/bblanchon/ArduinoJson.git tmp/ArduinoJson
	cp -v tmp/ArduinoJson/JsonParser/*.cpp lib/
	cp -v tmp/ArduinoJson/JsonParser/*.h lib/
	rm -r tmp/ArduinoJson
	
	git clone https://github.com/nmattisson/HttpClient.git tmp/HttpClient
	cp -v tmp/HttpClient/firmware/{*.cpp,*.h} lib/
	rm -r tmp/HttpClient

	// https://github.com/ladyada/Adafruit_CharacterOLED


clean:
	rm -f firmware_*.bin
	test -d tmp && rm -rf tmp || true