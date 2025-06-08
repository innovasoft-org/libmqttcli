#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>	/* close */
#include <netdb.h> /* gethostbyname */
#include <sys/ioctl.h> /* ioctl */
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h> /* va_start, va_arg_, va_end */
#include <time.h> /* time() */
#include <signal.h> /* sigaction() ... */
#include <pthread.h>
#include <stdio.h>

#include "main.h"
#include "utils.h"
#include "utils.h"
#include "../../api/mqtt_cli.h"

#define STATE_STOPPED     ( (uint8_t) 0 )
#define STATE_STARTED     ( (uint8_t) 1 )
#define STATE_DISCOVERY   ( (uint8_t) 1 )
#define STATE_SYNCHRO     ( (uint8_t) 2 )
#define STATE_OPERATIONAL ( (uint8_t) 3 )

const char* status_topic = "homeassistant/status";
const char* base_topic = "homeassistant/switch";
const char* command_topic = "set";
const char* state_topic = "state";
const char* availability_topic = "available";
const char* payload_on = "ON";
const char* payload_off = "OFF";
const char* payload_available = "online";
const char* payload_not_available = "offline";
const char* state_on = "ON";
const char* state_off = "OFF";

/** Program context */
static context_t ctx;

/* Stores current switch state (on or off) */
static uint8_t toggle = 0;

/** Stores timer interval raised */
static int timer_int;

/** Stores program state in a separate variable (separate from ctx) */
static uint8_t state = STATE_STOPPED;

/** Stores program mutex */
static pthread_mutex_t mutex;

static struct option long_options[] = {
  {L_OPT_BUFFER_SIZE, required_argument,  0,  S_OPT_BUFFER_SIZE},
  {L_OPT_HOST,        required_argument,  0,  S_OPT_HOST},
  {L_OPT_MQTT_VERSION,required_argument,  0,  S_OPT_MQTT_VERSION},
  {L_OPT_PASSWORD,    required_argument,  0,  S_OPT_PASSWORD},
  {L_OPT_PORT,        required_argument,  0,  S_OPT_PORT},
  {L_OPT_REUSE_ADDR,  required_argument,  0,  S_OPT_REUSE_ADDR},
  {L_OPT_UNIQUE_ID,   required_argument,  0,  S_OPT_UNIQUE_ID},
  {L_OPT_USERID,      required_argument,  0,  S_OPT_USERID},
  {L_OPT_USERNAME,    required_argument,  0,  S_OPT_USERNAME},
  {L_OPT_VERBOSE,     no_argument,        0,  S_OPT_VERBOSE},
  {NULL,              no_argument,        0,  0}
};

/**
 * @brief Parse the command line arguments and set some global flags.
 * @param argc Number of arguments passed to program
 * @param argv Values of arguments
 */
int validate_args(int argc, char **argv) {
	int idx, substr_len, c;
  char* substr = NULL;
  size_t length;

	/* Set default values */
  memset( &ctx, 0x00, sizeof(context_t) );
  length = sizeof(DEFAULT_IP) / sizeof(char);
  memcpy(ctx.ip, DEFAULT_IP, length);
  ctx.ip[length] = '\0';
	ctx.port = DEFAULT_PORT;
  ctx.optval_reuse_addr = 0x00;
  ctx.buffer_size = DEFAULT_BUFFER_SIZE;
  ctx.non_blocking = 1;
  ctx.mqtt_version = 5;

  if(NULL == strstr(argv[0], PROGRAM_NAME)) {
    TOLOG(LOG_ERR,"Program name was changed to %s",argv[0]);
    return RESULT_FAILURE;
  }

  while( 1 ) {
    c = getopt_long( argc, argv,"vh:p:b:t:m:I:N:P:", long_options, &idx );
    /* Detect the end of the options */
    if( c == -1) {
      break;
    }
    switch(c) {
      case S_OPT_BUFFER_SIZE:
        ctx.buffer_size = atoi( optarg );
        break;
      case S_OPT_HOST:
        length = strlen( optarg );
        if(length > sizeof(ctx.ip) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid IP format");
        }
        memcpy(ctx.ip, optarg, length);
        break;
      case S_OPT_MQTT_VERSION:
        ctx.mqtt_version = atoi( optarg );
        break;
      case S_OPT_PASSWORD:
        length = strlen( optarg );
        if(length > sizeof(ctx.password) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid 'password' length");
        }
        memcpy(ctx.password, optarg, length);        
        break;
      case S_OPT_PORT:
        ctx.port = atoi( optarg );
        break;
      case S_OPT_REUSE_ADDR:
        ctx.optval_reuse_addr = 1;
        break;
      case S_OPT_UNIQUE_ID:
        length = strlen( optarg );
        if(length > (sizeof(ctx.uniqueid) / sizeof(char) - 1) ) {
          TOLOG(LOG_ERR, "Invalid 'unique-id' length");
        }
        memcpy(ctx.uniqueid, optarg, length);        
        break;
      case S_OPT_USERID:
        length = strlen( optarg );
        if(length > sizeof(ctx.userid) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid 'userid' length");
        }
        memcpy(ctx.userid, optarg, length);        
        break;
      case S_OPT_USERNAME:
        length = strlen( optarg );
        if(length > sizeof(ctx.username) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid 'username' length");
        }
        memcpy(ctx.username, optarg, length);        
        break;
      case S_OPT_VERBOSE:
        ctx.verbose = 1;
        ctx.log_fd = stdout;
        ctx.log_max_level = LOG_INFO;
        break;
      case '?':
        return RESULT_FAILURE;
        break;
      default:
        exit(1);
    }
  }

  if( ctx.username[0] == 0x00 ) {
    TOLOG(LOG_ERR, "'username' option required");
    return RESULT_FAILURE;
  }

  if( ctx.password[0] == 0x00 ) {
    TOLOG(LOG_ERR, "'password' option required");
    return RESULT_FAILURE;
  }

  if( ctx.uniqueid[0] == 0x00 ) {
    TOLOG(LOG_ERR, "'unique' option required");
    return RESULT_FAILURE;
  }

	return RESULT_OK;
}

