
#define STR_MAX 128

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "proj.h"

char strtemp[STR_MAX];

uint8_t extract_hex(const char *str, uint8_t *rv)
{
    uint8_t i=0;
    const char *p = str;
    char c = *p;
    
    *rv = 0;

    // ignore spaces before the number
    while (c == 32) {
        c = *++p;
    }

    while (i<2) {
        if (((c > 47) && (c < 58)) || ((c > 96) && (c < 103)) || ((c > 64) && (c < 71))) {
            // go lowercase (A-F -> a-f)
            if ((c > 64) && (c < 71)) {
                c += 32;
            }

            *rv = *rv << 4;
            if ((c > 47) && (c < 58)) {
                *rv += c - 48;
            } else if ((c > 96) && (c < 103)) {
                *rv += c - 87;
            }
        } else {
            return 0;
        }

        i++;
        //p++;
        c = *++p;
    }
    return i;
}

uint8_t compute_xor_hash(const char *msg, const uint16_t len, uint8_t *rv)
{
    uint16_t i;

    *rv = 85;

    for (i=0;i<len;i++) {
        *rv ^= msg[i];    
    } 

    return EXIT_SUCCESS;
}

uint8_t check_xor_hash(const char *msg, const uint16_t len)
{
    uint8_t sender_hash;
    uint8_t computed_hash;

    if (msg[len-3] == '*') {
        extract_hex(&msg[len-2], &sender_hash);
        compute_xor_hash(msg, len-3, &computed_hash);

        if (sender_hash == computed_hash) {
            return EXIT_SUCCESS;
        } else {
            return EXIT_FAILURE;
        }
    } else {
        return EXIT_FAILURE;
    }
}


/*
void main()
{
    uint8_t hash;
    char hash_str[4];

    snprintf(strtemp, STR_MAX-3, "V120200");
    compute_hash(strtemp, strlen(strtemp), &hash);
    snprintf(hash_str, 4, "*%02x", hash);
    strncat(strtemp, hash_str, STR_MAX);

    fprintf(stdout, "%s\n", strtemp);

    if (check_hash(strtemp, strlen(strtemp)) == EXIT_SUCCESS) {
        fprintf(stdout, "hash ok\n");
    } else {
        fprintf(stdout, "hash failed\n");
    }
    
}
*/

