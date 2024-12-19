#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../src/user/user_cfg.h"

void printb(const uint8_t *buf, const size_t buf_len) {
  size_t i;
  for(i=0; i < buf_len; ++i) {
    printf("0x%02x ", buf[i]);
  }
  printf("\r\n");
}

int main() {
  FILE *f = NULL;
  char* file_name = "../../bin/user_data.bin";
  struct user_cfg cfg;
  uint8_t *buffer = NULL;
  size_t buffer_len = sizeof(struct user_cfg);

  printf("udata\r\n");

  if( NULL == (f = fopen(file_name, "r"))) {
    printf("%s does not exist!\r\n");
    exit(-1);
  }

  if( NULL == (buffer = (uint8_t*) malloc( buffer_len )) ) {
    printf("Not enough memory!\r\n");
    exit(-1);
  } 

  fseek(f, 0, SEEK_SET);
  fread(buffer, buffer_len, 1, f);

  printf( "raw = ");
  printb(buffer, buffer_len);
  printf( "\r\n");

  if(buffer[0] == 0xFF && buffer[1] == 0xFF && buffer[2] == 0xFF && buffer[3] == 0xFF) {
    printf("Incorrect data!\r\n");
    exit(-1);
  }

  free(buffer);

  fseek(f, 0, SEEK_SET);
  fread(&cfg, sizeof(struct user_cfg), 1, f);

  printf( "app_pass         = %s\r\n", cfg.ap_pass);
  printf( "app_pass_len     = %d\r\n", cfg.ap_pass_len);
  printf( "code_version     = ");
  printb(cfg.code_version, cfg.code_version_len);
  printf( "code_version_len = %d\r\n", cfg.code_version_len);
  printf( "dev_id           = %s\r\n", cfg.dev_id);
  printf( "dev_id_len       = %d\r\n", cfg.dev_id_len);
  printf( "dev_mode         = %c\r\n", cfg.dev_mode);
  printf( "dev_model        = %s\r\n", cfg.dev_model);
  printf( "dev_model_len    = %d\r\n", cfg.dev_model_len);
  printf( "dev_type         = %s\r\n", cfg.dev_type);
  printf( "dev_type_len     = %d\r\n", cfg.dev_type_len);
  printf( "sdk_version      = %s\r\n", cfg.sdk_version);
  printf( "sdk_version_len  = %d\r\n", cfg.sdk_version_len);
  printf( "wifi_pass        = %s\r\n", cfg.wifi_pass);
  printf( "wifi_pass_len    = %d\r\n", cfg.wifi_pass_len);
  printf( "wifi_ssid        = %s\r\n", cfg.wifi_ssid);
  printf( "wifi_ssid_len    = %d\r\n", cfg.wifi_ssid_len);

  fclose(f);

  return 0;
}