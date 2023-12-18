#ifndef __MQTT_COMMON__H__
#define __MQTT_COMMON__H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/** MQTT version */
#define MQTT_VERSION          5
/** Maximum buffer size */
#define BUFFER_SIZE           1024
/** Minimum length of the Protocol Name */
#define MIN_PROTOCOL_NAME_LEN 4
/** Maximum length of the Protocol Name */
#define MAX_PROTOCOL_NAME_LEN 16
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
#define MAX_TOPIC_LEN         512
/** Minimum length of the message */
#define MIN_MESSAGE_LEN       0
/** Maximum length of the message */
#define MAX_MESSAGE_LEN       512
/** Minimum properties length */
#define MIN_PROPERTIES_LEN    0
/** Maximum properties length */
#define MAX_PROPERTIES_LEN    255
/** Minimum will payload length */
#define MIN_WILL_PAYLOAD_LEN  0
/** Maximum will payload length */
#define MAX_WILL_PAYLOAD_LEN  512
/** Maximum number of topic filters in subscribe packet */
#define MAX_TOPIC_FILTERS     8
/** Maximum number of packet identifiers to use */
#define MAX_PKT_ID            ((uint8_t) 32)

//#ifndef TOLOG//
//    #define TOLOG(level, msg) __android_log_write(ANDROID_LOG_DEBUG, "NDK", msg)
//    #define TOLOG(level, msg)
//#endif

#ifndef LOGLN
    #define LOGLN(level, msg)
#endif

/** Define architecture specific attributes */
#define __ATTR

/** IP reserved for all devices (multicast/broadcast sending purpose) */
#define IP_ALL_DEVICES ((uint32_t) 0xFFFFFFFF)

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
#define PTYPE_AUTH        ((uint8_t) 0xF0)

/** Error codes */
#define MQTT_SUCCESS             ((uint16_t) 0x0A01)
#define MQTT_INVALID_ARGS        ((uint16_t) 0x0A02)
#define MQTT_INVALID_STATE       ((uint16_t) 0x0A03)
#define MQTT_INVALID_CONF        ((uint16_t) 0x0A04)
#define MQTT_OUT_OF_MEM          ((uint16_t) 0x0A05)
#define MQTT_PTYPE_NOT_SUPPORTED ((uint16_t) 0x0A06)
#define MQTT_MALFORMED_PACKET    ((uint16_t) 0x0A07)
#define MQTT_NO_DEVICE           ((uint16_t) 0x0A08)
#define MQTT_PENDING_DATA        ((uint16_t) 0x0A09)
#define MQTT_PKT_REJECTED        ((uint16_t) 0x0A0A)
#define MQTT_NO_PKT_ID           ((uint16_t) 0x0A0B)

/** MQTT Reason codes */
typedef enum mqtt_rc {
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
} mqtt_rc_t;

/** Value Length structure */
typedef struct {
  size_t length;
  uint8_t *value;
} vl_t;

/** Value Length Options structure */
typedef struct {
  size_t length;
  uint8_t options;
  uint8_t *value;  
} vlo_t;

/** CONNECT packet data */
typedef struct {
  /** Connect Flags */
  uint8_t connect_flags;
  /** Flags */
  uint8_t flags;
  /** Keep Alive */
  uint16_t keep_alive;
  /** Password */
  vl_t password;
  /** Properties */
  vl_t properties;
  /** Protocol Name */
  vl_t protocol_name;
  /** Protocol Version */
  uint8_t protocol_version;
  /** User Identifier */
  vl_t user_id;
  /** User Name */
  vl_t user_name;
  /** Will Payload */
  vl_t will_payload;
  /** Will Properties */
  vl_t will_properties;
  /** Will Topic */
  vl_t will_topic;
} mqtt_connect_t;

/** CONNACK packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Properties */
  vl_t properties;
  /** Connect Reason Code */
  uint8_t rc;
  /** Connect Acknowledge Flags */
  uint8_t connect_ack_flags;  
} mqtt_connack_t;

/** PUBLISH packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Application Message */
  vl_t message;
  /** Properties */
  vl_t properties;
  /** Topic Name */
  vl_t topic;
} mqtt_publish_t;

/** PUBACK packet data */
typedef struct {
  /** Packet Identifier */
  uint16_t id;
  /** Flags */
  uint8_t flags;
  /** Properties */
  vl_t properties;
  /** Reason Code */
  uint8_t rc;
} mqtt_puback_t;

/** SUBSCRIBE packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  vl_t properties;
  /** Topic Filter List */
  vlo_t topic[MAX_TOPIC_FILTERS];
} mqtt_subscribe_t;

/** SUBACK packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Properties */
  vl_t properties;
  /** Reason Codes */
  uint8_t rc[MAX_TOPIC_FILTERS];
  /** Reason Codes Length */
  uint8_t rc_len;
} mqtt_suback_t;

/** UNSUBSCRIBE packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */  
  uint16_t id;
  /** Properties */
  vl_t properties;
  /** Topic Filters */
  vl_t topic[MAX_TOPIC_FILTERS];
} mqtt_unsubscribe_t;

/** UNSUBACK packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Packet Identifier */
  uint16_t id;
  /** Properties */
  vl_t properties;
  /** Reason Codes */
  uint8_t rc[MAX_TOPIC_FILTERS];
  /** Reason Codes Length */
  uint8_t rc_len;
} mqtt_unsuback_t;

/** DISCONNECT packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Properties */
  vl_t properties;
  /** Disconnect Reason Code */
  uint8_t rc;
} mqtt_disconnect_t;

/** AUTH packet data */
typedef struct {
  /** Flags */
  uint8_t flags;
  /** Properties */
  vl_t properties;
  /** Authentication Reason Code */
  uint8_t rc;
} mqtt_auth_t;

/** Channel unique identification data */
typedef struct {
  uint32_t ip_address;
  uint32_t user_id;
} mqtt_channel_t;

/** Callback for DISCONNECT packet received */
typedef void (*cb_mqtt_disconnect_t)(const mqtt_disconnect_t *pkt, const mqtt_channel_t *channel);
/** Callback for CONNECT packet received */
typedef mqtt_rc_t (*cb_mqtt_connect_t)   (const mqtt_connect_t *pkt, const mqtt_channel_t *channel);
/** Callback for CONNACK packet received */
typedef void (*cb_mqtt_connack_t)   (const mqtt_connack_t *pkt, const mqtt_channel_t *channel);
/** Callback for PUBLISH packet received */
typedef mqtt_rc_t (*cb_mqtt_publish_t)   (const mqtt_publish_t *pkt, const mqtt_channel_t *channel);
/** Callback for SUBSCRIBE packet received */
typedef mqtt_rc_t (*cb_mqtt_subscribe_t) (const mqtt_subscribe_t *pkt, const mqtt_channel_t *channel);
/** Callback for PUBACK packet received */
typedef void (*cb_mqtt_puback_t)    (const mqtt_puback_t *pkt, const mqtt_channel_t *channel);
/** Callback for SUBACK packet received */
typedef void (*cb_mqtt_suback_t)    (const mqtt_suback_t *pkt, const mqtt_channel_t *channel);

#endif // __MQTT_COMMON_H__