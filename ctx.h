class Ctx {
public:
	Ctx() :
			mqttPort(1883),
					sleepTime(60) {
		memset(mqttUser, 0, 128);
		memset(mqttPassword, 0, 128);
		memset(mqttFingerprint, 0, 128);
		memset(mqttClientName, 0, 21);
		strcpy(mqttUpdateStatusTopic, "stat/display/needUpdate");
		strcpy(mqttCommandTopic, "cmd/display/upload");
	}
	~Ctx() {
		if (customMqttServer) {
			delete customMqttServer;
		}
		if (customMqttPort) {
			delete customMqttPort;
		}
		if (customMqttUser) {
			delete customMqttUser;
		}
		if (customMqttPassword) {
			delete customMqttPassword;
		}
		if (customMqttFingerprint) {
			delete customMqttFingerprint;
		}
		if (customMqttUpdateStatusTopic) {
			delete customMqttUpdateStatusTopic;
		}
		if (customMqttCommandTopic) {
			delete customMqttCommandTopic;
		}
		if (customSleepTime) {
			delete customSleepTime;
		}
		if (customFirmwareUrl) {
			delete customFirmwareUrl;
		}
	}
	void initWifiManagerParameters() {
		customMqttServer = new WiFiManagerParameter("server", "MQTT server", mqttServer, 40);
		itoa(mqttPort, mqttPortAsString, 10);
		customMqttPort = new WiFiManagerParameter("port", "MQTT port", mqttPortAsString, 6);
		customMqttUser = new WiFiManagerParameter("MQTT user", "MQTT user name", mqttUser, 128);
		customMqttPassword = new WiFiManagerParameter("MQTT password", "MQTT password", mqttPassword, 128);
		customMqttFingerprint = new WiFiManagerParameter("MQTT TLS fingerprint", "MQTT server certifacte fingerprint", mqttFingerprint, 128);
		customMqttUpdateStatusTopic = new WiFiManagerParameter("updateTopic", "MQTT update topic", mqttUpdateStatusTopic, 128);
		customMqttCommandTopic = new WiFiManagerParameter("commandTopic", "MQTT command topic", mqttCommandTopic, 128);
		itoa(sleepTime, sleepTimeAsString, 10);
		customSleepTime = new WiFiManagerParameter("sleepTime", "sleep time in seconds", sleepTimeAsString, 33);
		customFirmwareUrl = new WiFiManagerParameter("firmwareUrl", "base URL for firmware images", firmwareUrl, 128);
	}
	void updateParameters() {
		strcpy(mqttServer, customMqttServer->getValue());
		mqttPort = atoi(customMqttPort->getValue());
		strcpy(mqttUser, customMqttUser->getValue());
		strcpy(mqttPassword, customMqttPassword->getValue());
		strcpy(mqttFingerprint, customMqttFingerprint->getValue());
		strcpy(mqttUpdateStatusTopic, customMqttUpdateStatusTopic->getValue());
		strcpy(mqttCommandTopic, customMqttCommandTopic->getValue());
		sleepTime = atoi(customSleepTime->getValue());
		strcpy(firmwareUrl, customFirmwareUrl->getValue());
	}
	bool isMqttEnabled() {
		return (mqttServer && strlen(mqttServer) > 0);
	}

	char mqttServer[40];
	char mqttPortAsString[6];
	int mqttPort;
	char mqttUser[128];
	char mqttPassword[128];
	char mqttFingerprint[128];
	char mqttClientName[21];
	char mqttUpdateStatusTopic[128];
	char mqttCommandTopic[128];
	char sleepTimeAsString[33];
	long sleepTime;
	char firmwareUrl[128];

	WiFiManagerParameter *customMqttServer;
	WiFiManagerParameter *customMqttPort;
	WiFiManagerParameter *customMqttUser;
	WiFiManagerParameter *customMqttPassword;
	WiFiManagerParameter *customMqttFingerprint;
	WiFiManagerParameter *customMqttUpdateStatusTopic;
	WiFiManagerParameter *customMqttCommandTopic;
	WiFiManagerParameter *customSleepTime;
	WiFiManagerParameter *customFirmwareUrl;

};
