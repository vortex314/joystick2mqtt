#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_
#include <ArduinoJson.h>
#include <Log.h>
#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <limero.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
struct ButtonEvent {
  int button;
  int value;
};
struct AxisEvent {
  int axis;
  int value;
};

class Joystick : public Actor {
public:
  typedef void (*EventHandler)(int kind, int value);
  typedef enum { EV_BUTTON, EV_AXIS, EV_INIT } EventClass;

  QueueFlow<ButtonEvent> buttonEvent;
  QueueFlow<AxisEvent> axisEvent;
  TimerSource deviceLookupTimer;
  TimerSource devicePollTimer;

private:
  std::string _device;
  int _fd;
  bool _connected;
  EventHandler _eventHandler;
  void *_eventHandlerContext;
  unsigned char _axes = 2;
  unsigned char _buttons = 2;
  int *_axis = 0;
  char *_button = 0;

  void emitEvent();

public:
  Joystick(Thread &thread, const char *name = "joystick");
  bool config(JsonObject &json);
  bool init();
  int connect();
  int disconnect();
  bool exists();
  bool connected();
  int onEvent(void *context, EventHandler handler);
  int getEvent(EventClass &ev, int &instance, int &value);
};
#endif