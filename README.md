# MQTT Client Library
## Introduction
The aim of this repo is to demonstrate possibilities of the library `libmqttcli.a`
## Features
- Relatively easy to use. The user does not need to know details of MQTT protocol.
- Whole library contains proprietary implementation. The only external standard functions are used: `malloc`, `memcpy` and `memcmp`.
- The library was totally written in C89.
- Specific behaviour could be added using callback functions.
## Usage
### Initialisation
/the first step is to initialize the library.
```
mqtt_cli cli;

mqtt_cli_init( &cli );
```
### Configuration
The second step is to configure the library.
```
const uint16_t timeout = 1;
const uint32_t srv_ip = 0xC0A80201;
const uint16_t keep_alive = 60;

cli.set_timeout( &cli, timeout);
cli.set_br_ip( &cli, srv_ip);
cli.set_br_keepalive( &cli, keep_alive );
```
### Sending publish package
```
rc = dev->publish_ex( dev, &params, &output);
```
### Sending subscribe package
### Processing packages
## Examples
| Link | Description |
|------|-------------|
|[mqtt.c](src/mqtt.c/README.md)| Demonstrates using publish and subscribe packets in MQTT protocol. Could be used as a diagnostic tool. |
|[hadev.c](src/hadev.c/README.md)| Simulator of the Home Assistant device. It is using discovery process to automatically add the device. |
