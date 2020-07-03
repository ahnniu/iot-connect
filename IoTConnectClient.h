#ifndef __IOT_CONNECT_CLIENT_H__
#define __IOT_CONNECT_CLIENT_H__

#include "mbed.h"
#include "IoTConnectEntry.h"
#include "IoTConnectDevice.h"
#include <MQTTClientMbedOs.h>
#include "IoTConnectError.h"

// typedef struct {
//     MQTT::Message msg;
//     char* buf;
// }IoTConnectMessage;

#define MQTT_PUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_PUB_BUFFER_MAX
#define MQTT_SUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_SUB_BUFFER_MAX
#define MQTT_CLIENT_THREAD_STACK_SIZE MBED_CONF_IOT_CONNECT_MQTT_CLIENT_THREAD_STACK_SIZE


class IoTConnectClient
{

public:
    CircularBuffer<MQTT::Message*, MQTT_PUB_BUFFER_MSG_NUMBER> pubs;
    CircularBuffer<MQTT::Message*, MQTT_SUB_BUFFER_MSG_NUMBER> subs;

    Callback<void(MQTT::Message*)> on_received;

public:
    IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device);
    ~IoTConnectClient();

    int connect();
    int disconnect();
    bool is_connected();
    // void set_event_handler(Callback<void()> _on_connection_lost);

    int subscribe(MQTT::QoS qos, Callback<void(MQTT::Message*)> _on_received);
    int pub(MQTT::Message* _msg);

    int start_main_loop();

private:
    IoTConnectAuthType auth_type;
    const IoTConnectEntry* entry;
    IoTConnectDevice* device;
    TLSSocket* socket;
    NetworkInterface* network;
    MQTTClient* mqtt_client;


    Thread thread;


private:

    void thread_main_loop();
    // void sub_handle_internal(MQTT::MessageData& _data);
};


// This is a template class, it is usually in a single .h file
// Blow is the implemention


#endif