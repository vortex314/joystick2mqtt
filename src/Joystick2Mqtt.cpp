#include "Joystick2Mqtt.h"
const char* signalString[] = {"PIPE_ERROR",
                              "SELECT_ERROR",
                              "DEVICE_CONNECT",
                              "DEVICE__DISCONNECT",
                              "DEVICE__RXD",
                              "DEVICE__ERROR",
                              "MQTT_CONNECT_SUCCESS",
                              "MQTT_CONNECT_FAIL",
                              "MQTT_SUBSCRIBE_SUCCESS",
                              "MQTT_SUBSCRIBE_FAIL",
                              "MQTT_PUBLISH_SUCCESS",
                              "MQTT_PUBLISH_FAIL",
                              "MQTT_DISCONNECTED",
                              "MQTT_MESSAGE_RECEIVED",
                              "MQTT_ERROR",
                              "TIMEOUT"
                             };

#define USB() logger.application(_device.c_str())

Joystick2Mqtt::Joystick2Mqtt()
	: _deviceBuffer(2048),_jsonDocument(),_msg(1024) {
	_mqttConnectionState = MS_DISCONNECTED;
}

Joystick2Mqtt::~Joystick2Mqtt() {}
void Joystick2Mqtt::setConfig(Config config) { _config = config; }
void Joystick2Mqtt::device(string devJs) { _device = devJs; }
void Joystick2Mqtt::setLogFd(FILE* logFd) { _logFd = logFd; }

void Joystick2Mqtt::init() {
	_startTime = Sys::millis();
	_config.setNameSpace("mqtt");
	_config.get("connection", _mqttConnection, "tcp://localhost:1883");
	_config.get("keepAliveInterval", _mqttKeepAliveInterval, 5);
	_config.get("willMessage", _mqttWillMessage, "false");
    _mqttDevice =  Sys::hostname();
    int offset = _device.find_last_of('/');
    _mqttObject = _device.substr(offset+1);
    _mqttSrc =  "src/"+_mqttDevice+"/"+_mqttObject;
	_mqttWillQos = 0;
	_mqttWillRetained = false;
	_mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
	_mqttClientId = _mqttObject + std::to_string(getpid());
    INFO(" MQTT device : %s object : %s ",_mqttDevice.c_str(),_mqttObject.c_str());
    string willTopicDefault;
    string_format(willTopicDefault,"src/%s/%s/alive",_mqttDevice.c_str(),_mqttObject);
	_config.get("willTopic", _mqttWillTopic, willTopicDefault.c_str());

	if(pipe(_signalFd) < 0) {
		INFO("Failed to create pipe: %s (%d)", strerror(errno), errno);
	}

	if(fcntl(_signalFd[0], F_SETFL, O_NONBLOCK) < 0) {
		INFO("Failed to set pipe non-blocking mode: %s (%d)", strerror(errno), errno);
	}
}

void Joystick2Mqtt::threadFunction(void* pv) { run(); }

