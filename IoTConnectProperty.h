#ifndef __IOT_CONNECT_PROPERTY_H__
#define __IOT_CONNECT_PROPERTY_H__

#include "jsmn.h"
#include "IoTConnectError.h"

#define IOT_CONNECT_PROPERTY_KEYS_MAX 32


class IoTConnectStringProperty {

public:
    IoTConnectStringProperty(const char* _key, const char* _value);
    ~IoTConnectStringProperty();

    const char* get_key() const;
    const char* get_value();
    void set_value(const char* _new_value);

private:
    const char* key;
    char* buf;
};


class IoTConnectProperty
{
public:
    IoTConnectProperty();
    ~IoTConnectProperty();

    int add(IoTConnectStringProperty* _prop, Callback<void(void*)> _on_change = NULL);
    int prop(const char* _key, void** _obj, jsmntype_t* _type = 0);
    void* prop(const char* _key);

    int to_json();

private:

    int calc_json_str_len();

private:

    typedef struct {
        const char* key;
        jsmntype_t type;
        void* obj;
        Callback<void(void*)> on_change;
    }PropToken;

    PropToken tokens[IOT_CONNECT_PROPERTY_KEYS_MAX];
    char* jstr;
};


#endif