/**
 * @brief Handler for the system signals
 * @param sig Signal value
 */
void signal_handler(int sig)
{
	switch(sig)
	{
		case SIGALRM:
      pthread_mutex_lock(&mutex);
			timer_int = 1;
      pthread_mutex_unlock(&mutex);
			break;
		case SIGINT:
		case SIGTERM:
      pthread_mutex_lock(&mutex);
			state = STATE_STOPPED;
      pthread_mutex_unlock(&mutex);
			break;
		case SIGHUP:
			TOLOG(LOG_WARNING,"Received SIGHUP signal.");
			break;
		default:
			TOLOG(LOG_WARNING,"Unhandled signal %s", strsignal(sig));
			break;
	}
}

/**
 * @brief Prints available options for the program
 */
void usage(const char* program) {
  printf("NAME\r\n");
	printf("       %s\r\n", program);
  printf("SYNOPSIS\r\n");
  printf("       %s %s", program, "[options]\r\n");
  printf("OPTIONS\r\n");
  printf(" -%c <size>, --%s <size>\r\n\t%s\r\n",           S_OPT_BUFFER_SIZE, L_OPT_BUFFER_SIZE,   "Sets buffer size, which is dynamically allocated.");
  printf(" -%c <host_name>, --%s <host_name>\r\n\t%s\r\n", S_OPT_HOST,        L_OPT_HOST,          "Sets remote host name or IP address.");
  printf(" -%c <password>, --%s <password>\r\n\t%s\r\n",   S_OPT_PASSWORD,    L_OPT_PASSWORD,      "Sets password used during CONNECT packet creation.");
  printf(" -%c <port>, --%s <port>\r\n\t%s\r\n",           S_OPT_PORT,        L_OPT_PORT,          "Sets the remote port to be used.");
  printf(" --%s\r\n\t%s\r\n",                                                 L_OPT_REUSE_ADDR,    "Turns on to reuse the the address.");
  printf(" --%s <unique_id>\r\n\t%s\r\n",                                     L_OPT_UNIQUE_ID,     "Sets device's unique id");
  printf(" -%c <user_id>, --%s <user_id>\r\n\t%s\r\n",     S_OPT_USERID,      L_OPT_USERID,        "Sets user_id used during CONNECT packet creation. If not specified user_id is auto generated.");
  printf(" -%c <user_name>, --%s <user_name>\r\n\t%s\r\n", S_OPT_USERNAME,    L_OPT_USERNAME,      "Sets user_name used during CONNECT packet creation.");
  printf(" --%s <version>\r\n\t%s\r\n",                                       L_OPT_MQTT_VERSION,  "Sets MQTT protocol's version (4 or 5). Default: 5.");
  printf(" -%c, --%s\r\n\t%s\r\n",                         S_OPT_VERBOSE,     L_OPT_VERBOSE,       "Runs the program in verbose mode.");
	printf("\r\n");
}

