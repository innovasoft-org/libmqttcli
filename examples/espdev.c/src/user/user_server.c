#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

#include "../include/user_config.h"
#include "user_log.h"
#include "user_net.h"
#include "user_cfg.h"

#define ISDIGIT(V) ( V==0x30 || V==0x31 || V==0x32 || V==0x33 || V==0x34 || V==0x35 || V==0x36 || V==0x37 || V==0x38 || V==0x39 )

const uint8_t SERVER_NAME[] = "ESP-HTTPDv1";
const uint8_t HTTP_VERSION[] = "HTTP/1.1";
const uint8_t HTTP_SC_200[] = "200 OK";
const uint8_t HTTP_SC_400[] = "400 Bad Request";
const uint8_t HTTP_SC_404[] = "404 Not Found";
const uint8_t HTTP_SC_500[] = "500 Internal Server Error";
const uint8_t HTTP_SC_505[] = "505 HTTP Version Not Supported";
const uint8_t HTTP_HDR_SERVER[]            = "Server";
const uint8_t HTTP_HDR_ACCEPT_RANGES[]     = "Accept-Ranges";
const uint8_t HTTP_HDR_CONTENT_TYPE[]      = "Content-Type";
const uint8_t HTTP_HDR_CONTENT_LENGTH[]    = "Content-Length";
const uint8_t HTTP_HDR_TRANSFER_ENCODING[] = "Transfer-Encoding";
const uint8_t HTTP_HEADER_END[] = "\r\n\r\n";
const uint8_t *html = "\
<!DOCTYPE html>\
<html>\
<head>\
  <title>Configuration</title>\
  <meta charset=\"UTF-8\">\
<style>\
fieldset { border: none; margin: 10px; }\
.col1 { width: 200px; }\
.legend { border-bottom: 1px solid black; padding: 2px 0px; width: 100%; font-weight: bold; text-transform: uppercase; }\
.center { text-align: center; }\
.err {color: red;}\
</style>\
<script>\
async function postAndProcess(url, data) {\
  const fetchOptions = {\
	  method: \"POST\",\
	  headers: {\
		  \"Content-Type\": \"application/json\",\
		},\
		body: data,\
	};\
  try {\
    const response = await fetch(url, fetchOptions);\
    if (!response.ok) {\
      throw new Error(`Response status: ${response.status}`);\
    }\
    const json = await response.json();\
    if(json.errors.length > 0) {\
      for (const error of json.errors) {\
        if( error.field.length === 0 ) {\
          document.getElementById( \"content\" ).innerHTML = \"<h1 style=\\\"background-color:Tomato;\\\">\" + error.message + \"</h1>\";\
          break;\
        }\
        document.getElementById( error.field+\"_e\" ).innerHTML = \"&larr;\" + error.message;\
      }\
    }\
    else {\
      document.getElementById( \"content\" ).innerHTML = \"<h1 style=\\\"background-color:LightGreen;\\\">Data saved successfully.</h1>\";\
    }\
  } catch (error) {\
    console.error(error.message);\
  }\
}\
window.addEventListener(\"DOMContentLoaded\", (event) => {\
  const f = document.getElementById(\"update-form\");\
  const collection = document.getElementsByClassName(\"err\");\
  if(f && collection) {\
    if (window.fetch) {\
      const url = f.getAttribute(\"action\");\
      if( url ) {\
        f.addEventListener(\"submit\", (event) => {\
          const formData = new FormData( f );\
          const data = JSON.stringify(Object.fromEntries(formData));\
          if( formData && data ) {\
            event.preventDefault();\
            for(const element of collection) {\
              element.innerHTML = \"\";\
            }\
            postAndProcess(url, data);\
          }\
          else {\
            console.error(\"formData od data are null\");\
          }\
        });\
      } else {\
        console.error(\"url is null\");\
      }\
    } else {\
      console.error(\"fetch() not supported\");\
    }\
  } else {\
    console.error(\"getElementById() returned null\");\
  }\
});\
</script>\
</head>\
\
<body>\
<div id=\"content\">\
<form action=\"http://192.168.4.1\" method=\"POST\" id=\"update-form\">\
<fieldset>\
  <div class=\"legend\"> Wi-Fi settings </div>\
  <table>\
    <tr>\
      <td class=\"col1\" align=\"right\"><label for=\"wifi_ssid\">* SSID:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"wifi_ssid\" id=\"wifi_ssid\" required minlength=\"4\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"wifi_ssid_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"wifi_pass\">* Password:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"wifi_pass\" id=\"wifi_pass\" required minlength=\"4\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"wifi_pass_e\"></td>\
    </tr>\
  </table>\
  <div class=\"legend\">Broker settings</div>\
  <table>\
    <tr>\
      <td class=\"col1\" align=\"right\"><label for=\"br_host\">* Host Name:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"br_host\" id=\"br_host\" required minlength=\"4\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"br_host_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"br_port\">* Port:</label></td>\
      <td align=\"left\"><input type=\"number\" name=\"br_port\" id=\"br_port\" required min=\"0\" max=\"65535\"></td>\
      <td class=\"err\" id=\"br_port_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"br_userid\">User ID:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"br_userid\" id=\"br_userid\" minlength=\"4\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"br_userid_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"br_username\">User Name:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"br_username\" id=\"br_username\" minlength=\"0\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"br_username_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"br_pass\">Password:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"br_pass\" id=\"br_pass\" minlength=\"0\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"br_pass_e\"></td>\
    </tr>\
  </table>\
  <div class=\"legend\">HA Settings</div>\
  <table>\
    <tr>\
      <td class=\"col1\" align=\"right\"><label for=\"ha_base_t\">* Base topic:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_base_t\" id=\"ha_base_t\" required value=\"homeassistant\" minlength=\"1\" maxlength=\"64\"></td>\
      <td class=\"err\" id=\"ha_base_t_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_node_id\">Node ID:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_node_id\" id=\"ha_node_id\" value=\"\" minlength=\"1\" maxlength=\"32\"></td>\
      <td class=\"err\" id=\"ha_node_id_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_cmd_t\">* Command topic:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_cmd_t\" id=\"ha_cmd_t\" required value=\"set\" minlength=\"1\" maxlength=\"32\"></td>\
      <td class=\"err\" id=\"ha_cmd_t_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_stat_t\">* State topic:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_stat_t\" id=\"ha_stat_t\" required value=\"state\" minlength=\"1\" maxlength=\"32\"></td>\
      <td class=\"err\" id=\"ha_stat_t_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_avty_t\">* Availability topic:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_avty_t\" id=\"ha_avty_t\" required value=\"available\" minlength=\"1\" maxlength=\"32\"></td>\
      <td class=\"err\" id=\"ha_avty_t_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_pl_on\">* Payload on:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_pl_on\" id=\"ha_pl_on\" required value=\"ON\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_pl_on_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_pl_off\">* Payload off:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_pl_off\" id=\"ha_pl_off\" required value=\"OFF\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_pl_off_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_pl_avail\">* Payload available:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_pl_avail\" id=\"ha_pl_avail\" required value=\"online\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_pl_avail_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_pl_not_avail\">* Payload not available:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_pl_not_avail\" id=\"ha_pl_not_avail\" required value=\"offline\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_pl_not_avail_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_stat_on\">* State on:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_stat_on\" id=\"ha_stat_on\" required value=\"ON\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_stat_on_e\"></td>\
    </tr>\
    <tr>\
      <td align=\"right\"><label for=\"ha_stat_off\">* State off:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"ha_stat_off\" id=\"ha_stat_off\" required value=\"OFF\" minlength=\"1\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"ha_stat_off_e\"></td>\
    </tr>\
  </table>\
  <div class=\"legend\">Update authorization</div>\
  <table>\
    <tr>\
      <td class=\"col1\" align=\"right\"><label for=\"dev_id\">* Device ID:</label></td>\
      <td align=\"left\"><input type=\"text\" name=\"dev_id\" id=\"dev_id\" required  minlength=\"4\" maxlength=\"16\"></td>\
      <td class=\"err\" id=\"dev_id_e\"></td>\
    </tr>\
  </table>\
</fieldset>\
<div class=\"center\">\
  <input text-align=\"center\" type=\"submit\" value=\"Update\">\
</div>\
</form>\
</div>\
</body>\
</html>";

