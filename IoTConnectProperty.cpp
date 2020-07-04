#include "mbed.h"
#include "IoTConnectProperty.h"
#include "mbed_trace.h"


#define TRACE_GROUP  "IoTConnectProperty"

IoTConnectStringProperty::IoTConnectStringProperty(const char* _key, const char* _value) :
    key(_key),
    buf(NULL)
{
    if (_value) {
        size_t value_len = strlen(_value);
        buf = (char*)malloc(value_len + 1);
        buf[value_len] = '\0';
    }
}

IoTConnectStringProperty::IoTConnectStringProperty(const char* _key, bool _value) :
    key(_key),
    buf(NULL)
{
    size_t buf_len = 6;

    buf = (char*)malloc(buf_len);
    memset(buf, 0, buf_len);

    if (_value) {
        sprintf(buf, "true");
    } else
    {
        sprintf(buf, "false");
    }
}

IoTConnectStringProperty::IoTConnectStringProperty(const char* _key, int _value) :
    key(_key),
    buf(NULL)
{
    // 32bit integer
    size_t buf_len = 12;

    buf = (char*)malloc(buf_len);
    memset(buf, 0, buf_len);

    sprintf(buf, "%d", _value);
}

IoTConnectStringProperty::~IoTConnectStringProperty()
{
    if (buf) {
        free(buf);
        buf = NULL;
    }
}

const char* IoTConnectStringProperty::get_key() const
{
    return key;
}

const char* IoTConnectStringProperty::get_value()
{
    return buf;
}

void IoTConnectStringProperty::get_value(const char** pval_in_str)
{
    *pval_in_str = buf;
}

void IoTConnectStringProperty::set_value(const char* _new_value)
{
    size_t new_value_len = strlen(_new_value);

    set_value(_new_value, new_value_len);
}

void IoTConnectStringProperty::set_value(const char* _new_value, size_t _len)
{
    size_t new_value_len;
    char * new_value_buf;

    if (!_new_value) {
        return;
    }

    new_value_len = _len;
    new_value_buf = (char*)malloc(new_value_len + 1);
    memset(new_value_buf, 0, new_value_len + 1);
    sprintf(new_value_buf, "%.*s", new_value_len, _new_value);

    // free old value buf
    if (buf) {
        free(buf);
    }

    buf = new_value_buf;

}

IoTConnectBoolProperty::IoTConnectBoolProperty(const char* _key, bool _value) :
    IoTConnectStringProperty(_key, _value)
{

}

IoTConnectBoolProperty::~IoTConnectBoolProperty()
{

}

void IoTConnectBoolProperty::get_value(bool* pbool_val)
{
    const char* val_in_str = NULL;

    if (pbool_val == NULL) {
        return;
    }

    IoTConnectStringProperty::get_value(&val_in_str);
    if (val_in_str && strcmp(val_in_str, "true") == 0) {
        *pbool_val = true;
    } else {
        *pbool_val = false;
    }
}

bool IoTConnectBoolProperty::get_value()
{
    bool val = false;
    get_value(&val);

    return val;
}

void IoTConnectBoolProperty::set_value(bool _new_value)
{
    char buf[6];
    memset(buf, 0, 6);

    if (_new_value) {
        sprintf(buf, "true");
    } else {
        sprintf(buf, "false");
    }
    IoTConnectStringProperty::set_value(buf);
}


IoTConnectIntProperty::IoTConnectIntProperty(const char* _key, int _value) :
    IoTConnectStringProperty(_key, _value)
{

}

IoTConnectIntProperty::~IoTConnectIntProperty()
{

}

void IoTConnectIntProperty::get_value(int* pint_val)
{
    const char* val_in_str = NULL;

    if (pint_val == NULL) {
        return;
    }

    IoTConnectStringProperty::get_value(&val_in_str);
    *pint_val = strtoimax(val_in_str, NULL, 10);
}

int IoTConnectIntProperty::get_value()
{
    int val = 0;
    get_value(&val);
    return val;
}

