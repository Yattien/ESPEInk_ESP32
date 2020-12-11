/**
 * Replacement for waveshares ino-File "Loader.ino"
 * Author: Jendaw, 12.03.2020
 *
 * Depends:
 * - WifiManager-ESP32 https://github.com/Brunez3BD/WIFIMANAGER-ESP32
 * - DNSServer-ESP32   https://github.com/zhouhan0126/DNSServer---esp32
 * - WebServer-ESP32   https://github.com/zhouhan0126/WebServer-esp32
 *
 */

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <rom/rtc.h>
#include "ctx.h"

#include "srvr.h" // Server functions

// -----------------------------------------------------------------------------------------------------
const int FW_VERSION = 6; // for OTA
// -----------------------------------------------------------------------------------------------------
const char *CONFIG_FILE = "/config.json";
const float TICKS_PER_SECOND = 80000000; // 80 MHz processor
const int UPTIME_SEC = 10;
int LED_BUILTIN = 2; // neeed to be defined for board "ESP32 Dev Module" - comment it for "ESP32 Devkit v1"

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

char accessPointName[24];
bool isMqttEnabled = false;

Ctx ctx;

// -----------------------------------------------------------------------------------------------------
void setup() {
	int resetReason = (int) rtc_get_reset_reason(0);
	Serial.begin(115200);
	Serial.println("\r\nESPEInk_ESP32 v" + String(FW_VERSION) + ", reset reason=" + resetReason + "...");
	Serial.println("Entering setup...");

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	getConfig();
	initMqttClientName();

	ctx.initWifiManagerParameters();
	setupWifi();
	ctx.updateParameters();
	isMqttEnabled = ctx.isMqttEnabled();

	Serial.println(" Using configuration:");
	Serial.printf("  MQTT Server: %s:%d, Client: %s\r\n", ctx.mqttServer, ctx.mqttPort, ctx.mqttClientName);
	Serial.printf("  MQTT UpdateStatusTopic: %s\r\n", ctx.mqttUpdateStatusTopic);
	Serial.printf("  MQTT CommandTopic: %s\r\n", ctx.mqttCommandTopic);
	Serial.printf("  sleep time: %ld\r\n", ctx.sleepTime);
	Serial.printf("  firmware base URL: %s\r\n", ctx.firmwareUrl);

	if (resetReason != 6) {
		getUpdate();
	}

	EPD_initSPI();

	myIP = WiFi.localIP();
	setupMqtt();

	Serial.println("Setup complete.");
}

// -----------------------------------------------------------------------------------------------------
void getConfig() {
	if (SPIFFS.begin()) {
		if (SPIFFS.exists(CONFIG_FILE)) {
			Serial.println(" Reading config file");
			File configFile = SPIFFS.open(CONFIG_FILE, "r");
			if (configFile) {
				DynamicJsonDocument jsonDocument(512);
				DeserializationError error = deserializeJson(jsonDocument, configFile);
				if (!error) {
					strlcpy(ctx.mqttServer, jsonDocument["mqttServer"] | "", sizeof ctx.mqttServer);
					ctx.mqttPort = jsonDocument["mqttPort"] | 1883;
					strlcpy(ctx.mqttUser, jsonDocument["mqttUser"] | "", sizeof ctx.mqttUser);
					strlcpy(ctx.mqttPassword, jsonDocument["mqttPassword"] | "", sizeof ctx.mqttPassword);
					strlcpy(ctx.mqttFingerprint, jsonDocument["mqttFingerprint"] | "", sizeof ctx.mqttFingerprint);
					strlcpy(ctx.mqttClientName, jsonDocument["mqttClientName"] | "", sizeof ctx.mqttClientName);
					strlcpy(ctx.mqttUpdateStatusTopic, jsonDocument["mqttUpdateStatusTopic"] | "", sizeof ctx.mqttUpdateStatusTopic);
					strlcpy(ctx.mqttCommandTopic, jsonDocument["mqttCommandTopic"] | "", sizeof ctx.mqttCommandTopic);
					ctx.sleepTime = jsonDocument["sleepTime"] | 0;
					strlcpy(ctx.firmwareUrl, jsonDocument["firmwareUrl"] | "", sizeof ctx.firmwareUrl);
					Serial.println(" Config file read.");

				} else {
					Serial.println("  Failed to load json config.");
				}
				configFile.close();
			}
		} else {
			Serial.println("  No config file found (initial start?).");
		}
	} else {
		Serial.println("  Failed to mount FS (probably initial start), continuing w/o config...");
	}
}

// -----------------------------------------------------------------------------------------------------
void initAccessPointName() {
	sprintf(accessPointName, "ESPEInk-AP-%s", getMAC().c_str());
}

// -----------------------------------------------------------------------------------------------------
void initMqttClientName() {
	if (!strlen(ctx.mqttClientName)) {
		sprintf(ctx.mqttClientName, "ESPEInk_%s", getMAC().c_str());
	}
}

