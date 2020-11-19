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
                              "TIMEOUT"};

Joystick2Mqtt::Joystick2Mqtt()
    : _deviceBuffer(2048), _jsonDocument(), _msg(1024) {
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
  _mqttDevice = Sys::hostname();
  int offset = _device.find_last_of('/');
  _mqttObject = _device.substr(offset + 1);
  _mqttSrc = "src/" + _mqttDevice + "/" + _mqttObject;
  _mqttWillQos = 0;
  _mqttWillRetained = false;
  _mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
  _mqttClientId = _mqttObject + std::to_string(getpid());
  INFO(" MQTT device : %s object : %s ", _mqttDevice.c_str(),
       _mqttObject.c_str());
  string willTopicDefault;
  string_format(willTopicDefault, "src/%s/%s/alive", _mqttDevice.c_str(),
                _mqttObject);
  _config.get("willTopic", _mqttWillTopic, willTopicDefault.c_str());

  mqtt.lastWill(_mqttWillTopic, "false", 1, false);
  if (pipe(_signalFd) < 0) {
    INFO("Failed to create pipe: %s (%d)", strerror(errno), errno);
  }
  mqtt.onStateChange(this, [](void* context, Mqtt::MqttConnectionState state) {
    Joystick2Mqtt* me = (Joystick2Mqtt*)context;
    me->signal(state == Mqtt::MS_DISCONNECTED ? MQTT_DISCONNECTED
                                              : MQTT_CONNECTED);
  });

  if (fcntl(_signalFd[0], F_SETFL, O_NONBLOCK) < 0) {
    INFO("Failed to set pipe non-blocking mode: %s (%d)", strerror(errno),
         errno);
  }
  _config.setNameSpace("joystick");
  _config.get("timeout", _deviceExistTimer, 1000);
  INFO(" check device existance every %d msec", _deviceExistTimer);
}

void Joystick2Mqtt::threadFunction(void* pv) { run(); }

void Joystick2Mqtt::run() {
  string line;
  Timer mqttConnectTimer;
  Timer deviceConnectTimer;
  Timer mqttPublishTimer;
  Timer deviceTimer;

  mqttConnectTimer.atInterval(2000).doThis([this]() {
    if (mqtt.state() == Mqtt::MS_DISCONNECTED) {
      mqtt.connect();
    }
  });
  deviceConnectTimer.atInterval(5000).doThis([this]() {
    if (!_deviceConnected) {
      deviceConnect();
    }
  });
  mqttPublishTimer.atInterval(1000).doThis([this]() {
    std::string sUpTime = std::to_string((Sys::millis() - _startTime) / 1000);
    mqtt.publish("src/" + _mqttDevice + "/" + _mqttObject + "/alive", "true", 0,
                0);
    mqtt.publish("src/" + _mqttDevice + "/system/upTime", sUpTime, 0, 0);
    mqtt.publish("src/" + _mqttDevice + "/" + _mqttObject, _device, 0, 0);
  });
  deviceTimer.atInterval(_deviceExistTimer).doThis([this]() {
    struct stat buf;
    if (lstat(_device.c_str(), &buf) < 0) {
      INFO(" lstat failed");
      deviceDisconnect();
      WARN(" device %s disconnnected", _device.c_str());
    } else {
      INFO(" %s exists ", _device.c_str());
    }
    mqtt.publish("src/" + _mqttDevice + "/" + _mqttObject + "/connected",
                _deviceConnected ? "true" : "false", 0, 0);
  });
  if (mqtt.state() == Mqtt::MS_DISCONNECTED) mqtt.connect();
  deviceConnect();
  while (true) {
    while (true) {
      Signal s = waitSignal(1000);

      //		INFO("signal = %s", signalString[s]);
      mqttConnectTimer.check();
      deviceTimer.check();
      mqttPublishTimer.check();
      deviceConnectTimer.check();
      switch (s) {
        case TIMEOUT: {
          break;
        }
        case DEVICE_RXD: {
          deviceRxd();
          break;
        }
        case DEVICE_ERROR: {
          deviceDisconnect();
          break;
        }
        case MQTT_CONNECTED: {
          INFO("MQTT_CONNECT_SUCCESS %s ", _mqttDevice.c_str());
          mqtt.subscribe(_mqttSubscribedTo);
          break;
        }
        case MQTT_DISCONNECTED: {
          WARN("MQTT_DISCONNECTED %s ", _mqttDevice.c_str());
          break;
        }
        case MQTT_SUBSCRIBE_SUCCESS: {
          INFO("MQTT_SUBSCRIBE_SUCCESS %s ", _mqttDevice.c_str());
          break;
        }
        case MQTT_SUBSCRIBE_FAIL: {
          WARN("MQTT_SUBSCRIBE_FAIL %s ", _mqttDevice.c_str());
          mqtt.disconnect();
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
          WARN("received signal [%d] %s for %s ", s, signalString[s],
               _mqttDevice.c_str());
        }
      }
    }
  }
  WARN(" exited run loop !!");
}