void IoTConnectIntProperty::set_value(int _new_value)
{
    char buf[12];
    memset(buf, 0, 12);
    sprintf(buf, "%d", _new_value);
    IoTConnectStringProperty::set_value(buf);
}

IoTConnectProperty::IoTConnectProperty() :
    jstr(NULL)
{
    int i;
    for (i = 0; i < IOT_CONNECT_PROPERTYS_MAX; i++) {
        tokens[i].key = NULL;
        tokens[i].type = IOT_CONNECT_PROPERTY_TYPE_UNDEFINED;
        tokens[i].obj = NULL;
        tokens[i].on_change = NULL;
    }
}

IoTConnectProperty::~IoTConnectProperty()
{
    if (jstr) {
        free(jstr);
    }
}

int IoTConnectProperty::add(IoTConnectStringProperty* _prop, Callback<void(void*)> _on_change)
{
    int i;

    if (!_prop) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    for (i = 0; i < IOT_CONNECT_PROPERTYS_MAX; i++) {
        if (tokens[i].key == NULL) {
            tokens[i].key = _prop->get_key();
            tokens[i].type = IOT_CONNECT_PROPERTY_TYPE_STRING;
            tokens[i].obj = _prop;

            if (_on_change) {
                tokens[i].on_change = _on_change;
            }

            return 0;
        }
    }

    return IOT_CONNECT_ERROR_PROPERTY_FULL;
}

int IoTConnectProperty::prop(const char* _key, void** _obj, IoTConnectPropertyType* _type)
{
    int i;

    if (!_key) {
        return IOT_CONNECT_ERROR_INVAL;
    }


    for (i = 0; i < IOT_CONNECT_PROPERTYS_MAX; i++) {
        if (strcmp(tokens[i].key, _key) == 0) {
            if (_type) {
                *_type = tokens[i].type;
            }

            if (_obj) {
                *_obj = tokens[i].obj;
            }

            return 0;
        }

    }

    return IOT_CONNECT_ERROR_PROPERTY_NOT_FOUND;

}

void* IoTConnectProperty::prop(const char* _key)
{
    int ret;
    void* _obj;

    ret = prop(_key, &_obj, NULL);

    if (ret == 0) {
        return _obj;
    }

    return NULL;
}

// Note: String should have a buf to store the real str
int IoTConnectProperty::to_json(const char** _ppjson)
{
    int i;
    int len;
    char *p;

    if (_ppjson == NULL) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    len = calc_json_str_len();

    // properties may change, the old json string would be timeout
    // free the old json string and generate a new buf
    if (jstr) {
        free(jstr);
    }

    jstr = (char*)malloc(len + 1);
    if (jstr == NULL) {
        return IOT_CONNECT_ERROR_OUT_OF_MEM;
    }

    // Empty Properties
    if (len == 2) {
        jstr[0] = '{';
        jstr[1] = '}';
        jstr[2] = '\0';

        return 0;
    }

    p = jstr;
    *p++ = '{';

    for (i = 0; i < IOT_CONNECT_PROPERTYS_MAX; i++) {
        if (tokens[i].key == NULL) {
            break;
        }
        sprintf(p, "\"%s\":", tokens[i].key);
        p += strlen(tokens[i].key) + 3;
        switch (tokens[i].type) {
            case IOT_CONNECT_PROPERTY_TYPE_STRING:
            case IOT_CONNECT_PROPERTY_TYPE_INT:
            case IOT_CONNECT_PROPERTY_TYPE_BOOL:
            {
                const char * str_val = NULL;
                ((IoTConnectStringProperty*)tokens[i].obj)->get_value(&str_val);
                sprintf(p, "\"%s\"", str_val);
                p += strlen(str_val) + 2;
                break;
            }
            default:
                p -= strlen(tokens[i].key) + 3;   // not support
                break;
        }
        *p++ = ',';
    }
    // there is one more ,
    *--p = '}';
    *++p = '\0';

    *_ppjson = jstr;

    return 0;
}

