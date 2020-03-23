# ESPEInk_ESP32
Erweiterung des ESP32-Waveshare-Treibers um Wifi-Einrichtungsassistent, Deepsleep und MQTT-Funktionalität als Ergänzung zum FHEM-Modul `ESPEInk`.

[![GitHub release](https://img.shields.io/github/v/release/Yattien/ESPEInk_ESP32?include_prereleases)](https://github.com/Yattien/ESPEInk_ESP32/releases)
![GitHub All Releases](https://img.shields.io/github/downloads/Yattien/ESPEInk_ESP32/total)

# Waveshare-Treiberversion
[19.02.2020](https://www.waveshare.com/wiki/File:E-Paper_ESP32_Driver_Board_Code.7z)

# Installation
Das fertige Image kann per OTA (siehe auch OTA-Beispiel-Sketche) oder auch per [`esptool`](https://github.com/espressif/esptool) auf den ESP32 geladen werden.

# Features
* **Einrichtungsmanager** für WiFi, MQTT und Deepsleep-Zeit
  - Verbinden mit dem AP `ESPEInk-APSetup` und im Browser die URL `http://192.168.4.1` aufrufen
  - MQTT-Server ist _optional_, falls keiner angegeben wird, wird kein MQTT verwendet
  - Sleeptime in Sekunden ist _optional_, wird keine angegeben, läuft der ESP ständig (=ähnlich der Original-Firmware)
  - Firmware-Basis-URL ist _optional_, wird keine angegeben, wird keine OTA-Update-Anfrage durchgeführt
  - die Einrichtungsdaten werden im ESP gespeichert und nicht mehr im Quellcode; sie sind damit auch nach einem Update noch verfügbar
* **OTA-Firmware-Update**
  - unterhalb der Firmware-Basis-URL werden zwei Dateien erwartet, Hauptnamensbestandteil ist die klein geschriebene (WiFi-)MAC-Adresse des ESP
  - `<MAC>.version` eine Datei, die nur eine Zahl, die Versionsnummer des zugehörigen Firmwareimages, enthält
  - ist die Versionsnummer in der Datei `-1`, wird ein Reset des ESP durchgeführt, um den Einrichtungsmanager erneut aufzurufen
  - `<MAC>.bin` das Firmware-Image
  - ein Update erfolgt nur, wenn die aktuelle Versionsnummer kleiner als die auf dem Webserver ist (ja, ist derzeit viel Handarbeit :wink:)
* **Deepsleep**
  - der ESP-Webserver wird 10s für Aktionen gestartet, danach geht er schlafen - falls eine Schlafdauer konfiguriert ist
  - wird ein manueller Upload über die Webseite oder ein ESPEInk-Upload gestartet, wird der Schlaf solange verzögert, bis das `SHOW`-Kommando zurückkehrt oder über die Abort-Seite abgebrochen wird
* **MQTT-Szenario**, wie unten stehend beschrieben
  - Funktioniert nicht :warning: standalone, benötigt also eine eingerichtete FHEM-Gegenseite

# Abhängigkeiten
Da ich bereits viele Libs in meiner Arduino-Bauumgebung habe und diese somit nicht mehr "clean" ist, können es auch mehr Abhängigkeiten sein, die zusätzlich installiert werden müssen. Für Tipps bin ich dankbar :)

* ArduinoJSON >=6
* [WifiManager-ESP32](https://github.com/Brunez3BD/WIFIMANAGER-ESP32)
* [DNSServer-ESP32](https://github.com/zhouhan0126/DNSServer---esp32)
* [WebServer-ESP32](https://github.com/zhouhan0126/WebServer-esp32)
* ?

# Pin-Belegung ESP32 Devkit v1
e-Paper HAT|GPIO ESP32
-----------|----------
CLK|13
DIN|14
CS|15
BUSY|25
RST|26
DC|27
GND|GND
VCC|3.3V

# MQTT-Szenario
Im Zusammenspiel mit dem FHEM-Modul `ESPEInk` kann man das EInk-Display dazu bringen, weniger Strom zu verbrauchen. Dazu kann man so vorgehen:
* das Attribut `interval` in ESPEInk steht auf einem hohen Wert, da kein automatischer Upload erfolgen soll (noch notwendiger Workaround)
* ein DOIF triggert die Konvertierung
* ein weiteres DOIF reagiert auf Änderungen im ESPEInk-Reading `result_picture` und setzt das MQTT-Topic `stat/display/needUpdate` mit dem QOS 1 (=wird im MQTT-Server zwischengespeichert, solange der ESP schläft)
* ein MQTT_DEVICE wartet auf das ESP-Signal im Topic `cmd/display/upload`
* der ESP erwacht und befragt das Topic `stat/display/needUpdate`, ob es was zu tun gibt. Falls nicht, geht er wieder schlafen (Wachzeit ~ 5s).
* falls es etwas zu tun gibt, startet der ESP seinen Webserver und setzt das MQTT-Topic `cmd/display/upload`
* das MQTT_DEVICE reagiert darauf und startet den ESPEink-upload
    
## Benötigt wird
* ein eingerichtetes und funktionierendes FHEM-Modul `ESPEInk`
	```
	defmod display ESPEInk /opt/fhem/www/images/displayBackground.png
	attr display boardtype ESP8266
	attr display colormode color
	attr display convertmode level
	attr display definitionFile /opt/fhem/FHEM/eink.cfg
	attr display devicetype 7.5inch_e-Paper_HAT_(B)
	attr display picturefile /opt/fhem/www/images/displayBackground.png
	attr display interval 48600
	attr display timeout 50
	attr display url eink-panel
	```
* mosquitto als MQTT-Server (MQTT2_* kann leider kein QOS oder ich hab nicht herausgefunden wie)
* MQTT als Verbindung zu mosquitto
	```
	defmod mqtt_device MQTT <mqtt-server>:1883
	```
* ein DOIF als Trigger für die Konvertierung (derzeit ist die Konvertierung und der Upload noch gekoppelt), im Beispiel reagiere ich auf die Änderungen in einem dummy-Device `display_status`, welches bereits alle relevante Informationen enthält	
	```
	defmod display_status_doif DOIF ([display_status]) (\
		set display convert\
	)
	attr display_status_doif do always
	```
* ein DOIF, welches bei einem Update des konvertierten ESPEInk-Bildes reagiert und das Topic mit qos=1 setzt
	```
	defmod display_doif DOIF ([display:result_picture]) (\
		set mqtt_device publish qos:1 stat/display/needUpdate true\
	)
	attr display_doif do always
	```
* ein MQTT_DEVICE um auf die Nachrage des ESP zu reagieren (wichtig ist, dass das Attribut `subscribeReading_cmd` heißt, ansonsten wird das FHEM-Kommando nicht ausgeführt, zumindest nicht in meinen Tests)
	```
	defmod display_mqtt MQTT_DEVICE
	attr display_mqtt IODev mqtt_device
	attr display_mqtt subscribeReading_cmd {fhem("set display upload")} cmd/display/upload
	```