void Joystick2Mqtt::run() {
	string line;
	Timer mqttConnectTimer;
	Timer serialConnectTimer;
	Timer mqttPublishTimer;
	Timer serialTimer;

	mqttConnectTimer.atInterval(2000).doThis([this]() {
		if(_mqttConnectionState != MS_CONNECTING) {
			mqttConnect();
		}
	});
	serialConnectTimer.atInterval(5000).doThis([this]() {
		if(!_deviceConnected) {
			deviceConnect();
		}
	});
	mqttPublishTimer.atInterval(1000).doThis([this]() {
		std::string sUpTime = std::to_string((Sys::millis() - _startTime) / 1000);
		mqttPublish("src/" + _mqttDevice + "/"+_mqttObject+"/alive", "true", 0, 0);
		mqttPublish("src/" + _mqttDevice + "/system/upTime", sUpTime, 0, 0);
		mqttPublish("src/" + _mqttDevice + "/"+_mqttObject, _device, 0, 0);
	});
	serialTimer.atDelta(5000).doThis([this]() {
		if(_deviceConnected) {
			deviceDisconnect();
			WARN(" disconnecting serial no new data received in %d msec", 5000);
			deviceConnect();
		}
	});
	if(_mqttConnectionState != MS_CONNECTING) mqttConnect();
	deviceConnect();
	while(true) {
		while(true) {
			Signal s = waitSignal(1000);

	//		INFO("signal = %s", signalString[s]);
			mqttConnectTimer.check();
			serialTimer.check();
			mqttPublishTimer.check();
			serialConnectTimer.check();
			switch(s) {
				case TIMEOUT: {
						if(!_deviceConnected) {
							//						serialConnect();
						}
						break;
					}
				case DEVICE_RXD: {
						deviceRxd();
						if(deviceGetBlock(line)) {
							serialTimer.atDelta(5000);
							deviceHandleBlock(line);
							line.clear();
						}
						break;
					}
				case DEVICE_ERROR: {
						deviceDisconnect();
						break;
					}
				case MQTT_CONNECT_SUCCESS: {
						INFO("MQTT_CONNECT_SUCCESS %s ", _mqttDevice.c_str());
						mqttConnectionState(MS_CONNECTED);
						mqttSubscribe(_mqttSubscribedTo);
						break;
					}
				case MQTT_CONNECT_FAIL: {
						WARN("MQTT_CONNECT_FAIL %s ", _mqttDevice.c_str());
						mqttConnectionState(MS_DISCONNECTED);
						break;
					}
				case MQTT_DISCONNECTED: {
						WARN("MQTT_DISCONNECTED %s ", _mqttDevice.c_str());
						mqttConnectionState(MS_DISCONNECTED);
						break;
					}
				case MQTT_SUBSCRIBE_SUCCESS: {
						INFO("MQTT_SUBSCRIBE_SUCCESS %s ", _mqttDevice.c_str());
						break;
					}
				case MQTT_SUBSCRIBE_FAIL: {
						WARN("MQTT_SUBSCRIBE_FAIL %s ", _mqttDevice.c_str());
						mqttDisconnect();
						break;
					}
				case MQTT_ERROR: {
						WARN("MQTT_ERROR %s ", _mqttDevice.c_str());
						break;
					}
				case PIPE_ERROR: {
						WARN("PIPE_ERROR %s ", _mqttDevice.c_str());
						break;
					}
				case MQTT_PUBLISH_SUCCESS: {
						break;
					}
				case MQTT_MESSAGE_RECEIVED: {
						break;
					}
				default: {
						WARN("received signal [%d] %s for %s ", s,signalString[s], _mqttDevice.c_str());
					}
			}
		}
	}
	WARN(" exited run loop !!");
}

void Joystick2Mqtt::signal(uint8_t m) {
	if(write(_signalFd[1], (void*)&m, 1) < 1) {
		INFO("Failed to write pipe: %s (%d)", strerror(errno), errno);
	}
	//	INFO(" signal '%c' ",m);
}

