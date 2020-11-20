#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_
#include <Log.h>
#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
using namespace std;
class Joystick {
 public:
  typedef void (*EventHandler)(int kind, int value);

 private:
  string _device;
  bool _deviceConnected;
  EventHandler _eventHandler;
  void* _eventHandlerContext;
  unsigned char _axes = 2;
  unsigned char _buttons = 2;
  int* _axis;
  char* _button;

 public:
  int fd;
  Joystick(string device);
  int open();
  int close();
  bool exists();
  bool connected();
  int onEvent(void* context, EventHandler handler);
  int getEvent(int& kind, int& value);
};
#endif