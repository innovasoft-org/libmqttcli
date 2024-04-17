#ifndef __MQTT_COMMON__H__
#define __MQTT_COMMON__H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct mqtt_common_ctx;

/** Maximum buffer size */
#define DEFAULT_BUFSIZE  1024
/** Default QoS: 0 */
#define DEFAULT_QOS      0
/** Default timeout */
#define DEFAULT_TIMEOUT  1
/** MQTT version */
#define DEFAULT_VERSION  5

/** Minimum length of the Protocol Name */
#define MIN_PROTOCOL_NAME_LEN 4
/** Maximum length of the Protocol Name */
#define MAX_PROTOCOL_NAME_LEN 16
/** Minimum length of the Authentication Method */
#define MIN_AUTHMETHOD_LEN    1
/** Maximum length of the Authentication Method */
#define MAX_AUTHMETHOD_LEN    32
/** Minimum length of the User ID */
#define MIN_USERID_LEN        8
/** Maximum length of the User ID */
#define MAX_USERID_LEN        23
/** Minimum length of the User Name */
#define MIN_USERNAME_LEN      0
/** Maximum length of the User Name */
#define MAX_USERNAME_LEN      64
/** Minimum length of the Password */
#define MIN_PASSWORD_LEN      0
/** Maximum length of the Password */
#define MAX_PASSWORD_LEN      64
/** Minimum length of the topic */
#define MIN_TOPIC_LEN         1
/** Maximum length of the topic */
#define MAX_TOPIC_LEN         1024
/** Minimum length of the message */
#define MIN_MESSAGE_LEN       0
/** Maximum length of the message */
#define MAX_MESSAGE_LEN       1024
/** Minimum properties length */
#define MIN_PROPERTIES_LEN    0
/** Maximum properties length */
#define MAX_PROPERTIES_LEN    255
/** Minimum will payload length */
#define MIN_WILL_PAYLOAD_LEN  1
/** Maximum will payload length */
#define MAX_WILL_PAYLOAD_LEN  1024
/** Maximum number of topic filters in subscribe packet */
#define MAX_TOPIC_FILTERS     8
/** Maximum number of packet identifiers to use */
#define MAX_PKT_ID            ((uint8_t) 32)

#ifndef LOGLN
    #define LOGLN(level, msg)
#endif

/** Define architecture specific attributes */
#define __ATTR

/** Packet typed */
#define PTYPE_NONE        ((uint8_t) 0x00)
#define PTYPE_CONNECT     ((uint8_t) 0x01)
#define PTYPE_CONNACK     ((uint8_t) 0x02)
#define PTYPE_PUBLISH     ((uint8_t) 0x03)
#define PTYPE_PUBACK      ((uint8_t) 0x04)
#define PTYPE_PUBREC      ((uint8_t) 0x05)
#define PTYPE_PUBREL      ((uint8_t) 0x06)
#define PTYPE_PUBCOMP     ((uint8_t) 0x07)
#define PTYPE_SUBSCRIBE   ((uint8_t) 0x08)
#define PTYPE_SUBACK      ((uint8_t) 0x09)
#define PTYPE_UNSUBSCRIBE ((uint8_t) 0x0A)
#define PTYPE_UNSUBACK    ((uint8_t) 0x0B)
#define PTYPE_PINGREQ     ((uint8_t) 0x0C)
#define PTYPE_PINGRESP    ((uint8_t) 0x0D)
#define PTYPE_DISCONNECT  ((uint8_t) 0x0E)
#define PTYPE_AUTH        ((uint8_t) 0x0F)

/** Error codes */
#define MQTT_SUCCESS             ( (uint16_t) 0x0A01 )
#define MQTT_INVALID_ARGS        ( (uint16_t) 0x0A02 )
#define MQTT_INVALID_STATE       ( (uint16_t) 0x0A03 )
#define MQTT_INVALID_CONF        ( (uint16_t) 0x0A04 )
#define MQTT_OUT_OF_MEM          ( (uint16_t) 0x0A05 )
#define MQTT_PTYPE_NOT_SUPPORTED ( (uint16_t) 0x0A06 )
#define MQTT_MALFORMED_PACKET    ( (uint16_t) 0x0A07 )
#define MQTT_NO_DEVICE           ( (uint16_t) 0x0A08 )
#define MQTT_PENDING_DATA        ( (uint16_t) 0x0A09 )
#define MQTT_PKT_REJECTED        ( (uint16_t) 0x0A0A )
#define MQTT_NO_PKT_ID           ( (uint16_t) 0x0A0B )
#define MQTT_NOT_CONNECTED       ( (uint16_t) 0x0A0C )
#define MQTT_NOT_SUPPORTED       ( (uint16_t) 0x0A0D )
#define MQTT_NOT_INITIALIZED     ( (uint16_t) 0x0A0E )

