#include "mbed_trace.h"
#include "IoTConnectClient.h"
#include "AzureRootCert.h"
#include "MQTTPacket.h"


#define TRACE_GROUP  "IoTConnectClient"
#define CLIENT_SUB_BINDS_SIZE MBED_CONF_IOT_CONNECT_MQTT_CLIENT_INSTANCE_MAX
#define CLIENT_TOPIC_NAME_LEN 100

typedef struct {
    const char* topic;
    IoTConnectClient* client;
}ClientSubTopicBind;

static ClientSubTopicBind binds[CLIENT_SUB_BINDS_SIZE];

static int client_sub_topic_bind(IoTConnectClient* _client, const char* _topic)
{
    int i;
    for (i = 0; i < CLIENT_SUB_BINDS_SIZE; i++) {
        if (binds[i].topic == NULL) {
            binds[i].topic = _topic;
            binds[i].client = _client;

            return 0;
        }
    }

    return IOT_CONNECT_ERROR_CLIENT_OUT_OF_INSTANCE;
}

static void mqtt_string_clone(MQTTString& a, char* bptr, int blen)
{
	int alen = 0;
	char* aptr;

	if (a.cstring)
	{
		aptr = a.cstring;
		alen = strlen(a.cstring);
	}
	else
	{
		aptr = a.lenstring.data;
		alen = a.lenstring.len;
	}
	snprintf(bptr, alen < blen ? alen : blen, "%s", (char*)aptr);
}