const uint8_t EMPTY[]             = "";
const uint8_t WIFI_SSID[]         = "wifi_ssid";
const uint8_t WIFI_PASS[]         = "wifi_pass";
const uint8_t BR_HOST[]           = "br_host";
const uint8_t BR_PORT[]           = "br_port";
const uint8_t BR_USERID[]         = "br_userid";
const uint8_t BR_USERNAME[]       = "br_username";
const uint8_t BR_PASS[]           = "br_pass";
const uint8_t HA_BASE_T[]         = "ha_base_t";
const uint8_t HA_NODE_ID[]        = "ha_node_id";
const uint8_t HA_CMD_T[]          = "ha_cmd_t";
const uint8_t HA_STAT_T[]         = "ha_stat_t";
const uint8_t HA_AVTY_T[]         = "ha_avty_t";
const uint8_t HA_PL_ON[]          = "ha_pl_on";
const uint8_t HA_PL_OFF[]         = "ha_pl_off";
const uint8_t HA_PL_AVAIL[]       = "ha_pl_avail";
const uint8_t HA_PL_NOT_AVAIL[]   = "ha_pl_not_avail";
const uint8_t HA_STAT_ON[]        = "ha_stat_on";
const uint8_t HA_STAT_OFF[]       = "ha_stat_off";
const uint8_t DEV_ID[]            = "dev_id";
const uint8_t E_VALUE_INVALID[]   = "Value is invalid";
const uint8_t E_LENGTH_INVALID[]  = "Length is invalid";
const uint8_t E_MEMORY[]          = "Memory failure";

