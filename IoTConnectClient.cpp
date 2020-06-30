#include "mbed.h"
#include "mbed_trace.h"
#include "AzureRootCert.h"
#include "IoTConnectClient.h"
#include <MQTTClientMbedOs.h>

#define TRACE_GROUP  "IoTConnectClient"

IoTConnectClient::IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device) :
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

int IoTConnectClient::connect()
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
            ret = IOT_CONNECT_ERROR_INVAL_CLIENT_CA;
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