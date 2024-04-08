# MQTT Client Library
## Trademarks
All referenced product or service names and trademarks are the property of their respective owners.
| Trademark | Description |
|------|-------------|
| MQTT | MQTT is an OASIS standard messaging protocol for the Internet of Things (IoT). It is designed as an extremely lightweight publish/subscribe messaging transport that is ideal for connecting remote devices with a small code footprint and minimal network bandwidth. MQTT today is used in a wide variety of industries, such as automotive, manufacturing, telecommunications, oil and gas, etc. [^1] |
| PlantUML | PlantUML is a versatile component that enables swift and straightforward diagram creation. Users can draft a variety of diagrams using a simple and intuitive language.[^2]|
| Home Assistant| Open source home automation that puts local control and privacy first. Powered by a worldwide community of tinkerers and DIY enthusiasts. Perfect to run on a Raspberry Pi or a local server.[^3] |
## Introduction
The aim of this repo is to demonstrate possibilities as well as usage of the `libmqttcli.a` library.
## Features
- Supports the client side of the MQTT [^1] protocol.
- Is relatively easy to use. The user does not need to know details of the MQTT protocol. The library automatically prepares the following packets: `CONNECT`, `AUTH`, `DISCONNECT`, `PUBACK` and `PINGREQ`. The user via library interface is allowed to prepare the following packets: `PUBLISH`, `SUBSCRIBE` and `DISCONNECT`.
- The whole library contains proprietary implementation. There are no third party implementations included. The following external standard functions are used: `malloc`, `memcpy` and `memcmp`.
- The library is totally written in C89[^4] standard.
- The library does not establish any internet connection. Received or prepared packets shall be exchanged by the user.
- Specific behavior could be added using callback functions, e.g., client authentication to the broker.
## Usage
### Requirements
The library is not exchanging packets via internet as well as it is not conscious of elapsed time, therefore the user shall support the library with these aspects. To achieve this the following elements shall be designed in the program flow:
- The timer should be configured to generate an interrupt after a specified period of time, hereinafter referred to as `timeout`. If the `timeout` was detected then empty packet shall be presented to the `process` function as it was presented on <a href="#fig01">Fig. 1</a>.
- If any data was received it shall be presented to the `process` function (<a href="#fig01">Fig. 1</a>).
- If shall be checked if `process` function has prepared any data to send. If any data was prepared then it must be send to the broker. The `process` function shall be repeated until it returns the `MQTT_SUCCESS` reason code, hereinafter referred to as `rc`.
- If the `process` function has returned other `rc` than `MQTT_PENDING_DATA`, then an appropriate actions shall be take into account.

<p align="center">
  <a name="fig01"> 
  <img src="doc/program_flow.svg" /> </br>
  <b>Fig. 1. Program flow (generated with PlantUML). </b>
  </a>
</p>

The following sections names corresponds to the lightblue actions presented on <a href="#fig01">Fig. 1</a>.