typedef enum {
  /** Byte */
  TYPE_INT8 = 1,
  /** Two Byte Integer */
  TYPE_INT16 = 2,
  /** Four Byte Integer */
  TYPE_INT32 = 3,
  /** Variable Byte Integer */
  TYPE_INTVAR = 4,
  /** UTF-8 Encoded String */
  TYPE_UTF8 = 5,
  /** UTF-8 String Pair */
  TYPE_UTF8_PAIR = 6,
  /** Binary Data */
  TYPE_BINARY = 7,
  /** MQTT Data Type */
} mqtt_type_t;

typedef enum {
  /** Success / Granted QoS 0 */
  RC_SUCCESS = 0x00,
  /** Granted QoS 1 */
  RC_GRANTED_QOS_1 = 0x01,
  /** Granted QoS 2 */
  RC_GRANTED_QOS_2 = 0x02,
  /** Disconnect with Will Message */
  RC_DISCONNECT_WILL_MSG = 0x04,
  /** No matching subscribers */
  RC_NO_MATCH_SUBSCRIBERS = 0x10,
  /** No subscription existed */
  RC_NO_SUBSCRIPTION_EXIST = 0x11,
  /** Continue authentication */
  RC_CONTINUE_AUTH = 0x18,
  /** Re-authenticate */
  RC_RE_AUTH = 0x19,
  /** Unspecified error */
  RC_UNSPECIFIED_ERR = 0x80,
  /** Malformed Packet */
  RC_MALFORMED_PKT = 0x81,
  /** Protocol Error */
  RC_PROTOCOL_ERR = 0x82,
  /** Implementation specific error */
  RC_IMPL_SPEC_ERR = 0x83,
  /** Unsupported Protocol Version */
  RC_UNSUPPORTED_PROT_VER = 0x84,
  /** Client Identifier not valid */
  RC_CLIENT_ID_NOT_VALID = 0x85,
  /** Bad User Name or Password */
  RC_BAD_USR_OR_PASS = 0x86,
  /** Not authorized */
  RC_NOT_AUTHORIZED = 0x87,
  /** Server unavailable */
  RC_SEV_UNAVAILABLE = 0x88,
  /** Server busy */
  RC_SRV_BUSY = 0x89,
  /** Banned */
  RC_BANNED = 0x8A,
  /** Server shutting down */
  RC_SRV_SHOUTING_DOWN = 0x8B,
  /** Bad authentication method */
  RC_BAD_AUTH_METHOD = 0x8C,
  /** Keep Alive timeout */
  RC_KEEP_ALIVE_TIMEOUT = 0x8D,
  /** Session taken over */
  RC_SESSION_TAKEN_OVER = 0x8E,
  /** Topic Filter invalid */
  RC_TOPIC_FILTER_INV = 0x8F,
  /** Topic Name invalid */
  RC_TOPIC_NAME_INV = 0x90,
  /** Packet Identifier in use */
  RC_PKT_ID_IN_USE = 0x91,
  /** Packet Identifier not found */
  RC_PKT_ID_NOT_FOUND = 0x92,
  /** Receive Maximum exceeded */
  RC_RECV_MAX_EXCEEDED = 0x93,
  /** Topic Alias invalid */
  RC_TOPIC_ALIAS_LARGE = 0x94,
  /** Packet too large */
  RC_PKT_TOO_LARGE = 0x95,
  /** Message rate too high */
  RC_MSG_RATE_HIGH = 0x96,
  /** Quota exceeded */
  RC_QUOTA_EXCEEDED = 0x97,
  /** Administrative action */
  RC_ADMIN_ACTION = 0x98,
  /** Payload format invalid */
  RC_PAYLOAD_INV = 0x99,
  /** Retain not supported */
  RC_RETAIN_NOT_SUPPORTED = 0x9A,
  /** QoS not supported */
  RC_QOS_NOT_SUPPORTED = 0x9B,
  /** Use another server */
  RC_USE_ANOTHER_SRV = 0x9C,
  /** Server moved */
  RC_SRV_MOVED = 0x9D,
  /** Shared Subscriptions not supported */
  RC_SHARE_SUBS_NOT_SUPPORTED = 0x9E,
  /** Connection rate exceeded */
  RC_CORRECT_RATe_EXCEEDED = 0x9F,
  /** Maximum connect time */
  RC_MAX_CONNECT_TIME = 0xA0,
  /** Subscription Identifiers not supported */
  RC_SUBSCRIPTION_ID_NOT_SUPPORTED = 0xA1,
  /** Wildcard Subscriptions not supported */
  RC_WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED = 0xA2
  /** MQTT Reason code */
} mqtt_rc_t;

