
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <sys/select.h>
#include <fcntl.h>

#include "config.h"
#include "ampy_mixer.h"
#include "pga2311_helper.h"
#include "lm4780_helper.h"
#include "proj.h"
#include "widget.h"
#include "mixer_controls.h"
#include "sysdep1.h"

#define STR_LEN 256

//volatile sig_atomic_t keep_going = 1;

int view_mode = VIEW_MIXER;
const char view_mode_name[2][10] = { "mixer", "power amp" };
const char card_name[2][22] = { "Ampy audio mixer REV1", "Ampy power amp REV2" };
const char chip_name[2][26] = { "Texas Instruments PGA2311", "Texas Instruments LM4780" };

// glue for sysdep1.c
int portfd_is_socket = 0;
int portfd_is_connected = 0;

unsigned int tx_err = 0;

/*
void catch_alarm(int sig)
{
    keep_going = 0;
    exit(EXIT_SUCCESS);
}
*/

int stty_init(char *stty_device, int *fd_dev)
{
    int n = 0;

    if (stty_device == NULL) {
        stty_device = "/dev/ttyUSB0";
    }

#if defined(O_NDELAY) && defined(F_SETFL)
    *fd_dev = open(stty_device, O_RDWR|O_NDELAY|O_NOCTTY);
    if (*fd_dev >= 0) {
        // Cancel the O_NDELAY flag.
        n = fcntl(*fd_dev, F_GETFL, 0);
        fcntl(*fd_dev, F_SETFL, n & ~O_NDELAY);
    }
#else
    *fd_dev = open(stty_device, O_RDWR|O_NOCTTY);
#endif

    if (*fd_dev < 0) {
        fprintf(stderr, "error: cannot open %s\n", stty_device);
        return EXIT_FAILURE;
    }

    portfd_is_connected = 1;

    // port init
    m_setparms(*fd_dev, "9600", "N", "8", "1", 1, 0);

    return EXIT_SUCCESS;
}

int fd_read_ready(int fd_dev, struct timeval* timeout)
{
    fd_set read_fd_set;

    FD_ZERO(&read_fd_set);
    FD_SET(fd_dev, &read_fd_set);

    //timeout.tv_sec = 3;
    //timeout.tv_usec = 0;

    switch (select(fd_dev + 1, &read_fd_set, NULL, NULL, timeout)) {
    case -1:
        fprintf(stderr, "select() error during read\n");
        return EXIT_FAILURE;
        break;
    case 0:
        //fprintf(stderr, "select() timeout during read\n");
        return EXIT_FAILURE;
        break;
    case 1:
        return EXIT_SUCCESS;
        break;
    }

    return EXIT_FAILURE;
}

int ampy_tx_cmd(int *fd_dev, char *tx_buff, uint8_t tx_buff_len, char *rx_buff, uint8_t *rx_buff_len, uint8_t retries)
{
    fd_set write_fd_set;
    struct timeval timeout;
    char buff;
    uint8_t fail;
    uint8_t keep_reading;
    
    if (*fd_dev < 0) {
        if (stty_init(stty_device, fd_dev) == EXIT_FAILURE) {
            printf("error: ampy hardware not connected\n");
            exit(EXIT_FAILURE);
        }
    }

    FD_ZERO(&write_fd_set);
    FD_SET(*fd_dev, &write_fd_set);

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    switch (select(*fd_dev + 1, NULL, &write_fd_set, NULL, &timeout)) {
    case -1:
        fprintf(stderr, "select() error during write\n");
        return EXIT_FAILURE;
        break;
    case 0:
        fprintf(stderr, "select() timeout during write\n");
        return EXIT_FAILURE;
        break;
    case 1:
        break;
    }

    fail = 0;
    keep_reading = 1;

    while (fail<retries) {
        //signal(SIGALRM, catch_alarm);
        //alarm(1);
        if (write(*fd_dev, tx_buff, tx_buff_len) != tx_buff_len) {
            fail++;
            tx_err++;
        } else {
            *rx_buff_len = 0;
            timeout.tv_sec = 0;
            timeout.tv_usec = 10000;

            if (fd_read_ready(*fd_dev, &timeout) == EXIT_SUCCESS) {
                while (keep_reading) {
                    if (read(*fd_dev, &buff, 1) == 1) {
                        if ((buff == '\n') || *rx_buff_len > (STR_LEN - 2)) {
                            keep_reading = 0;
                        } else {
                            rx_buff[*rx_buff_len] = buff;
                            (*rx_buff_len)++;
                        }
                    }
                }
                rx_buff[*rx_buff_len] = 0; // terminate string
                return EXIT_SUCCESS;
            } else {
                fail++;
                tx_err++;
                usleep(10000);
            }
        }
    }

    return EXIT_FAILURE;
}

uint8_t extract_hex(char *str, uint8_t * rv)
{
    uint8_t i = 0;
    char *p = str;
    char c = *p;

    *rv = 0;

    while ((i < 2)
           && (((c > 47) && (c < 58)) || ((c > 96) && (c < 103))
               || ((c > 64) && (c < 71)))) {

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
        i++;
        //p++;
        c = *++p;
    }

    return i;
}

int get_mixer_values(int *fd_dev)
{
    char input[STR_LEN];
    uint8_t input_len;
    uint8_t i;


    if (ampy_tx_cmd(fd_dev, "showreg\r\n", 9, input, &input_len, 20) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // first 28 bytes are the s struct in hex, next 8 are the a struct
    for (i = 0; i < 14; i++) {
        extract_hex(input + (i * 2), (uint8_t *) (&s) + i);
    }

    for (i = 0; i < 4; i++) {
        extract_hex(input + (i * 2) + 28, (uint8_t *) (&a) + i);
    }

    amp[0] = ampy_get_detect(1);
    amp[1] = ampy_get_detect(2);
    amp[2] = ampy_get_mute(1);
    amp[3] = ampy_get_mute(2);
    amp[4] = ampy_get_status(1);
    amp[5] = ampy_get_status(2);

    return EXIT_SUCCESS;
}

int set_mixer_volume(int *fd_dev, const uint8_t pga_id, const uint8_t mute,
                     const uint8_t right, const uint8_t left)
{
    char str_temp[STR_LEN];
    char input[STR_LEN];
    uint8_t input_len;

    snprintf(str_temp, STR_LEN, "v%02x%02x%02x%02x\r\n", pga_id, mute, right,
             left);

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 10) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int set_amp_registers(int *fd_dev, const uint8_t ver, const uint8_t snd_detect, const uint8_t mute_flag)
{
    char str_temp[STR_LEN];
    char input[STR_LEN];
    uint8_t input_len;

    snprintf(str_temp, STR_LEN, "a%02x%02x%02x\r\n", ver, snd_detect, mute_flag);

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 10) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int store_registers(int *fd_dev, uint8_t type)
{
    char str_temp[STR_LEN];
    char input[STR_LEN];
    uint8_t input_len;

    if (type == VIEW_MIXER) {
        snprintf(str_temp, STR_LEN, "storemix\r\n");
    } else if (type == VIEW_AMP) {
        snprintf(str_temp, STR_LEN, "storeamp\r\n");
    } else {
        return EXIT_FAILURE;
    }

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 4) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