// -----------------------------------------------------------------------------------------------------
void setupWifi() {
	Serial.println(" Connecting to WiFi...");

	WiFiManager wifiManager;
	wifiManager.setDebugOutput(false);
	wifiManager.setTimeout(300);
	requestMqttParameters(&wifiManager);
	initAccessPointName();
	if (!wifiManager.autoConnect(accessPointName)) {
		Serial.println("  Failed to connect, resetting.");
		WiFi.disconnect();
		delay(3000);
		ESP.restart();
		delay(3000);
	}
	Serial.println(" Connected to WiFi.");
}

// -----------------------------------------------------------------------------------------------------
void requestMqttParameters(WiFiManager *wifiManager) {
	wifiManager->addParameter(ctx.customMqttServer);
	wifiManager->addParameter(ctx.customMqttPort);
	wifiManager->addParameter(ctx.customMqttUser);
	wifiManager->addParameter(ctx.customMqttPassword);
	wifiManager->addParameter(ctx.customMqttFingerprint);
	wifiManager->addParameter(ctx.customMqttUpdateStatusTopic);
	wifiManager->addParameter(ctx.customMqttCommandTopic);
	wifiManager->addParameter(ctx.customSleepTime);
	wifiManager->addParameter(ctx.customFirmwareUrl);
	wifiManager->setSaveConfigCallback(saveConfigCallback);
}

// -----------------------------------------------------------------------------------------------------
void saveConfigCallback() {
	Serial.println(" Saving config...");
	if (!SPIFFS.begin(false)) {
		if (!SPIFFS.begin(true)) {
			Serial.println(" an Error has occurred while mounting SPIFFS");
			delay(30000);
			ESP.restart();
		}
	}
	File configFile = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
	if (!configFile) {
		Serial.println("  Failed to open config file for writing.");
		delay(30000);
		ESP.restart();
	}
	DynamicJsonDocument jsonDocument(512);
	jsonDocument["mqttServer"] = ctx.mqttServer;
	jsonDocument["mqttPort"] = ctx.mqttPort;
	jsonDocument["mqttUser"] = ctx.mqttUser;
	jsonDocument["mqttPassword"] = ctx.mqttPassword;
	jsonDocument["mqttFingerprint"] = ctx.mqttFingerprint;
	jsonDocument["mqttClientName"] = ctx.mqttClientName;
	jsonDocument["mqttUpdateStatusTopic"] = ctx.mqttUpdateStatusTopic;
	jsonDocument["mqttCommandTopic"] = ctx.mqttCommandTopic;
	jsonDocument["sleepTime"] = ctx.sleepTime;
	jsonDocument["firmwareUrl"] = ctx.firmwareUrl;
	if (serializeJson(jsonDocument, configFile) == 0) {
		Serial.println("  Failed to write to file.");
		delay(30000);
		ESP.restart();
	}
	configFile.close();
	Serial.println(" Config saved.");
}

// -----------------------------------------------------------------------------------------------------
String getMAC() {
	uint8_t mac[6];
	WiFi.macAddress(mac);

	char result[14];
	snprintf(result, sizeof(result), "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return String(result);
}

// -----------------------------------------------------------------------------------------------------
void factoryReset() {
	Serial.println("Resetting WLAN settings...");
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true, true);
	delay(1000);
	ESP.restart();
}

