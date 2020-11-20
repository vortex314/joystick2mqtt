#include <Joystick.h>

Joystick::Joystick(string device) { _device = device; }

int Joystick::open() {
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
  _axis = (int*)calloc(_axes, sizeof(int));
  _button = (char*)calloc(_buttons, sizeof(char));

  INFO("Driver version is %d.%d.%d.", version >> 16, (version >> 8) & 0xff,
       version & 0xff);
  INFO(" Axes : %d Buttons : %d ", axes, buttons);
  _deviceConnected = true;
  return E_OK;
  return 0;
}

int Joystick::close() {
  close(fd);
  fd = -1;
  _deviceConnected = false;
  return 0;
}

bool Joystick::exists() { return 0; }

bool Joystick::connected() { return _deviceConnected; }

int Joystick::getEvent(int& kind, int& value) { return 0; }
