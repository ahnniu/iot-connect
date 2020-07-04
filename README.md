# iot-connect
This is the Avnet IoTConnect mbed SDK. More info about IoTConnect, Please refer to https://www.iotconnect.io/. This could also work with Microsoft Azure IoT hub, more about Azure IoT hub, please refer to https://docs.microsoft.com/en-us/azure/iot-hub/ .

This mbed SDK, is an IoTConnect MQTT client library, More about mbed, please refer to https://os.mbed.com/

## Features

### Features have been supported

- Authentication
  - Symmetric Key
  - X.509 certificate
- MQTT - Lowlevel
  - Publish
  - Subscribe
- Device Property - Highlevel, users could get/set properties instead of managing of a RAW MQTT message
  - Support String / Int / Bool property types
  - Support Muti properties in a device
  - Set a property and publish to IoT hub
  - Subscribe IoT hub, it will manage the device properties, if any property has been changed, an on_change() callback is called, in callback, users could do things according to the new property

### Features to be supported

- Device Twins
- OTA

## Install mbed Development Environment

This is a mbed library, so you should install the development env first. The Arm Mbed OS ecosystem includes many tools designed to work with Mbed OS and projects:

- [Arm Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.0/build-tools/mbed-studio.html)
- [Arm Mbed CLI](https://os.mbed.com/docs/mbed-os/v6.0/build-tools/mbed-cli.html)
- [Arm Mbed Online Compiler](https://os.mbed.com/docs/mbed-os/v6.0/build-tools/mbed-online-compiler.html)
- [Exporting to third party toolchains page](https://os.mbed.com/docs/mbed-os/v6.0/build-tools/third-party-build-tools.html).

### mbed-cli

I prefer to use Arm Mbed CLI, refer [this link](https://os.mbed.com/docs/mbed-os/v6.0/build-tools/install-and-set-up.html) to install and set up it.

I will show you how to install mbed cli on Ubuntu Linux.

#### install mbed-cli

```bash
sudo apt install python3 python3-pip git mercurial
python3 -m pip install mbed-cli
# To verify Mbed CLI installed correctly, run 
mbed --help
```

#### Install a compiler

Download [GNU Arm Embedded version 9 (9-2019-q4-major)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

#### Configure compiler location with Mbed CLI

```
mbed config -G GCC_ARM_PATH "/home/build/tools/gcc_arm/gcc-arm-none-eabi-9-2019-q4-major/bin"
```



### Quick Start

### Update CY8CKIT-062-WIFI-BT KitProg firmware

> If you skip this, it may not support the hex files that mbed compiled

First, Download Firmware-loader from https://github.com/cypresssemiconductorco/Firmware-loader/releases

```
fw-loader.exe --update-kp3
```

More info refer to https://github.com/cypresssemiconductorco/Firmware-loader

### Get the sample code

```bash
git clone https://github.com/paul-embest/iot-connect-example-psoc sample
cd sample
mbed config root .
```

### Deploy - Fetch the dependence libs

```
mbed deploy
```

### Configuration

#### Wi-Fi settings

In mbed_app.json, change Wi-Fi settings.

```json
{
    "macros": [
        "MBEDTLS_SHA1_C=1",
        "MQTTCLIENT_QOS0",
        "MBEDTLS_USER_CONFIG_FILE=\"mbedtls_azure_config.h\""
    ],
    "target_overrides": {
        "*": {
            "rtos.main-thread-stack-size": 5120,
            "nsapi.default-wifi-security": "WPA_WPA2",
            "nsapi.default-wifi-ssid": "\"Embest_WiFi\"",
            "nsapi.default-wifi-password": "\"Emb-666@\"",
            "mbed-trace.enable": 1,
            "mbed-trace.max-level": "TRACE_LEVEL_DEBUG",
            "mbed-mqtt.max-connections": "5",
            "mbed-mqtt.max-packet-size": "1024"
        },
        "CY8CKIT_062_WIFI_BT": {
            "platform.stdio-baud-rate": 115200,
            "platform.stdio-convert-newlines": true
        }
    }
}
```



#### Entry Settings

in entry_embest_settings.h, define your entry just like the below example

```c
#define ENTRY_EMBEST_CPID "617449FD62B649E7AD1050DB4E95096E"
#define ENTRY_EMBEST_COMPANY_NAME "Embest Tech"

#define ENTRY_EMBEST_MQTT_SERVER_HOST_NAME "poc-iotconnect-iothub-eu.azure-devices.net"
#define ENTRY_EMBEST_MQTT_SERVER_PORT 8883
```

If you are using Microsoft Azure IoT hub, change it like below:

```c
#define ENTRY_PSOC_CPID ""
#define ENTRY_PSOC_COMPANY_NAME "Azure IoT Hub - psoc"

#define ENTRY_PSOC_MQTT_SERVER_HOST_NAME "psoc.azure-devices.net"
#define ENTRY_PSOC_MQTT_SERVER_PORT 8883
```

#### Device Settings

In devices_settings.h

```c
// Symmetric Key device
#define TESTING_LED3_NAME "LED 3"
#define TESTING_LED3_ID "led3"
#define TESTING_LED3_PWD "SharedAccessSignature sr=poc-iotconnect-iothub-eu.azure-devices.net%2Fdevices%2F617449FD62B649E7AD1050DB4E95096E-led3&sig=bLYUC%2FaG1zKnajvmqtGwzvONL58mLs9oZzQ5YPl68Ss%3D&se=1625032925"

// X.509 certificate device
#define TESTING_LED2_NAME "LED 2"
#define TESTING_LED2_ID "led2"
#define TESTING_LED2_PWD "1234"

#define TESTING_LED2_CLIENT_CERT_PEM                                       \
    "-----BEGIN CERTIFICATE-----\r\n"                                      \
    "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\r\n" \
    "-----END CERTIFICATE-----\r\n"


#define TESTING_LED2_CLIENT_PRIVATE_KEY                                    \
    "-----BEGIN RSA PRIVATE KEY-----\r\n"                                  \
    "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\r\n"     \
    "-----END RSA PRIVATE KEY-----\r\n"

// Symmetric Key device
#define TESTING_AZ_LED1_NAME "Azure LED 1"
#define TESTING_AZ_LED1_ID "az-led1"
#define TESTING_AZ_LED1_PWD "SharedAccessSignature sr=psoc.azure-devices.net%2Fdevices%2Faz-led1&sig=rdpqq8XnFX9k3GcNPi43tRT2VOwTyD8yBjEmRE8mI50%3D&se=1597445777"

```

### Compile

```bash
mbed target CY8CKIT_062_WIFI_BT
mbed toolchain GCC_ARM
mbed compile
# ./BUILD/CY8CKIT_062_WIFI_BT/GCC_ARM/sample.hex will be generated
```

### Board Setup and Load the .Hex

- Use the USB lead to connect your Mbed to a PC. The status light will come on, indicating it has power.  After a few seconds of activity, the PC will recognise the Mbed Microcontroller as a standard USB drive.
- Copy the sample.hex file to the USB drive

### Demo

> NOTE: The main difference between IoTConnect and your Microsoft Azure IoT hub is the device ID, it makes a new Device ID with (cpid + device id in IoTConnect), But in debug / development periods, Microsoft has many tools can help you debug. 

#### Microsfot Azure IoT Hub

**Demo - Subscribe**

Login https://portal.azure.com/ and enter the device page, Clike 'Message to device', this allow you to send a message to device. message could be:

```json
{"color":"red"}
```

After sent this message, you could see the board, check the RGB led color.

**Demo - Publish**

First, run the az tools to monitor the IoT hub, if your device publish a message, the monitor will print it.

```bash
# az iot hub monitor-events -n {iot hub name}
az iot hub monitor-events -n psoc
```

If you press the button1 on the board, the monitor will print:

```
Starting event monitor, use ctrl-c to stop...
{
    "event": {
        "origin": "az-led1",
        "interface": "",
        "module": "",
        "payload": "{\"color\":\"green\"}"
    }
}
```



## API Reference

> Here is just a brief description. More API document should be generated by Doxygen. But of course, the API should add doxygen comments first.

### class IoTConnectEntry

This is used to define an Entry in IoTConnect, the most important info is CPID and MQTT Host Name.

```c
#define ENTRY_EMBEST_CPID "617449FD62B649E7AD1050DB4E95096E"
#define ENTRY_EMBEST_COMPANY_NAME "Embest Tech"

#define ENTRY_EMBEST_MQTT_SERVER_HOST_NAME "poc-iotconnect-iothub-eu.azure-devices.net"
#define ENTRY_EMBEST_MQTT_SERVER_PORT 8883

IoTConnectEntry embest(ENTRY_EMBEST_COMPANY_NAME, ENTRY_EMBEST_CPID);
embest.set_mqtt(ENTRY_EMBEST_MQTT_SERVER_HOST_NAME,
                ENTRY_EMBEST_MQTT_SERVER_PORT);
```

If you Use Microsoft Azure IoT Hub

```c
#define ENTRY_PSOC_CPID ""
#define ENTRY_PSOC_COMPANY_NAME "Azure IoT Hub - psoc"

#define ENTRY_PSOC_MQTT_SERVER_HOST_NAME "psoc.azure-devices.net"
#define ENTRY_PSOC_MQTT_SERVER_PORT 8883

IoTConnectEntry hub(ENTRY_PSOC_COMPANY_NAME, ENTRY_PSOC_CPID);
hub.set_mqtt(ENTRY_PSOC_MQTT_SERVER_HOST_NAME, ENTRY_PSOC_MQTT_SERVER_PORT);
```



### class IoTConnectStringProperty / IotConnectBoolProperty / IoTConnectIntProperty

This is string / bool / int type property

### class IoTConnectProperty

This used to add properties of a device.

### class IoTConnectDevice

This is node device, it's a sub class of IoTConnectProperty. A device should belongs to a IoTConnectEntry.

### class IoTConnectClient

This is a MQTT client. it setup a TLS socket connection and connect to the MQTT broke.