Joystick2Mqtt::Signal Joystick2Mqtt::waitSignal(uint32_t timeout) {
	Signal returnSignal = TIMEOUT;
	uint8_t buffer;
	fd_set rfds;
	fd_set wfds;
	fd_set efds;
	struct timeval tv;
	int retval;
	//    uint64_t delta=1000;

	// Wait up to 1000 msec.
	uint64_t delta = timeout;

	tv.tv_sec = 1;
	tv.tv_usec = (delta * 1000) % 1000000;

	// Watch serialFd and tcpFd  to see when it has input.
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
	if(_deviceConnected) {
		FD_SET(_deviceFd, &rfds);
		FD_SET(_deviceFd, &efds);
	}
	if(_signalFd[0]) {
		FD_SET(_signalFd[0], &rfds);
		FD_SET(_signalFd[0], &efds);
	}
	int maxFd = _deviceFd < _signalFd[0] ? _signalFd[0] : _deviceFd;
	maxFd += 1;

	retval = select(maxFd, &rfds, NULL, &efds, &tv);

	if(retval < 0) {
		WARN(" select() : error : %s (%d)", strerror(errno), errno);
		returnSignal = SELECT_ERROR;
	} else if(retval > 0) { // one of the fd was set
		if(FD_ISSET(_deviceFd, &rfds)) {
			returnSignal = DEVICE_RXD;
		}
		if(FD_ISSET(_signalFd[0], &rfds)) {
			::read(_signalFd[0], &buffer, 1); // read 1 event
			returnSignal = (Signal)buffer;
		}
		if(FD_ISSET(_deviceFd, &efds)) {
			WARN("serial  error : %s (%d)", strerror(errno), errno);
			returnSignal = DEVICE_ERROR;
		}
		if(FD_ISSET(_signalFd[0], &efds)) {
			WARN("pipe  error : %s (%d)", strerror(errno), errno);
			returnSignal = PIPE_ERROR;
		}
	} else {

		TRACE(" timeout %llu", Sys::millis());
		returnSignal = TIMEOUT;
	}

	return (Signal)returnSignal;
}

Erc Joystick2Mqtt::deviceConnect() {

	INFO("Connecting to '%s' ....", _device.c_str());
	_deviceFd = ::open(_device.c_str(), O_EXCL | O_RDWR | O_NOCTTY | O_NDELAY);

	if(_deviceFd == -1) {
		ERROR("connect: Unable to open '%s' errno : %d : %s ", _device.c_str(), errno, strerror(errno));
		_deviceFd = 0;
		return errno;
	}

	INFO("open '%s' succeeded.fd=%d", _device.c_str(), _deviceFd);
    int version = 0x000800;
    unsigned char axes = 2;
	unsigned char buttons = 2;
#define NAME_LENGTH 128
	char name[NAME_LENGTH] = "Unknown";

    
	ioctl(_deviceFd, JSIOCGVERSION, &version);
	ioctl(_deviceFd, JSIOCGAXES, &axes);
	ioctl(_deviceFd, JSIOCGBUTTONS, &buttons);
	ioctl(_deviceFd, JSIOCGNAME(NAME_LENGTH), name);
    _axis = (int*)calloc(_axes, sizeof(int));
    _button = (char*)calloc(_buttons, sizeof(char));
    
    INFO("Driver version is %d.%d.%d.\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);
    INFO(" Axes : %d Buttons : %d ",axes,buttons);
	_deviceConnected = true;
	return E_OK;
}

void Joystick2Mqtt::deviceDisconnect() {
	close(_deviceFd);
	_deviceFd = 0;
	_deviceConnected = false;
}

void Joystick2Mqtt::deviceRxd() {
	if(!_deviceConnected) return;
	int erc;

	while(true) {
        /*
		struct JS_DATA_TYPE js;
        erc = read(_deviceFd, &js, JS_RETURN) ;
		if(erc == JS_RETURN ) {
            INFO("Axes: X:%3d Y:%3d Buttons: A:%s B:%s\r",
				js.x, js.y, (js.buttons & 1) ? "on " : "off", (js.buttons & 2) ? "on " : "off");
		} else if(erc < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) return;
			DEBUG(" read returns %d => errno : %d = %s", erc, errno, strerror(errno));
			signal(DEVICE_ERROR);
		} else {
			return;
		}*/
        
        struct js_event js;
        erc = read(_deviceFd, &js, sizeof(struct js_event)) ;
		if(erc == sizeof(struct js_event)) {
            switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
                INFO(" button : %d = %d ",js.number,js.value);
                string_format(_topic,"%s/button%d",_mqttSrc.c_str(),js.number);
                string_format(_value,"%d",js.value);
                mqttPublish(_topic,_value, 0, 0);
				_button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
                INFO(" axis : %d = %d ",js.number,js.value);
                string_format(_topic,"%s/axis%d",_mqttSrc.c_str(),js.number);
                string_format(_value,"%d",js.value);
                mqttPublish(_topic,_value, 0, 0);
				_axis[js.number] = js.value;
				break;
			}
            
       /*     if (_axes) {
				printf("Axes: ");
				for (i = 0; i < _axes; i++)
					printf("%2d:%6d ", i, _axis[i]);
			}

			if (_buttons) {
				printf("Buttons: ");
				for (i = 0; i < _buttons; i++)
					printf("%2d:%s ", i, _button[i] ? "on " : "off");
			}
            			fflush(stdout);*/

		} else if(erc < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) return;
			DEBUG(" read returns %d => errno : %d = %s", erc, errno, strerror(errno));
			signal(DEVICE_ERROR);
		} else {
			return;
		}
        
	}
}

