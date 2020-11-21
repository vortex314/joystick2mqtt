#include "Joystick2Mqtt.h"
const char* signalString[] = {"PIPE_ERROR",
                              "SELECT_ERROR",
                              "DEVICE_CONNECT",
                              "DEVICE_DISCONNECT",
                              "DEVICE_RXD",
                              "DEVICE_ERROR",
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

Joystick2Mqtt::Joystick2Mqtt() {}

Joystick2Mqtt::~Joystick2Mqtt() {}

void Joystick2Mqtt::setConfig(Config config) { _config = config; }

void Joystick2Mqtt::device(string devJs) {
  _joystick.device(devJs);
  _device = devJs;
}

void Joystick2Mqtt::setLogFd(FILE* logFd) { _logFd = logFd; }

void Joystick2Mqtt::init() {
  _startTime = Sys::millis();
  _mqttDevice = Sys::hostname();
  int offset = _device.find_last_of('/');
  _mqttObject = _device.substr(offset + 1);
  _mqttSrc = "src/" + _mqttDevice + "/" + _mqttObject;
  _mqttSubscribedTo = "dst/" + _mqttDevice + "/#";
  _mqttClientId = _mqttObject + std::to_string(getpid());
  INFO(" MQTT device : %s object : %s ", _mqttDevice.c_str(),
       _mqttObject.c_str());

  if (pipe(_signalFd) < 0) {
    INFO("Failed to create pipe: %s (%d)", strerror(errno), errno);
  }

  JsonObject jo = _config.root()["mqtt"];
  _mqtt.config(jo);
  _mqtt.onStateChange(this, [](void* context, Mqtt::MqttConnectionState state) {
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
    if (_mqtt.state() == Mqtt::MS_DISCONNECTED) {
      _mqtt.connect();
    }
  });
  deviceConnectTimer.atInterval(5000).doThis([this]() {
    if (!_joystick.connected() && _mqtt.state() == Mqtt::MS_CONNECTED) {
      _joystick.connect();
    }
  });
  mqttPublishTimer.atInterval(1000).doThis([this]() {
    std::string sUpTime = std::to_string((Sys::millis() - _startTime) / 1000);
    _mqtt.publish(_mqttSrc + "/alive", "true");
    _mqtt.publish("src/" + _mqttDevice + "/system/upTime", sUpTime);
    _mqtt.publish(_mqttSrc + "/device", _device);
  });
  deviceTimer.atInterval(_deviceExistTimer).doThis([this]() {
    if (!_joystick.exists()) _joystick.disconnect();
    _mqtt.publish(_mqttSrc + "/connected",
                  _joystick.connected() ? "true" : "false");
  });
  if (_mqtt.state() == Mqtt::MS_DISCONNECTED) _mqtt.connect();
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
          Joystick::EventClass cls;
          int instance;
          int value;
          while (_joystick.getEvent(cls, instance, value) == 0) {
            string_format(_value, "%d", value);
            if (cls == Joystick::EV_BUTTON)
              string_format(_topic, "%s/button%d", _mqttSrc.c_str(), instance);
            else if (cls == Joystick::EV_AXIS)
              string_format(_topic, "%s/axis%d", _mqttSrc.c_str(), instance);
            else
              WARN(" unexpected event %d = %d ", instance, value);
            _mqtt.publish(_topic, _value);
          }
          break;
        }
        case DEVICE_ERROR: {
          _joystick.disconnect();
          break;
        }
        case MQTT_CONNECTED: {
          INFO("MQTT_CONNECT_SUCCESS %s ", _mqttDevice.c_str());
          _mqtt.subscribe(_mqttSubscribedTo);
          _joystick.connect();
          break;
        }
        case MQTT_DISCONNECTED: {
          WARN("MQTT_DISCONNECTED %s ", _mqttDevice.c_str());
          _joystick.disconnect();
          break;
        }
        case MQTT_SUBSCRIBE_SUCCESS: {
          INFO("MQTT_SUBSCRIBE_SUCCESS %s ", _mqttDevice.c_str());
          break;
        }
        case MQTT_SUBSCRIBE_FAIL: {
          WARN("MQTT_SUBSCRIBE_FAIL %s ", _mqttDevice.c_str());
          _mqtt.disconnect();
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
  if (_joystick.connected()) {
    FD_SET(_joystick.fd, &rfds);
    FD_SET(_joystick.fd, &efds);
  }
  if (_signalFd[0]) {
    FD_SET(_signalFd[0], &rfds);
    FD_SET(_signalFd[0], &efds);
  }
  int maxFd = _joystick.fd < _signalFd[0] ? _signalFd[0] : _joystick.fd;
  maxFd += 1;

  retval = select(maxFd, &rfds, NULL, &efds, &tv);
  if (retval < 0) {
    WARN(" select() : error : %s (%d)", strerror(errno), errno);
    returnSignal = SELECT_ERROR;
  } else if (retval > 0) {  // one of the fd was set
    if (FD_ISSET(_joystick.fd, &rfds)) {
      returnSignal = DEVICE_RXD;
    }
    if (FD_ISSET(_signalFd[0], &rfds)) {
      ::read(_signalFd[0], &buffer, 1);  // read 1 event
      returnSignal = (Signal)buffer;
    }
    if (FD_ISSET(_joystick.fd, &efds)) {
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