static bool mqtt_is_topic_matched(const char* topicFilter, MQTTString& topicName)
{
    const char* curf = topicFilter;
    char* curn = topicName.lenstring.data;
    char* curn_end = curn + topicName.lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

static void client_sub_handle_internal(MQTT::MessageData& _data)
{
    MQTT::Message &_msg = _data.message;
    static char topic[CLIENT_TOPIC_NAME_LEN];
    IoTConnectClient* client = NULL;
    int i;

    mqtt_string_clone(_data.topicName, topic, CLIENT_TOPIC_NAME_LEN);

    tr_info("Topic[%s] - subscribe new message#%d with %d bytes payload arrived", topic, _msg.id, _msg.payloadlen);
    tr_debug("Dump message payload");
    tr_debug("%.*s", _msg.payloadlen, _msg.payload);

    for (i = 0; i < CLIENT_SUB_BINDS_SIZE; i++) {
        if (binds[i].topic == NULL) {
            break;
        }

        if (mqtt_is_topic_matched(binds[i].topic, _data.topicName)) {
            tr_debug("Found bind IoTConnectClient object by topic name");
            client = binds[i].client;
            break;
        }
    }

    if (client) {
        char* buf;
        size_t buf_len = _msg.payloadlen + 1;
        buf = (char*)malloc(buf_len);
        if (!buf) {
            tr_error("Out of memory when buffer message");
            return;
        }
        memcpy(buf, _msg.payload, _msg.payloadlen);
        buf[_msg.payloadlen] = '\0';

        MQTT::Message* msg_new = (MQTT::Message*)malloc(sizeof(MQTT::Message));
        if (!msg_new) {
            tr_error("Out of memory when buffer message");
            return;
        }
        memset(msg_new, 0, sizeof(MQTT::Message));
        msg_new->qos = _msg.qos;
        msg_new->retained = _msg.retained;
        msg_new->dup = _msg.dup;
        msg_new->id = _msg.id;
        msg_new->payload = buf;
        msg_new->payloadlen = _msg.payloadlen;

        if (client->on_received) {
            tr_info("Client has a customized on_received callback");
            tr_debug("NOTE: This won't update device properties because client has handler this message");
            client->on_received(msg_new);
        } else {
            tr_debug("Update device properties according to the message");
            client->update_props_on_recieved(msg_new);
        }

        // Free buffer
        if (msg_new) {
            free(msg_new);
        }
        if (buf) {
            free(buf);
        }
    }
}

IoTConnectClient::IoTConnectClient(NetworkInterface *_network, IoTConnectDevice *_device) :
    network(_network),
    device(_device),
    auth_type(IOT_CONNECT_AUTH_SYMMETRIC_KEY),
    entry(NULL),
    socket(new TLSSocket),
    mqtt_client(NULL),
    on_received(NULL),
    on_connection_lost(NULL),
    thread(osPriorityNormal, MQTT_CLIENT_THREAD_STACK_SIZE),
    msg_id_pub_props(0)
{
    if (_device) {
        entry = _device->get_entry();
        auth_type = _device->get_auth_type();
    }

    mqtt_client = new MQTTClient(socket);

}

IoTConnectClient::~IoTConnectClient() {
    MQTT::Message* msg = NULL;

    disconnect();
    delete mqtt_client;
    delete socket;

    while(!pubs.empty()) {
        pubs.pop(msg);
        if (msg) {
            if (msg->payload) {
                free(msg->payload);
            }
            free(msg);
        }
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

int IoTConnectClient::disconnect()
{
    return 0;
}

bool IoTConnectClient::is_connected()
{
    return mqtt_client->isConnected();
}


int IoTConnectClient::subscribe(MQTT::QoS qos, Callback<void(MQTT::Message*)> _on_received)
{
    const char* topic_sub = device->get_mqtt_topic_sub();

    int rc = mqtt_client->subscribe(topic_sub, qos, client_sub_handle_internal);
    if (rc != MQTT::SUCCESS) {
        tr_error("Subscribe topic:%s with QoS:%d failed", topic_sub, qos);
        return IOT_CONNECT_ERROR_CLIENT_SUB;
    }

    on_received = _on_received;

    client_sub_topic_bind(this, topic_sub);

    return 0;
}

int IoTConnectClient::pub(MQTT::Message* _msg)
{
    MQTT::Message* msg_to_pub;
    char* buf = NULL;
    size_t buf_len = 0;

    if (!_msg) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    if (_msg->payloadlen <= 0) {
        return IOT_CONNECT_ERROR_INVAL;
    }

    if (pubs.full()) {
        return IOT_CONNECT_ERROR_CLIENT_PUB_FULL;
    }

    buf_len = _msg->payloadlen;
    buf = (char*)malloc(buf_len);
    if (!buf) {
        return IOT_CONNECT_ERROR_OUT_OF_MEM;
    }
    memcpy(buf, _msg->payload, _msg->payloadlen);

    msg_to_pub = (MQTT::Message*)malloc(sizeof(MQTT::Message));
    memcpy(msg_to_pub, _msg, sizeof(MQTT::Message));
    msg_to_pub->payload = buf;

    pubs.push(msg_to_pub);

    return 0;
}

int IoTConnectClient::start_main_loop()
{
    osStatus ret;
    ret = thread.start(callback(this, &IoTConnectClient::thread_main_loop));

    if (ret != osOK) {
        tr_error("Start thread failed with osStatus: %d", ret);
        return IOT_CONNECT_ERROR_CLIENT_THREAD;
    }

    return 0;
}

void IoTConnectClient::thread_main_loop()
{
    const char* topic_pub = device->get_mqtt_topic_pub();
    while (1) {
        if (!is_connected()) {
            // Disconneted, call a callback then sleep or terminal thread?
            tr_error("Connection lost");
            if (on_connection_lost) {
                on_connection_lost();
            }
        }

        if (mqtt_client->yield(100) != MQTT::SUCCESS) {
            // error occurs when yield, coninue to check is the connection is lost.
            continue;
        }

        if (!pubs.empty()) {
            MQTT::Message* pub_msg = NULL;
            pubs.pop(pub_msg);

            if (pub_msg == NULL) {
                continue;
            }

            int rc = mqtt_client->publish(topic_pub, *pub_msg);
            if(rc != MQTT::SUCCESS) {
                tr_error("Topic[%s] publish message#%d failed\n", topic_pub, pub_msg->id);
            }
            tr_info("Topic[%s] publish message#%d succeed", topic_pub, pub_msg->id);
            #if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_DEBUG
            tr_array((uint8_t*)pub_msg->payload, pub_msg->payloadlen);
            #endif

            // destrory the message
            if (pub_msg->payload) {
                free(pub_msg->payload);
            }
            free(pub_msg);
            pub_msg = NULL;
        }

    }
}

void IoTConnectClient::update_props_on_recieved(MQTT::Message* _msg)
{
    const char* js = (const char*)_msg->payload;
    device->update(js);
}

void IoTConnectClient::set_event_handler(Callback<void()> _on_connection_lost)
{
    if (_on_connection_lost) {
        on_connection_lost = _on_connection_lost;
    }
}

int IoTConnectClient::pub_props(MQTT::QoS _qos)
{
    int r;
    const char* json = NULL;
    MQTT::Message pub_msg;

    r = device->to_json(&json);

    if (r != 0) {
        return r;
    }

    pub_msg.qos = _qos;
    pub_msg.retained = false;
    pub_msg.dup = false;
    pub_msg.id = msg_id_pub_props++;
    pub_msg.payload = (void*)json;
    pub_msg.payloadlen = strlen(json);

    r = pub(&pub_msg);

    return r;
}