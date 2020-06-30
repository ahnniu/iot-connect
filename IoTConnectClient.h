#ifndef __IOT_CONNECT_CLIENT_H__
#define __IOT_CONNECT_CLIENT_H__

#include "mbed.h"
#include "mbed_trace.h"
#include "IoTConnectEntry.h"
#include "IoTConnectDevice.h"
#include "AzureRootCert.h"
#include "IoTConnectClient.h"
#include <MQTTClientMbedOs.h>

// typedef struct {
//     MQTT::Message msg;
//     char* buf;
// }IoTConnectMessage;


typedef enum {
    IOT_CONNECT_ERROR_OK                     =  0,        /*!< no error */

    IOT_CONNECT_ERROR_INVAL                  = -1001,

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

template<int MQTT_PUB_BUFFER_MSG_NUMBER = 1 , int MQTT_SUB_BUFFER_MSG_NUMBER = 1>
class IoTConnectClient
{
private:
    IoTConnectAuthType auth_type;
    const IoTConnectEntry* entry;
    IoTConnectDevice* device;
    TLSSocket* socket;
    NetworkInterface* network;
    MQTTClient* mqtt_client;

    CircularBuffer<MQTT::Message, MQTT_PUB_BUFFER_MSG_NUMBER> pubs;
    CircularBuffer<MQTT::Message, MQTT_SUB_BUFFER_MSG_NUMBER> subs;

public:
    IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device);
    ~IoTConnectClient();

    int connect();

    // void pub(MQTT::Message& _msg);
};


// This is a template class, it is usually in a single .h file
// Blow is the implemention

#define TRACE_GROUP  "IoTConnectClient"

template<int p, int s>
IoTConnectClient<p, s>::IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device) :
    network(_network),
    device(_device),
    auth_type(IOT_CONNECT_AUTH_SYMMETRIC_KEY),
    entry(NULL),
    socket(new TLSSocket),
    mqtt_client(NULL)
{
    if (_device) {
        entry = _device->get_entry();
        auth_type = _device->get_auth_type();
    }

}

template<int p, int s>
int IoTConnectClient<p, s>::connect()
{
    int ret = socket->open(network);
    if (ret != NSAPI_ERROR_OK) {
        tr_error("Could not open socket! Error code: %d", ret);
        return ret;
    }

    {
        ret = socket->set_root_ca_cert(azure_root_certs);
        if (ret != NSAPI_ERROR_OK) {
            tr_error("Could not set ca cert! Returned %d\n", ret);
            return ret;
        }
    }

    if (auth_type == IOT_CONNECT_AUTH_CLIENT_SIDE_CERT) {

        const char* _client_cert_pem;
        const char* _client_key_pem;

        _client_cert_pem = device->get_cert_pem();
        _client_key_pem = device->get_private_key_pem();


        if (!_client_cert_pem || !_client_key_pem) {
            ret = IOT_CONNECT_ERROR_INVAL;
            tr_error("Client Cert pem or private key is null with cert auth type! Error code: %d", ret);
            return ret;
        }

        ret = socket->set_client_cert_key(_client_cert_pem, _client_key_pem);
        if (ret != NSAPI_ERROR_OK) {
            tr_error("Could not set keys! Returned %d\n", ret);
            return ret;
        }

    }


    {
        SocketAddress a;
        network->gethostbyname(entry->get_mqtt_server_host_name(), &a);
        a.set_port(entry->get_mqtt_port());
        tr_info("Try to connect to %s(ip: %s):%d",
                entry->get_mqtt_server_host_name(),
                a.get_ip_address(), a.get_port()
        );
        socket->set_timeout(30000);
        ret = socket->connect(a);

        if (ret != NSAPI_ERROR_OK) {
            tr_error("Could not connect! Returned %d\n", ret);
            return ret;
        }

        tr_info("Connection established");
    }

    mqtt_client = new MQTTClient(socket);

    tr_info("MQTT Client is trying to connect to the service ...\n");
    {
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        // Azure Iot Hub - Should be be 4 (3.1.1)
        data.MQTTVersion = 4;
        data.clientID.cstring = (char*)device->get_client_id();
        data.username.cstring = (char*)device->get_user_name();
        data.password.cstring = (char*)device->get_pwd();

        tr_debug("MQTT Client - mqtt version:\t\%s", data.MQTTVersion == 4 ? "3.1.1" : "3.1");
        tr_debug("MQTT Client - client id:\t%s", data.clientID.cstring);
        tr_debug("MQTT Client - username:\t%s", data.username.cstring);
        tr_debug("MQTT Client - password:\t%s", data.password.cstring);

        int rc = mqtt_client->connect(data);
        if (rc != MQTT::SUCCESS) {
            tr_info("ERROR: rc from MQTT connect is %d\n", rc);
            return rc;
        }
    }

    tr_info("MQTT Client is connected\n");

    return 0;
}


#endif