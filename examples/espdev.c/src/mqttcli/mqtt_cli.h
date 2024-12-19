#ifndef __MQTT_CLI_H__
#define __MQTT_CLI_H__

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
    /**
     * @brief Prepares PUBLISH packet which will be send during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create PUBLISh packet
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended. 
     */
    uint16_t (*publish) (const mqtt_cli_t *self, const mqtt_publish_params_t *params);
    /**
     * @brief Prepares PUBLISH packet which will be send after this function or during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create PUBLISh packet
     * @param output pointer to the structure representing outgoing packet data to be send manually
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended.
     * 
     * @note If out_buf was specified, the the prepared packet will be stored in this buffer, it will be not send in the following process() function, therefore the user shall send prepared bytes.
     * @note If out_buf was set to NULL, then the prepared packet will be send during the next process() function execution.
     */
    uint16_t (*publish_ex) (const mqtt_cli_t *self, const mqtt_publish_params_t *params, clv_t *output);
    /**
     * @brief Prepares SUBSCRIBE packet which will be send during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create SUBSCRIBE packet
     * 
     * @returns MQTT_SUCCESS if the subscribe packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended. 
     */
    uint16_t (*subscribe) (const mqtt_cli_t *self, const mqtt_subscribe_params_t *params);
    /**
     * @brief Prepares SUBSCRIBE packet which will be send after this function or during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create SUBSCRIBE packet
     * @param output pointer to the structure representing outgoing packet data to be send manually
     * 
     * @returns MQTT_SUCCESS if the subscribe packet was created successfully, otherwise: 
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended. 
     */
    uint16_t (*subscribe_ex) (const mqtt_cli_t *self, const mqtt_subscribe_params_t *params, clv_t *output);
    /**
     * @brief Prepares UNSUBSCRIBE packet which will be send during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create UNSUBSCRIBE packet
     * 
     * @returns MQTT_SUCCESS if the subscribe packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended. 
     */
    uint16_t (*unsubscribe) (const mqtt_cli_t *self, const mqtt_unsubscribe_params_t *params);
    /**
     * @brief Prepares UNSUBSCRIBE packet which will be send after this function or during the following process() execution.
     * 
     * @param self pointer to the private context data
     * @param params pointer to the structure representing parameters used to create UNSUBSCRIBE packet
     * @param output pointer to the structure representing outgoing packet data to be send manually
     * 
     * @returns MQTT_SUCCESS if the subscribe packet was created successfully, otherwise: 
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker or
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended. 
     */
    uint16_t (*unsubscribe_ex) (const mqtt_cli_t *self, const mqtt_unsubscribe_params_t *params, clv_t *output);
    /**
     * @brief Processes received packets, empty packets, sends responses if needed.
     * 
     * @param self pointer to the private context data
     * @param data pointer to the structure representing incoming/outgoing data
     * @param channel pointer to mqtt_channel_t structure representing uniquely the source of incoming data or destination of outgoing data.
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if invalid arguments were provided,
     *          MQTT_INVALID_CONF is the configuration is missing,
     *          MQTT_INVALID_STATE if conditions of use was not satisfied,
     *          MQTT_NOT_CONNECTED if the client was not connected to the broker,
     *          MQTT_NO_PKT_ID if the available space in queue for packets has ended,
     *          MQTT_PENDING_DATA if there is another data to be send (see notes),
     *          MQTT_OUT_OF_MEM if number of bytes prepared to send is greater than specified buf_len,
     *          MQTT_PKT_REJECTED if incoming packet was rejected (e.g. connection was not established or broken)
     *          MQTT_MALFORMED_PACKET if incoming packet is malformed (the user could close the connection or send DISCONNECT packet) or
     *          MQTT_PTYPE_NOT_SUPPORTED if incoming packet type is not supported.
     * 
     * @note If the function has returned MQTT_PENDING_DATA, then it shall be invoked again with length set to 0 and channel IP address as well user id set to 0.
     */
    uint16_t (*process) (const mqtt_cli_t *self, clv_t *data, mqtt_channel_t *channel);
    /** 
     * @brief Prepares the DISCONNECT packet which will be send in the following process() function execution.
     * 
     * @param self pointer to the private context data
     */
    void     (*disconnect) (const mqtt_cli_t *self);
    /**
     * @brief Sets Keep Alive value used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param keep_alive new Keep Alive value.
     * 
     * @note This setting is optional.
     * @note Default value is 60s.
     */
    void     (*set_br_keepalive) (const mqtt_cli_t *self, const uint16_t keep_alive);
    /**
     * @brief Gets current Keep Alive value which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param keep_alive pointer to the unsigned short value representing Keep Alive
     */
    void     (*get_br_keepalive) (const mqtt_cli_t *self, uint16_t *keep_alive);
    /**
     * @brief Sets IP which uniquely identifies the broker.
     * Library is using this setting to determine if incoming packet was sent from the broker.
     * 
     * @param self pointer to the private context data
     * @param ip value representing ip.
     * 
     * @note This setting is optional.
     * @note Default value is 127.0.0.1.
     */
    void     (*set_br_ip) (const mqtt_cli_t *self, const uint32_t ip);
    /**
     * @brief Gets IP which uniquely identifies the broker.
     * 
     * @param self pointer to the private context data
     * @param ip pointer to the value representing ip.
     */
    void     (*get_br_ip) (const mqtt_cli_t *self, uint32_t *ip);
    /**
     * @brief Sets user id which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param user_id pointer to array of characters ended wit '\0', which represents user id
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if specified arguments are invalid or
     *          MQTT_OUT_OF_MEM if there is not enough memory.
     * 
     * @note This setting is mandatory.
     * @note Default value is NULL.
     */
    uint16_t (*set_br_userid) (const mqtt_cli_t *self, const lv_t *user_id);
    /** 
     * @brief Gets user id which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param user_id pointer to pointer to array of characters, which represents user id
     */
    void     (*get_br_userid) (const mqtt_cli_t *self, lv_t *user_id);
    /** 
     * @brief Sets user name which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param user_name pointer to array of characters ended wit '\0', which represents user name
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if specified arguments are invalid or
     *          MQTT_OUT_OF_MEM if there is not enough memory.
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    uint16_t (*set_br_username) (const mqtt_cli_t *self, const lv_t *user_name);
    /** 
     * @brief Gets user name which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param user_id pointer to pointer to array of characters, which represents user id
     */
    void     (*get_br_username) (const mqtt_cli_t *self, lv_t *user_name);
    /** 
     * @brief Sets user password which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param password pointer to array of characters ended wit '\0', which represents password
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if specified arguments are invalid or
     *          MQTT_OUT_OF_MEM if there is not enough memory.
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    uint16_t (*set_br_password) (const mqtt_cli_t *self, const lv_t *password);
    /** 
     * @brief Gets user password which is used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param password pointer to pointer to array of characters, which represents password
     */
    void     (*get_br_password) (const mqtt_cli_t *self, lv_t *password);
    /**
     * @brief Sets the will parameters which are used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param will pointer to the structure representing the will parameters
     * 
     * @returns MQTT_SUCCESS if the publish packet was created successfully, otherwise:
     *          MQTT_INVALID_ARGS if specified arguments are invalid or
     *          MQTT_OUT_OF_MEM if there is not enough memory.
     * 
     * @note The Will Topic is mandatory.
     * @note The Will Payload is optional, NULL value is accepted.
     * @note The Will Properties is optional, NULL value is accepted.
     * @note Default Will Topic is NULL.
     * @note Default Will Payload is NULL.
     * @note Default Will Properties is NULL.
     * @note Default Will QOS is 0.
     * @note Default Will Retain is 0.
     */
    uint16_t (*set_br_will) (const mqtt_cli_t *self, const mqtt_will_params_t *will);
    /**
     * @brief Gets the will parameters which are used during CONNECT packet creation.
     * 
     * @param self pointer to the private context data
     * @param will pointer to the structure representing the will parameters
     */
    void     (*get_br_will) (const mqtt_cli_t *self, mqtt_will_params_t *will);
    /**
     * @brief Checks is the Client is already connected.
     * 
     * @param self pointer to the private context data
     * @param is_connected pointer to the value determining if the client is already connected
     */
    void     (*is_connected) (const mqtt_cli_t *self, uint8_t *is_connected);
    /**
     * @brief Obtains the total packet length.
     * 
     * @param self pointer to the private context data
     * @param packet pointer to the packet data for which the length will be calculated
     * @param length pointer to calculated length of the packet
     */
    void     (*get_pkt_length) (const mqtt_cli_t *self, lv_t *packet, size_t *length);
    /** 
     * @brief Obtains MQTT this library version.
     * 
     * @param self pointer to the private context data
     * @param version pointer to the 32-bit value representing the version
     * 
     * @note The obtained version has a format: a.b.c.d
     */
    void     (*get_lib_version) (const mqtt_cli_t *self, uint32_t *version);
    /** 
     * @brief Obtains the last packet type which was prepared.
     * 
     * @param self pointer to the private context data
     * @param last_pkt pointer to the 8-bit value representing last prepared packet type.
     */
    void     (*get_last_pkt) (const mqtt_cli_t *self, uint8_t *last_pkt);
    /**
     * @brief Obtains configured buffer size.
     * 
     * @param self pointer to the private context data
     * @param last_pkt pointer to the 16-bit value representing configured internal buffer size.
     */
    void     (*get_buffersize) (const mqtt_cli_t *self, uint16_t *buffersize);
    /**
     * @brief Sets callback on packet DISCONNECT received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_disconnect function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_disconnect) (const mqtt_cli_t *self, cb_mqtt_disconnect_t cb_mqtt_disconnect);
    /**
     * @brief Sets callback on packet PUBACK received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_puback function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_puback) (const mqtt_cli_t *self, cb_mqtt_puback_t cb_mqtt_puback);
    /**
     * @brief Sets callback on packet PUBLISH received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_publish function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_publish) (const mqtt_cli_t *self, cb_mqtt_publish_t cb_mqtt_publish);
    /** 
     * @brief Sets callback on packet SUBACK received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_suback function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_suback) (const mqtt_cli_t *self, cb_mqtt_suback_t cb_mqtt_suback);
    /** 
     * @brief Sets callback on packet CONNACK received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_connack function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_connack) (const mqtt_cli_t *self, cb_mqtt_connack_t cb_mqtt_connack);
    /**
     * @brief Sets callback on packet CONNECT being prepared or AUTH packet received.
     * 
     * @param self pointer to the private context data
     * @param cb_mqtt_auth function pointer
     * 
     * @note This value is optional.
     * @note Default value is NULL.
     */
    void     (*set_cb_auth) (const mqtt_cli_t *self, cb_mqtt_auth_t cb_mqtt_auth);
};

