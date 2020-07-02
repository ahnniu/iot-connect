#ifndef __IOT_CONNECT_CLIENT_H__
#define __IOT_CONNECT_CLIENT_H__

#include "mbed.h"
#include "IoTConnectEntry.h"
#include "IoTConnectDevice.h"
#include <MQTTClientMbedOs.h>

// typedef struct {
//     MQTT::Message msg;
//     char* buf;
// }IoTConnectMessage;

#define MQTT_PUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_PUB_BUFFER_MAX
#define MQTT_SUB_BUFFER_MSG_NUMBER MBED_CONF_IOT_CONNECT_MQTT_SUB_BUFFER_MAX
#define MQTT_CLIENT_THREAD_STACK_SIZE MBED_CONF_IOT_CONNECT_MQTT_CLIENT_THREAD_STACK_SIZE


typedef enum {
    IOT_CONNECT_ERROR_OK                     =  0,        /*!< no error */

    IOT_CONNECT_ERROR_INVAL                  = -1001,
    IOT_CONNECT_ERROR_OUT_OF_MEM             = -1002,

    IOT_CONNECT_ERROR_CLIENT_PUB_FULL        = -1101,
    IOT_CONNECT_ERROR_CLIENT_SUB_OVERFLOW    = -1102,
    IOT_CONNECT_ERROR_CLIENT_SUB             = -1103,
    IOT_CONNECT_ERROR_CLIENT_OUT_OF_INSTANCE = -1104,
    IOT_CONNECT_ERROR_CLIENT_THREAD          = -1105,


    IOT_CONNECT_ERROR_NS_WOULD_BLOCK         = -3001,     /*!< no data is not available but call is non-blocking */
    IOT_CONNECT_ERROR_NS_UNSUPPORTED         = -3002,     /*!< unsupported functionality */
    IOT_CONNECT_ERROR_NS_PARAMETER           = -3003,     /*!< invalid configuration */
    IOT_CONNECT_ERROR_NS_NO_CONNECTION       = -3004,     /*!< not connected to a network */
    IOT_CONNECT_ERROR_NS_NO_SOCKET           = -3005,     /*!< socket not available for use */
    IOT_CONNECT_ERROR_NS_NO_ADDRESS          = -3006,     /*!< IP address is not known */
    IOT_CONNECT_ERROR_NS_NO_MEMORY           = -3007,     /*!< memory resource not available */
    IOT_CONNECT_ERROR_NS_NO_SSID             = -3008,     /*!< ssid not found */
    IOT_CONNECT_ERROR_NS_DNS_FAILURE         = -3009,     /*!< DNS failed to complete successfully */
    IOT_CONNECT_ERROR_NS_DHCP_FAILURE        = -3010,     /*!< DHCP failed to complete successfully */
    IOT_CONNECT_ERROR_NS_AUTH_FAILURE        = -3011,     /*!< connection to access point failed */
    IOT_CONNECT_ERROR_NS_DEVICE_ERROR        = -3012,     /*!< failure interfacing with the network processor */
    IOT_CONNECT_ERROR_NS_IN_PROGRESS         = -3013,     /*!< operation (eg connect) in progress */
    IOT_CONNECT_ERROR_NS_ALREADY             = -3014,     /*!< operation (eg connect) already in progress */
    IOT_CONNECT_ERROR_NS_IS_CONNECTED        = -3015,     /*!< socket is already connected */
    IOT_CONNECT_ERROR_NS_CONNECTION_LOST     = -3016,     /*!< connection lost */
    IOT_CONNECT_ERROR_NS_CONNECTION_TIMEOUT  = -3017,     /*!< connection timed out */
    IOT_CONNECT_ERROR_NS_ADDRESS_IN_USE      = -3018,     /*!< Address already in use */
    IOT_CONNECT_ERROR_NS_TIMEOUT             = -3019,     /*!< operation timed out */
    IOT_CONNECT_ERROR_NS_BUSY                = -3020,     /*!< device is busy and cannot accept new operation */
}IoTConnectError;


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