typedef struct {
  /** Length of significant data in the buffer */
  size_t length;
  /** Buffer */
  uint8_t *value;
  /** Length Value structure */
} lv_t;

typedef struct {
  /** Total capacity of the buffer */
  const size_t capacity;
  /** Current buffer length */
  size_t length;
  /** Buffer value */
  uint8_t *const value;
  /** Capacity Length Value structure */
} clv_t;

typedef struct {
  /** Internal buffer size */
  uint16_t bufsize;
  /** Quality of Service */
  uint8_t qos;
  /** Representation of timeout in seconds */
  uint16_t timeout;
  /** MQTT protocol version */
  uint8_t version;
} mqtt_params_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Application Message */
  lv_t message;
  /** Properties */
  lv_t properties;
  /** Topic Name */
  lv_t topic;
  /** PUBLISH packet parameters */
} mqtt_publish_params_t;

typedef struct {
  /** Length of the topic filter */
  size_t length;
  /** Subscription options */
  uint8_t options;
  /** Single topic filter value */
  uint8_t *value;
  /** Single subscription filter */
} mqtt_subscribe_filter_t;

typedef struct {
  /** Single payload */
  mqtt_subscribe_filter_t filter;  
  /** Properties */
  lv_t properties;
  /** SUBSCRIBE packet parameters */
} mqtt_subscribe_params_t;

typedef struct {
  /** Properties */
  lv_t properties;
  /** Single topic Filters */
  lv_t filter;
  /** UNSUBSCRIBE packet parameters */
} mqtt_unsubscribe_params_t;

typedef struct {
  /** Properties */
  lv_t properties;
  /** DISCONNECT packet parameters */
} mqtt_disconnect_params_t;

typedef struct mqtt_cli_ctx_cb mqtt_cli_ctx_cb_t;

struct mqtt_cli_ctx_cb {
  /** Private common context */
  struct mqtt_common_ctx *ctx;
  /**
   * @brief Calculates offset of the specified tag, number of bytes used to decode the data length and data length
   * 
   * @param[in] tag the tag which shall be find within specified buffer
   * @param[in] buf pointer to the buffer
   * @param[out] offset pointer to the offset of the tag
   * @param[out] used pointer to the number of bytes used to decode the data length
   * @param[in,out] length pointer to the total length of specified buffer as well as number of bytes inside data field
   * 
   * @note To calculate offset of the data field, the following formula shall be used: offset + 1 + used
   * @note For UTF-8 String Pair the length is equal to the number of bytes in a name part.
   */
  void     __ATTR (*find_property) (const uint8_t tag, const uint8_t *buf, size_t *offset, size_t *used, size_t *length);

  uint16_t __ATTR (*publish) (const mqtt_cli_ctx_cb_t *self, const mqtt_publish_params_t *params);
  uint16_t __ATTR (*subscribe) (const mqtt_cli_ctx_cb_t *self, const mqtt_subscribe_params_t *params);
  uint16_t __ATTR (*unsubscribe) (const mqtt_cli_ctx_cb_t *self, const mqtt_unsubscribe_params_t *params);
  uint16_t __ATTR (*disconnect) (const mqtt_cli_ctx_cb_t *self, const mqtt_disconnect_params_t *params);
  /** client common callback context */
};

/** AUTH packet context */
typedef struct mqtt_auth_ctx_cb mqtt_auth_ctx_cb_t;

