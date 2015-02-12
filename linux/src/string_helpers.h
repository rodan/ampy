#ifndef __STRING_HELPERS_H__
#define __STRING_HELPERS_H__

uint8_t extract_hex(const char *str, uint8_t *rv);
uint8_t compute_xor_hash(const char *msg, const uint16_t len, uint8_t *rv);
uint8_t check_xor_hash(const char *msg, const uint16_t len);

#endif
