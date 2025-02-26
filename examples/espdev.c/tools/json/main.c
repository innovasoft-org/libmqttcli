#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ICACHE_FLASH_ATTR
#define LOG_DEBUG 1
#define TOLOG( LEVEL, MSG ) 
#define FUN_E_ARGS -1
#define FUN_E_VALUE -2
#define FUN_OK 0

/** System configuration parameter: dev_mode */
#define DEV_MODE          "devMode"
/** System configuration parameter: wifi_ssid */
#define WIFI_SSID         "wifiSsid"
/** System configuration parameter: wifi_pass */
#define WIFI_PASS         "wifiPass"

/** Accepted json string */
typedef enum { 
  STRING_NULL = 0, 
  STRING_DEV_MODE, 
  STRING_WIFI_SSID, 
  STRING_WIFI_PASS,
  STRING_GPIO0,
  STRING_GPIO2,
  STRING_GPIO4,
  STRING_GPIO5,
  STRING_GPIO12,
  STRING_GPIO13,
  STRING_GPIO14,
  STRING_GPIO15
} json_string;

/** System configuration parameter: mode */
#define MODE              "mode"

/** System configuration parameter: ssid */
#define SSID              "ssid"

/** System configuration parameter: pass */
#define PASS              "pass"

static uint16_t ICACHE_FLASH_ATTR
set_sys_conf(const char* value, const size_t len) {
  extern struct user_cfg cfg;
  int offset = 0, value_offset, value_len, buf_len;
  json_string string;

  TOLOG(LOG_DEBUG, "set_sys_conf()");

  if(4 > len || NULL == value) {
    return FUN_E_ARGS;
  }

  /* json: '{' */
  if(value[offset] != '{') {
    TOLOG(LOG_DEBUG, "json: '{' - syntax\r\n");
    return FUN_E_ARGS;
  }
  ++offset;
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: '{' - offset\r\n");
    return FUN_E_ARGS;
  }

json_param:

  /* json: '"' */
  while(value[offset] == ' ' && len > offset) {
    ++offset;
  }
  if(value[offset] != '"') {
    TOLOG(LOG_DEBUG, "json: '\"' - syntax\r\n");
    return FUN_E_ARGS;
  }
  ++offset;
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: '\"' - offset\r\n");
    return FUN_E_ARGS;
  }

  /* json: string */
  string = STRING_NULL;
  if( !memcmp( &value[offset], DEV_MODE, sizeof(DEV_MODE)-1 ) ) {
    string = STRING_DEV_MODE;
    offset += sizeof(DEV_MODE) - 1;
  }
  else if( !memcmp( &value[offset], WIFI_SSID, sizeof(WIFI_SSID)-1 ) ) {
    string = STRING_WIFI_SSID;
    offset += sizeof(WIFI_SSID) - 1;
  }
  else if( !memcmp( &value[offset], WIFI_PASS, sizeof(WIFI_PASS)-1 ) ) {
    string = STRING_WIFI_PASS;
    offset += sizeof(WIFI_PASS) - 1;
  }
  else {
    TOLOG(LOG_DEBUG, "json: string - unknown\r\n");
    return FUN_E_ARGS;
  }
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: string - offset\r\n");
    return FUN_E_ARGS;
  }

  /* json: '"' */
  if(value[offset] != '"') {
    TOLOG(LOG_DEBUG, "json: '\"' - syntax\r\n");
    return FUN_E_ARGS;
  }
  ++offset;
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: '\"' - offset\r\n");
    return FUN_E_ARGS;
  }

  /* json: ':' */
  while(value[offset] == ' ' && len > offset) {
    ++offset;
  }
  if(value[offset] != ':') {
    TOLOG(LOG_DEBUG, "json: ':' - syntax\r\n");
    return FUN_E_ARGS;
  }
  ++offset;  
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: ':' - offset\r\n");
    return FUN_E_ARGS;
  }

  /* json: '"' */
  while(value[offset] == ' ' && len > offset) {
    ++offset;
  }
  if(value[offset] != '"') {
    TOLOG(LOG_DEBUG, "json: '\"' - syntax\r\n");
    return FUN_E_ARGS;
  }
  ++offset;
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: '\"' - offset\r\n");
    return FUN_E_ARGS;
  }

  /* json: value */
  value_offset = offset;
  value_len = 0;
  while(value[offset] != '"' && len > offset) {
    ++offset;
    ++value_len;
  }
  ++offset;
  if(len < offset) {
    TOLOG(LOG_DEBUG, "json: value - offset\r\n");
    return FUN_E_ARGS;
  }

  switch(string) {
    case STRING_DEV_MODE:
      printf("STRING_DEV_MODE\r\n");
      break;
    case STRING_WIFI_SSID:
      printf("STRING_WIFI_SSID\r\n");
      break;
    case STRING_WIFI_PASS:
      printf("STRING_WIFI_PASS\r\n");
      break;
    default:
      return FUN_E_ARGS;
  }

  /* json: '}' */
  while(value[offset] == ' ' && len > offset) {
    ++offset;
  }
  if(value[offset] == '}') {
    return FUN_OK;
  }
  else if(value[offset] == ',') {
    ++offset;
    if(len < offset) {
      TOLOG(LOG_DEBUG, "json: ',' - offset\r\n");
      return FUN_E_ARGS;
    }
    goto json_param;
  }

  return FUN_E_ARGS;
}

int main() {
  int rc;
  //char value[] = "{ \"mode\" : \"O\" , \"ssid\" : \"Posejdon\" , \"pass\" : \"05377749\" }";
  char value[] = "{\"devMode\":\"O\",\"wifiSsid\":\"Posejdon\",\"wifiPass\":\"05377749\"}";
  printf("value = %s\r\n", value);
  printf("value_len = %d\r\n", sizeof(value));
  rc = set_sys_conf(value, sizeof(value));
  printf("rc = %d\r\n", rc);
  return rc;
}