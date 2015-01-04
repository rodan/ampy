
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include "ampy_mixer.h"
#include "cli.h"
#include "proj.h"
#include "pga2311_helper.h"
#include "lm4780_helper.h"
#include "mixer_controls.h"
#include "mixer_widget.h"
#include "mainloop.h"
#include "sysdep1.h"

uint8_t show_interface;

int debug = 0;

void show_cli_help(void)
{
    fprintf(stdout, "Usage: ampy_mixer [OPTION]\n\n");
    fprintf(stdout,
            "Mandatory arguments to long options are mandatory for short options too.\n");
    fprintf(stdout,
            "  -h, --help              this help\n"
            "  -d, --device=NAME       stty device name\n"
            "  -v, --volume=pgaid,mute,vol_r,vol_l\n"
            "  -s, --show              display current mixer settings\n"
            "  -e, --debug             show extra info\n");
}

void display_mixer_values(void)
{
    uint8_t i;
    fprintf(stdout, "Current mixer values:\n");

    for (i = 1; i < 7; i++) {
        fprintf(stdout, " %s\t\t%3d %3d %s\n",
                controls[0][i - 1].name,
                mixer_get_vol_struct(i, CH_RIGHT),
                mixer_get_vol_struct(i, CH_LEFT),
                mixer_get_mute_struct(i) ? "live" : "mute");
    }
}

static void parse_options(int argc, char *argv[])
{
    static const char short_options[] = "hsed:v:";
    static const struct option long_options[] = {
        {.name = "help",.val = 'h'},
        {.name = "device",.has_arg = 1,.val = 'd'},
        {.name = "volume",.has_arg = 1,.val = 'v'},
        {.name = "show",.val = 's'},
        {.name = "debug",.val = 'e'}
    };
    int option;
    uint8_t t_int[4];
    uint8_t i;

    show_interface = 1;

    while ((option = getopt_long(argc, argv, short_options,
                                 long_options, NULL)) != -1) {
        switch (option) {
        case '?':
        case 'h':
            show_cli_help();
            exit(EXIT_SUCCESS);
            break;
        case 's':
            //fprintf(stdout, "fd_device: %d\n", fd_device);
            if (get_mixer_values(&fd_device) == EXIT_FAILURE) {
                exit(1);
            }
            display_mixer_values();
            show_interface = 0;
            break;
        case 'd':
            stty_device = optarg;
            break;
        case 'e':
            debug = 1;
            break;
        case 'v':
            for (i = 0; i < 4; i++) {
                extract_hex(optarg, &t_int[i]);
                optarg += 2;
            }
            set_mixer_volume(&fd_device, t_int[0], t_int[1], t_int[2], t_int[3]);
            show_interface = 0;
            break;
        default:
            fprintf(stderr, "unknown option: %c\n", option);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    if (!isatty(fileno(stdin)))
        return 0;

    fd_device = -1;

    memset(&s, 0, sizeof(s));
    memset(&a, 0, sizeof(a));
    memset(&amp, 0, 6);

    parse_options(argc, argv);

    if (show_interface) {
        get_mixer_values(&fd_device);
        initialize_curses(1);
	    create_mixer_widget();
	    mainloop();
        app_shutdown();
    }

    if (fd_device > 0) {
        flush_fd(fd_device);
        close(fd_device);
    }

    if (debug) {
        fprintf(stdout, "\nDebug info:\n");
        fprintf(stdout, " tx errors:\t%d\n", tx_err);
        fprintf(stdout, " rx errors:\t%d\n", rx_err);
        fprintf(stdout, " invalid data:\t%d\n", tx_inval);
    }

    return 0;
}