void show_info() {
  if( !ctx.verbose ) {
    return;
  }

  printf("Home Assistant Device Simulator (c) 2024\r\n");
  printf("\r\n");
}

void log_write(int level, char* filename, int line, char *fmt,...) {
  static const char* const level_name[] = {"EMERG", "ALERT","CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG"};
  static char date[20];
  va_list         list;
  char            *p, *r;
  int             e;
  float           f;

  if(level > ctx.log_max_level || ctx.log_fd == NULL) {
    return;
  }

  if( level < LOG_WARNING ) {
    get_date(date,11);
    date[10] = ' ';
    get_time(date+11, 9);
    fprintf(ctx.log_fd,"%s ",date);
  }
  fprintf(ctx.log_fd,"%s ",level_name[level]);
  if( level < LOG_WARNING ) {
    fprintf(ctx.log_fd,"%s @ %d: ",filename,line);
  }
  va_start( list, fmt );

  for ( p = fmt ; *p ; ++p )
  {
    if ( *p != '%' ) /* If simple string */
    {
      /* Simple string */
      fputc( *p, ctx.log_fd );
    }
    else
    {
      /* Formatted string */
      switch ( *++p )
      {
        /* string */
        case 's':
        {
          r = va_arg( list, char * );
          fprintf(ctx.log_fd,"%s", r);
          continue;
        }
        /* integer */
        case 'd':
        {
          e = va_arg( list, int );
          fprintf(ctx.log_fd,"%d", e);
          continue;
        }
        case 'f':
        {
          f = va_arg( list, double );
          fprintf(ctx.log_fd,"%f", f);
          continue;
        }
        default:
          fputc( *p, ctx.log_fd );
      }
    }
  }

  va_end( list );
  fputc( '\r', ctx.log_fd );
  fputc( '\n', ctx.log_fd );
}

int send_data(int sock, uint8_t *buf, size_t *length, uint8_t *log_str, size_t log_str_len) {
  fd_set writefds;
  struct timeval tv;
  int result = RESULT_OK;
  size_t send_len;

  tv.tv_sec = 0;
	tv.tv_usec = 0;

  if(*length == 0) {
    return RESULT_OK;
  }

  /* Prepare the read and write socket sets for network I/O notification */
  FD_ZERO(&writefds);
  /* Set read and write notification for the socket */
  FD_SET(sock, &writefds);

  /* Wait until data could be send or timeout will raised */
  if( -1 == (result = select( sock+1, NULL, &writefds, NULL, &tv)) ) {
    if(EINTR == errno ) {
      return RESULT_EXIT;
    }
    else {
      TOLOG(LOG_ERR,"select( ... ), errno = %d", errno);
       return RESULT_FAILURE;
    }
  }
  else if(result == 0) {
    /* timeout */
    return RESULT_FAILURE;
  }

  /* Check if send could be performed */
  if (FD_ISSET(sock, &writefds)) {
    if( -1 == (send_len = send(sock, buf, *length, 0))) {
      if(errno == ECONNRESET) {
        TOLOG(LOG_INFO,"Connection reset");
        return RESULT_EXIT;
      }
      TOLOG(LOG_WARNING,"send( ... ), errno = %d", errno);
      return RESULT_FAILURE;
    }
    if(ctx.verbose) {
      memset(log_str, 0x00, log_str_len);
      format_data(buf, send_len, log_str, log_str_len);
      log_str[1] = '<';
      log_str[2] = '=';
      printf("%s\r\n", log_str);
    }
  }

  return RESULT_OK;
}

int process_and_send_data(int sock, mqtt_cli_t *cli, clv_t *data, mqtt_channel_t *channel, uint8_t *log_str, size_t log_str_len) {
  fd_set writefds;
  struct timeval tv;
  int result = RESULT_OK;
  uint16_t rc;
  size_t send_len;

  tv.tv_sec = 0;
	tv.tv_usec = 5000;

  /* Processing and sending */
  do {
    rc = cli->process( cli, data, channel);
    if(rc != MQTT_SUCCESS && rc != MQTT_PENDING_DATA) {
      TOLOG(LOG_ERR, "process( ... ), rc = %d", rc);
      result = RESULT_FAILURE;
      break;
    }

    if( data->length ) {
      if( (result = send_data(sock, data->value, &(data->length), log_str, log_str_len)) != RESULT_OK) {
        break;
      }
    }

    data->length = 0;
  } while( rc == MQTT_PENDING_DATA ); /* Processing and sending */

  return result;
}