/** Accepted json string */
typedef enum { 
  ID_EMPTY = 0, 
  ID_WIFI_SSID, 
  ID_WIFI_PASS,
  ID_BR_HOST,
  ID_BR_PORT,
  ID_BR_USERID,
  ID_BR_USERNAME,
  ID_BR_PASS,
  ID_HA_BASE_T,
  ID_HA_NODE_ID,
  ID_HA_CMD_T,
  ID_HA_STAT_T,
  ID_HA_AVTY_T,
  ID_HA_PL_ON,
  ID_HA_PL_OFF,
  ID_HA_PL_AVAIL,
  ID_HA_PL_NOT_AVAIL,
  ID_HA_STAT_ON,
  ID_HA_STAT_OFF,
  ID_DEV_ID,
  ID_E_VALUE_INVALID,
  ID_E_LENGTH_INVALID,
  ID_E_MEMORY
} string_ids;

const uint8_t *STRINGS[] = {
  EMPTY,
  WIFI_SSID,
  WIFI_PASS,
  BR_HOST,
  BR_PORT,
  BR_USERID,
  BR_USERNAME,
  BR_PASS,
  HA_BASE_T,
  HA_NODE_ID,
  HA_CMD_T,
  HA_STAT_T,
  HA_AVTY_T,
  HA_PL_ON,
  HA_PL_OFF,
  HA_PL_AVAIL,
  HA_PL_NOT_AVAIL,
  HA_STAT_ON,
  HA_STAT_OFF,
  DEV_ID,
  E_VALUE_INVALID,
  E_LENGTH_INVALID,
  E_MEMORY
};

const uint16_t NUMBER_OF_FIELDS = ID_DEV_ID;

uint8_t *send_buffer;
size_t   send_buffer_offset;
size_t   send_buffer_len;
os_timer_t server_restart_timer;

static void ICACHE_FLASH_ATTR server_restart_cb(void *arg) {
  system_restart();
}

