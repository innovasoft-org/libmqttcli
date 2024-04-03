#ifndef __UTILS_H__
#define __UTILS_H__

int get_time(char* buf, int len);

int get_date(char* buf, int len);

size_t htos(const unsigned char* in_buf, int in_len, char* out_buf, int out_len);

size_t stoh(const char *in, uint8_t *out, size_t out_len);

void format_data(const unsigned char* in_buf, size_t in_len, char* out_buf, size_t out_len);

#endif /* __UTILS_H__ */