### Initializing the library
The first step is to initialize the library. It shall be performed as follows:
```C
mqtt_cli cli;
mqtt_params_t params = {.bufsize=2048, .timeout=1};

if(MQTT_SUCCESS != mqtt_cli_init( &cli, &params ) {
  /* ... error processing ... */
}
```
### Configuring the library
The second step is to configure the library:
```C
const uint32_t srv_ip = 0xC0A80201;
const uint16_t keep_alive = 60;
mqtt_cli cli;

/* ... initializing the library ... */

cli.set_br_ip( &cli, srv_ip);
cli.set_br_keepalive( &cli, keep_alive );
```
### Processing the data
```C
uint16_t rc = MQTT_SUCCESS;
mqtt_cli cli;
uint8_t buffer[1024] = { 0 };
clv_t data = { .capacity=sizeof(buffer)/sizeof(uint8_t), .value=buffer };
mqtt_channel_t channel = { };

/* ... initializing and configuring the library ... */

do {
  rc = cli->process( &cli, &data, &channel);
  if(rc != MQTT_SUCCESS && rc != MQTT_PENDING_DATA) {
    /* ... error processing ... */
  }

  if( data.length ) {
    /* ... sending data ... */
  }

} while( rc == MQTT_PENDING_DATA );
```
### Preparing *PUBLISH* package
```C
const char *topic = "sensor01";
const char *message = "ON";
uint8_t buffer[1024] = { 0 };
clv_t data = { .capacity=sizeof(buffer)/sizeof(uint8_t), .value=buffer };
mqtt_publish_params_t params = { };
mqtt_cli cli;

/* ... initializing and configuring the library ... */

params.message = (lv_t) { .length=strlen(message), .value=(uint8_t*)message  };
params.topic = (lv_t) { .length=strlen(topic), .value=(uint8_t*)topic  };
if(MQTT_SUCCESS != cli.publish_ex( &cli, &params, &data)) {
  /* ... error processing ... */
}

/* ... sending the data ... */
```
### Preparing *PUBLISH* package inside a callback
```C
static const char* base_topic = "homeassistant/switch/hadev123456";
static const char* state_topic = "state";

mqtt_rc_t cb_publish(const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel) {
  mqtt_rc_t rc = RC_SUCCESS;
  mqtt_publish_params_t publish_params = { };

  /* Publishing current state */
  publish_params.topic.value = buffer->value;
  publish_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, state_topic );
  publish_params.message = pkt->message;
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    rc =  RC_IMPL_SPEC_ERR;
  }

  return rc;
}

int main() {
  uint16_t rc = MQTT_SUCCESS;
  mqtt_cli cli;
  clv_t data = { };
  mqtt_channel_t channel = { };

  /* ... initializing and configuring the library ... */

  cli.set_cb_publish( &cli, cb_publish );

  /* ... processing timeout or received packet ... */
  do {
    rc = cli->process( &cli, &data, &channel);
    if(rc != MQTT_SUCCESS && rc != MQTT_PENDING_DATA) {
      /* ... error processing ... */
    }

    if( data.length ) {
      /* ... sending data ... */
    }

  } while( rc == MQTT_PENDING_DATA );
}
```
### Preparing *SUBSCRIBE* package
```C
const char *topic = "homeassistant/dev12345678/state";
uint8_t buffer[1024] = { 0 };
clv_t data = { .capacity=sizeof(buffer)/sizeof(uint8_t), .value=buffer };
mqtt_subscribe_params_t subscribe_params = { };
mqtt_cli cli;

/* ... initializing and configuring the library ... */

params.filter = (lv_t) { .length=strlen(topic), .value=(uint8_t*)topic  };
if(MQTT_SUCCESS != self->subscribe(&cli, &subscribe_params, &data)) {
  /* ... error processing ... */
}

/* ... sending the data ... */
```
### Preparing *SUBSCRIBE* package inside a callback
```C
static const char* base_topic = "homeassistant/switch/hadev123456";
const char* command_topic = "set";

mqtt_rc_t cb_connack(const mqtt_cli_ctx_cb_t *self, const mqtt_connack_t *pkt, const mqtt_channel_t *channel) {
  mqtt_rc_t rc = RC_SUCCESS;
  mqtt_subscribe_params_t subscribe_params = { };

  /* Subscribing to receive commands */
  subscribe_params.filter.value = buffer->value;
  subscribe_params.filter.length = sprintf( buffer->value, "%s/%s", base_topic, command_topic );
  if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
    rc =  RC_IMPL_SPEC_ERR;
  }

  return rc;
}

int main() {
  uint16_t rc = MQTT_SUCCESS;
  mqtt_cli cli;
  clv_t data = { };
  mqtt_channel_t channel = { };

  /* ... initializing and configuring the library ... */

  cli.set_cb_connack( &cli, cb_connack );

  /* ... processing timeout or received packet ... */
  do {
    rc = cli.process( &cli, &data, &channel);
    if(rc != MQTT_SUCCESS && rc != MQTT_PENDING_DATA) {
      /* ... error processing ... */
    }

    if( data.length ) {
      /* ... sending data ... */
    }

  } while( rc == MQTT_PENDING_DATA );
}
```
### Releasing the library resources
To avoid memory leaks in the program, the library resources must be released if only they are not needed anymore.
```C
mqtt_cli cli;

/* ... initializing and configuring the library ... */

mqtt_cli_destr( &cli );
```
## Examples
| Link | Description |
|------|-------------|
|[mqtt.c](examples/mqtt.c/README.md)| Demonstrates using publish and subscribe packets in MQTT protocol. Could be used as a diagnostic tool. |
|[hadev.c](examples/hadev.c/README.md)| Simulator of the Home Assistant [^3] device. It is supporting discovery process to automatically add the device in Home Assistant board. |

## References
[^1]: [https://mqtt.org](https://mqtt.org)
[^2]: [https://plantuml.com](https://plantuml.com)
[^3]: [https://www.home-assistant.io](https://www.home-assistant.io)
[^4]: [https://web.archive.org/web/20200909074736if_/https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf](https://web.archive.org/web/20200909074736if_/https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf)
