#ifndef _MQTT_H_
#define _MQTT_H_
#include <Bytes.h>
#include <Log.h>

#include <string>

#include "MQTTAsync.h"

using namespace std;

class Mqtt {
 public:
  typedef enum {
    MS_CONNECTED,
    MS_DISCONNECTED,
    MS_CONNECTING
  } MqttConnectionState;
  typedef void (*StateChangeCallback)(void*, MqttConnectionState);
  typedef void (*OnMessageCallback)(void*, string, string);

 private:
  MQTTAsync_token _deliveredtoken;
  MQTTAsync _client;
  string _connection;
  string _clientId;
  int _keepAliveInterval;

  MqttConnectionState _connectionState;
  void state(MqttConnectionState newState);
  string _lastWillTopic;
  string _lastWillMessage;
  int _lastWillQos;
  bool _lastWillRetain;
  StateChangeCallback _stateChangeCallback;
  void* _stateChangeContext;
  OnMessageCallback _onMessageCallback;
  void* _onMessageContext;

  static void onConnectionLost(void* context, char* cause);
  static int onMessage(void* context, char* topicName, int topicLen,
                       MQTTAsync_message* message);
  static void onDisconnect(void* context, MQTTAsync_successData* response);
  static void onConnectFailure(void* context, MQTTAsync_failureData* response);
  static void onConnectSuccess(void* context, MQTTAsync_successData* response);
  static void onSubscribeSuccess(void* context,
                                 MQTTAsync_successData* response);
  static void onSubscribeFailure(void* context,
                                 MQTTAsync_failureData* response);
  static void onPublishSuccess(void* context, MQTTAsync_successData* response);
  static void onPublishFailure(void* context, MQTTAsync_failureData* response);
  static void onDeliveryComplete(void* context, MQTTAsync_token response);

 public:
  Mqtt();
  ~Mqtt();
  void init();
  int connection(string);
  int client(string);
  int connect();
  int disconnect();
  int publish(string topic, string message, int qos, bool retain);
  int publish(string topic, Bytes message, int qos, bool retain);
  int subscribe(string topic);
  int lastWill(string topic, string message, int qos, bool retain);
  void onStateChange(void* context, StateChangeCallback fp);
  void onMessage(void* context, OnMessageCallback);
  MqttConnectionState state();
};
#endif