mqtt_rc_t cb_connack(const mqtt_cli_ctx_cb_t *self, const mqtt_connack_t *pkt, const mqtt_channel_t *channel) {
  mqtt_rc_t rc = RC_SUCCESS;
  mqtt_subscribe_params_t subscribe_params = { };
  uint8_t *buffer = NULL;

  /* Resources allocation */
  if( NULL == (buffer = (unsigned char*) malloc (ctx.buffer_size ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    goto finish;
  }

  /* Subscribe to receive homeassistant status (it shall be configured in mosquito plugin) */
  subscribe_params.filter.value = buffer;
  subscribe_params.filter.length = sprintf( buffer, status_topic);
  if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

finish:
  /* Resources deallocation */
  if(NULL != buffer) {
    free( buffer );
    buffer = NULL;
  }
  return rc;
}

mqtt_rc_t cb_publish(const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel) {
  mqtt_rc_t rc = RC_SUCCESS;
  mqtt_publish_params_t publish_params = { };
  mqtt_subscribe_params_t subscribe_params = { };
  size_t offset;
  uint8_t *message, *buffer = NULL;

  /* Resources allocation */
  if( NULL == (buffer = (unsigned char*) malloc (ctx.buffer_size ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    goto finish;
  }

  /* Check if Home Assistant is online */
  if( (strlen(status_topic) == pkt->topic.length) && 
      (0 == memcmp(status_topic, pkt->topic.value, pkt->topic.length)) &&
      (strlen(payload_available) == pkt->message.length) &&
      (0 == memcmp(payload_available, pkt->message.value,pkt->message.length)) ) {
    /* Publishing configuration */
    publish_params.topic.value = buffer;
    publish_params.topic.length = sprintf( buffer, "%s/%s/config", base_topic, ctx.uniqueid );
    message = publish_params.message.value = buffer + publish_params.topic.length;
    offset = 0;
    message[0] = '{';
    offset += 1;
    offset += sprintf( message + offset, "\"~\": \"%s/%s\",", base_topic, ctx.uniqueid );
    offset += sprintf( message + offset, "\"device_class\": \"switch\",");
    offset += sprintf( message + offset, "\"name\": null,");
    offset += sprintf( message + offset, "\"uniq_id\": \"%s\",", ctx.uniqueid);
    offset += sprintf( message + offset, "\"cmd_t\": \"~/%s\",", command_topic);
    offset += sprintf( message + offset, "\"stat_t\": \"~/%s\",", state_topic);
    offset += sprintf( message + offset, "\"avty_t\": \"~/%s\",", availability_topic);
    offset += sprintf( message + offset, "\"schema\": \"json\",");
    offset += sprintf( message + offset, "\"pl_on\": \"%s\",", payload_on);
    offset += sprintf( message + offset, "\"pl_off\": \"%s\",", payload_off);
    offset += sprintf( message + offset, "\"pl_avail\" : \"%s\",", payload_available);
    offset += sprintf( message + offset, "\"pl_not_avail\": \"%s\",", payload_not_available);
    offset += sprintf( message + offset, "\"stat_on\": \"%s\",", state_on);
    offset += sprintf( message + offset, "\"stat_off\": \"%s\",", state_off);
    offset += sprintf( message + offset, "\"ret\": \"false\",");
    offset += sprintf( message + offset, "\"opt\": \"false\",");
    offset += sprintf( message + offset, "\"dev\": {\"ids\": \"%s\",\"name\": \"%s\",\"mf\": \"ACME\",\"mdl\": \"xya\",\"sw\": \"1.0\",\"sn\": \"%s\",\"hw\": \"1.0rev2\"},", ctx.uniqueid, ctx.uniqueid, ctx.uniqueid);
    offset += sprintf( message + offset, "\"o\": {\"name\":\"mqttcli\",\"sw\": \"1.0\",\"url\": \"https://innovasoft.org\"}");
    offset += sprintf( message + offset, "}");
    publish_params.message.length = offset;
    publish_params.flags = 0x01;
    if( MQTT_SUCCESS != self->publish(self, &publish_params) ) {
      rc =  RC_IMPL_SPEC_ERR;
      goto finish;   
    }

    /* Subscribing to receive commands */
    subscribe_params.filter.value = buffer;
    subscribe_params.filter.length = sprintf( buffer, "%s/%s/%s", base_topic, ctx.uniqueid, command_topic );
    if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
      rc =  RC_IMPL_SPEC_ERR;
      goto finish;   
    }
    pthread_mutex_lock(&mutex);
    state = STATE_DISCOVERY;
    pthread_mutex_unlock(&mutex);
    goto finish;;
  }

  if(state == STATE_OPERATIONAL) {
    /* Updating device internal state */
    if(pkt->message.length == strlen(payload_on) && 0 == memcmp( pkt->message.value, payload_on, pkt->message.length) ) {
      toggle = 1;
    }
    else if(pkt->message.length == strlen(payload_off) && 0 == memcmp( pkt->message.value, payload_off, pkt->message.length) ) {
      toggle = 0;
    }
    else {
      rc =  RC_PAYLOAD_INV;
      goto finish; 
    }

    /* Publishing current state */
    publish_params.topic.value = buffer;
    publish_params.topic.length = sprintf( buffer, "%s/%s/%s", base_topic, ctx.uniqueid, state_topic );
    publish_params.message = pkt->message;
    if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
      rc =  RC_IMPL_SPEC_ERR;
    }
  }


finish:
  /* Resources deallocation */
  if(NULL != buffer) {
    free( buffer );
    buffer = NULL;
  }
  return rc;
}

void* thread_func(void* arg) {
  uint8_t *buffer = NULL, *recv_buf = NULL, timeout_counter = 2, current_state, current_timer_int;
  uint16_t rc;
  uint32_t srv_ip, version;
  char *log_str = NULL;
  int log_str_len = 0, sock = 0, *result;
  size_t recv_buf_len = 0, length, recv_buf_off, recv_len, i;
  clv_t *data = NULL;
  struct sockaddr_in server;
  struct hostent *host = NULL;
  mqtt_cli_t cli;
  mqtt_channel_t channel;
  mqtt_publish_params_t publish_params = {  };
  mqtt_will_params_t will_params = (mqtt_will_params_t) { };
  mqtt_params_t mqtt_params = { .max_pkt_id=8, .timeout=1, .version=4 };
  time_t now;
  fd_set readfds;
  struct timeval tv;
  lv_t packet, cli_userid, cli_username, cli_password;

  if( NULL == (result = malloc( sizeof(int) * 1))) {
    pthread_exit( NULL );
  }

  result[0] = RESULT_OK;

  if(NULL == arg) {
    TOLOG(LOG_CRIT, "arg is NULL");
    result[0] = RESULT_FAILURE;
    goto finish;
  }

  if(ctx.verbose) {
    printf("Thread with id %lu was started.\r\n", pthread_self());
  }

  /* Allocating resources */
  if( NULL == (data = (clv_t*) malloc( sizeof(clv_t) ) ) ) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  if( NULL == (buffer = malloc( ctx.buffer_size ) ) ) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  memcpy( data, &(clv_t) { .capacity=ctx.buffer_size, .length=0, .value=buffer }, sizeof(clv_t) );
  if( NULL == (recv_buf = (unsigned char*) malloc (ctx.buffer_size ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  recv_buf_len = ctx.buffer_size;
  memset( recv_buf, 0x00, recv_buf_len );
  log_str_len = 2*ctx.buffer_size + ctx.buffer_size;
  if( NULL == (log_str = (unsigned char*) malloc ( log_str_len ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result[0] = RESULT_FAILURE;
    goto finish;
  }

  /* Create the TCP/IP socket */
	if( -1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		TOLOG(LOG_CRIT, "socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), errno = %d", errno);
		result[0] = RESULT_FAILURE;
    goto finish;
	}

  server.sin_family = AF_INET;
  server.sin_port = htons( ctx.port );
  server.sin_addr.s_addr = inet_addr( ctx.ip );

  /* Get host by name (if needed) */
  if(server.sin_addr.s_addr == INADDR_NONE) {
    host = gethostbyname( ctx.ip );
    if(host == NULL) {
      TOLOG(LOG_ERR, "Server name resolving was impossible, errno = %d", errno);
      result[0] = RESULT_FAILURE;
      goto finish;
    }
    memcpy( &server.sin_addr, host->h_addr_list[0], host->h_length );
    if(ctx.verbose) {
      printf("%s resolved to %d.%d.%d.%d\r\n",  ctx.ip,
                                                (server.sin_addr.s_addr>> 0 & 0xff),
                                                (server.sin_addr.s_addr>> 8 & 0xff),  
                                                (server.sin_addr.s_addr>>16 & 0xff),
                                                (server.sin_addr.s_addr>>24 & 0xff));
    }
  }

  srv_ip = 0;
  srv_ip  = (server.sin_addr.s_addr>>24 & 0xff);
  srv_ip |= (server.sin_addr.s_addr>>16 & 0xff) << 8;
  srv_ip |= (server.sin_addr.s_addr>> 8 & 0xff) << 16;
  srv_ip |= (server.sin_addr.s_addr>> 0 & 0xff) << 24;  

  if( 1 == ctx.optval_reuse_addr) {
  	/* Enable to reuse address */
    if( -1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ctx.optval_reuse_addr, sizeof(ctx.optval_reuse_addr))) {
      TOLOG(LOG_ERR,"setsockopt(...,SOL_SOCKET,SO_REUSEADDR,...), errno = %d", errno);
      result[0] = RESULT_FAILURE;
      goto finish;
    }
  }

  /* Connecting to the server (broker) */
  if(ctx.verbose) {
    printf("Connecting to the server (broker)...");
  }
  if( -1 == connect(sock, (struct sockaddr*)&server, sizeof(server) ) ) {
    TOLOG(LOG_ERR,"connect( ... ), errno = %d", errno);
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  if(ctx.verbose) {
    printf("OK\r\n");
  }

  /* Start non-blocking mode */
  if( -1 == ioctl(sock, FIONBIO, (char*) &ctx.non_blocking) ) {
    TOLOG(LOG_ERR,"ioctl(sock, FIONBIO, ... ), errno = %d", errno);
    result[0] = RESULT_FAILURE;
    goto finish;
  }

  /* Initializing MQTT client */
  memset( &cli, 0x00, sizeof(cli));
  if(ctx.verbose) {
    mqtt_cli_get_lib_version( &version );
    printf("Initializing MQTT client (v%08x)...", version);
  }
  mqtt_params.bufsize = ctx.buffer_size;
  mqtt_params.timeout = 1;
  mqtt_params.version = ctx.mqtt_version;
  mqtt_params.qos = 0;
  mqtt_params.max_pkt_id = 8;
  if( MQTT_SUCCESS != mqtt_cli_init_ex( &cli, &mqtt_params ) ) {
    TOLOG(LOG_ERR,"mqtt_cli_init( ... ), rc = %d", rc);
    result[0] = RESULT_FAILURE;
    goto finish;    
  }
  if(ctx.verbose) {
    printf("OK\r\n");
  }

  /* Configuring MQTT client */
  if(ctx.verbose) {
    printf("Configuring MQTT client...");
  }
  cli.set_cb_connack( &cli, cb_connack );
  cli.set_cb_publish( &cli, cb_publish );
  cli.set_br_ip( &cli, srv_ip);
  cli.set_br_keepalive( &cli, (uint16_t) 60);
  will_params.topic.value = data->value;
  will_params.topic.length = sprintf( data->value, "%s/%s/%s", base_topic, ctx.uniqueid, availability_topic );
  will_params.payload.value = data->value + will_params.topic.length;
  will_params.payload.length = sprintf( data->value + will_params.topic.length, "%s", payload_not_available );
  if( MQTT_SUCCESS != (rc = cli.set_br_will( &cli, &will_params) ) ) {
    TOLOG(LOG_ERR,"cli.set_br_will( ... ), rc = %d", rc);
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  if(ctx.userid[0] == 0) {
    now = time(NULL);
    ctx.userid[0] = 'U';
    strftime(&(ctx.userid[1]), sizeof(ctx.userid)-1, "%Y%m%d%H%M%S", localtime(&now));
  }
  cli_userid.length = strlen( ctx.userid );
  cli_userid.value = ctx.userid;
  if( MQTT_SUCCESS != (rc = cli.set_br_userid( &cli, &cli_userid )) ) {
    TOLOG(LOG_ERR,"cli.set_br_userid( ... ), rc = %d", rc);
    result[0] = RESULT_FAILURE;
    goto finish;
  }
  cli_username.length = strlen (ctx.username);
  cli_username.value = ctx.username;
  if( MQTT_SUCCESS != (rc = cli.set_br_username( &cli, &cli_username )) ) {
    TOLOG(LOG_ERR,"cli.set_br_username( ... ), rc = %d", rc);
    result[0] = RESULT_FAILURE;
    goto finish;   
  }
  cli_password.length = strlen(ctx.password);
  cli_password.value = ctx.password;
  if( MQTT_SUCCESS != (rc = cli.set_br_password( &cli, &cli_password )) ) {
    TOLOG(LOG_ERR,"cli.set_br_password( ... ), rc = %d", rc);
    result[0] = RESULT_FAILURE;
    goto finish;   
  }
  if(ctx.verbose) {
    printf("OK\r\n");
  }

	tv.tv_sec = 1;
	tv.tv_usec = 0;
  recv_buf_off = 0;
  while( 1 ) {
    pthread_mutex_lock(&mutex);
    current_state = state;
    current_timer_int = timer_int;
    timer_int = 0;
    pthread_mutex_unlock(&mutex);

    if( STATE_STOPPED == current_state ) {
      break;
    }

    /* Processing timeout (if any) */
    if( current_timer_int ) {
      channel.ip_address = 0;
      channel.user_id = 0;
      data->length = 0;
      result[0] = process_and_send_data(sock, &cli, data, &channel, log_str, log_str_len);
      if(result[0] == RESULT_FAILURE) {
        TOLOG(LOG_ERR, "Sending failed");
        break;
      }
      else if(result[0] == RESULT_EXIT) {
        TOLOG(LOG_ERR, "Connection closed");
        break;
      }

      if( current_state == STATE_DISCOVERY && timeout_counter > 0 ) {
        --timeout_counter;
      }
      else if (current_state == STATE_DISCOVERY) {
        current_state = STATE_SYNCHRO;
        pthread_mutex_lock(&mutex);
        state = current_state;
        pthread_mutex_unlock(&mutex);
      }
    }

    if( current_state == STATE_SYNCHRO ) {
      /* Publishing current availability */
      publish_params.topic.value = data->value;
      publish_params.topic.length = sprintf( data->value, "%s/%s/%s", base_topic, ctx.uniqueid, availability_topic );
      publish_params.message.value = data->value + publish_params.topic.length;
      publish_params.message.length = sprintf( data->value + publish_params.topic.length, "%s", payload_available);
      cli.publish( &cli, &publish_params);

      /* Publishing current state */
      publish_params.topic.value = data->value;
      publish_params.topic.length = sprintf( data->value, "%s/%s/%s", base_topic, ctx.uniqueid, state_topic );
      publish_params.message.value = data->value + publish_params.topic.length;
      publish_params.message.length = sprintf( data->value + publish_params.topic.length, "%s", ( toggle > 0 ) ? state_on : state_off);
      cli.publish( &cli, &publish_params);
      current_state = STATE_OPERATIONAL;
      pthread_mutex_lock(&mutex);
      state = current_state;
      pthread_mutex_unlock(&mutex);
    }

    if( current_state == STATE_OPERATIONAL) {
      /* Prepare the read stdin (fd = 0) sets for network I/O notification */
      FD_ZERO(&readfds);
      /* Set read notification for the socket */
      FD_SET(0, &readfds);	  /* Wait until the socket has data ready to be read (until timeout occurs) */
      if( -1 == (result[0] = select( 1, &readfds, NULL, NULL, &tv)) ) {
        if(EINTR == errno ) {
          continue;
        }
        TOLOG(LOG_ERR,"select( ... ), errno = %d", errno);
        result[0] = RESULT_FAILURE;
        goto finish;
      }
      if(result[0] && FD_ISSET(0, &readfds) && 0x20 == getchar()) {
        publish_params.topic.value = data->value;
        publish_params.topic.length = sprintf( data->value, "%s/%s/%s", base_topic, ctx.uniqueid, state_topic);
        publish_params.message.value = data->value + publish_params.topic.length;
        toggle = (toggle > 0) ? 0 : 1;
        publish_params.message.length = sprintf( data->value + publish_params.topic.length, "%s", ( toggle > 0 ) ? state_on : state_off);
        cli.publish( &cli, &publish_params);
        publish_params.message.length = sprintf( data->value + publish_params.topic.length, "%s", ( toggle > 0 ) ? state_on : state_off);
        cli.publish( &cli, &publish_params);
      }
    }

    /* Prepare the read socket sets for network I/O notification */
    FD_ZERO(&readfds);
    /* Set read notification for the socket */
    FD_SET(sock, &readfds);
	  /* Wait until the socket has data ready to be read (until timeout occurs) */
	  if( -1 == (result[0] = select( sock+1, &readfds, NULL, NULL, &tv)) ) {
      if(EINTR == errno ) {
        continue;
      }
		  TOLOG(LOG_ERR,"select( ... ), errno = %d", errno);
      result[0] = RESULT_FAILURE;
      goto finish;
	  }
    /* Check if timeout has occurred */
    if( result[0] == 0 ) {
      /* do nothing */
      ;
    }
    /* Check if there is something to read */
	  else if (FD_ISSET(sock, &readfds)) {
      /* Receive and process */
      while( 1 ) {
        if(recv_buf_off < 0) {
          TOLOG(LOG_ERR, "It was impossible to receive all data,");
          goto finish;
        }

        recv_len = recv(sock, recv_buf+recv_buf_off, recv_buf_len, 0);

        if( recv_len < 0) {
          if(errno == EWOULDBLOCK) {
            break;
          }
          else if(errno == ECONNRESET) {
            TOLOG(LOG_INFO,"Connection reset");
            goto finish;
          }
          else {
            TOLOG(LOG_ERR,"recv( ... ), errno = %d", errno);
            goto finish;
          }
        }
        else if(recv_len == 0) {
          TOLOG(LOG_INFO,"Connection reset by peer");
          goto finish;
        }

        recv_buf_len -= recv_len;
        recv_buf_off += recv_len;
        length = 0;

        packet.length = recv_buf_off;
        packet.value = recv_buf;
        mqtt_get_pkt_length(&packet, &length);

        if( recv_buf_off >= length) {
          memcpy( data->value, recv_buf, length);
          memmove( recv_buf, recv_buf+length, recv_buf_off - length);
          recv_buf_off -= length;
          recv_buf_len += length;

          if(ctx.verbose) {
            memset(log_str, 0x00, log_str_len);
            format_data(data->value, length, log_str, log_str_len);
            log_str[1] = '=';
            log_str[2] = '>';
            printf("%s\r\n", log_str);
          }

          channel.ip_address = srv_ip;
          channel.user_id = 0;
          data->length = length;
          result[0] = process_and_send_data(sock, &cli, data, &channel, log_str, log_str_len);
          if(result[0] == RESULT_FAILURE) {
            TOLOG(LOG_ERR, "Sending failed");
            break;
          }
          else if(result[0] == RESULT_EXIT) {
            TOLOG(LOG_ERR, "Connection closed");
            break;
          }
        }
        break;
      } /* Receive and process */
	  }
    tv.tv_sec = 0;
    tv.tv_usec = 200;
    select(0, NULL, NULL, NULL, &tv);
  } /* while loop */

  /* Exit the program */
finish:
  if(ctx.verbose) {
    printf("Thread with id %lu was stopped.\r\n", pthread_self());
  }
  /* Deallocating resources (if any) */
  if( NULL != data) {
    if(NULL != data->value) {
      free( data->value );
    }
    free( data );
    data = NULL;
  }
  if(sock) {
    close( sock );
  }
  if (NULL != log_str) {
    free( log_str );
  }
  if( NULL != recv_buf) {
    free( recv_buf );
  }
  if( NULL != cli.ctx) {
    mqtt_cli_destr( &cli );
  }
  return result;
}

int main(int argc, char** argv) {
  pthread_t thread = 0;
  int *thread_result, result;
  struct sigaction sa;
  struct itimerval timer;

  /* Validate arguments */
  if(validate_args(argc, argv)) {
    usage(argv[0]);
    result = RESULT_FAILURE;
    goto finish;
  }

  show_info();

  state = STATE_STARTED;

  /* Configure signal_handler as the signal handler for SIGALRM */
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &signal_handler;
  sigaction (SIGALRM, &sa, NULL);

  /* Configure signal handler to stop the program on Ctrl+C pressed */
  sigaction(SIGINT, &sa, NULL);

	/* Configure the timer to expire after n sec... */
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;

	/* ... and every n sec after that. */
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;

  pthread_mutex_init(&mutex, NULL);
  result = pthread_create(&thread, NULL, thread_func, ctx.uniqueid);
  if(result != 0) {
    if(ctx.verbose) {
      perror("pthread_create\r\n");
    }
    goto finish;
  }

	/* Start a real timer. It counts down whenever this process is
	   executing. */
	setitimer (ITIMER_REAL, &timer, NULL);

  if(ctx.verbose) {
    printf("Press Ctrl+c to stop\r\n");
  }

  if(0 != thread) {
    pthread_join(thread, (void*) &thread_result);
    if(NULL != thread_result) {
      if(thread_result[0] == RESULT_OK && ctx.verbose) {
        printf("Thread finished successfully.\r\n");
        result = RESULT_OK;
      }
      else if(ctx.verbose) {
        printf("Thread finished with failure.\r\n");
        result = RESULT_FAILURE;
      }
      free( thread_result );
      thread_result = NULL;
    }
    else if(ctx.verbose) {
      printf("Thread result is NULL.\r\n");
      result = RESULT_FAILURE;
    }
  }
  
finish:
  pthread_mutex_destroy(&mutex);
  return result;
}