// -----------------------------------------------------------------------------------------------------
void getUpdate() {
	if (!ctx.firmwareUrl || strlen(ctx.firmwareUrl) == 0) {
		return;
	}

	String mac = getMAC();
	String firmwareUrl = String(ctx.firmwareUrl);
	firmwareUrl.concat(mac);
	String firmwareVersionUrl = firmwareUrl;
	firmwareVersionUrl.concat(".version");

	Serial.printf(" Checking for firmware update, version file '%s'...\r\n", firmwareVersionUrl.c_str());
	HTTPClient httpClient;
	httpClient.begin(firmwareVersionUrl);
	int httpCode = httpClient.GET();
	if (httpCode == 200) {
		String newFWVersion = httpClient.getString();
		Serial.printf("  Current firmware version: %d, available version: %s", FW_VERSION, newFWVersion.c_str());
		int newVersion = newFWVersion.toInt();
		if (newVersion > FW_VERSION) {
			Serial.println(" Updating...");
			String firmwareImageUrl = firmwareUrl;
			firmwareImageUrl.concat(".bin");
			t_httpUpdate_return ret = httpUpdate.update(espClient, firmwareImageUrl);

			switch (ret) {
				case HTTP_UPDATE_FAILED:
					Serial.printf("  Update failed: (%d): %s\r\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
					break;

				case HTTP_UPDATE_NO_UPDATES:
					Serial.printf("  Update file '%s' found\r\n", firmwareImageUrl.c_str());
					break;
					
				case HTTP_UPDATE_OK:
					Serial.printf("  Updated\r\n");
					break;
			}
		} else if (newVersion == -1) {
			factoryReset();

		} else {
			Serial.println("  We are up to date.");
		}

	} else if (httpCode == 404) {
		Serial.println("  No firmware version file found.");

	} else {
		Serial.printf("  Firmware version check failed, got HTTP response code %d.\r\n", httpCode);
	}
	httpClient.end();
}

// -----------------------------------------------------------------------------------------------------
void setupMqtt() {
	mqttClient.setServer(ctx.mqttServer, ctx.mqttPort);
	mqttClient.setCallback(callback);
}

// -----------------------------------------------------------------------------------------------------
void callback(char* topic, byte* message, unsigned int length) {
	String messageTemp;

	for (int i = 0; i < length; i++) {
		messageTemp += (char) message[i];
	}

	if (String(topic) == ctx.mqttUpdateStatusTopic
			&& messageTemp == "true") {
		isUpdateAvailable = true;
	}

	Serial.print("Callback called, isUpdateAvailable=");
	Serial.println(messageTemp);
}

// -----------------------------------------------------------------------------------------------------
void disconnect() {
	if (mqttClient.connected()) {
		Serial.println("Disconnecting from MQTT...");
		mqttClient.disconnect();
		delay(100);
	}
}

// -----------------------------------------------------------------------------------------------------
void reconnect() {
	Serial.println("Connecting to MQTT...");
	while (!mqttClient.connected()) {
		verifyFingerprint();
		// clientID, username, password, willTopic, willQoS, willRetain, willMessage, cleanSession
		if (!mqttClient.connect(ctx.mqttClientName, ctx.mqttUser, ctx.mqttPassword, NULL, 0, 0, NULL, 0)) {
			delay(1000);
		} else {
			boolean rc = mqttClient.subscribe(ctx.mqttUpdateStatusTopic, 1);
			if (rc) {
				Serial.printf(" subscribed to %s\n", ctx.mqttUpdateStatusTopic);
			} else {
				Serial.printf(" subscription to %s failed: %d\n", ctx.mqttUpdateStatusTopic, rc);
			}
		}
	}
}

// -----------------------------------------------------------------------------------------------------
void verifyFingerprint() {
	if (ctx.mqttFingerprint && strlen(ctx.mqttFingerprint) > 0) {
		if(!espClient.verify(ctx.mqttFingerprint, ctx.mqttServer)) {
			Serial.printf("MQTT fingerprint '%s' does not match, rebooting...\r\n", ctx.mqttFingerprint);
			Serial.flush();
			delay(200);
			ESP.restart();
		} else {
			Serial.println("MQTT fingerprint does match");
		}
	}
}

// -----------------------------------------------------------------------------------------------------
void loop() {
	if (!isDisplayUpdateRunning && isMqttEnabled && !mqttClient.connected()) {
		reconnect();
		Serial.println(" Reconnected, waiting for incoming MQTT messages...");
		// get max 100 messages
		for (int i = 0; i < 100; ++i) {
			mqttClient.loop();
			delay(10);
		}
		if (!isUpdateAvailable) {
			Serial.println(" No update available.");
		}
	}

	static unsigned long startCycle = ESP.getCycleCount();
	unsigned long currentCycle = ESP.getCycleCount();
	unsigned long difference;
	if (currentCycle < startCycle) {
		difference = (4294967295 - startCycle + currentCycle);
	} else {
		difference = (currentCycle - startCycle);
	}

	if ((isMqttEnabled && isUpdateAvailable)
			|| (isMqttEnabled && isDisplayUpdateRunning)
			|| !isMqttEnabled) {
		static bool serverStarted = false;
		if (!serverStarted) {
			server.begin();
			serverStarted = true;

			if (isUpdateAvailable) {
				mqttClient.publish(ctx.mqttCommandTopic, "true");
				delay(100);
			}
			Serial.printf("Webserver started, waiting %sfor data\r\n", isMqttEnabled ? "" : "10s ");

		} else {
			Srvr__loop();

			int decile = fmod(difference / (TICKS_PER_SECOND / 100.0), 100.0);
			static bool ledStatus = false;
			if (!ledStatus && decile == 95) {
				digitalWrite(LED_BUILTIN, HIGH);
				ledStatus = true;
			} else if (ledStatus && decile == 0) {
				digitalWrite(LED_BUILTIN, LOW);
				ledStatus = false;
			}
		}
	}

	bool isTimeToSleep = false;
	if (isMqttEnabled && !isUpdateAvailable) {
		isTimeToSleep = true;

	} else if (difference > UPTIME_SEC * TICKS_PER_SECOND) {
		isTimeToSleep = true;
	}

	if (!isDisplayUpdateRunning) {
		if (isTimeToSleep) {
			if (ctx.sleepTime > 0) {
				disconnect();
				Serial.printf("\r\nGoing to sleep for %ld seconds.\r\n", ctx.sleepTime);
				ESP.deepSleep(ctx.sleepTime * 1000000);
				delay(100);

			} else { // avoid overheating
				startCycle = ESP.getCycleCount();
				delay(1000);
			}
		}
	}
}
