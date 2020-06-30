#include "mbed.h"
#include "IoTConnectEntry.h"
#include "IoTConnectDevice.h"


IoTConnectDevice::IoTConnectDevice(const char* _device_id, const char* _device_name,
                                   const char* _pwd, const IoTConnectEntry* _entry) :
    device_id(_device_id),
    device_name(_device_name),
    pwd(_pwd),
    cert_pem(NULL),
    private_key_pem(NULL),
    client_id(NULL),
    user_name(NULL),
    entry(_entry),
    topic_pub(NULL),
    topic_sub(NULL)
{
    client_id = init_client_id(_device_id, _entry);
    user_name = init_user_name(client_id, _entry);
    topic_pub = init_mqtt_topic_pub(client_id);
    topic_sub = init_mqtt_topic_sub(client_id);
}

IoTConnectDevice::~IoTConnectDevice()
{
    if (client_id) {
        delete[] client_id;
    }

    if (user_name) {
        delete[] user_name;
    }

    if (topic_pub) {
        delete[] topic_pub;
    }

    if (topic_sub) {
        delete[] topic_sub;
    }
}

const char* IoTConnectDevice::get_client_id() const
{
    return client_id;
}

const char* IoTConnectDevice::get_user_name() const
{
    return user_name;
}

const char* IoTConnectDevice::get_pwd() const
{
    return pwd;
}

void IoTConnectDevice::set_cert(const char *_cert_pem, const char *_private_key_pem)
{
    cert_pem = _cert_pem;
    private_key_pem = _private_key_pem;
}

const char* IoTConnectDevice::get_cert_pem() const
{
    return cert_pem;
}

const char* IoTConnectDevice::get_private_key_pem() const
{
    return private_key_pem;
}

char* IoTConnectDevice::init_client_id(const char* _device_id, const IoTConnectEntry* _entry)
{
    const char* cpid = _entry->get_cpid();
    // "{cpid}-{_device_id}"
    size_t buf_size = strlen(cpid) + strlen(_device_id) + 1 + 1;

    char* _client_id = new char[buf_size];
    memset(_client_id, 0, buf_size);

    if (strlen(cpid) > 0) {
        sprintf(_client_id, "%s-%s", cpid, _device_id);
    } else {
        sprintf(_client_id, "%s", _device_id);
    }

    return _client_id;
}

char* IoTConnectDevice::init_user_name(const char* _client_id, const IoTConnectEntry* _entry)
{
    const char* mqtt_server = _entry->get_mqtt_server_host_name();
    // "{mqtt_server_host_name}/{client_id}/?api-version=2018-06-30"
    size_t buf_size = strlen(mqtt_server) + strlen(_client_id) + 26;

    char* _user_name = new char[buf_size];
    memset(_user_name, 0, buf_size);
    sprintf(_user_name, "%s/%s/?api-version=2018-06-30", mqtt_server, _client_id);

    return _user_name;
}

const IoTConnectEntry* IoTConnectDevice::get_entry() const
{
    return entry;
}

char* IoTConnectDevice::init_mqtt_topic_pub(const char* _client_id)
{
    size_t buf_size = strlen(_client_id) + 30;

    char* _topic_pub = new char[buf_size];
    memset(_topic_pub, 0, buf_size);
    sprintf(_topic_pub, "devices/%s/messages/events/", _client_id);

    return _topic_pub;
}

char* IoTConnectDevice::init_mqtt_topic_sub(const char* _client_id)
{
    size_t buf_size = strlen(_client_id) + 36;

    char* _topic_sub = new char[buf_size];
    memset(_topic_sub, 0, buf_size);
    sprintf(_topic_sub, "devices/%s/messages/devicebound/#", _client_id);

    return _topic_sub;
}

const char* IoTConnectDevice::get_mqtt_topic_pub() const
{
    return topic_pub;
}

const char* IoTConnectDevice::get_mqtt_topic_sub() const
{
    return topic_sub;
}

IoTConnectAuthType IoTConnectDevice::get_auth_type() const
{
    return (cert_pem ? IOT_CONNECT_AUTH_CLIENT_SIDE_CERT : IOT_CONNECT_AUTH_SYMMETRIC_KEY);
}