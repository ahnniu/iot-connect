#ifndef __IOT_CONNECT_DEVICE_H__
#define __IOT_CONNECT_DEVICE_H__

#include "IoTConnectCommon.h"
#include "IoTConnectEntry.h"
#include "IoTConnectProperty.h"

typedef enum {
    IOT_CONNECT_AUTH_SYMMETRIC_KEY = 0,
    IOT_CONNECT_AUTH_CLIENT_SIDE_CERT = 1
}IoTConnectAuthType;

class IoTConnectDevice : public IoTConnectProperty {
private:
    const IoTConnectEntry* entry;

    const char* device_id;
    const char* device_name;
    const char* pwd;

    char* client_id;
    char* user_name;

    const char* cert_pem;
    const char* private_key_pem;

    const char* topic_pub;
    const char* topic_sub;


public:
    IoTConnectDevice(const char* _device_id, const char* _device_name, const char* _pwd, const IoTConnectEntry* _entry);

    const char* get_client_id() const;
    const char* get_user_name() const;
    const char* get_pwd() const;

    void set_cert(const char *_cert_pem, const char *_private_key_pem);
    const char* get_cert_pem() const;
    const char* get_private_key_pem() const;

    const IoTConnectEntry* get_entry() const;

    const char* get_mqtt_topic_pub() const;
    const char* get_mqtt_topic_sub() const;

    IoTConnectAuthType get_auth_type() const;

    ~IoTConnectDevice();

private:
    char* init_client_id(const char* _device_id, const IoTConnectEntry* _entry);
    char* init_user_name(const char* _client_id, const IoTConnectEntry* _entry);
    char* init_mqtt_topic_pub(const char* _client_id);
    char* init_mqtt_topic_sub(const char* _client_id);

};


#endif