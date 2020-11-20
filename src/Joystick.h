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
  typedef enum { EV_BUTTON, EV_AXIS, EV_INIT } EventClass;

 private:
  string _device;
  bool _connected;
  EventHandler _eventHandler;
  void* _eventHandlerContext;
  unsigned char _axes = 2;
  unsigned char _buttons = 2;
  int* _axis = 0;
  char* _button = 0;

 public:
  int fd;
  Joystick();
  int device(string);
  int connect();
  int disconnect();
  bool exists();
  bool connected();
  int onEvent(void* context, EventHandler handler);
  int getEvent(EventClass& ev, int& instance, int& value);
};
#endif