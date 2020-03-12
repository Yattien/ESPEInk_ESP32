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
#include "ctx.h"

#include "srvr.h" // Server functions

// -----------------------------------------------------------------------------------------------------
const int FW_VERSION = 2; // for OTA
// -----------------------------------------------------------------------------------------------------
const char *AP_NAME = "ESPEInk-APSetup";
const char *MQTT_CLIENT_NAME = "ESPEInk";
const char *CONFIG_FILE = "/config.json";
const float TICKS_PER_SECOND = 80000000; // 80 MHz processor
//int LED_BUILTIN = 2;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool shouldSaveConfig = false;
bool isUpdateAvailable = false;
bool isMqttEnabled = false;

Ctx ctx;

// -----------------------------------------------------------------------------------------------------
void setup() {
	Serial.begin(115200);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	getConfig(&ctx);

	ctx.initWifiManagerParameters();
	setupWifi(ctx);
	ctx.updateParameters();
	isMqttEnabled = ctx.isMqttEnabled();

	Serial.printf(" MQTT Server: %s:%d\r\n", ctx.mqttServer, ctx.mqttPort);
	Serial.printf(" MQTT UpdateStatusTopic: %s\r\n", ctx.mqttUpdateStatusTopic);
	Serial.printf(" MQTT CommandTopic: %s\r\n", ctx.mqttCommandTopic);
	Serial.printf(" sleep time: %ld\r\n", ctx.sleepTime);
	Serial.printf(" firmware base URL: %s\r\n", ctx.firmwareUrl);
	saveConfig(ctx);

	getUpdate();

	EPD_initSPI();

	myIP = WiFi.localIP();

	mqttClient.setServer(ctx.mqttServer, ctx.mqttPort);
	mqttClient.setCallback(callback);

	Serial.println("Setup complete.");
}

// -----------------------------------------------------------------------------------------------------
void getConfig(Ctx *ctx) {
	if (SPIFFS.begin()) {
		if (SPIFFS.exists(CONFIG_FILE)) {
			Serial.println("Reading config file");
			File configFile = SPIFFS.open(CONFIG_FILE, "r");
			if (configFile) {
				DynamicJsonDocument jsonDocument(512);
				DeserializationError error = deserializeJson(jsonDocument, configFile);
				if (!error) {
					strlcpy(ctx->mqttServer, jsonDocument["mqttServer"] | "", sizeof ctx->mqttServer);
					ctx->mqttPort = jsonDocument["mqttPort"] | 1883;
					strlcpy(ctx->mqttUpdateStatusTopic, jsonDocument["mqttUpdateStatusTopic"] | "", sizeof ctx->mqttUpdateStatusTopic);
					strlcpy(ctx->mqttCommandTopic, jsonDocument["mqttCommandTopic"] | "", sizeof ctx->mqttCommandTopic);
					ctx->sleepTime = jsonDocument["sleepTime"] | 0;
					strlcpy(ctx->firmwareUrl, jsonDocument["firmwareUrl"] | "", sizeof ctx->firmwareUrl);

				} else {
					Serial.println(" failed to load json config");
				}
				configFile.close();
			}
		}
	} else {
		Serial.println("Failed to mount FS");
	}
}

// -----------------------------------------------------------------------------------------------------
void setupWifi(const Ctx &ctx) {
	WiFiManager wifiManager;
	requestMqttParameters(ctx, &wifiManager);
	if (!wifiManager.autoConnect(AP_NAME)) {
		Serial.println("failed to connect, resetting");
		WiFi.disconnect();
		delay(1000);
		ESP.restart();
	}
	Serial.println("Connected.");
}

// -----------------------------------------------------------------------------------------------------
void requestMqttParameters(const Ctx &ctx, WiFiManager *wifiManager) {
	wifiManager->addParameter(ctx.customMqttServer);
	wifiManager->addParameter(ctx.customMqttPort);
	wifiManager->addParameter(ctx.customMqttUpdateStatusTopic);
	wifiManager->addParameter(ctx.customMqttCommandTopic);
	wifiManager->addParameter(ctx.customSleepTime);
	wifiManager->addParameter(ctx.customFirmwareUrl);
	wifiManager->setSaveConfigCallback(saveConfigCallback);
}