static uint16_t ICACHE_FLASH_ATTR parse_config_json(const char* buf, const size_t buf_len, uint8_t *error_list) {
  extern struct user_cfg cfg;
  size_t offset = 0, value_offset, value_len, length;
  string_ids string;

  TOLOG(LOG_DEBUG, "\r\nparse_config_json()\r\n");

  if(4 > buf_len || NULL == buf) {
    return FUN_E_ARGS;
  }

  /* json: '{' */
  if(buf[offset] != '{') {
    return FUN_E_ARGS;
  }
  ++offset;
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

json_param:

  /* json: '"' */
  while(buf[offset] == ' ' && buf_len > offset) {
    ++offset;
  }
  if(buf[offset] != '"') {
    return FUN_E_ARGS;
  }
  ++offset;
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  /* json: string */
  string = ID_EMPTY;
  if( !os_memcmp( &buf[offset], WIFI_SSID, ARRAYLEN(WIFI_SSID)-1 ) ) {
    string = ID_WIFI_SSID;
    offset += sizeof(WIFI_SSID) - 1;
  }
  else if( !os_memcmp( &buf[offset], WIFI_PASS, ARRAYLEN(WIFI_PASS)-1 ) ) {
    string = ID_WIFI_PASS;
    offset += sizeof(WIFI_PASS) - 1;
  }
  else if( !os_memcmp( &buf[offset], BR_HOST, ARRAYLEN(BR_HOST)-1 ) ) {
    string = ID_BR_HOST;
    offset += sizeof(BR_HOST) - 1;
  }
  else if( !os_memcmp( &buf[offset], BR_PORT, ARRAYLEN(BR_PORT)-1 ) ) {
    string = ID_BR_PORT;
    offset += sizeof(BR_PORT) - 1;
  }
  else if( !os_memcmp( &buf[offset], BR_USERID, ARRAYLEN(BR_USERID)-1 ) ) {
    string = ID_BR_USERID;
    offset += sizeof(BR_USERID) - 1;
  }
  else if( !os_memcmp( &buf[offset], BR_USERNAME, ARRAYLEN(BR_USERNAME)-1 ) ) {
    string = ID_BR_USERNAME;
    offset += sizeof(BR_USERNAME) - 1;
  }
  else if( !os_memcmp( &buf[offset], BR_PASS, ARRAYLEN(BR_PASS)-1 ) ) {
    string = ID_BR_PASS;
    offset += sizeof(BR_PASS) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_BASE_T, ARRAYLEN(HA_BASE_T)-1 ) ) {
    string = ID_HA_BASE_T;
    offset += sizeof(HA_BASE_T) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_NODE_ID, ARRAYLEN(HA_NODE_ID)-1 ) ) {
    string = ID_HA_NODE_ID;
    offset += sizeof(HA_NODE_ID) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_CMD_T, ARRAYLEN(HA_CMD_T)-1 ) ) {
    string = ID_HA_CMD_T;
    offset += sizeof(HA_CMD_T) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_STAT_T, ARRAYLEN(HA_STAT_T)-1 ) ) {
    string = ID_HA_STAT_T;
    offset += sizeof(HA_STAT_T) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_AVTY_T, ARRAYLEN(HA_AVTY_T)-1 ) ) {
    string = ID_HA_AVTY_T;
    offset += sizeof(HA_AVTY_T) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_PL_ON, ARRAYLEN(HA_PL_ON)-1 ) ) {
    string = ID_HA_PL_ON;
    offset += sizeof(HA_PL_ON) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_PL_OFF, ARRAYLEN(HA_PL_OFF)-1 ) ) {
    string = ID_HA_PL_OFF;
    offset += sizeof(HA_PL_OFF) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_PL_AVAIL, ARRAYLEN(HA_PL_AVAIL)-1 ) ) {
    string = ID_HA_PL_AVAIL;
    offset += sizeof(HA_PL_AVAIL) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_PL_NOT_AVAIL, ARRAYLEN(HA_PL_NOT_AVAIL)-1 ) ) {
    string = ID_HA_PL_NOT_AVAIL;
    offset += sizeof(HA_PL_NOT_AVAIL) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_STAT_ON, ARRAYLEN(HA_STAT_ON)-1 ) ) {
    string = ID_HA_STAT_ON;
    offset += sizeof(HA_STAT_ON) - 1;
  }
  else if( !os_memcmp( &buf[offset], HA_STAT_OFF, ARRAYLEN(HA_STAT_OFF)-1 ) ) {
    string = ID_HA_STAT_OFF;
    offset += sizeof(HA_STAT_OFF) - 1;
  }
  else if( !os_memcmp( &buf[offset], DEV_ID, ARRAYLEN(DEV_ID)-1 ) ) {
    string = ID_DEV_ID;
    offset += sizeof(DEV_ID) - 1;
  }
  else {
    return FUN_E_ARGS;
  }
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  /* json: '"' */
  if(buf[offset] != '"') {
    return FUN_E_ARGS;
  }
  ++offset;
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  /* json: ':' */
  while(buf[offset] == ' ' && buf_len > offset) {
    ++offset;
  }
  if(buf[offset] != ':') {
    return FUN_E_ARGS;
  }
  ++offset;  
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  /* json: '"' */
  while(buf[offset] == ' ' && buf_len > offset) {
    ++offset;
  }
  if(buf[offset] != '"') {
    return FUN_E_ARGS;
  }
  ++offset;
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  /* json: value */
  value_offset = offset;
  value_len = 0;
  while(buf[offset] != '"' && buf_len > offset) {
    ++offset;
    ++value_len;
  }
  ++offset;
  if(buf_len < offset) {
    return FUN_E_ARGS;
  }

  switch(string) {
    case ID_WIFI_SSID:
      if(value_len > ARRAYLEN(cfg.wifi_ssid)) {
        error_list[ID_WIFI_SSID] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.wifi_ssid[0], &buf[value_offset], value_len);
      cfg.wifi_ssid_len = value_len;
      break;
    case ID_WIFI_PASS:
      if(value_len > ARRAYLEN(cfg.wifi_pass)) {
        error_list[ID_WIFI_PASS] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.wifi_pass[0], &buf[value_offset], value_len);
      cfg.wifi_pass_len = value_len;
      break;
    case ID_BR_HOST:
      if(value_len > ARRAYLEN(cfg.br_host)) {
        error_list[ID_BR_HOST] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.br_host[0], &buf[value_offset], value_len);
      cfg.br_host_len = value_len;
      break;
    case ID_BR_PORT:
      if(value_len > sizeof(cfg.br_port)) {
        error_list[ID_BR_PORT] = ID_E_LENGTH_INVALID;
        break;
      }
      cfg.br_port = 0;
      while( ISDIGIT( buf[value_offset] ) && buf_len > value_offset ) {
        cfg.br_port = cfg.br_port * 10 + (uint32_t) ( buf[value_offset++] - 0x30 );
      }
      break;
    case ID_BR_USERID:
      if(value_len > ARRAYLEN(cfg.br_userid)) {
        error_list[ID_BR_USERID] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.br_userid[0], &buf[value_offset], value_len);
      cfg.br_userid_len = value_len;
      break;
    case ID_BR_USERNAME:
      if(value_len > ARRAYLEN(cfg.br_username)) {
        error_list[ID_BR_USERNAME] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.br_username[0], &buf[value_offset], value_len);
      cfg.br_username_len = value_len;
      break;
    case ID_BR_PASS:
      if(value_len > ARRAYLEN(cfg.br_pass)) {
        error_list[ID_BR_PASS] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.br_pass[0], &buf[value_offset], value_len);
      cfg.br_pass_len = value_len;
      break;
    case ID_HA_BASE_T:
      if(value_len > ARRAYLEN(cfg.ha_base_t)) {
        error_list[ID_HA_BASE_T] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_base_t[0], &buf[value_offset], value_len);
      cfg.ha_base_t_len = value_len;
      break;
    case ID_HA_NODE_ID:
      if(value_len > ARRAYLEN(cfg.ha_node_id)) {
        error_list[ID_HA_NODE_ID] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_node_id[0], &buf[value_offset], value_len);
      cfg.ha_node_id_len = value_len;
      break;
    case ID_HA_CMD_T:
      if(value_len > ARRAYLEN(cfg.ha_cmd_t)) {
        error_list[ID_HA_CMD_T] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_cmd_t[0], &buf[value_offset], value_len);
      cfg.ha_cmd_t_len = value_len;
      break;
    case ID_HA_STAT_T:
      if(value_len > ARRAYLEN(cfg.ha_stat_t)) {
        error_list[ID_HA_STAT_T] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_stat_t[0], &buf[value_offset], value_len);
      cfg.ha_stat_t_len = value_len;
      break;
    case ID_HA_AVTY_T:
      if(value_len > ARRAYLEN(cfg.ha_avty_t)) {
        error_list[ID_HA_AVTY_T] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_avty_t[0], &buf[value_offset], value_len);
      cfg.ha_avty_t_len = value_len;
      break;
    case ID_HA_PL_ON:
      if(value_len > ARRAYLEN(cfg.ha_pl_on)) {
        error_list[ID_HA_PL_ON] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_pl_on[0], &buf[value_offset], value_len);
      cfg.ha_pl_on_len = value_len;
      break;
    case ID_HA_PL_OFF:
      if(value_len > ARRAYLEN(cfg.ha_pl_off)) {
        error_list[ID_HA_PL_OFF] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_pl_off[0], &buf[value_offset], value_len);
      cfg.ha_pl_off_len = value_len;
      break;
    case ID_HA_PL_AVAIL:
      if(value_len > ARRAYLEN(cfg.ha_pl_avail)) {
        error_list[ID_HA_PL_AVAIL] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_pl_avail[0], &buf[value_offset], value_len);
      cfg.ha_pl_avail_len = value_len;
      break;
    case ID_HA_PL_NOT_AVAIL:
      if(value_len > ARRAYLEN(cfg.ha_pl_not_avail)) {
        error_list[ID_HA_PL_NOT_AVAIL] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_pl_not_avail[0], &buf[value_offset], value_len);
      cfg.ha_pl_not_avail_len = value_len;
      break;
    case ID_HA_STAT_ON:
      if(value_len > ARRAYLEN(cfg.ha_stat_on)) {
        error_list[ID_HA_STAT_ON] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_stat_on[0], &buf[value_offset], value_len);
      cfg.ha_stat_on_len = value_len;
      break;
    case ID_HA_STAT_OFF:
      if(value_len > ARRAYLEN(cfg.ha_stat_off)) {
        error_list[ID_HA_STAT_OFF] = ID_E_LENGTH_INVALID;
        break;
      }
      os_memcpy(&cfg.ha_stat_off[0], &buf[value_offset], value_len);
      cfg.ha_stat_off_len = value_len;
      break;
    case ID_DEV_ID:
      if(value_len > ARRAYLEN(cfg.dev_id)) {
        error_list[ID_DEV_ID] = ID_E_LENGTH_INVALID;
        break;
      }
      if( os_memcmp( &buf[value_offset], &cfg.dev_id[0], cfg.dev_id_len ) ) {
        error_list[ID_DEV_ID] = ID_E_VALUE_INVALID;
        break;
      }
      break;
    default:
      return FUN_E_ARGS;
  }

  /* json: '}' */
  while(buf[offset] == ' ' && buf_len > offset) {
    ++offset;
  }
  if(buf[offset] == '}') {
    return FUN_OK;
  }
  else if(buf[offset] == ',') {
    ++offset;
    if(buf_len < offset) {
      return FUN_E_ARGS;
    }
    goto json_param;
  }

  return FUN_E_ARGS;
}

