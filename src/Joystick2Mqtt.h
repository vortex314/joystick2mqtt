#ifndef JOYSTICK2MQTT_H
#define JOYSTICK2MQTT_H

// For convenience
#include <Bytes.h>
#include <Erc.h>
#include <Mqtt.h>
#include <Sys.h>
#include <Timer.h>
#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

#include "CircBuf.h"
#include "Config.h"

using namespace std;

class Joystick2Mqtt {
  string _device;       // /dev/input/jsx
  string _deviceShort;  // jsx
  int _deviceFd;
  CircBuf _deviceBuffer;
  uint32_t _deviceExistTimer = 1000;

  unsigned char _axes = 2;
  unsigned char _buttons = 2;
  int* _axis;
  char* _button;

  bool _deviceConnected = false;

  Mqtt mqtt;

  int _signalFd[2];  // pipe fd to wakeup in select
  // SERIAL

  // MQTT
  StaticJsonDocument<2048> _jsonDocument;

  string _mqttDevice;  // hostname
  string _mqttObject;  // jsx
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
  string _topic;
  string _value;
  Bytes _msg;

  //	bool _mqttConnected=false;
  string _mqttSubscribedTo;

  Config _config;

  typedef enum { JSON_OBJECT, JSON_ARRAY, PROTOBUF } Protocol;
  Protocol _protocol;

  typedef enum { CRC_ON, CRC_OFF } Crc;
  Crc _crc;

  FILE* _logFd;

 public:
  typedef enum {
    PIPE_ERROR = 0,
    SELECT_ERROR,
    DEVICE_CONNECT,
    DEVICE_DISCONNECT,
    DEVICE_RXD,
    DEVICE_ERROR,
    MQTT_CONNECTED,
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

  
};

#endif  // SERIAL2MQTT_H