bool Joystick2Mqtt::deviceGetBlock(string& line) {
	if(!_deviceConnected) return false;
	while(_deviceBuffer.hasData()) {
		char ch = _deviceBuffer.read();
		if(ch == '\n') {
			return true;
		} else if(ch == '\r') {
		} else {
			if(line.length() > 1024) {
				line.clear();
				WARN(" %s buffer garbage  > 1024 flushed ", _device.c_str());
			}
			line += ch;
		}
	}
	return false;
}

std::vector<string> split(const string& text, char sep) {
	std::vector<string> tokens;
	std::size_t start = 0, end = 0;
	while((end = text.find(sep, start)) != string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

unsigned short crc16(const unsigned char* data_p, unsigned char length) {
	unsigned char x;
	unsigned short crc = 0xFFFF;

	while(length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
	}
	return crc;
}
/*
	* extract CRC and verify in syntax => ["ABCD",2,..]
 */
bool Joystick2Mqtt::checkCrc(std::string& line) {
	if(line.length() < 9) return false;
	std::string crcStr = line.substr(2, 4);
	uint32_t crcFound;
	sscanf(crcStr.c_str(), "%4X", &crcFound);
	std::string line2 = line.substr(0, 2) + "0000" + line.substr(6, string::npos);
	uint32_t crcCalc = crc16((uint8_t*)line2.data(), line2.length());
	return crcCalc == crcFound;
}

void Joystick2Mqtt::genCrc(std::string& line) {
	uint32_t crcCalc = crc16((uint8_t*)line.data(), line.length());
	char hexCrc[5];
	sprintf(hexCrc, "%4.4X", crcCalc);
	std::string lineCopy = "[\"";
	lineCopy += hexCrc + line.substr(6, string::npos);
	line = lineCopy.c_str();
}

/*
 * JSON protocol : [CRC,CMD,TOPIC,MESSAGE,QOS,RETAIN]
 * CMD : 0:PING,1:PUBLISH,2:PUBACK,3:SUBSCRIBE,4:SUBACK,...
 * ping : ["0000",0,"someText"]
 * publish : ["ABCD",1,"dst/topic1","message1",0,0]
 * subscribe : ["ABCD",3,"dst/myTopic"]
 *
 */

typedef enum { SUBSCRIBE = 0, PUBLISH} CMD;

void Joystick2Mqtt::deviceHandleBlock(string& line) {
	std::vector<string> token;
	_jsonDocument.clear();
	if(_protocol == JSON_ARRAY && line.length() > 2 && line[0] == '[' && line[line.length() - 1] == ']') {
		deserializeJson(_jsonDocument, line);
		if(_jsonDocument.is<JsonArray>()) {
			JsonArray array = _jsonDocument.as<JsonArray>();
			int cmd = array[0];
			if(cmd == PUBLISH) {
				std::string topic = array[1];
				std::string message = array[2];
				uint32_t qos=0;
				if ( array.size()>3 )  qos = array[3];
				bool retained = false;
				if ( array.size()> 4) retained = array[4];
				mqttPublish(topic, message, qos, retained);
				return;
			} else if(cmd == SUBSCRIBE) {
				std::string topic = array[1];
				mqttSubscribe(topic);
				return;
			}
		}
	} else if(_protocol == JSON_OBJECT && line.length() > 2 && line[0] == '{' && line[line.length() - 1] == '}') {
		deserializeJson(_jsonDocument, line);
		if(_jsonDocument.is<JsonObject>()) {
			JsonObject json = _jsonDocument.as<JsonObject>();
			if(json.containsKey("cmd")) {
				string cmd = json["cmd"];
				if(cmd.compare("MQTT-PUB") == 0 && json.containsKey("topic") && json.containsKey("message")) {
					int qos = 0;
					bool retained = false;
					string topic = json["topic"];
					token = split(topic, '/');
					if(token[1].compare(_mqttDevice) != 0) {
						WARN(" subscribed topic differ %s <> %s ", token[1].c_str(), _mqttDevice.c_str());
						_mqttDevice = token[1];
						_mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
						mqttSubscribe(_mqttSubscribedTo);
					}
					string message = json["message"];
					/*                    Bytes msg(1024);
					                    msg.append((uint8_t*)message.c_str(),message.length());*/
					mqttPublish(topic, message, qos, retained);
					return;
				} else if(cmd.compare("MQTT-SUB") == 0 && json.containsKey("topic")) {
					string topic = json["topic"];
					mqttSubscribe(topic);
					return;
				} else {
					WARN(" invalid command from device : %s", line.c_str());
				}
			}
		}
	}
	fprintf(stdout, "%s\n", line.c_str());
	if(_logFd != NULL) {
		fprintf(_logFd, "%s\n", line.c_str());
		fflush(_logFd);
	}

	mqttPublish("src/" + _device + "/joystick2mqtt/log", line, 0, false);
}



/*
 *
                                                @     @  @@@@@  @@@@@@@ @@@@@@@
                                                @@   @@ @     @    @       @
                                                @ @ @ @ @     @    @       @
                                                @  @  @ @     @    @       @
                                                @     @ @   @ @    @       @
                                                @     @ @    @     @       @
                                                @     @  @@@@ @    @       @

 *
 */
const char* mqttConnectionStates[] = {"MS_CONNECTED", "MS_DISCONNECTED", "MS_CONNECTING", "MS_DISCONNECTING"};
void Joystick2Mqtt::mqttConnectionState(MqttConnectionState st) {
	INFO(" MQTT connection state %s => %s ", mqttConnectionStates[_mqttConnectionState], mqttConnectionStates[st]);
	_mqttConnectionState = st;
}

Erc Joystick2Mqtt::mqttConnect() {
	int rc;
	if(_mqttConnectionState == MS_CONNECTING || _mqttConnectionState == MS_CONNECTED) return E_OK;

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_willOptions will_opts = MQTTAsync_willOptions_initializer;

	INFO(" MQTT connecting %s ... for %s ", _mqttConnection.c_str(), _mqttObject.c_str());
	mqttConnectionState(MS_CONNECTING);
	MQTTAsync_create(&_client, _mqttConnection.c_str(), _mqttClientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessage, onDeliveryComplete); // TODO add ondelivery

	conn_opts.keepAliveInterval = _mqttKeepAliveInterval;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnectSuccess;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = this;
	will_opts.message = _mqttWillMessage.c_str();
	will_opts.topicName = _mqttWillTopic.c_str();
	will_opts.qos = _mqttWillQos;
	will_opts.retained = _mqttWillRetained;
	conn_opts.will = &will_opts;
	if((rc = MQTTAsync_connect(_client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		WARN("Failed to start connect, return code %d", rc);
		return E_NOT_FOUND;
	}
	mqttConnectionState(MS_CONNECTING);
	return E_OK;
}

void Joystick2Mqtt::mqttDisconnect() {
	mqttConnectionState(MS_DISCONNECTING);
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnect;
	disc_opts.context = this;
	int rc = 0;
	if((rc = MQTTAsync_disconnect(_client, &disc_opts)) != MQTTASYNC_SUCCESS) {
		WARN("Failed to disconnect, return code %d", rc);
		return;
	}
	MQTTAsync_destroy(&_client);
	mqttConnectionState(MS_DISCONNECTED);
}

void Joystick2Mqtt::mqttSubscribe(string topic) {
	int qos = 0;
	if(_mqttConnectionState != MS_CONNECTED) return;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	INFO("Subscribing to topic %s for client %s using QoS%d", topic.c_str(), _mqttClientId.c_str(), qos);
	opts.onSuccess = onSubscribeSuccess;
	opts.onFailure = onSubscribeFailure;
	opts.context = this;
	int rc = E_OK;

	if((rc = MQTTAsync_subscribe(_client, topic.c_str(), qos, &opts)) != MQTTASYNC_SUCCESS) {
		ERROR("Failed to start subscribe, return code %d", rc);
		signal(MQTT_SUBSCRIBE_FAIL);
	} else {
		INFO(" subscribe send ");
	}
}

void Joystick2Mqtt::onConnectionLost(void* context, char* cause) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	//   me->mqttDisconnect();
	me->signal(MQTT_DISCONNECTED);
}

int Joystick2Mqtt::onMessage(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	Bytes msg((uint8_t*)message->payload, message->payloadlen);
	string topic(topicName, topicLen);

	INFO(" did I just receive a message ?? ");

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	me->signal(MQTT_MESSAGE_RECEIVED);
	return 1;
}

void Joystick2Mqtt::onDeliveryComplete(void* context, MQTTAsync_token response) {
	//    Joystick2Mqtt* me = (Joystick2Mqtt*)context;
}

void Joystick2Mqtt::onDisconnect(void* context, MQTTAsync_successData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_DISCONNECTED);
}

void Joystick2Mqtt::onConnectFailure(void* context, MQTTAsync_failureData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_CONNECT_FAIL);
	me->mqttConnectionState(MS_DISCONNECTED);
}

void Joystick2Mqtt::onConnectSuccess(void* context, MQTTAsync_successData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_CONNECT_SUCCESS);
	me->mqttConnectionState(MS_CONNECTED);
}

