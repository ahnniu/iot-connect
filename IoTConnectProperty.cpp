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
    sprintf(new_value_buf, " %.*s", new_value_len, _new_value);
    new_value_buf[new_value_len] = '\0';

    // free old value buf
    if (buf) {
        free(buf);
    }

    buf = new_value_buf;

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
int IoTConnectProperty::to_json()
{
    int i;
    int len = calc_json_str_len();
    char *p;

    MBED_ASSERT(len > 0);

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
            case IOT_CONNECT_PROPERTY_TYPE_STRING: {
                const char * str_val = ((IoTConnectStringProperty*)tokens[i].obj)->get_value();
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

    return 0;
}

const char* IoTConnectProperty::get_json()
{
    int r = to_json();

    if (r != 0) {
        return jstr;
    }

    return NULL;
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
            case IOT_CONNECT_PROPERTY_TYPE_STRING: {
                const char* str_val = ((IoTConnectStringProperty*)tokens[i].obj)->get_value();
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

    return 0;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}


int IoTConnectProperty::update(const char* _json)
{
    int r;
    int i, j;
    const char* js = _json;

    jsmn_parser parser;
    jsmntok_t t[IOT_CONNECT_PROPERTYS_MAX];

    jsmn_init(&parser);

    r = jsmn_parse(&parser, js, strlen(js), t, sizeof(t) / sizeof(t[0]));

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
            if (tokens[i].key == NULL) {
                break;
            }

            if (jsoneq(js, &t[i], tokens[i].key) == 0) {

                jsmntok_t* t_val = &t[i + 1];
                const char* to_read = js + t_val->start;
                int read_len = t_val->end - t_val->start;

                switch (tokens[i].type)
                {
                    case IOT_CONNECT_PROPERTY_TYPE_STRING:
                    case IOT_CONNECT_PROPERTY_TYPE_INT:
                    case IOT_CONNECT_PROPERTY_TYPE_BOOL:
                        ((IoTConnectStringProperty*)tokens[i].obj)->set_value(to_read, read_len);
                        // value token has been parse
                        i++;
                        break;
                    // TODO: To support Array, Object type
                    default:
                        tr_err("Property[%s] has an unsupport value type: %d", tokens[i].key, tokens[i].type);
                        i++;
                        break;
                }
                break;
            }
        }
        if (!found_prop) {
            tr_err("Property[%.*s] Unkown detect", t[i].end - t[i].start, js + t[i].start);
        }

    }

    return 0;
}