/** AUTH packet structure */
struct mqtt_auth_ctx_cb {
  /** Private common context */
  struct mqtt_common_ctx *ctx;
  /** 
   * @brief Sets authentication data in response to received authentication data.
   * 
   * @param self pointer to private context data
   * @param auth_data authentication data
   * 
   * @return MQTT_SUCCESS on success, otherwise MQTT_INV_ARGS is specified argument were wrong.
   */
  uint16_t (*authenticate) (const mqtt_auth_ctx_cb_t *self, const lv_t *auth_method, const lv_t *auth_data);
  /**
   * @brief Obtains current authentication step.
   * 
   * @param self pointer to private context data
   * @param auth_step pointer to the current authentication step
   * 
   * @return MQTT_SUCCESS on success, otherwise MQTT_INV_ARGS is specified argument were wrong.
   */
  uint16_t (*get_auth_step) (const mqtt_auth_ctx_cb_t *self, uint8_t *auth_step);
  /**
   * @brief Calculates offset of the specified tag, number of bytes used to decode the data length and data length
   * 
   * @param[in] tag the tag which shall be find within specified buffer
   * @param[in] buf pointer to the buffer
   * @param[out] offset pointer to the offset of the tag
   * @param[out] used pointer to the number of bytes used to decode the data length
   * @param[in,out] length pointer to the total length of specified buffer as well as number of bytes inside data field
   * 
   * @note To calculate offset of the data field, the following formula shall be used: offset + 1 + used
   * @note For UTF-8 String Pair the length is equal to the number of bytes in a name part.
   */
  void   __ATTR (*find_property) (const uint8_t tag, const uint8_t *buf, size_t *offset, size_t *used, size_t *length);
};

typedef struct {
  /** Connect Flags */
  uint8_t connect_flags;
  /** Flags */
  uint8_t flags;
  /** Keep Alive */
  uint16_t keep_alive;
  /** Password */
  lv_t password;
  /** Properties */
  lv_t properties;
  /** Protocol Name */
  lv_t protocol_name;
  /** Protocol Version */
  uint8_t protocol_version;
  /** User Identifier */
  lv_t user_id;
  /** User Name */
  lv_t user_name;
  /** Will Payload */
  lv_t will_payload;
  /** Will Properties */
  lv_t will_properties;
  /** Will Topic */
  lv_t will_topic;
  /** CONNECT packet data */
} mqtt_connect_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Properties */
  lv_t properties;
  /** Connect Reason Code */
  uint8_t rc;
  /** Connect Acknowledge Flags */
  uint8_t connect_ack_flags;
  /** CONNACK packet data */
} mqtt_connack_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Application Message */
  lv_t message;
  /** Properties */
  lv_t properties;
  /** Topic Name */
  lv_t topic;
  /** PUBLISH packet data */
} mqtt_publish_t;

typedef struct {
  /** Packet Identifier */
  uint16_t id;
  /** Flags */
  uint8_t flags;
  /** Properties */
  lv_t properties;
  /** Reason Code */
  uint8_t rc;
  /** PUBACK packet data */
} mqtt_puback_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** Single subscription filters */
  mqtt_subscribe_filter_t filter;
  /** SUBSCRIBE packet data */
} mqtt_subscribe_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** List of filters specified for each subscription */
  mqtt_subscribe_filter_t filters[MAX_TOPIC_FILTERS];
  /** SUBSCRIBE all packet data */
} mqtt_subscribe_all_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** Reason Codes */
  uint8_t rc[MAX_TOPIC_FILTERS];
  /** Reason Codes Length */
  uint8_t rc_len;
  /** SUBACK packet data */
} mqtt_suback_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** Topic Filters */
  lv_t filter;
  /** UNSUBSCRIBE packet data */
} mqtt_unsubscribe_t;

typedef struct {
  /** Private common context */
  struct mqtt_common_ctx *ctx;
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** List of filters for each unsubscription */
  lv_t filters[MAX_TOPIC_FILTERS];
  /** UNSUBSCRIBE all packet data */
} mqtt_unsubscribe_all_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Properties */
  lv_t properties;
  /** Reason Codes */
  uint8_t rc[MAX_TOPIC_FILTERS];
  /** Reason Codes Length */
  uint8_t rc_len;
  /** UNSUBACK packet data */
} mqtt_unsuback_t;

typedef struct {
  /** Flags */
  uint8_t flags;
  /** Properties */
  lv_t properties;
  /** Disconnect Reason Code */
  uint8_t rc;
  /** DISCONNECT packet data */
} mqtt_disconnect_t;

typedef struct  {
  /** Flags */
  uint8_t flags;
  /** Properties */
  lv_t properties;
  /** Authentication Reason Code */
  uint8_t rc;
  /** AUTH packet data */
} mqtt_auth_t;

typedef struct {
  /** IPv4 address */
  uint32_t ip_address;
  /** User unique ID */
  uint32_t user_id;
/** Channel unique identification context */
} mqtt_channel_t;

