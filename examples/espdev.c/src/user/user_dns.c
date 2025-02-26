/*
This code was inspired by this article:
https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/Extended5DNS.html
*/

#include <user_interface.h>
#include "osapi.h"
#include "user_net.h"
#include "user_dns.h"
#include "user_cfg.h"
#include "user_log.h"

/* Determines the number of DNS requests which will be send  */
#define MAX_DNS_RETRY 4

/* Structure of the bytes for a DNS header */
typedef struct {
  uint16_t xid;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
} dns_header_t;

typedef struct {
  uint16_t compression;
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t length;
  ip_addr_t addr;
} __attribute__((packed)) dns_rr_a_t;

/** Timer used to indicate error */
os_timer_t dns_timeout_timer;

extern uint8_t big_buffer[1024];
static uint8_t *qname = &big_buffer[0];
static size_t qname_len;
static uint8_t dns_retry;

void ICACHE_FLASH_ATTR dns_send_request();
void dns_build_qname (uint8_t *hostname, size_t hostname_len, uint8_t *qname, size_t *qname_len);
void dns_prepare_request (uint8_t *qname, size_t qname_len, uint8_t *request, size_t *request_len);
uint16_t dns_parse_response (uint8_t *qname, size_t qname_len, uint8_t *response, dns_rr_a_t *record);

static void ICACHE_FLASH_ATTR dns_timeout_cb(void *arg) {
  /* Disarm idle timer */
  os_timer_disarm(&dns_timeout_timer);

  if(dns_retry) {
    --dns_retry;
    dns_send_request();
    return;
  }

  /* There was no DNS response in time - force to reconnect */
  net_connect( NULL );
}

void ICACHE_FLASH_ATTR dns_ready_cb() {
  dns_retry = MAX_DNS_RETRY;

  dns_send_request();
}

void ICACHE_FLASH_ATTR dns_recv_cb(void *arg, char *response, unsigned short len) {
  dns_rr_a_t record;

  /* Disarm idle timer */
  os_timer_disarm(&dns_timeout_timer);

  /* Parse the response (if any) */
  if( 0 == dns_parse_response (qname, qname_len, response, &record)) {
    /* Connect to the resolved IP address */
    net_connect( &(record.addr) );
  }
  else {
    /* signal failure */
    net_connect( NULL );
  }
}

void ICACHE_FLASH_ATTR dns_sent_cb(void *arg) {
}

void ICACHE_FLASH_ATTR dns_send_request() {
  extern struct user_cfg cfg;
  uint8_t *request;
  size_t request_len;

  /* Build QNAME */
  qname = &big_buffer[0];
  dns_build_qname(cfg.br_host, cfg.br_host_len, qname, &qname_len);

  /* Build DNS request */
  request = &big_buffer[qname_len + 2];
  dns_prepare_request(qname, qname_len, request, &request_len);

  /* Start timeout timer */
  os_timer_disarm(&dns_timeout_timer);
  os_timer_setfn(&dns_timeout_timer, (os_timer_func_t *)dns_timeout_cb, NULL);
  os_timer_arm(&dns_timeout_timer, DELAY_5_SEC, 0);

  /* Send the request */
  net_udp_send(request, request_len);
}

void dns_build_qname(uint8_t *hostname, size_t hostname_len, uint8_t *qname, size_t *qname_len) {
  uint8_t count, *prev;
  size_t i;

  /* Leave the first byte blank for the first field length */
  os_memcpy (qname + 1, hostname, hostname_len);

  /* Example:
     +---+---+---+---+---+---+---+---+---+---+---+
     | a | b | c | . | d | e | . | c | o | m | \0|
     +---+---+---+---+---+---+---+---+---+---+---+

     becomes:
     +---+---+---+---+---+---+---+---+---+---+---+---+
     | 3 | a | b | c | 2 | d | e | 3 | c | o | m | 0 |
     +---+---+---+---+---+---+---+---+---+---+---+---+
   */

  *qname_len = 0;
  count = 0;
  prev = qname;
  for (i = 0; i < hostname_len; ++i) {
    if (hostname[i] == '.') {
      *prev = count;
      prev = qname + i + 1;
      count = 0;
    }
    else {
       ++count;
    }
  }
  *prev = count;
  /* Add termination character */
  qname[hostname_len + 1] = '\0';
  *qname_len = hostname_len + 2;
}

uint16_t dns_parse_response (uint8_t *qname, size_t qname_len, uint8_t *response, dns_rr_a_t *record) {
  uint8_t *p, *questions[16];
  uint16_t *type, *class, *rdlength;
  uint32_t *ttl;
  size_t i, label_len, questions_len, answers_len;
  dns_header_t *header;
  dns_rr_a_t *records;

  /* First, check the header for an error response code */
  p = response;
  header = (dns_header_t *) response;
  if ((ntohs (header->flags) & 0xf) != 0) {
    // "Failed to get response"
    return -1;
  }
  p += sizeof (dns_header_t);
  questions_len = ntohs(header->qdcount);
  answers_len = ntohs(header->ancount);

  if(questions_len > sizeof(questions)/sizeof(questions[0]) ) {
    // "Unsupported questions length"
    return -1;
  }

  if(answers_len > sizeof(questions)/sizeof(questions[0]) ) {
    // "Unsupported answers length"
    return -1;
  }

  /* Read Questions (if any) */
  for(i=0; i < questions_len; ++i) {
    questions[i] = p;
    if( 0 != memcmp(p, qname, qname_len )) {
      questions[i] = NULL;
    }
    /* read QNAME */
    do {
      /* obtain length of subdomain */
      label_len = *p++;
      /* skip the label */
      p += label_len;
    } while(label_len != 0);
    /* skip QTYPE */
    p += 2;
    /* skip QCLASS */
    p += 2;
  }

  /* Print Answers (if any) */
  records = (dns_rr_a_t*) p;
  for(i=0; i < answers_len; ++i) {
    if(questions[i] != NULL) {
      *record = records[i];
      break;
    }
  }

  return 0;
}

void dns_prepare_request (uint8_t *qname, size_t qname_len, uint8_t *packet, size_t *packet_len) {
  uint8_t *p;
  uint16_t qtype, qclass;
  size_t header_len, qtype_len, qclass_len;
  dns_header_t header;

  /* Set lengths */
  *packet_len = 0;
  header_len = sizeof(header);
  qtype_len = sizeof(qtype);
  qclass_len = sizeof(qclass);

  /* Set pointer to packet */
  p = packet;

  /* Set up the DNS header */
  os_memset (&header, 0, header_len);
  header.xid = htons (0x1234);    /* Randomly chosen ID */
  header.flags = htons (0x0100);  /* Q=0, RD=1 */
  header.qdcount = htons (1);     /* Sending 1 question */

  /* Copy the header first */
  os_memcpy (p, &header, header_len);
  p += header_len;

  /* Copy the QNAME */
  os_memcpy (p, qname, qname_len);
  p += qname_len;

  /* Set up the DNS question */
  qtype = htons (1);  /* QTYPE 1=A */
  qclass = htons (1); /* QCLASS 1=IN */

  /* Copy the QTYPE field */
  os_memcpy(p, &qtype, qtype_len);
  p += qtype_len;
  /* Copy the QCLASS field */
  os_memcpy (p, &qclass, qclass_len);

  /* Calculate DNS Question length */
  *packet_len = header_len + qname_len + qtype_len + qclass_len;
}
