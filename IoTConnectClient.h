#ifndef __IOT_CONNECT_CLIENT_H__
#define __IOT_CONNECT_CLIENT_H__

#include "mbed.h"
#include "IoTConnectEntry.h"
#include "IoTConnectDevice.h"
#include <MQTTClientMbedOs.h>
#include "IoTConnectError.h"

#define MQTT_PUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_PUB_BUFFER_MAX
#define MQTT_SUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_SUB_BUFFER_MAX
#define MQTT_CLIENT_THREAD_STACK_SIZE MBED_CONF_IOT_CONNECT_MQTT_CLIENT_THREAD_STACK_SIZE


class IoTConnectClient
{

public:
    CircularBuffer<MQTT::Message*, MQTT_PUB_BUFFER_MSG_NUMBER> pubs;

    Callback<void(MQTT::Message*)> on_received;

public:
    IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device);
    ~IoTConnectClient();

    int connect();
    int disconnect();
    bool is_connected();
    void set_event_handler(Callback<void()> _on_connection_lost);

    int subscribe(MQTT::QoS qos, Callback<void(MQTT::Message*)> _on_received = NULL);
    int pub(MQTT::Message* _msg);

    int start_main_loop();

    void update_props_on_recieved(MQTT::Message* _msg);
    int pub_props(MQTT::QoS _qos = MQTT::QOS0);

private:
    IoTConnectAuthType auth_type;
    const IoTConnectEntry* entry;
    IoTConnectDevice* device;
    TLSSocket* socket;
    NetworkInterface* network;
    MQTTClient* mqtt_client;

    Thread thread;

    Callback<void()> on_connection_lost;

    int msg_id_pub_props;

private:

    void thread_main_loop();

};


#endif