typedef struct {
  /** The Value-Length structure representing the Will Payload */
  lv_t payload;
  /** The Value-Length structure representing the Will Properties */
  lv_t properties;
  /** The value representing the Quality Of Service (QOS) of the Will */
  uint8_t qos;
  /** The value representing the retain option of the Will */
  uint8_t retain;
  /** The Value-Length structure representing the Will Topic */
  lv_t topic;
/** MQTT will parameters */
} mqtt_will_params_t;

/** 
 * @brief Callback definition for CONNECT packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to CONNECT packet structure
 * @param channel pointer to channel structure
 */
typedef mqtt_rc_t (*cb_mqtt_connect_t)         (const mqtt_cli_ctx_cb_t *self, const mqtt_connect_t *pkt, const mqtt_channel_t *channel);
/**
 * @brief Callback definition for CONNACK packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to CONNACK packet structure
 * @param channel pointer to channel structure
 */
typedef mqtt_rc_t (*cb_mqtt_connack_t)         (const mqtt_cli_ctx_cb_t *self, const mqtt_connack_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for PUBLISH packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to PUBLISH packet structure
 * @param channel pointer to channel structure
 */
typedef mqtt_rc_t (*cb_mqtt_publish_t)         (const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for PUBACK packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to PUBACK packet structure
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_puback_t)          (const mqtt_cli_ctx_cb_t *self, const mqtt_puback_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for SUBSCRIBE packet with all topic filters received.
 * @param self pointer to the callback context
 * @param pkt pointer to SUBSCRIBE packet structure with all topic filters received
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_subscribe_all_t)   (const mqtt_cli_ctx_cb_t *self, const mqtt_subscribe_all_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for SUBSCRIBE packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to SUBSCRIBE packet structure
 * @param channel pointer to channel structure
 */
typedef mqtt_rc_t (*cb_mqtt_subscribe_t)       (const mqtt_cli_ctx_cb_t *self, const mqtt_subscribe_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for SUBACK packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to SUBACK packet structure
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_suback_t)          (const mqtt_cli_ctx_cb_t *self, const mqtt_suback_t *pkt, const mqtt_channel_t *channel);
/** 
 * Callback definition for UNSUBSCRIBE packet with all topic filters received.
 * @param self pointer to the callback context
 * @param pkt pointer to UNSUBSCRIBE packet structure with all topic filters
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_unsubscribe_all_t) (const mqtt_cli_ctx_cb_t *self, const mqtt_unsubscribe_all_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for UNSUBSCRIBE packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to UNSUBSCRIBE packet structure
 * @param channel pointer to channel structure
 */
typedef mqtt_rc_t (*cb_mqtt_unsubscribe_t)     (const mqtt_cli_ctx_cb_t *self, const mqtt_unsubscribe_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for UNSUBACK packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to UNSUBACK packet structure
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_unsuback_t)        (const mqtt_cli_ctx_cb_t *self, const mqtt_unsuback_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for DISCONNECT packet received.
 * @param self pointer to the callback context
 * @param pkt pointer to DISCONNECT packet structure
 * @param channel pointer to channel structure
 */
typedef void      (*cb_mqtt_disconnect_t)      (const mqtt_cli_ctx_cb_t *self, const mqtt_disconnect_t *pkt, const mqtt_channel_t *channel);
/** 
 * @brief Callback definition for AUTH packet received or CONNECT being prepared.
 * @param self pointer to the callback context
 * @param pkt pointer to AUTH packet structure
 * @param channel pointer to channel structure
 * @note During CONNECT packet preparation this callback is called to determine if authentication shall be used during connection establishment with the broker.
 * @note During CONNECT packet preparation this callback has pointer to channel set to NULL.
 * @note RC_CONTINUE_AUTH as return value is always expected if only authentication step was accepted.
 * @note Authentication method (if any) is present in pkt->properties. To obtain its offset, use: mqtt_find_property(0x15, buf, &offset, &used, size_t &length);
 * @note Authentication data (if any) is present in pkt->properties. To obtain its offset, use: mqtt_find_property(0x16, buf, &offset, &used, size_t &length);
 * @note To return authentication data use: pkt->authenticate(pkt, data);
 */
typedef mqtt_rc_t (*cb_mqtt_auth_t)            (const mqtt_auth_ctx_cb_t *self, const mqtt_auth_t *pkt, const mqtt_channel_t *channel);

#endif // __MQTT_COMMON_H__