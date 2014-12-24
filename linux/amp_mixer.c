
// tool that outputs the contents of the fm24* F-RAM chip
// it works in conjunction with the DEBUG_GPRS compilation option

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <panel.h>
#include "pga2311_helper.h"
#include "proj.h"

#include "widget.h"

#define STR_LEN 256

volatile sig_atomic_t keep_going = 1;

const char ch_name[7][11] = { "front     ",
    "rear      ",
    "line-in   ",
    "spdif     ",
    "f-r pan   ",
    "center    ",
    "subwoofer "
};

void catch_alarm(int sig)
{
    keep_going = 0;
    exit(EXIT_SUCCESS);
    //signal (sig, catch_alarm);
}

int stty_init(char *stty_device, int *fd_dev)
{
    char str_temp[STR_LEN];

    if (stty_device == NULL) {
        stty_device = "/dev/ttyUSB0";
    }

    /*
       stty -a -F /dev/ttyUSB0 # should look like

       speed 57600 baud; rows 0; columns 0; line = 0;
       intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>;
       eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R;
       werase = ^W; lnext = ^V; flush = ^O; min = 1; time = 5;
       -parenb -parodd cs8 hupcl -cstopb cread clocal -crtscts
       ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff
       -iuclc -ixany -imaxbel -iutf8
       -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
       -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt
       -echoctl -echoke
     */

    snprintf(str_temp, STR_LEN,
             "stty -F %s 9600 cs8 raw ignbrk -onlcr -iexten -echo -echoe -echok -echoctl -echoke time 5",
             stty_device);

    if (system(str_temp) == -1) {
        fprintf(stderr, "error: stty cannot be executed\n");
        return EXIT_FAILURE;
    }

    *fd_dev = open(stty_device, O_RDWR);
    if (*fd_dev < 0) {
        fprintf(stderr, "error: cannot open %s\n", stty_device);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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

int get_mixer_values(int *dev)
{
    char buff;
    char input[STR_LEN];
    uint8_t i;

    if (*dev < 0) {
        if (stty_init(stty_device, dev) == EXIT_FAILURE) {
            printf("err\n");
            return EXIT_FAILURE;
        }
    }

    signal(SIGALRM, catch_alarm);
    i = write(*dev, "st\r\n", 4);
    alarm(1);

    i = 0;
    keep_going = 1;

    while (keep_going) {
        if (read(*dev, &buff, 1) == 1) {
            if ((buff == '\n') || i > (STR_LEN - 2)) {
                keep_going = 0;
            } else {
                input[i] = buff;
                i++;
            }
            //printf("%c", buff);
            alarm(1);
        }
    }
    alarm(0);
    input[i] = 0;
    
    //printf("i %d %d %s\n", i, *dev, input);

    extract_hex(input, &famp_mute[0]);
    extract_hex(input + 2, &famp_mute[1]);
    for (i = 0; i < 14; i++) {
        extract_hex(input + (i * 2) + 4, (uint8_t *) (&s) + i);
    }

    return EXIT_SUCCESS;
}

int set_mixer_volume(int dev, const uint8_t pga_id, const uint8_t mute,
                     const uint8_t right, const uint8_t left)
{
    char str_temp[STR_LEN];

    if (dev < 0) {
        if (stty_init(stty_device, &dev) == EXIT_FAILURE) {
            printf("err\n");
            return EXIT_FAILURE;
        }
    }

    snprintf(str_temp, STR_LEN, "v%02x%02x%02x%02x\r\n", pga_id, mute, right,
             left);

    if (write(dev, str_temp, strlen(str_temp)) < 1) {
        return EXIT_FAILURE;
    }
    usleep(100);

    return EXIT_SUCCESS;
}

/*
int main_loop()
{
    //char *ttydevice;

    //ttydevice = getenv("dev");
    // if pipe is used, I need more than an empty output
    //setvbuf(stdout, NULL, _IONBF, 0);

    //ncurses_init();

	struct pollfd *pollfds = NULL;
	int nfds = 0, n;
	struct widget *active_widget;
	unsigned short revents;
	int key;
	int err;

	for (;;) {
		update_panels();
		doupdate();

		active_widget = get_active_widget();
		if (!active_widget)
			break;

		n = 1 + snd_mixer_poll_descriptors_count(mixer);
		if (n != nfds) {
			free(pollfds);
			nfds = n;
			pollfds = ccalloc(nfds, sizeof *pollfds);
			pollfds[0].fd = fileno(stdin);
			pollfds[0].events = POLLIN;
		}
		err = snd_mixer_poll_descriptors(mixer, &pollfds[1], nfds - 1);
		if (err < 0)
			fatal_alsa_error("cannot get poll descriptors", err);
		n = poll(pollfds, nfds, -1);
		if (n < 0) {
			if (errno == EINTR) {
				pollfds[0].revents = 0;
				doupdate(); // handle SIGWINCH
			} else {
				fatal_error("poll error");
			}
		}
		if (pollfds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
			break;
		if (pollfds[0].revents & POLLIN)
			--n;
		if (n > 0) {
			err = snd_mixer_poll_descriptors_revents(mixer, &pollfds[1], nfds - 1, &revents);
			if (err < 0)
				fatal_alsa_error("cannot get poll events", err);
			if (revents & (POLLERR | POLLNVAL))
				close_mixer_device();
			else if (revents & POLLIN)
				snd_mixer_handle_events(mixer);
		}
		key = wgetch(active_widget->window);
		while (key != ERR) {
#ifdef KEY_RESIZE
			if (key == KEY_RESIZE)
				window_size_changed();
			else
#endif
				active_widget->handle_key(key);
			active_widget = get_active_widget();
			if (!active_widget)
				break;
			key = wgetch(active_widget->window);
		}
		if (!active_widget)
			break;
		if (controls_changed) {
			controls_changed = FALSE;
			create_controls();
			control_values_changed = FALSE;
			display_controls();
		} else if (control_values_changed) {
			control_values_changed = FALSE;
			display_controls();
		}
	}
	free(pollfds);
    
    return EXIT_SUCCESS;
}
*/