void Joystick2Mqtt::signal(uint8_t m) {
  if (write(_signalFd[1], (void*)&m, 1) < 1) {
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

  // Watch deviceFd and tcpFd  to see when it has input.
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  FD_ZERO(&efds);
  if (_deviceConnected) {
    FD_SET(_deviceFd, &rfds);
    FD_SET(_deviceFd, &efds);
  }
  if (_signalFd[0]) {
    FD_SET(_signalFd[0], &rfds);
    FD_SET(_signalFd[0], &efds);
  }
  int maxFd = _deviceFd < _signalFd[0] ? _signalFd[0] : _deviceFd;
  maxFd += 1;

  retval = select(maxFd, &rfds, NULL, &efds, &tv);
  INFO(" return %d ", retval);
  if (retval < 0) {
    WARN(" select() : error : %s (%d)", strerror(errno), errno);
    returnSignal = SELECT_ERROR;
  } else if (retval > 0) {  // one of the fd was set
    if (FD_ISSET(_deviceFd, &rfds)) {
      returnSignal = DEVICE_RXD;
    }
    if (FD_ISSET(_signalFd[0], &rfds)) {
      ::read(_signalFd[0], &buffer, 1);  // read 1 event
      returnSignal = (Signal)buffer;
    }
    if (FD_ISSET(_deviceFd, &efds)) {
      WARN("device  error : %s (%d)", strerror(errno), errno);
      returnSignal = DEVICE_ERROR;
    }
    if (FD_ISSET(_signalFd[0], &efds)) {
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

  if (_deviceFd == -1) {
    ERROR("connect: Unable to open '%s' errno : %d : %s ", _device.c_str(),
          errno, strerror(errno));
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

  INFO("Driver version is %d.%d.%d.\n", version >> 16, (version >> 8) & 0xff,
       version & 0xff);
  INFO(" Axes : %d Buttons : %d ", axes, buttons);
  _deviceConnected = true;
  return E_OK;
}

void Joystick2Mqtt::deviceDisconnect() {
  close(_deviceFd);
  _deviceFd = 0;
  _deviceConnected = false;
}

void Joystick2Mqtt::deviceRxd() {
  if (!_deviceConnected) return;
  int erc;

  while (true) {
    struct js_event js;
    erc = read(_deviceFd, &js, sizeof(struct js_event));
    if (erc == sizeof(struct js_event)) {
      switch (js.type & ~JS_EVENT_INIT) {
        case JS_EVENT_BUTTON:
          INFO(" button : %d = %d ", js.number, js.value);
          string_format(_topic, "%s/button%d", _mqttSrc.c_str(), js.number);
          string_format(_value, "%d", js.value);
          mqtt.publish(_topic, _value, 0, 0);
          _button[js.number] = js.value;
          break;
        case JS_EVENT_AXIS:
          INFO(" axis : %d = %d ", js.number, js.value);
          string_format(_topic, "%s/axis%d", _mqttSrc.c_str(), js.number);
          string_format(_value, "%d", js.value);
          mqtt.publish(_topic, _value, 0, 0);
          _axis[js.number] = js.value;
          break;
      }

    } else if (erc < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) return;
      DEBUG(" read returns %d => errno : %d = %s", erc, errno, strerror(errno));
      signal(DEVICE_ERROR);
    } else {
      return;
    }
  }
}
