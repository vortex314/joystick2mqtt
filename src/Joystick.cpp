#include <Joystick.h>
Joystick::Joystick() {}

int Joystick::device(string device) {
  _device = device;
  return 0;
}

int Joystick::connect() {
  INFO("Connecting to '%s' ....", _device.c_str());
  fd = ::open(_device.c_str(), O_EXCL | O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd == -1) {
    ERROR("connect: Unable to open '%s' errno : %d : %s ", _device.c_str(),
          errno, strerror(errno));
    fd = 0;
    return errno;
  }

  INFO("open '%s' succeeded.fd=%d", _device.c_str(), fd);
  int version = 0x000800;
  unsigned char axes = 2;
  unsigned char buttons = 2;
#define NAME_LENGTH 128
  char name[NAME_LENGTH] = "Unknown";

  ioctl(fd, JSIOCGVERSION, &version);
  ioctl(fd, JSIOCGAXES, &axes);
  ioctl(fd, JSIOCGBUTTONS, &buttons);
  ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);
  if (_axis == 0) _axis = (int*)calloc(_axes, sizeof(int));
  if (_button == 0) _button = (char*)calloc(_buttons, sizeof(char));

  INFO("Driver version is %d.%d.%d.", version >> 16, (version >> 8) & 0xff,
       version & 0xff);
  INFO(" Axes : %d Buttons : %d ", axes, buttons);
  _connected = true;
  return 0;
}

int Joystick::disconnect() {
  if (!_connected) return 0;
  ::close(fd);
  fd = -1;
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

int Joystick::getEvent(EventClass& cls, int& instance, int& value) {
  int rc;
  struct js_event js;
  rc = read(fd, &js, sizeof(struct js_event));
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
