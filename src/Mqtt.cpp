#include <Mqtt.h>
const char* mqttConnectionStates[] = {"MS_CONNECTED", "MS_DISCONNECTED",
                                      "MS_CONNECTING", "MS_DISCONNECTING"};

Mqtt::Mqtt() {
  _connectionState = MS_DISCONNECTED;
  _connection = "tcp://test.mosquitto.org:1883";
}

Mqtt::~Mqtt() {}

int Mqtt::lastWill(string topic, string message, int qos, bool retain) {
  _lastWillMessage = message;
  _lastWillQos = qos;
  _lastWillTopic = topic;
  _lastWillRetain = retain;
  return 0;
}

int Mqtt::connection(string connectionString) {
  _connection = connectionString;
  return 0;
}
int Mqtt::client(string cl) {
  _clientId = cl;
  return 0;
}

void Mqtt::onStateChange(void* context, Mqtt::StateChangeCallback fp) {
  _stateChangeContext = context;
  _stateChangeCallback = fp;
}

void Mqtt::state(MqttConnectionState newState) {
  INFO(" MQTT connection state %s => %s ",
       mqttConnectionStates[_connectionState], mqttConnectionStates[newState]);

  if (_connectionState != MS_CONNECTED && newState == MS_CONNECTED) {
    _stateChangeCallback(_stateChangeContext, MS_CONNECTED);
  } else if (_connectionState != MS_DISCONNECTED &&
             newState == MS_DISCONNECTED) {
    _stateChangeCallback(_stateChangeContext, MS_DISCONNECTED);
  }
  _connectionState = newState;
}

Mqtt::MqttConnectionState Mqtt::state() { return _connectionState; }

int Mqtt::connect() {
  int rc;
  if (_connectionState == MS_CONNECTING || _connectionState == MS_CONNECTED)
    return 0;

  MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
  MQTTAsync_willOptions will_opts = MQTTAsync_willOptions_initializer;

  INFO(" MQTT connecting %s ...  ", _connection.c_str());
  state(MS_CONNECTING);
  MQTTAsync_create(&_client, _connection.c_str(), _clientId.c_str(),
                   MQTTCLIENT_PERSISTENCE_NONE, NULL);

  MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessage,
                         onDeliveryComplete);  // TODO add ondelivery

  conn_opts.keepAliveInterval = _keepAliveInterval;
  conn_opts.cleansession = 1;
  conn_opts.onSuccess = onConnectSuccess;
  conn_opts.onFailure = onConnectFailure;
  conn_opts.context = this;
  will_opts.message = _lastWillTopic.c_str();
  will_opts.topicName = _lastWillTopic.c_str();
  will_opts.qos = _lastWillQos;
  will_opts.retained = _lastWillRetain;
  conn_opts.will = &will_opts;
  if ((rc = MQTTAsync_connect(_client, &conn_opts)) != MQTTASYNC_SUCCESS) {
    WARN("Failed to start connect, return code %d", rc);
    return ECONNREFUSED;
  }
  state(MS_CONNECTING);
  return 0;
}

int Mqtt::disconnect() {
  MQTTAsync_disconnectOptions disc_opts =
      MQTTAsync_disconnectOptions_initializer;
  disc_opts.onSuccess = onDisconnect;
  disc_opts.context = this;
  int rc = 0;
  if ((rc = MQTTAsync_disconnect(_client, &disc_opts)) != MQTTASYNC_SUCCESS) {
    WARN("Failed to disconnect, return code %d", rc);
    return EIO;
  }
  MQTTAsync_destroy(&_client);
  state(MS_DISCONNECTED);
  return 0;
}

int Mqtt::subscribe(string topic) {
  int qos = 0;
  if (state() != MS_CONNECTED) return 0;
  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
  INFO("Subscribing to topic %s for client %s using QoS%d", topic.c_str(),
       _clientId.c_str(), qos);
  opts.onSuccess = onSubscribeSuccess;
  opts.onFailure = onSubscribeFailure;
  opts.context = this;
  int rc = 0;

  if ((rc = MQTTAsync_subscribe(_client, topic.c_str(), qos, &opts)) !=
      MQTTASYNC_SUCCESS) {
    ERROR("Failed to start subscribe, return code %d", rc);
    return EAGAIN;
  } else {
    INFO(" subscribe send ");
  }
  return 0;
}

void Mqtt::onConnectionLost(void* context, char* cause) {
  Mqtt* me = (Mqtt*)context;
  me->state(MS_DISCONNECTED);
}

int Mqtt::onMessage(void* context, char* topicName, int topicLen,
                    MQTTAsync_message* message) {
  Mqtt* me = (Mqtt*)context;
  string msg((char*)message->payload, message->payloadlen);
  string topic(topicName, topicLen);

  me->_onMessageCallback(me->_onMessageContext, topic, msg);

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);
  return 1;
}

void Mqtt::onDeliveryComplete(void* context, MQTTAsync_token response) {
  //    Mqtt* me = (Mqtt*)context;
}

void Mqtt::onDisconnect(void* context, MQTTAsync_successData* response) {
  Mqtt* me = (Mqtt*)context;
  me->state(MS_DISCONNECTED);
}

void Mqtt::onConnectFailure(void* context, MQTTAsync_failureData* response) {
  Mqtt* me = (Mqtt*)context;
  me->state(MS_DISCONNECTED);
}

void Mqtt::onConnectSuccess(void* context, MQTTAsync_successData* response) {
  Mqtt* me = (Mqtt*)context;
  me->state(MS_CONNECTED);
}

void Mqtt::onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
  Mqtt* me = (Mqtt*)context;
}

void Mqtt::onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
  Mqtt* me = (Mqtt*)context;
}

int Mqtt::publish(string topic, string message, int qos, bool retained) {
  if (!_connectionState == MS_CONNECTED) {
    return 0;
  }
  INFO(" MQTT PUB : %s = %s ", topic.c_str(), message.c_str());

  qos = 1;
  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
  MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

  //   INFO("mqttPublish %s",topic.c_str());

  int rc = E_OK;
  opts.onSuccess = onPublishSuccess;
  opts.onFailure = onPublishFailure;
  opts.context = this;

  pubmsg.payload = (void*)message.data();
  pubmsg.payloadlen = message.length();
  pubmsg.qos = qos;
  pubmsg.retained = retained;

  if ((rc = MQTTAsync_sendMessage(_client, topic.c_str(), &pubmsg, &opts)) !=
      MQTTASYNC_SUCCESS) {
    state(MS_DISCONNECTED);
    ERROR("MQTTAsync_sendMessage failed.");
    return ECOMM;
  }
  return 0;
}
void Mqtt::onPublishSuccess(void* context, MQTTAsync_successData* response) {
  Mqtt* me = (Mqtt*)context;
}
void Mqtt::onPublishFailure(void* context, MQTTAsync_failureData* response) {
  Mqtt* me = (Mqtt*)context;
}