/**
 * @brief Initializes MQTT protocol with specified parameters.
 * 
 * @param obj pointer to the private context data
 * @param params pointer protocol parameters
 * @returns MQTT_SUCCESS if the library was successfully initialized, otherwise:
 *          MQTT_OUT_OF_MEM if there was not enough memory to initialize the library or
 *          MQTT_INVALID_ARGS if specified parameters are out of acceptable ranges.
 * 
 * @note It is not possible to change these parameters afters successful initialization.
 */
uint16_t __ATTR mqtt_cli_init_ex(mqtt_cli_t *obj, mqtt_params_t *params);
/**
 * @brief Initializes MQTT protocol using default parameters
 * 
 * @param obj pointer to the private context data
 * @param params pointer protocol parameters
 * @returns MQTT_SUCCESS if the library was successfully initialized, otherwise:
 *          MQTT_OUT_OF_MEM if there was not enough memory to initialize the library.
 * 
 * @note It is not possible to change these parameters afters successful initialization.
 * @note Default parameters are equal as follows:
 *       bufsize = 1024 B,
 *       qos = 0,
 *       timeout = 1 second and
 *       version = 5
 */
uint16_t __ATTR mqtt_cli_init(mqtt_cli_t *obj);
/**
 * @brief Releases the library's resources. 
 * 
 * @param obj pointer to the private context data
 */
void     __ATTR mqtt_cli_destr(mqtt_cli_t *obj);

#ifdef __cplusplus
}
#endif

#endif // __MQTT_CLI_H__
