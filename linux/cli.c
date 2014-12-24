
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <inttypes.h>
#include "amp_mixer.h"
#include "cli.h"
#include "proj.h"
#include "pga2311_helper.h"

#include "widget.h"

uint8_t show_interface;

static void show_help(void)
{
    fprintf(stdout, "Usage: amp_mixer [OPTION]\n\n");
    fprintf(stdout,
            "Mandatory arguments to long options are mandatory for short options too.\n");
    fprintf(stdout,
            "  -h, --help              this help\n"
            "  -d, --device=NAME       stty device name\n"
            "  -v, --volume=pgaid,mute,vol_r,vol_l\n"
            "  -i, --interface         display ncurses interface\n"
            "  -s, --show              display current mixer settings\n");
}

void display_mixer_values(void)
{
    uint8_t i;
    fprintf(stdout, "Current mixer values:\n");

    for (i = 1; i < 6; i++) {
        fprintf(stdout, " %s  %d %d %s\n",
                ch_name[i - 1],
                mixer_get_vol_struct(i, CH_RIGHT),
                mixer_get_vol_struct(i, CH_LEFT),
                mixer_get_mute_struct(i) ? "live" : "mute");
    }
    fprintf(stdout, " %s  %d   %s\n",
            ch_name[5],
            mixer_get_vol_struct(i, CH_RIGHT),
            mixer_get_mute_struct(6) ? "live" : "mute");
    fprintf(stdout, " %s  %d   %s\n",
            ch_name[6],
            mixer_get_vol_struct(i, CH_LEFT),
            mixer_get_mute_struct(6) ? "live" : "mute");

}

static void parse_options(int argc, char *argv[])
{
    static const char short_options[] = "hsid:v:";
    static const struct option long_options[] = {
        {.name = "help",.val = 'h'},
        {.name = "device",.has_arg = 1,.val = 'd'},
        {.name = "volume",.has_arg = 1,.val = 'v'},
        {.name = "show",.val = 's'},
        {.name = "interface",.val = 'i'},
        {}
    };
    int option;
    uint8_t t_int[4];
    uint8_t i;

    show_interface = 0;

    while ((option = getopt_long(argc, argv, short_options,
                                 long_options, NULL)) != -1) {
        switch (option) {
        case '?':
        case 'h':
            show_help();
            exit(EXIT_SUCCESS);
            break;
        case 's':
            //fprintf(stdout, "fd_device: %d\n", fd_device);
            get_mixer_values(&fd_device);
            display_mixer_values();
            break;
        case 'd':
            stty_device = optarg;
            break;
        case 'i':
            show_interface = 1;
            break;
        case 'v':
            for (i = 0; i < 4; i++) {
                extract_hex(optarg, &t_int[i]);
                optarg += 2;
            }
            set_mixer_volume(fd_device, t_int[0], t_int[1], t_int[2], t_int[3]);
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
    parse_options(argc, argv);

    if (show_interface) {
        if (ncurses_init() == EXIT_SUCCESS) {
            ncurses_main_w();
        }
    }

    return 0;
}
