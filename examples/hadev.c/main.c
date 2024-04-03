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

#include "main.h"
#include "utils.h"
#include "utils.h"
#include "../../api/mqtt_cli.h"

const char* unique_id = "hadev123456";
const char* base_topic = "homeassistant/switch/hadev123456";
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

/** Global buffer used to send and perform operations on the data */
static clv_t* buffer;

/* Stores current switch state (on or off) */
static uint8_t toggle = 0;

static struct option long_options[] = {
  {L_OPT_HOST,        required_argument,  0,  S_OPT_HOST},
  {L_OPT_PORT,        required_argument,  0,  S_OPT_PORT},
  {L_OPT_BUFFER_SIZE, required_argument,  0,  S_OPT_BUFFER_SIZE},
  {L_OPT_REUSE_ADDR,  required_argument,  0,  S_OPT_REUSE_ADDR},
  {L_OPT_USERID,      required_argument,  0,  S_OPT_USERID},
  {L_OPT_USERNAME,    required_argument,  0,  S_OPT_USERNAME},
  {L_OPT_PASSWORD,    required_argument,  0,  S_OPT_PASSWORD},
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
  ctx.time_stamps = 0;
  ctx.non_blocking = 1;
  ctx.timeout = 1;

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
      case S_OPT_VERBOSE:
        ctx.verbose = 1;
        ctx.log_fd = stdout;
        ctx.log_max_level = LOG_INFO;
        break;
      case S_OPT_USERID:
        length = strlen( optarg );
        if(length > sizeof(ctx.userid) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid userid length");
        }
        memcpy(ctx.userid, optarg, length);        
        break;
      case S_OPT_USERNAME:
        length = strlen( optarg );
        if(length > sizeof(ctx.username) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid username length");
        }
        memcpy(ctx.username, optarg, length);        
        break;
      case S_OPT_PASSWORD:
        length = strlen( optarg );
        if(length > sizeof(ctx.password) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid password length");
        }
        memcpy(ctx.password, optarg, length);        
        break;
      case S_OPT_HOST:
        length = strlen( optarg );
        if(length > sizeof(ctx.ip) / sizeof(char) ) {
          TOLOG(LOG_ERR, "Invalid IP format");
        }
        memcpy(ctx.ip, optarg, length);
        break;
      case S_OPT_PORT:
        ctx.port = atoi( optarg );
        break;
      case S_OPT_BUFFER_SIZE:
        ctx.buffer_size = atoi( optarg );
        break;
      case S_OPT_REUSE_ADDR:
        ctx.optval_reuse_addr = 1;
        break;
      case '?':
        return RESULT_FAILURE;
        break;
      default:
        exit(1);
    }
  }

  if( ctx.username[0] == 0x00 ) {
    TOLOG(LOG_ERR, "'--username' option required");
    return RESULT_FAILURE;
  }

  if( ctx.password[0] == 0x00 ) {
    TOLOG(LOG_ERR, "'--password' option required");
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
			ctx.timer_int = 1;
			break;
		case SIGINT:
		case SIGTERM:
			ctx.state = 0;
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
  printf("       %s %s", program, "--pub [options]\r\n");
  printf("       %s %s", program, "--sub [options]\r\n");
  printf("OPTIONS\r\n");
  printf(" -%c user_id, --%s user_id\r\n\t%s\r\n",     S_OPT_USERID,      L_OPT_USERID,        "Sets user_id used during CONNECT packet creation. If not specified user_id is auto generated.");
  printf(" -%c user_name, --%s user_name\r\n\t%s\r\n", S_OPT_USERNAME,    L_OPT_USERNAME,      "Sets user_name used during CONNECT packet creation.");
  printf(" -%c password, --%s password\r\n\t%s\r\n",   S_OPT_PASSWORD,    L_OPT_PASSWORD,      "Sets password used during CONNECT packet creation.");
  printf(" -%c size, --%s size\r\n\t%s\r\n",           S_OPT_BUFFER_SIZE, L_OPT_BUFFER_SIZE,   "Sets buffer size, which is dynamically allocated.");
  printf(" -%c host_name, --%s host_name\r\n\t%s\r\n", S_OPT_HOST,        L_OPT_HOST,          "Sets remote host name or IP address.");
  printf(" -%c port, --%s port\r\n\t%s\r\n",           S_OPT_PORT,        L_OPT_PORT,          "Sets the remote port to be used.");
  printf(" -%c, --%s\r\n\t%s\r\n",                     S_OPT_VERBOSE,     L_OPT_VERBOSE,       "Runs the program in verbose mode.");
  printf(" --%s\r\n\t%s\r\n",                          L_OPT_REUSE_ADDR,                       "Turns on to reuse the the address.");
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
  uint8_t *message;
  int offset;
  mqtt_publish_params_t publish_params = { };
  mqtt_subscribe_params_t subscribe_params = { };

  /* Publishing configuration */
  publish_params.topic.value = buffer->value;
  publish_params.topic.length = sprintf( buffer->value, "%s/config", base_topic );
  message = publish_params.message.value = buffer->value + publish_params.topic.length;
  offset = 0;
  message[0] = '{';
  offset += 1;
  offset += sprintf( message + offset, "\"~\": \"%s\",", base_topic );
  offset += sprintf( message + offset, "\"name\": null,");
  offset += sprintf( message + offset, "\"uniq_id\": \"%s\",", unique_id);
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
  offset += sprintf( message + offset, "\"ret\": \"true\",");
  offset += sprintf( message + offset, "\"opt\": \"false\",");
  offset += sprintf( message + offset, "\"dev\": {\"ids\": \"ea334450945afc\",\"name\": \"acme_dev\",\"mf\": \"ACME\",\"mdl\": \"xya\",\"sw\": \"1.0\",\"sn\": \"ea334450945afc\",\"hw\": \"1.0rev2\"},");
  offset += sprintf( message + offset, "\"o\": {\"name\":\"mqttcli\",\"sw\": \"1.0\",\"url\": \"https://innovasoft.org\"}");
  offset += sprintf( message + offset, "}");
  publish_params.message.length = offset;
  if( MQTT_SUCCESS != self->publish(self, &publish_params) ) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

  /* Publishing current device availability */
  publish_params.topic.value = buffer->value;
  publish_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, availability_topic );
  message = publish_params.message.value = buffer->value + publish_params.topic.length;
  publish_params.message.length = sprintf( message, "%s", payload_available );
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

  /* Publishing current device state */
  publish_params.topic.value = buffer->value;
  publish_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, state_topic );
  message = publish_params.message.value = buffer->value + publish_params.topic.length;
  publish_params.message.length = sprintf( message, "%s", ( toggle > 0 ) ? state_on : state_off );
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

  /* Subscribing to receive commands */
  subscribe_params.filter.value = buffer->value;
  subscribe_params.filter.length = sprintf( buffer->value, "%s/%s", base_topic, command_topic );
  if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

