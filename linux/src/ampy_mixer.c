
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
#include "log.h"
#include "string_helpers.h"

#define STR_LEN 256

//volatile sig_atomic_t keep_going = 1;

int view_mode = VIEW_MIXER;
const char view_mode_name[2][10] = { "mixer", "power amp" };
const char card_name[2][22] = { "Ampy audio mixer REV1", "Ampy power amp REV2" };
const char chip_name[2][26] = { "Texas Instruments PGA2311", "Texas Instruments LM4780" };

// glue for sysdep1.c
int portfd_is_socket = 0;
int portfd_is_connected = 0;

unsigned int rx_err = 0;
unsigned int tx_err = 0;
unsigned int tx_inval = 0;

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
        stty_device = DFL_PORT;
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
    // 9600, 8N1, no hardware flow control, no software flow control
    m_setparms(*fd_dev, "9600", "N", "8", "1", 0, 0);

    return EXIT_SUCCESS;
}

int fd_read_ready(int fd_dev, struct timeval* timeout)
{
    fd_set read_fd_set;

    FD_ZERO(&read_fd_set);
    FD_SET(fd_dev, &read_fd_set);

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

int ampy_tx_cmd(int *fd_dev, char *tx_buff, uint8_t tx_buff_len, char *rx_buff, uint8_t *rx_buff_len, const uint8_t exp_rx_buff_len, const uint8_t retries)
{
    fd_set write_fd_set;
    struct timeval timeout;
    char buff;
    uint8_t fail;
    uint8_t keep_reading;
    uint8_t rx_count;


    if (*fd_dev < 0) {
        if (stty_init(stty_device, fd_dev) == EXIT_FAILURE) {
            printf("error: ampy hardware not connected\n");
            log_write("ee hardware not connected\n");
            exit(EXIT_FAILURE);
        }
    }

    FD_ZERO(&write_fd_set);
    FD_SET(*fd_dev, &write_fd_set);

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    flush_fd(*fd_dev);

    switch (select(*fd_dev + 1, NULL, &write_fd_set, NULL, &timeout)) {
    case -1:
        fprintf(stderr, "select() error during write\n");
        log_write("ee select() error during write\n");
        return EXIT_FAILURE;
        break;
    case 0:
        fprintf(stderr, "select() timeout during write\n");
        log_write("ee select() error during write\n");
        return EXIT_FAILURE;
        break;
    case 1:
        break;
    }

    fail = 0;

    // a full reply should be received in about 
    //   60ms for a wired ftdi connection
    while (fail<retries) {
        //signal(SIGALRM, catch_alarm);
        //alarm(1);
        log_write("tx %s", tx_buff);
        if (write(*fd_dev, tx_buff, tx_buff_len) != tx_buff_len) {
            fail++;
            tx_err++;
            log_write("ee tx error #%d, fail %d/%d\n", tx_err, fail, retries);
            usleep(80000);
        } else {
            // when reading from a wireless serial connection (like bluetooth)
            // the select function returns success only about 100ms after the reply is expected
            timeout.tv_sec = 0;
            timeout.tv_usec = 200000;
            if (fd_read_ready(*fd_dev, &timeout) == EXIT_SUCCESS) {
                rx_count = 0;
                keep_reading = 1;
                while (keep_reading) {
                    if (read(*fd_dev, &buff, 1) == 1) {
                        if (rx_count > (STR_LEN - 2)) {
                            keep_reading = 0;
                        } else if (buff == '\n') {
                            keep_reading = 0;
                            rx_buff[rx_count] = buff;
                            rx_count++;
                        } else {
                            rx_buff[rx_count] = buff;
                            rx_count++;
                        }
                    }
                }

                rx_buff[rx_count] = 0; // terminate string

                // what we get back must end with 'ok\r\n'
                if (((exp_rx_buff_len != 0) && (rx_count != exp_rx_buff_len)) || 
                        (rx_buff[rx_count-4] != 'o') || (rx_buff[rx_count-3] != 'k')) {
                    //printf("[%c %c %d %d]", rx_buff[rx_count-4], rx_buff[rx_count-3], rx_count, exp_rx_buff_len);
                    fail++;
                    tx_inval++;
                    log_write("rx tx_invalid fail %d/%d msg=%s", fail, retries, rx_buff);
                    usleep(100000);
                } else {
                    *rx_buff_len = rx_count;
                    log_write("rx %s", rx_buff);
                    return EXIT_SUCCESS;
                }
            } else {
                fail++;
                rx_err++;
                log_write("ee rx error #%d, fail %d/%d\n", rx_err, fail, retries);
                usleep(80000);
            }
        }
    }

    return EXIT_FAILURE;
}

int get_sensors(int *fd_dev)
{
    char input[STR_LEN];
    uint8_t input_len;

    if (ampy_tx_cmd(fd_dev, "sensors\r\n", 9, input, &input_len, 0, 20) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    fprintf(stdout, "%s\n", input);

    return EXIT_SUCCESS;
}

int get_mixer_values(int *fd_dev)
{
    char input[STR_LEN];
    uint8_t input_len;
    uint8_t i;


    if (ampy_tx_cmd(fd_dev, "showreg\r\n", 12, input, &input_len, 41, 20) == EXIT_FAILURE) {
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
    uint8_t hash;
    char hash_str[6];

    snprintf(str_temp, STR_LEN-6, "v%02x%02x%02x%02x", pga_id, mute, right, left);
    compute_xor_hash(str_temp, 9, &hash);
    snprintf(hash_str, 6, "*%02x\r\n", hash);
    strncat(str_temp, hash_str, 6);

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 17, 10) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int set_amp_registers(int *fd_dev, const uint8_t ver, const uint8_t snd_detect, const uint8_t mute_flag)
{
    char str_temp[STR_LEN];
    char input[STR_LEN];
    uint8_t input_len;
    uint8_t hash;
    char hash_str[6];

    snprintf(str_temp, STR_LEN, "a%02x%02x%02x", ver, snd_detect, mute_flag);
    compute_xor_hash(str_temp, 7, &hash);
    snprintf(hash_str, 6, "*%02x\r\n", hash);
    strncat(str_temp, hash_str, 6);

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 15, 10) == EXIT_FAILURE) {
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

    if (ampy_tx_cmd(fd_dev, str_temp, strlen(str_temp), input, &input_len, 4, 4) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

