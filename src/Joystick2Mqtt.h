#ifndef JOYSTICK2MQTT_H
#define JOYSTICK2MQTT_H

#include "MQTTAsync.h"

// For convenience
#include <Sys.h>
#include <Erc.h>
#include <Bytes.h>
#include <vector>
#include "Config.h"
#include "CircBuf.h"
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <Timer.h>

#include <linux/input.h>
#include <linux/joystick.h>

using namespace std;




class Joystick2Mqtt {

		string _device; // /dev/input/jsx
        string _deviceShort; // jsx
        int _deviceFd;
        CircBuf _deviceBuffer;
        
        unsigned char _axes = 2;
        unsigned char _buttons = 2;
        int *_axis;
        char *_button;

        
        bool _deviceConnected=false;
        
		MQTTAsync_token _deliveredtoken;
		MQTTAsync _client;
		int _signalFd[2];   // pipe fd to wakeup in select
		// SERIAL

		// MQTT
		StaticJsonDocument<2048> _jsonDocument;

        string _mqttDevice; // hostname
        string _mqttObject; // jsx
        string _mqttSrc;
		string _mqttClientId; 
		string _mqttConnection;
		uint32_t _mqttKeepAliveInterval;
		string _mqttWillMessage;
		std::string _mqttWillTopic;
		uint16_t _mqttWillQos;
		bool _mqttWillRetained;
		string _mqttProgrammerTopic;
		uint64_t _startTime;

//	bool _mqttConnected=false;
		string _mqttSubscribedTo;

		Config _config;

		typedef enum {
			JSON_OBJECT,
			JSON_ARRAY,
			PROTOBUF
		} Protocol;
		Protocol _protocol;

		typedef enum {
			CRC_ON,
			CRC_OFF
		} Crc;
		Crc _crc;

		FILE* _logFd;
		typedef enum {
			MS_CONNECTED,
			MS_DISCONNECTED,
			MS_CONNECTING,
			MS_DISCONNECTING
		} MqttConnectionState;
		MqttConnectionState _mqttConnectionState;



	public:
		typedef enum {PIPE_ERROR=0,
		              SELECT_ERROR,
		              DEVICE_CONNECT,
		              DEVICE_DISCONNECT,
		              DEVICE_RXD,
		              DEVICE_ERROR,
		              MQTT_CONNECT_SUCCESS,
		              MQTT_CONNECT_FAIL,
		              MQTT_SUBSCRIBE_SUCCESS,
		              MQTT_SUBSCRIBE_FAIL,
		              MQTT_PUBLISH_SUCCESS,
		              MQTT_PUBLISH_FAIL,
		              MQTT_DISCONNECTED,
		              MQTT_MESSAGE_RECEIVED,
		              MQTT_ERROR,
		              TIMEOUT
		             } Signal;




		Joystick2Mqtt();
		~Joystick2Mqtt();
		void init();
		void run();
		void threadFunction(void*);
		void signal(uint8_t s);
		Signal waitSignal(uint32_t timeout);


		void setConfig(Config config);
		void device(string devJs);
		void setLogFd(FILE*);
		Erc deviceConnect();
		void deviceDisconnect();
		void deviceRxd();
		bool deviceGetBlock(string& line);
		void deviceHandleBlock(string& line);


		Erc mqttConnect();
		void mqttDisconnect();
		void mqttPublish(string topic,Bytes message,int qos,bool retained);
		void mqttPublish(string topic,string message,int qos,bool retained);
		void mqttSubscribe(string topic);

		static void onConnectionLost(void *context, char *cause);
		static int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
		static void onDisconnect(void* context, MQTTAsync_successData* response);
		static void onConnectFailure(void* context, MQTTAsync_failureData* response);
		static void onConnectSuccess(void* context, MQTTAsync_successData* response);
		static void onSubscribeSuccess(void* context, MQTTAsync_successData* response);
		static void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
		static void onPublishSuccess(void* context, MQTTAsync_successData* response);
		static void onPublishFailure(void* context, MQTTAsync_failureData* response);
		static void onDeliveryComplete(void* context, MQTTAsync_token response);

		void genCrc(std::string& line) ;
		bool checkCrc(std::string& line);
		void mqttConnectionState(MqttConnectionState);

};

#endif // SERIAL2MQTT_H
