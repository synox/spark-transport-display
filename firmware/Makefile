all: compile
	

flash:
	spark  cloud flash 50ff68065067545627390287 .
	mv $$(ls -t1 firmware_*.bin | head -1) firmware-latest.bin

flash_usb: compile
	spark flash 50ff68065067545627390287 --usb $$(ls -t1 firmware_*.bin | head -1)

compile: clean
	spark compile .
	mv $$(ls -t1 firmware_*.bin | head -1) firmware-latest.bin

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
	test -d tmp && rmdir tmp || true