finish:
  return rc;
}

mqtt_rc_t cb_publish(const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel) {
  mqtt_rc_t rc = RC_SUCCESS;
  mqtt_publish_params_t publish_params = { };
  size_t offset;

  /* Updating device internal state */
  if(pkt->message.length == strlen(payload_on) && 0 == memcmp( pkt->message.value, payload_on, pkt->message.length) ) {
    toggle = 1;
  }
  else if(pkt->message.length == strlen(payload_off) && 0 == memcmp( pkt->message.value, payload_off, pkt->message.length) ) {
    toggle = 0;
  }

  /* Publishing current state */
  publish_params.topic.value = buffer->value;
  publish_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, state_topic );
  publish_params.message = pkt->message;
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    rc =  RC_IMPL_SPEC_ERR;
  }

  return rc;
}

int main(int argc, char** argv) {
  uint8_t *recv_buf = NULL, *tmp_buf = NULL;
  uint16_t rc;
  uint32_t srv_ip;
  size_t length, recv_buf_len, recv_buf_off, recv_len, i;
  char *log_str = NULL, c;
  int result, optval, ret, log_str_len, sock;
  struct sigaction sa;
  struct sockaddr_in server;
  struct hostent *host = NULL;
  fd_set readfds;
  struct timeval tv;
  mqtt_cli_t cli;
  mqtt_channel_t channel;
  struct itimerval timer;
  time_t now;
  lv_t packet, cli_userid, cli_username, cli_password;
  mqtt_publish_params_t publish_params = {  };
  mqtt_will_params_t will_params = (mqtt_will_params_t) { };

  /* Initialize */
  memset( &cli, 0x00, sizeof(cli));

  /* Validate arguments */
  if(validate_args(argc, argv)) {
    usage(argv[0]);
    result = RESULT_FAILURE;
    goto finish;
  }

  show_info();

  if( NULL == (buffer = (clv_t*) malloc( sizeof(clv_t) ) ) ) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result = RESULT_FAILURE;
    goto finish;
  }

  if( NULL == (tmp_buf = malloc( ctx.buffer_size ) ) ) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result = RESULT_FAILURE;
    goto finish;
  }

  memcpy( buffer, &(clv_t) { .capacity=ctx.buffer_size, .length=0, .value=tmp_buf }, sizeof(clv_t) );

  if( NULL == (recv_buf = (unsigned char*) malloc (ctx.buffer_size ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result = RESULT_FAILURE;
    goto finish;
  }
  recv_buf_len = ctx.buffer_size;
  memset( recv_buf, 0x00, recv_buf_len );

  log_str_len = 2*ctx.buffer_size + ctx.buffer_size;
  if( NULL == (log_str = (unsigned char*) malloc ( log_str_len ))) {
    TOLOG(LOG_CRIT, "Not enough memory");
    result = RESULT_FAILURE;
    goto finish;
  }

  /* Configure signal_handler as the signal handler for SIGALRM */
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &signal_handler;
  sigaction (SIGALRM, &sa, NULL);

  /* Configure signal handler to stop the program on Ctrl+C pressed */
  sigaction(SIGINT, &sa, NULL);

  /* Create the TCP/IP socket */
	if( -1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		TOLOG(LOG_CRIT, "socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), errno = %d", errno);
		result = RESULT_FAILURE;
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
      result = RESULT_FAILURE;
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
      result = RESULT_FAILURE;
      goto finish;
    }
  }

  /* Initializing MQTT client */
  if(ctx.verbose) {
    printf("Initializing MQTT client...");
  }
  if( MQTT_SUCCESS != (rc = mqtt_cli_init( &cli )) ) {
    TOLOG(LOG_ERR,"mqtt_cli_init( ... ), rc = %d", rc);
    rc = RESULT_FAILURE;
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
  cli.set_timeout( &cli, (uint16_t) ctx.timeout);
  cli.set_br_keepalive( &cli, (uint16_t) 60);
  will_params.topic.value = buffer->value;
  will_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, availability_topic );
  will_params.payload.value = buffer->value + will_params.topic.length;
  will_params.payload.length = sprintf( buffer->value + will_params.topic.length, "%s", payload_not_available );
  if( MQTT_SUCCESS != (rc = cli.set_br_will( &cli, &will_params) ) ) {
    TOLOG(LOG_ERR,"cli.set_br_will( ... ), rc = %d", rc);
    result = RESULT_FAILURE;
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
    result = RESULT_FAILURE;
    goto finish;
  }
  cli_username.length = strlen (ctx.username);
  cli_username.value = ctx.username;
  if( MQTT_SUCCESS != (rc = cli.set_br_username( &cli, &cli_username )) ) {
    TOLOG(LOG_ERR,"cli.set_br_username( ... ), rc = %d", rc);
    result = RESULT_FAILURE;
    goto finish;   
  }
  cli_password.length = strlen(ctx.password);
  cli_password.value = ctx.password;
  if( MQTT_SUCCESS != (rc = cli.set_br_password( &cli, &cli_password )) ) {
    TOLOG(LOG_ERR,"cli.set_br_password( ... ), rc = %d", rc);
    result = RESULT_FAILURE;
    goto finish;   
  }
  if(ctx.verbose) {
    printf("OK\r\n");
  }

  /* Connecting to the server (broker) */
  if(ctx.verbose) {
    printf("Connecting to the server (broker)...");
  }
  if( -1 == connect(sock, (struct sockaddr*)&server, sizeof(server) ) ) {
    TOLOG(LOG_ERR,"connect( ... ), errno = %d", errno);
    result = RESULT_FAILURE;
    goto finish;
  }
  if(ctx.verbose) {
    printf("OK\r\n");
  }

  /* Start non-blocking mode */
  if( -1 == ioctl(sock, FIONBIO, (char*) &ctx.non_blocking) ) {
    TOLOG(LOG_ERR,"ioctl(sock, FIONBIO, ... ), errno = %d", errno);
    result = RESULT_FAILURE;
    goto finish;
  }

	/* Configure the timer to expire after n sec... */
	timer.it_value.tv_sec = ctx.timeout;
	timer.it_value.tv_usec = 0;

	/* ... and every n sec after that. */
	timer.it_interval.tv_sec = ctx.timeout;
	timer.it_interval.tv_usec = 0;

	/* Start a real timer. It counts down whenever this process is
	   executing. */
	setitimer (ITIMER_REAL, &timer, NULL);

  if(ctx.verbose) {
    printf("Press Ctrl+c to stop\r\n");
  }

  ctx.state = 1;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
  recv_buf_off = 0;
  while( 1 == ctx.state ) {
    /* Processing timeout (if any) */
    if( ctx.timer_int ) {
      ctx.timer_int = 0;
      channel.ip_address = 0;
      channel.user_id = 0;
      buffer->length = 0;
      result = process_and_send_data(sock, &cli, buffer, &channel, log_str, log_str_len);
      if(result == RESULT_FAILURE) {
        TOLOG(LOG_ERR, "Sending failed");
        break;
      }
      else if(result == RESULT_EXIT) {
        TOLOG(LOG_ERR, "Connection closed");
        break;
      }    
    }
    /* Prepare the read stdin (fd = 0) sets for network I/O notification */
    FD_ZERO(&readfds);
    /* Set read notification for the socket */
    FD_SET(0, &readfds);	  /* Wait until the socket has data ready to be read (until timeout occurs) */
	  if( -1 == (result = select( 1, &readfds, NULL, NULL, &tv)) ) {
      if(EINTR == errno ) {
        continue;
      }
		  TOLOG(LOG_ERR,"select( ... ), errno = %d", errno);
      result = RESULT_FAILURE;
      goto finish;
	  }
    if(result && FD_ISSET(0, &readfds) && 0x20 == getchar()) {
      publish_params.topic.value = buffer->value;
      publish_params.topic.length = sprintf( buffer->value, "%s/%s", base_topic, state_topic);
      publish_params.message.value = buffer->value + publish_params.topic.length;
      toggle = (toggle > 0) ? 0 : 1;
      publish_params.message.length = sprintf( buffer->value + publish_params.topic.length, "%s", ( toggle > 0 ) ? state_on : state_off);
      cli.publish( &cli, &publish_params);
    }

    /* Prepare the read socket sets for network I/O notification */
    FD_ZERO(&readfds);
    /* Set read notification for the socket */
    FD_SET(sock, &readfds);
	  /* Wait until the socket has data ready to be read (until timeout occurs) */
	  if( -1 == (result = select( sock+1, &readfds, NULL, NULL, &tv)) ) {
      if(EINTR == errno ) {
        continue;
      }
		  TOLOG(LOG_ERR,"select( ... ), errno = %d", errno);
      result = RESULT_FAILURE;
      goto finish;
	  }
    /* Check if timeout has occurred */
    if( result == 0 ) {
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
        cli.get_pkt_length(&cli, &packet, &length);

        if( recv_buf_off >= length) {
          memcpy( buffer->value, recv_buf, length);
          memmove( recv_buf, recv_buf+length, recv_buf_off - length);
          recv_buf_off -= length;
          recv_buf_len += length;

          if(ctx.verbose) {
            memset(log_str, 0x00, log_str_len);
            format_data(buffer->value, length, log_str, log_str_len);
            log_str[1] = '=';
            log_str[2] = '>';
            printf("%s\r\n", log_str);
          }

          channel.ip_address = srv_ip;
          channel.user_id = 0;
          buffer->length = length;
          result = process_and_send_data(sock, &cli, buffer, &channel, log_str, log_str_len);
          if(result == RESULT_FAILURE) {
            TOLOG(LOG_ERR, "Sending failed");
            break;
          }
          else if(result == RESULT_EXIT) {
            TOLOG(LOG_ERR, "Connection closed");
            break;
          }
        }

        break;
      } /* Receive and process */
	  }

  } /* while loop */

  /* Exit the program */
  printf("\r\nStopped!\r\n");
  rc = RC_SUCCESS;

finish:
  /* Close the socket if necessary */
  if(sock) {
    close( sock );
  }
  if(NULL != log_str) {
    free( log_str );
  }
  if(NULL != recv_buf) {
    free( recv_buf );
  }
  if(NULL != buffer->value) {
    free( buffer->value );
  }
  if( NULL != buffer) {
    free( buffer );
    buffer = NULL;
  }
  /* Destroy client */
  if( NULL != cli.ctx) {
    mqtt_cli_destr( &cli );
  }
  return result;
}