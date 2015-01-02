/*
 * mainloop.c - main loop
 * Copyright (c) Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <panel.h>
#include <unistd.h>
#include "mem.h"
#include "die.h"
#include "colors.h"
#include "widget.h"
#include "mixer_widget.h"
#include "mixer_display.h"
#include "mixer_controls.h"
#include "mainloop.h"
#include "proj.h"

WINDOW *curses_initialized;

void initialize_curses(bool use_color)
{
    curses_initialized = initscr();

    //signal(SIGINT, finish);
    cbreak();
    noecho();
    set_escdelay(100);
    window_size_changed();
	init_colors(use_color);
    //create_mixer_widget();
    //first_run = 1;
}

void app_shutdown(void)
{
  	if (curses_initialized) {
		clear();
		refresh();
		curs_set(1);
		endwin();
	}
}

void mainloop()
{
    struct widget *active_widget;
    int key;

    struct pollfd *pollfds = NULL;
    int nfds = 0, n;

    create_controls();
    display_controls();

    for (;;) {
		update_panels();
		doupdate();

		active_widget = get_active_widget();
		if (!active_widget)
			break;

        n = 1;
		if (n != nfds) {
			free(pollfds);
			nfds = n;
			pollfds = ccalloc(nfds, sizeof *pollfds);
			pollfds[0].fd = fileno(stdin);
			pollfds[0].events = POLLIN;
		}

		n = poll(pollfds, nfds, -1);
		if (n < 0) {
			if (errno == EINTR) {
				pollfds[0].revents = 0;
                controls_changed = TRUE;
				doupdate(); /* handle SIGWINCH */
			} else {
				fatal_error("poll error");
			}
		}
		if (pollfds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
			break;
		if (pollfds[0].revents & POLLIN)
			--n;

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
}