const char* IoTConnectProperty::get_json()
{
    const char* js;
    int r = to_json(&js);

    if (r != 0) {
        return NULL;
    }

    return js;
}

int IoTConnectProperty::calc_json_str_len()
{
    int i;
    int len = 0;

    for (i = 0; i < IOT_CONNECT_PROPERTYS_MAX; i++) {
        if (tokens[i].key == NULL) {
            break;
        }

        len += strlen(tokens[i].key) + 3;   // "key":
        switch (tokens[i].type)
        {
            case IOT_CONNECT_PROPERTY_TYPE_STRING:
            case IOT_CONNECT_PROPERTY_TYPE_INT:
            case IOT_CONNECT_PROPERTY_TYPE_BOOL:
            {
                const char * str_val = NULL;
                ((IoTConnectStringProperty*)tokens[i].obj)->get_value(&str_val);
                len += strlen(str_val) + 2;    // "value"
                break;
            }
            default:
                len -= strlen(tokens[i].key) + 3;   // not support
                break;
        }
        // ,
        len += 1;

    }

    if (len > 0) {
        // remove the end ,
        len -= 1;

        // add {}
        len += 2;
    }

    return len;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}


int IoTConnectProperty::update(const char* _json, size_t _len)
{
    int r;
    int i, j;
    const char* js = _json;

    if (_json == NULL) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    tr_debug("Dump devive properties json before update");
    tr_debug("%s", jstr);

    jsmn_parser parser;
    jsmntok_t t[IOT_CONNECT_PROPERTYS_MAX * 2];

    jsmn_init(&parser);

    r = jsmn_parse(&parser, js, _len, t, sizeof(t) / sizeof(t[0]));

    if (r < 0) {
        // Failed to parse JSON
        return IOT_CONNECT_ERROR_PROPERTY_JSON_PARSE;
    }

    if (r < 1 || t[0].type != JSMN_OBJECT) {
        return IOT_CONNECT_ERROR_PROPERTY_JSON_FORMAT;
    }

    for (i = 1; i < r; i++) {
        bool found_prop = false;
        for (j = 0; j < IOT_CONNECT_PROPERTYS_MAX; j++) {
            if (tokens[j].key == NULL) {
                break;
            }
            if (jsoneq(js, &t[i], tokens[j].key) == 0) {

                jsmntok_t* t_val = &t[i + 1];
                const char* to_read = js + t_val->start;
                int read_len = t_val->end - t_val->start;

                switch (tokens[j].type)
                {
                    case IOT_CONNECT_PROPERTY_TYPE_STRING:
                    case IOT_CONNECT_PROPERTY_TYPE_INT:
                    case IOT_CONNECT_PROPERTY_TYPE_BOOL:
                        ((IoTConnectStringProperty*)tokens[j].obj)->set_value(to_read, read_len);
                        tr_info("Property[%s] changed", tokens[j].key);
                        tr_debug("Note: It %s have an on_change() callback", tokens[j].on_change ? "does" : "doesn't");
                        if (tokens[j].on_change) {
                            tr_debug("call on_change() callback");
                            tokens[j].on_change(tokens[j].obj);
                        }
                        // value token has been parse
                        i++;
                        break;
                    // TODO: To support Array, Object type
                    default:
                        tr_err("Property[%s] has an unsupport value type: %d", tokens[j].key, tokens[i].type);
                        i++;
                        break;
                }

                found_prop = true;
                break;
            }
        }
        if (!found_prop) {
            tr_err("Property[%.*s] Unkown detect", t[i].end - t[i].start, js + t[i].start);
        }

    }

    tr_debug("Dump devive properties json after update");
    tr_debug("%s", get_json());

    return 0;
}

int IoTConnectProperty::update(const char* _json)
{
    size_t len;

    if (_json == NULL) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    len = strlen(_json);

    return update(_json, len);
}