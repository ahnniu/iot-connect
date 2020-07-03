#ifndef __IOT_CONNECT_ERROR_H__
#define __IOT_CONNECT_ERROR_H__


typedef enum {
    IOT_CONNECT_ERROR_OK                     =  0,        /*!< no error */

    IOT_CONNECT_ERROR_INVAL                  = -1001,
    IOT_CONNECT_ERROR_OUT_OF_MEM             = -1002,

    IOT_CONNECT_ERROR_CLIENT_PUB_FULL        = -1101,
    IOT_CONNECT_ERROR_CLIENT_SUB_OVERFLOW    = -1102,
    IOT_CONNECT_ERROR_CLIENT_SUB             = -1103,
    IOT_CONNECT_ERROR_CLIENT_OUT_OF_INSTANCE = -1104,
    IOT_CONNECT_ERROR_CLIENT_THREAD          = -1105,

    IOT_CONNECT_ERROR_PROPERTY_FULL          = -1201,
    IOT_CONNECT_ERROR_PROPERTY_NOT_FOUND     = -1202,
    IOT_CONNECT_ERROR_PROPERTY_JSON_FORMAT   = -1203,
    IOT_CONNECT_ERROR_PROPERTY_JSON_PARSE    = -1204,


    IOT_CONNECT_ERROR_NS_WOULD_BLOCK         = -3001,     /*!< no data is not available but call is non-blocking */
    IOT_CONNECT_ERROR_NS_UNSUPPORTED         = -3002,     /*!< unsupported functionality */
    IOT_CONNECT_ERROR_NS_PARAMETER           = -3003,     /*!< invalid configuration */
    IOT_CONNECT_ERROR_NS_NO_CONNECTION       = -3004,     /*!< not connected to a network */
    IOT_CONNECT_ERROR_NS_NO_SOCKET           = -3005,     /*!< socket not available for use */
    IOT_CONNECT_ERROR_NS_NO_ADDRESS          = -3006,     /*!< IP address is not known */
    IOT_CONNECT_ERROR_NS_NO_MEMORY           = -3007,     /*!< memory resource not available */
    IOT_CONNECT_ERROR_NS_NO_SSID             = -3008,     /*!< ssid not found */
    IOT_CONNECT_ERROR_NS_DNS_FAILURE         = -3009,     /*!< DNS failed to complete successfully */
    IOT_CONNECT_ERROR_NS_DHCP_FAILURE        = -3010,     /*!< DHCP failed to complete successfully */
    IOT_CONNECT_ERROR_NS_AUTH_FAILURE        = -3011,     /*!< connection to access point failed */
    IOT_CONNECT_ERROR_NS_DEVICE_ERROR        = -3012,     /*!< failure interfacing with the network processor */
    IOT_CONNECT_ERROR_NS_IN_PROGRESS         = -3013,     /*!< operation (eg connect) in progress */
    IOT_CONNECT_ERROR_NS_ALREADY             = -3014,     /*!< operation (eg connect) already in progress */
    IOT_CONNECT_ERROR_NS_IS_CONNECTED        = -3015,     /*!< socket is already connected */
    IOT_CONNECT_ERROR_NS_CONNECTION_LOST     = -3016,     /*!< connection lost */
    IOT_CONNECT_ERROR_NS_CONNECTION_TIMEOUT  = -3017,     /*!< connection timed out */
    IOT_CONNECT_ERROR_NS_ADDRESS_IN_USE      = -3018,     /*!< Address already in use */
    IOT_CONNECT_ERROR_NS_TIMEOUT             = -3019,     /*!< operation timed out */
    IOT_CONNECT_ERROR_NS_BUSY                = -3020,     /*!< device is busy and cannot accept new operation */
}IoTConnectError;

#endif