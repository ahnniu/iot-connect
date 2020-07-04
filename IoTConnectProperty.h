#ifndef __IOT_CONNECT_PROPERTY_H__
#define __IOT_CONNECT_PROPERTY_H__

#include "jsmn.h"
#include "IoTConnectError.h"

#define IOT_CONNECT_PROPERTYS_MAX 10

typedef enum {
    IOT_CONNECT_PROPERTY_TYPE_UNDEFINED = JSMN_UNDEFINED,
    IOT_CONNECT_PROPERTY_TYPE_OBJECT = JSMN_OBJECT,
    IOT_CONNECT_PROPERTY_TYPE_ARRAY = JSMN_ARRAY,
    IOT_CONNECT_PROPERTY_TYPE_STRING = JSMN_STRING,
    IOT_CONNECT_PROPERTY_TYPE_PRIMITIVE = JSMN_PRIMITIVE,
    IOT_CONNECT_PROPERTY_TYPE_INT = JSMN_PRIMITIVE + 1,
    IOT_CONNECT_PROPERTY_TYPE_BOOL = JSMN_PRIMITIVE + 2,
    IOT_CONNECT_PROPERTY_TYPE_NULL = JSMN_PRIMITIVE + 3
} IoTConnectPropertyType;

class IoTConnectStringProperty {

public:
    IoTConnectStringProperty(const char* _key, const char* _value);
    IoTConnectStringProperty(const char* _key, bool _value);
    IoTConnectStringProperty(const char* _key, int _value);
    ~IoTConnectStringProperty();

    const char* get_key() const;
    const char* get_value();

    void get_value(const char** pval_in_str);

    void set_value(const char* _new_value);
    void set_value(const char* _new_value, size_t _len);

private:
    const char* key;
    char* buf;
};

class IoTConnectBoolProperty : public IoTConnectStringProperty {

public:
    IoTConnectBoolProperty(const char* _key, bool _value);
    ~IoTConnectBoolProperty();

    void get_value(bool* pbool_val);
    bool get_value();
    void set_value(bool _new_value);
};

class IoTConnectIntProperty : public IoTConnectStringProperty {

public:
    IoTConnectIntProperty(const char* _key, int _value);
    ~IoTConnectIntProperty();

    void get_value(int* pint_val);
    int get_value();
    void set_value(int _new_value);
};

class IoTConnectProperty
{
public:
    IoTConnectProperty();
    ~IoTConnectProperty();

    int add(IoTConnectStringProperty* _prop, Callback<void(void*)> _on_change = NULL);
    int prop(const char* _key, void** _obj, IoTConnectPropertyType* _type = 0);
    void* prop(const char* _key);

    int to_json(const char** _ppjson);
    const char* get_json();
    int update(const char* _json);
    int update(const char* _json, size_t _len);

private:

    int calc_json_str_len();

private:

    typedef struct {
        const char* key;
        IoTConnectPropertyType type;
        void* obj;
        Callback<void(void*)> on_change;
    }PropToken;

    PropToken tokens[IOT_CONNECT_PROPERTYS_MAX];
    char* jstr;
};


#endif