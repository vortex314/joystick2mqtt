#include <Joystick.h>

Joystick::Joystick(Thread &thread, const char *name)
    : Actor(thread), buttonEvent(20, "buttonEvent"), axisEvent(20, "axisEvent"),
      deviceLookupTimer(thread, 2000, true, "deviceLookup"),
      devicePollTimer(thread, 10, true, "devicePoll") {}

bool Joystick::config(JsonObject &json) {
  std::string device = json["device"];
  _device = device;
  return true;
}
typedef enum { ET_RXD, ET_TXD, ET_ERROR, ET_TIMEOUT, ET_FAIL } EventType;

EventType pollFd(int fd, uint32_t timeout) {
  fd_set rfds;
  fd_set wfds;
  fd_set efds;
  struct timeval tv;
  int rc;
  // Wait up to 1000 msec.
  uint64_t delta = timeout;
  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (delta * 1000) % 1000000;

  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  FD_ZERO(&efds);
  FD_SET(fd, &rfds);
  FD_SET(fd, &efds);
  int maxFd = fd + 1;

  rc = select(maxFd, &rfds, NULL, &efds, &tv);
  if (rc < 0) {
    WARN(" select() : error : %s (%d)", strerror(errno), errno);
    return ET_FAIL;
  } else if (rc > 0) { // one of the fd was set
    if (FD_ISSET(fd, &rfds)) {
      return ET_RXD;
    } else if (FD_ISSET(fd, &efds)) {
      WARN("device  error : %s (%d)", strerror(errno), errno);
      return ET_ERROR;
    }
  }
  TRACE(" timeout %llu", Sys::millis());
  return ET_TIMEOUT;
}

bool Joystick::init() {
  deviceLookupTimer >> [&](const TimerMsg &tm) {
    if (exists() && connected())
      return;
    if (exists() && !connected()) {
      connect();
    } else if (!exists() && connected()) {
      disconnect();
    } else if (!exists() && !connected()) {
      INFO(" waiting for device %s", _device.c_str());
    }
  };
  devicePollTimer >> [&](const TimerMsg &tm) {
    if (connected()) {
      EventType et;
      while (true) {
        et = pollFd(_fd, 20);
        switch (et) {
        case ET_RXD: {
          emitEvent();
          break;
        }
        case ET_TIMEOUT: {
          return;
        }
        case ET_FAIL: {
          ERROR("select fail '%s' errno : %d : %s ", _device.c_str(), errno,
                strerror(errno));
          return;
        }
        case ET_ERROR: {
          ERROR("select error '%s' errno : %d : %s ", _device.c_str(), errno,
                strerror(errno));
          return;
        }
        }
      }
    }
  };
  return true;
}

void Joystick::emitEvent() {
  while (true) {
    Joystick::EventClass cls;
    int instance;
    int value;
    if (getEvent(cls, instance, value))
      break;
    if (cls == EV_AXIS) {
      axisEvent.on({instance, value});
    } else if (cls == EV_BUTTON) {
      buttonEvent.on({instance, value});
    } else {
      INFO(" unknown event ");
    }
  }
}

int Joystick::connect() {
  INFO("Connecting to '%s' ....", _device.c_str());
  _fd = ::open(_device.c_str(), O_EXCL | O_RDWR | O_NOCTTY | O_NDELAY);

  if (_fd == -1) {
    ERROR("connect: Unable to open '%s' errno : %d : %s ", _device.c_str(),
          errno, strerror(errno));
    _fd = -1;
    return errno;
  }

  INFO("open '%s' succeeded.fd=%d", _device.c_str(), _fd);
  int version = 0x000800;
  unsigned char axes = 2;
  unsigned char buttons = 2;
#define NAME_LENGTH 128
  char name[NAME_LENGTH] = "Unknown";

  ioctl(_fd, JSIOCGVERSION, &version);
  ioctl(_fd, JSIOCGAXES, &axes);
  ioctl(_fd, JSIOCGBUTTONS, &buttons);
  ioctl(_fd, JSIOCGNAME(NAME_LENGTH), name);

  if (_axis == 0)
    _axis = (int *)calloc(_axes, sizeof(int));
  if (_button == 0)
    _button = (char *)calloc(_buttons, sizeof(char));

  INFO("Driver version is %d.%d.%d.", version >> 16, (version >> 8) & 0xff,
       version & 0xff);
  INFO("Axes : %d Buttons : %d ", axes, buttons);
  // signal handling
  /*
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGIO, &sa, NULL))
      ERROR("sigaction failed ");

    raise(SIGIO);

    if (fcntl(_fd, F_SETOWN, getpid()) == -1)
      ERROR("fnctl to set F_SETOWN failed");

    if (fcntl(_fd, F_SETFL, O_ASYNC) < 0)
      ERROR("failed to set async signal %d : %s ", errno, strerror(errno));

    if (fcntl(_fd, F_SETSIG, SIGIO) < 0)
      ERROR("failed to set setsig signal %d : %s ", errno, strerror(errno));
  */
  _connected = true;
  INFO("connected ");
  return 0;
}

int Joystick::disconnect() {
  if (!_connected)
    return 0;
  ::close(_fd);
  _fd = -1;
  _connected = false;
  return 0;
}

bool Joystick::exists() {
  struct stat buf;
  if (lstat(_device.c_str(), &buf) < 0) {
    return false;
  } else {
    return true;
  }
}

bool Joystick::connected() { return _connected; }

int Joystick::getEvent(EventClass &cls, int &instance, int &value) {
  int rc;
  struct js_event js;
  rc = read(_fd, &js, sizeof(struct js_event));
  if (rc == sizeof(struct js_event)) {
    instance = js.number;
    value = js.value;
    int kind = (js.type & ~JS_EVENT_INIT);
    if (kind == JS_EVENT_BUTTON)
      cls = EV_BUTTON;
    else if (kind == JS_EVENT_AXIS)
      cls = EV_AXIS;
    else
      cls = EV_INIT;
    return 0;
  }
  return ENODATA;
}