void ICACHE_FLASH_ATTR server_sent_cb(void *arg) {
  extern uint8_t big_buffer[1024];
  extern const size_t big_buffer_len;
  uint16_t chunk_hdr_len, copy_len, send_len;
  size_t div = (size_t) (send_buffer_len / big_buffer_len);
  uint8_t *ptr = NULL;

  TOLOG(LOG_DEBUG, "server_sent_cb()\r\n");

  if( div >= 1 ) {
    ptr = (uint8_t*) &big_buffer[0];
    chunk_hdr_len = (uint16_t) os_sprintf(ptr, "%X\r\n", big_buffer_len);
    chunk_hdr_len = (uint16_t) os_sprintf(ptr, "%X\r\n", big_buffer_len - chunk_hdr_len - 2);
    copy_len = (uint16_t) (big_buffer_len - chunk_hdr_len - 2);
    os_memcpy( ptr+chunk_hdr_len, send_buffer+send_buffer_offset, copy_len );
    send_buffer_offset += (size_t) copy_len;
    send_buffer_len -= (size_t) copy_len;
    send_len = (uint16_t) big_buffer_len;
    ptr[send_len-2] = '\r';
    ptr[send_len-1] = '\n';
  }
  else if( send_buffer_len > 0 ) {
    ptr = (uint8_t*) &big_buffer[0];
    chunk_hdr_len = (uint16_t) os_sprintf(ptr, "%X\r\n", send_buffer_len);
    div = (size_t) ((size_t) ((size_t)chunk_hdr_len + (size_t)send_buffer_len + (size_t)2 ) / (size_t) big_buffer_len);
    if( div >= 1 ) {
      ptr = (uint8_t*) &big_buffer[0];
      chunk_hdr_len = (uint16_t) os_sprintf(ptr, "%X\r\n", big_buffer_len);
      chunk_hdr_len = (uint16_t) os_sprintf(ptr, "%X\r\n", big_buffer_len - chunk_hdr_len - 2);
      copy_len = (uint16_t) (big_buffer_len - chunk_hdr_len - 2);
      os_memcpy( ptr+chunk_hdr_len, send_buffer+send_buffer_offset, copy_len );
      send_buffer_offset += (size_t) copy_len;
      send_buffer_len -= (size_t) copy_len;
      send_len = (uint16_t) big_buffer_len;
      ptr[send_len-2] = '\r';
      ptr[send_len-1] = '\n';
    }
    else {
      os_memcpy( ptr+chunk_hdr_len, send_buffer+send_buffer_offset, send_buffer_len );
      send_buffer_offset += send_buffer_len;
      send_len = (uint16_t) (chunk_hdr_len + (uint16_t) send_buffer_len + (uint16_t) 2);    
      ptr[send_len-2] = '\r';
      ptr[send_len-1] = '\n';
      send_buffer_len = 0;
    }
  }
  else if( send_buffer_offset > 0) {
    send_buffer_offset = 0;
    ptr = (uint8_t*) &big_buffer[0];
    ptr[0] = '0';
    ptr[1] = '\r';
    ptr[2] = '\n';
    ptr[3] = '\r';
    ptr[4] = '\n';
    send_len = (uint16_t) 5;
  }

  if(send_len && ptr) {
    if( espconn_send( (struct espconn*) arg, (uint8*) big_buffer, send_len)) {
      TOLOG(LOG_ERR, "espconn_send() failed\r\n");
    }
    TOLOG(LOG_DEBUG, "Send successfully\r\n");
  }
}