// -----------------------------------------------------------------------------------------------------
void saveConfig(const Ctx &ctx) {
	if (shouldSaveConfig) {
		Serial.println("Saving config...");
		if (!SPIFFS.begin(false)) {
			if (!SPIFFS.begin(true)) {
				Serial.println(" an Error has occurred while mounting SPIFFS");
				delay(30000);
				ESP.restart();
			}
		}
		File configFile = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
		if (!configFile) {
			Serial.println(" failed to open config file for writing");
			delay(30000);
			ESP.restart();
		}
		DynamicJsonDocument jsonDocument(512);
		jsonDocument["mqttServer"] = ctx.mqttServer;
		jsonDocument["mqttPort"] = ctx.mqttPort;
		jsonDocument["mqttUpdateStatusTopic"] = ctx.mqttUpdateStatusTopic;
		jsonDocument["mqttCommandTopic"] = ctx.mqttCommandTopic;
		jsonDocument["sleepTime"] = ctx.sleepTime;
		jsonDocument["firmwareUrl"] = ctx.firmwareUrl;
		if (serializeJson(jsonDocument, configFile) == 0) {
			Serial.println(" failed to write to file");
			delay(30000);
			ESP.restart();
		}
		configFile.close();
		Serial.println("Config saved");
	}
}

// -----------------------------------------------------------------------------------------------------
void saveConfigCallback() {
	shouldSaveConfig = true;
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
	Serial.println("Resetting device...");
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true, true);
	delay(10000);
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

	Serial.printf("Checking for firmware update, version file '%s'.\r\n", firmwareVersionUrl.c_str());
	HTTPClient httpClient;
	httpClient.begin(firmwareVersionUrl);
	int httpCode = httpClient.GET();
	if (httpCode == 200) {
		String newFWVersion = httpClient.getString();
		Serial.printf(" current firmware version: %d, available version: %s", FW_VERSION, newFWVersion.c_str());
		int newVersion = newFWVersion.toInt();
		if (newVersion > FW_VERSION) {
			Serial.println(" updating...");
			String firmwareImageUrl = firmwareUrl;
			firmwareImageUrl.concat(".bin");
			t_httpUpdate_return ret = httpUpdate.update(espClient, firmwareImageUrl);

			switch (ret) {
				case HTTP_UPDATE_FAILED:
					Serial.printf(" update failed: (%d): %s\r\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
					break;

				case HTTP_UPDATE_NO_UPDATES:
					Serial.printf(" update file '%s' found\r\n", firmwareImageUrl.c_str());
					break;
			}
		} else if (newVersion == -1) {
			factoryReset();

		} else {
			Serial.println(" we are up to date");
		}

	} else if (httpCode == 404) {
		Serial.println(" no firmware version file found");

	} else {
		Serial.printf(" firmware version check failed, got HTTP response code %d\r\n", httpCode);
	}
	httpClient.end();
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
void reconnect() {
	Serial.println("Connecting to MQTT...");
	while (!mqttClient.connected()) {
		// clientID, username, password, willTopic, willQoS, willRetain, willMessage, cleanSession
		if (!mqttClient.connect(MQTT_CLIENT_NAME, NULL, NULL, NULL, 0, 0, NULL, 0)) {
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
void loop() {
	if (isMqttEnabled && !mqttClient.connected()) {
		reconnect();
		Serial.println(" reconnected, waiting for incoming MQTT message");
		// get max 100 messages
		for (int i = 0; i < 100; ++i) {
			mqttClient.loop();
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

	if ((isMqttEnabled && isUpdateAvailable) || !isMqttEnabled) {
		static bool serverStarted = false;
		if (!serverStarted) {
			server.begin();
			serverStarted = true;

			if (isUpdateAvailable) {
				mqttClient.publish(ctx.mqttCommandTopic, "true");
			}
			Serial.printf("Webserver started, waiting %sfor updates\r\n", isMqttEnabled ? "" : "10s ");

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
	if (!isTimeToSleep && ctx.sleepTime > 0) { // check if 10 seconds have passed
		if (difference > 10 * TICKS_PER_SECOND) {
			isTimeToSleep = true;
		}

	} else if (!isTimeToSleep && isMqttEnabled) {
		if (isMqttEnabled) {
			mqttClient.disconnect();
			delay(100);
		}
		isTimeToSleep = true;
	}

	if (!isDisplayUpdateRunning && isTimeToSleep) {
		Serial.printf("Going to sleep for %ld seconds.\r\n", ctx.sleepTime);
		ESP.deepSleep(ctx.sleepTime * 1000000);
		delay(100);
	}
}