void Joystick2Mqtt::onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_SUBSCRIBE_SUCCESS);
}

void Joystick2Mqtt::onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_SUBSCRIBE_FAIL);
}

void Joystick2Mqtt::mqttPublish(string topic, string message, int qos, bool retained) {
	DEBUG(" MQTT PUB : %s = %s ", topic.c_str(), message.c_str());
	_msg = message.c_str();
	mqttPublish(topic, _msg, qos, retained);
}

void Joystick2Mqtt::mqttPublish(string topic, Bytes message, int qos, bool retained) {
	if(!_mqttConnectionState==MS_CONNECTED) {
		return;
	}
	qos = 1;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	//   INFO("mqttPublish %s",topic.c_str());

	int rc = E_OK;
	opts.onSuccess = onPublishSuccess;
	opts.onFailure = onPublishFailure;
	opts.context = this;

	pubmsg.payload = message.data();
	pubmsg.payloadlen = message.length();
	pubmsg.qos = qos;
	pubmsg.retained = retained;

	if((rc = MQTTAsync_sendMessage(_client, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		signal(MQTT_DISCONNECTED);
		ERROR("MQTTAsync_sendMessage failed.");
	}
}
void Joystick2Mqtt::onPublishSuccess(void* context, MQTTAsync_successData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_PUBLISH_SUCCESS);
}
void Joystick2Mqtt::onPublishFailure(void* context, MQTTAsync_failureData* response) {
	Joystick2Mqtt* me = (Joystick2Mqtt*)context;
	me->signal(MQTT_PUBLISH_FAIL);
}