void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *pdata, unsigned short len) {
  extern uint8_t big_buffer[1024];
  extern const size_t big_buffer_len;
  extern struct user_cfg cfg;
  uint8_t error_list[NUMBER_OF_FIELDS + 1];
  uint8_t *ptr = NULL, save_cfg = 0x00;
  uint16_t rc, i;
  size_t offset, length, error_counter, content_len, content_off;
  uint8_t log_text[36] = {0};

  send_buffer_len = send_buffer_offset = 0;

  TOLOG(LOG_DEBUG, "Received:\r\n");
  TOLOG(LOG_DEBUG, pdata);

  if( 0 == os_memcmp( pdata, "GET", 3 )) {
    ptr = pdata;
    offset = 3;
    // skip whitespaces
    while( ptr[offset] == ' ' && offset < len ) {
      ++offset;
    }
    // check /
    if( ptr[offset] != '/') {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;
    }
    ++offset;
    if( ptr[offset] != ' ' ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_404);
      goto finish;      
    }
    ++offset;
    if( os_memcmp( &ptr[offset], HTTP_VERSION, sizeof(HTTP_VERSION)-1 ) ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_505);
      goto finish;     
    }
    ptr = &big_buffer[0];
    len =  os_sprintf(ptr      , "%s %s\r\n", HTTP_VERSION, HTTP_SC_200);
    len += os_sprintf(ptr + len, "%s: %s\r\n", HTTP_HDR_SERVER, SERVER_NAME);
    len += os_sprintf(ptr + len, "%s: bytes\r\n", HTTP_HDR_ACCEPT_RANGES);
    len += os_sprintf(ptr + len, "%s: text/html\r\n", HTTP_HDR_CONTENT_TYPE);
    len += os_sprintf(ptr + len, "%s: chunked\r\n", HTTP_HDR_TRANSFER_ENCODING);
    len += os_sprintf(ptr + len, "\r\n");

    send_buffer = (uint8_t*) &html[0];
    send_buffer_len = (size_t) os_strlen(html);
    send_buffer_offset = 0;
    goto finish;
  }
  else if( 0 == os_memcmp( pdata, "POST", 4 )) {
    ptr = pdata;
    offset = 4;
    // skip whitespaces
    while( ptr[offset] == ' ' && offset < len ) {
      ++offset;
    }
    // check /
    if( ptr[offset] != '/') {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;
    }
    ++offset;
    if( ptr[offset] != ' ' ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_404);
      goto finish;      
    }
    ++offset;
    if( os_memcmp( &ptr[offset], HTTP_VERSION, sizeof(HTTP_VERSION)-1 ) ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_505);
      goto finish;     
    }
    // Searching for "Content-Length:"
    while(offset < len) {
      if( 0 == os_memcmp( ptr+offset, HTTP_HDR_CONTENT_LENGTH, sizeof(HTTP_HDR_CONTENT_LENGTH)-1 ) ) {
        break;
      }
      ++offset;
    }
    if( offset >= len ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;     
    }
    // Skip the "Content-Length:"
    offset += sizeof(HTTP_HDR_CONTENT_LENGTH) + 1;
    // Skip empty characters
    while(ptr[offset] == ' ' && len > offset) {
      ++offset;
    }
    if( offset >= len ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;     
    }
    // Convert string to int
    length = 0;
    while( ISDIGIT( ptr[offset] ) && len > offset ) {
      length = length * 10 + ( ptr[offset++] - 0x30 );
    }
    if( offset >= len || length == 0 ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;     
    }
    // Searching for "\r\n\r\n"
    while(offset < len) {
      if( 0 == os_memcmp( &ptr[offset], HTTP_HEADER_END, sizeof(HTTP_HEADER_END)-1 ) ) {
        break;
      }
      ++offset;
    }
    if( offset >= len ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s\r\n", HTTP_VERSION, HTTP_SC_400);
      goto finish;     
    }
    offset += 4;
    // Clear data
    os_memset(error_list, 0x00, ARRAYLEN(error_list));
    // Parse the content
    rc = parse_config_json( &ptr[offset], length, error_list);
    if( FUN_OK != rc ) {
      ptr = &big_buffer[0];
      len =  os_sprintf(ptr, "%s %s [%d]\r\n", HTTP_VERSION, HTTP_SC_500, rc);
      goto finish;         
    }
    // Prepare response content:
    content_off = 512;
    error_counter = 0;
    os_memset( &big_buffer[0], 0x00, big_buffer_len );
    ptr = &big_buffer[ content_off ];
    content_len = os_sprintf(ptr, "{\"errors\":[");
    for(i=1; i <= NUMBER_OF_FIELDS; ++i) {
      if( 0x00 != error_list[i] ) {
        content_len +=  os_sprintf(ptr + content_len, "{\"field\": \"%s\", \"message\": \"%s\"},", STRINGS[i], STRINGS[error_list[i]] );
        ++error_counter;
      }
    }
    if(error_counter > 0) {
      /* Remove last ,*/
      content_len -= 1;
    }
    content_len +=  os_sprintf(ptr + content_len, "]}");
    if( 0 == error_counter ) {
      // ctx could be saved now
      save_cfg = 0x01;
    }
    ptr = &big_buffer[0];
    len =  os_sprintf(ptr      , "%s %s\r\n", HTTP_VERSION, HTTP_SC_200);
    len += os_sprintf(ptr + len, "%s: %s\r\n", HTTP_HDR_SERVER, SERVER_NAME);
    len += os_sprintf(ptr + len, "%s: bytes\r\n", HTTP_HDR_ACCEPT_RANGES);
    len += os_sprintf(ptr + len, "%s: application/json\r\n", HTTP_HDR_CONTENT_TYPE);
    len += os_sprintf(ptr + len, "%s: %d\r\n", HTTP_HDR_CONTENT_LENGTH, content_len);
    len += os_sprintf(ptr + len, "\r\n");
    len += os_sprintf(ptr + len, "%s\r\n", &big_buffer[ content_off ]);
    goto finish;
  }

finish:
  if( 0x01 == save_cfg ) {
    save_cfg = 0x00;
    cfg.dev_mode = MODE_OPE;
    if( FUN_OK != cfg_save()) {
      TOLOG(LOG_DEBUG, "\r\nConfig saved failed\r\n");
      ptr = &big_buffer[0];
      len = os_sprintf(ptr, "{\"errors\":[ {\"field\":\"\", \"message\":\"%s\"} ]}", STRINGS[ ID_E_MEMORY ]);
    }
    else {
      TOLOG(LOG_DEBUG, "\r\nConfig saved\r\n");
      /* Initialize restart timer once again */
      os_timer_setfn(&server_restart_timer, (os_timer_func_t *)server_restart_cb, NULL);
      os_timer_arm(&server_restart_timer, DELAY_5_SEC, 0);
    }
  }
  if(len && ptr) {
    if( espconn_send( (struct espconn*) arg, (uint8*) big_buffer, len)) {
      TOLOG(LOG_ERR, "espconn_send() failed\r\n");
    }
    ptr[len] = 0;
    TOLOG(LOG_DEBUG, big_buffer);
  }
}