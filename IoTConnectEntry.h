#ifndef __IOT_CONNECT_ENTRY_H__
#define __IOT_CONNECT_ENTRY_H__

class IoTConnectEntry
{
private:
    const char* company_name;
    const char* cpid;

    const char* mqtt_server_host_name;
    uint16_t mqtt_port;

public:

    IoTConnectEntry(const char* _company_name, const char* _cpid);

    void set_mqtt(const char* _server_host_name, uint16_t _port);


    const char* get_cpid() const;
    const char* get_company_name() const;

    const char* get_mqtt_server_host_name() const;
    uint16_t get_mqtt_port() const;

    ~IoTConnectEntry();

};

#endif