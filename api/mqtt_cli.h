#ifndef __MQTT_CLI__H__
#define __MQTT_CLI__H__

#include <stdint.h>
#include <stdlib.h>
#include "mqtt_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mqtt_cli_ctx;
struct mqtt_dev;
typedef struct mqtt_cli mqtt_cli_t;
typedef struct mqtt_dev mqtt_dev_t;

/** Stores all mqtt client data */
struct mqtt_cli {
    /** Private context data */
    struct mqtt_cli_ctx *ctx;
    /** Prepares PUBLISH packet which will be send during the following process() */
    uint16_t (*publish) (mqtt_cli_t *self, const char *topic, const char *message);
    /** Prepares PUBLISH packet and returns it to send */
    uint16_t (*publish_ex) (mqtt_cli_t *self, const char *topic, const char *message, const uint8_t *properties, const size_t properties_len, uint8_t *out_buf, size_t *out_buf_len);
    /** Prepares SUBSCRIBE packet which will be send during the following process() */
    uint16_t (*subscribe) (mqtt_cli_t *self, const char *topic);
    /** Prepares SUBSCRIBE packet and returns it to send */
    uint16_t (*subscribe_ex) (mqtt_cli_t *self, const char *topic, const uint8_t *properties, const size_t properties_len, uint8_t *out_buf, size_t *out_buf_len); 
    /** Processes received packets, empty packets, send responses if needed and retransmits packets  */
    uint16_t (*process) (mqtt_cli_t *self, uint8_t *buf, size_t buf_len, size_t *length, mqtt_channel_t *channel);

    void (*get_pkt_length) (mqtt_cli_t *self, uint8_t *buf, size_t buf_len, size_t *length);

    void (*set_timeout) (mqtt_cli_t *self, uint16_t timeout);
    void (*get_timeout) (mqtt_cli_t *self, uint16_t *timeout);

    void (*set_keep_alive) (mqtt_cli_t *self, uint16_t keep_alive);
    void (*get_keep_alive) (mqtt_cli_t *self, uint16_t *keep_alive);

    /** Obtains MQTT protocol version */
    void (*get_version) (mqtt_cli_t *self, uint32_t *version);
    /** Obtain last packet sent */
    void (*get_last_pkt) (mqtt_cli_t *self, uint8_t *last_pkt);
    /** Obtains devices added devices count */
    void (*get_connected_devices) (mqtt_cli_t *self, uint16_t *devices_count);
    /** Checks if specified device is still available */
    void (*is_alive) (mqtt_cli_t *self, mqtt_channel_t *channel, uint8_t *is_alive);
    /** Sets user id */
    uint16_t (*set_user_id) (mqtt_cli_t *self, const uint8_t *user_id);
    /** Gets user id */
    void (*get_user_id) (mqtt_cli_t *self, uint8_t **user_id);
    /** Sets user name */
    uint16_t (*set_user_name) (mqtt_cli_t *self, const uint8_t *user_name);
    /** Gets user name */
    void (*get_user_name) (mqtt_cli_t *self, uint8_t **user_name);
    /** Sets user password */
    uint16_t (*set_password) (mqtt_cli_t *self, const uint8_t *password);
    /** Gets user password */
    void (*get_password) (mqtt_cli_t *self, uint8_t **password);
    /** Gets MQTT connect flags */
    void (*get_connect_flags) (mqtt_cli_t *self, uint8_t *connect_flags);
    /** Enforces disconnect during empty receiving */
    void (*disconnect) (mqtt_cli_t *self);
    void (*set_broker_ip) (mqtt_cli_t *self, uint32_t broker_ip);
    void (*get_broker_ip) (mqtt_cli_t *self, uint32_t *broker_ip);
    /** Sets callback on received packet DISCONNECT */
    void (*set_cb_disconnect) (mqtt_cli_t *self, cb_mqtt_disconnect_t cb_mqtt_disconnect);
    /** Sets callback on received packet PUBACK */
    void (*set_cb_puback) (mqtt_cli_t *self, cb_mqtt_puback_t cb_mqtt_puback);
    /** Sets callback on received packet PUBLISH */
    void (*set_cb_publish) (mqtt_cli_t *self, cb_mqtt_publish_t cb_mqtt_publish);
    /** Sets callback on received packet SUBACK */
    void (*set_cb_suback) (mqtt_cli_t *self, cb_mqtt_suback_t cb_mqtt_suback);
    /** Sets callback on received packet CONNACK */
    void (*set_cb_connack) (mqtt_cli_t *self, cb_mqtt_connack_t cb_mqtt_connack);
#ifdef __MQTT_EXPOSED__ /* U sed only for test purpose */
    /** Get devices head */
    void (*get_devices_head) (mqtt_cli_t *self, mqtt_dev_t ***devices_head);
    /** Obtains the current protocol state */
    void (*get_state) (mqtt_cli_t *self, uint8_t *state);
#endif // __MQTT_EXPOSED__
};

uint16_t __ATTR mqtt_cli_init(mqtt_cli_t *obj);
void __ATTR mqtt_cli_destr(mqtt_cli_t *obj);

#ifdef __cplusplus
}
#endif

#endif // __MQTT_H__
