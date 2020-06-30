#include "mbed.h"
#include "IoTConnectEntry.h"

IoTConnectEntry::IoTConnectEntry(const char* _company_name, const char* _cpid) :
    company_name(_company_name),
    cpid(_cpid),
    mqtt_server_host_name(NULL),
    mqtt_port(8883)
{

}

IoTConnectEntry::~IoTConnectEntry()
{

}

const char* IoTConnectEntry::get_cpid() const
{
    return cpid;
}

const char* IoTConnectEntry::get_company_name() const
{
    return company_name;
}

void IoTConnectEntry::set_mqtt(const char* _server_host_name, uint16_t _port)
{
    mqtt_server_host_name = _server_host_name;
    mqtt_port = _port;
}


const char* IoTConnectEntry::get_mqtt_server_host_name() const
{
    return mqtt_server_host_name;
}

uint16_t IoTConnectEntry::get_mqtt_port() const
{
    return mqtt_port;
}
