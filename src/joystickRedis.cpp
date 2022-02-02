#include <Config.h>
#include <Hardware.h>
#include <Joystick.h>
#include "BrokerRedisJson.h"

#include <Log.h>
#include <limero.h>
#include <stdio.h>
#include <unistd.h>
Log logger;
Thread mainThread("main");
Thread deviceThread("device");
Joystick joystick(deviceThread);  // blocking thread
StaticJsonDocument<10240> jsonDoc;
/*
  load configuration file into JsonObject
*/
bool loadConfig(JsonDocument &doc, const char *name) {
  FILE *f = fopen(name, "rb");
  if (f == NULL) {
    ERROR(" cannot open config file : %s", name);
    return false;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *string = (char *)malloc(fsize + 1);
  fread(string, 1, fsize, f);
  fclose(f);

  string[fsize] = 0;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, string);
  if (error) {
    ERROR(" JSON parsing config file : %s failed.", name);
    return false;
  }
  return true;
}
/*

*/
FILE *logFd = 0;

void myLogFunction(uint8_t *s, size_t length) {
  fprintf(logFd, "%s\n", (const char*)s);
  fflush(logFd);
  fprintf(stdout, "%s\r\n", s);
}

void myLogInit(const char *logFile) {
  logFd = fopen(logFile, "a");
  if (logFd == NULL) {
    WARN("open logfile %s failed : %d %s", logFile, errno, strerror(errno));
  } else {
    logger.setWriter(myLogFunction);
  }
}

void commandArguments(JsonObject config, int argc, char **argv) {
  int opt;

  while ((opt = getopt(argc, argv, "f:m:l:v:")) != -1) {
    switch (opt) {
      case 'm':
        config["mqtt"]["connection"]=optarg;
        break;
      case 'f':
        config["configFile"]= optarg;
        break;
      case 'v': {
        char logLevel = optarg[0];
        config["log"]["level" ]=logLevel;
        break;
      }
      case 'l':
        config["log"]["file"]= optarg;
        break;
      default: /* '?' */
        fprintf(stderr,
                "Usage: %s [-v(TDIWE)] [-f configFile] [-l logFile] [-m "
                "mqtt_connection]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
  }
}
#include <LogFile.h>
LogFile logFile("wiringMqtt", 5, 2000000);

int main(int argc, char **argv) {
  Sys::init();
 INFO("Loading configuration." );
  JsonObject config;
  
  Thread brokerThread("worker");

  std::string dstPrefix="dst/joystick/";
  std::string srcPrefix="src/joystick/";

  JsonObject brokerConfig = config["broker"];

  INFO(" Launching Redis");
  BrokerRedis broker(brokerThread, brokerConfig);

  broker.init();
  broker.connect("joystick");

  config = jsonDoc["joystick"];
  joystick.config(config);
  joystick.init();

  joystick.axisEvent >> [srcPrefix,&broker](const AxisEvent &ae) {
    std::string topic = srcPrefix + "axis/" + std::to_string(ae.axis);
    std::string message = std::to_string(ae.value);
    broker.outgoing()->on({topic, ae.value});
  };

  joystick.buttonEvent >> [srcPrefix,&broker](const ButtonEvent &ae) {
    std::string topic = srcPrefix + "button/" + std::to_string(ae.button);
    std::string message = std::to_string(ae.value);
    broker.outgoing()->on({topic, ae.value});
  };

  deviceThread.start();
  brokerThread.start();
  mainThread.run();
}
