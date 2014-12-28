
#include <stdlib.h>
#include <ctype.h>
#include <curses.h>
#include <panel.h>
#include <inttypes.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

#include "widget.h"
#include "pga2311_helper.h"
#include "amp_mixer.h"
#include "proj.h"
#include "version.h"
#include "colors.h"
#include "mem.h"
#include "mixer_display.h"
#include "mixer_controls.h"

WINDOW *curses_initialized;

static void finish(int sig);

volatile uint8_t first_run = 1;

int screen_lines;
int screen_cols;

static int cursor_visibility = -1;
int focus_control_index;

static void on_handle_key(int key);
static void on_window_size_changed(void);
static void on_close(void);
static void create_mixer_widget(void);

bool control_values_changed;
bool controls_changed;

struct widget mixer_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

struct widget *get_active_widget(void)
{
	PANEL *active_panel;

	active_panel = panel_below(NULL);
	if (active_panel)
		return (void *) panel_userptr(active_panel);
	else
		return NULL;
}

void window_size_changed(void)
{
	PANEL *panel, *below;
	struct widget *widget;

	getmaxyx(stdscr, screen_lines, screen_cols);
	if (tigetflag("xenl") != 1 && tigetflag("am") != 1)
		--screen_lines;

	for (panel = panel_below(NULL); panel; panel = below) {
		below = panel_below(panel);
		widget = (void *) panel_userptr(panel);
		widget->window_size_changed();
	}
}

static void update_cursor_visibility(void)
{
	struct widget *active_widget;

	active_widget = get_active_widget();
	if (active_widget &&
	    active_widget->cursor_visibility != cursor_visibility) {
		cursor_visibility = active_widget->cursor_visibility;
		curs_set(cursor_visibility);
	}
}

void widget_init(struct widget *widget, int lines_, int cols, int y, int x,
		 chtype bkgd, unsigned int flags)
{
	WINDOW *old_window;

	if (y == SCREEN_CENTER)
		y = (screen_lines - lines_) / 2;
	if (x == SCREEN_CENTER)
		x = (screen_cols - cols) / 2;

	old_window = widget->window;
	widget->window = newwin(lines_, cols, y, x);
	if (!widget->window) {
		//fatal_error("cannot create window");
    }
	keypad(widget->window, TRUE);
	nodelay(widget->window, TRUE);
	leaveok(widget->window, !(flags & WIDGET_CURSOR_VISIBLE));
	wbkgdset(widget->window, bkgd);
	werase(widget->window);

	if (flags & WIDGET_BORDER)
		box(widget->window, 0, 0);
	if (flags & WIDGET_SUBWINDOW) {
		if (widget->subwindow)
			delwin(widget->subwindow);
		widget->subwindow = derwin(widget->window,
					   lines_ - 2, cols - 2, 1, 1);
		if (!widget->subwindow) {
			//fatal_error("cannot create subwindow");
        }
		wbkgdset(widget->subwindow, bkgd);
	}
	widget->cursor_visibility = !!(flags & WIDGET_CURSOR_VISIBLE);

	if (widget->panel) {
		replace_panel(widget->panel, widget->window);
	} else {
		widget->panel = new_panel(widget->window);
		if (!widget->panel) {
			//fatal_error("cannot create panel");
        }
		set_panel_userptr(widget->panel, widget);
	}

	//if (!widget->handle_key)
		widget->handle_key = on_handle_key;

	if (old_window)
		delwin(old_window);

	update_cursor_visibility();
}

void widget_free(struct widget *widget)
{
	if (widget->panel) {
		del_panel(widget->panel);
		widget->panel = NULL;
	}
	if (widget->subwindow) {
		delwin(widget->subwindow);
		widget->subwindow = NULL;
	}
	if (widget->window) {
		delwin(widget->window);
		widget->window = NULL;
	}

	update_cursor_visibility();
}

void ncurses_mainloop(void)
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
				//fatal_error("poll error");
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

    finish(0);
}

int ncurses_init(void)
{

    if ((curses_initialized = initscr()) == NULL) {
        fprintf(stderr, "Error initializing ncurses.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, finish);
    noecho();
    cbreak();
    set_escdelay(100);
    window_size_changed();
	init_colors(1);
    create_mixer_widget();
    first_run = 1;

    return EXIT_SUCCESS;
}

static void on_handle_key(int key)
{
	switch (key) {
	case KEY_F(3):
        view_mode = VIEW_MIXER;
        create_controls();
        display_card_info();
		display_controls();
        break;
	case KEY_F(4):
        view_mode = VIEW_AMP;
        create_controls();
        display_card_info();
		display_controls();
        break;
	case 27:
	case KEY_CANCEL:
	case KEY_F(10):
		mixer_widget.close();
        finish(0);
		break;
	case KEY_REFRESH:
	case 12:
	case 'L':
	case 'l':
		clearok(mixer_widget.window, TRUE);
		display_controls();
		break;
	case KEY_LEFT:
	case 'P':
	case 'p':
		if (focus_control_index > 0) {
			--focus_control_index;
            display_controls();
		}
		break;
	case KEY_RIGHT:
	case 'N':
	case 'n':
		if (focus_control_index < controls_count - 1) {
			++focus_control_index;
            display_controls();
		}
		break;
	case KEY_PPAGE:
		change_control_relative(focus_control_index, 5, CH_LEFT | CH_RIGHT);
		break;
	case KEY_NPAGE:
		change_control_relative(focus_control_index, -5, CH_LEFT | CH_RIGHT);
		break;
	case KEY_LL:
	case KEY_END:
		change_control_to_percent(focus_control_index, 0, CH_LEFT | CH_RIGHT);
		break;
	case KEY_UP:
	case '+':
	case 'K':
	case 'k':
	case 'W':
	case 'w':
		change_control_relative(focus_control_index, 2, CH_LEFT | CH_RIGHT);
		break;
	case KEY_DOWN:
	case '-':
	case 'J':
	case 'j':
	case 'X':
	case 'x':
		change_control_relative(focus_control_index, -2, CH_LEFT | CH_RIGHT);
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		change_control_to_percent(focus_control_index, (key - '0') * 10, CH_LEFT | CH_RIGHT);
		break;
	case 'Q':
	case 'q':
		change_control_relative(focus_control_index, 2, CH_LEFT);
		break;
	case 'Y':
	case 'y':
	case 'Z':
	case 'z':
		change_control_relative(focus_control_index, -2, CH_LEFT);
		break;
	case 'E':
	case 'e':
		change_control_relative(focus_control_index, 2, CH_RIGHT);
		break;
	case 'C':
	case 'c':
		change_control_relative(focus_control_index, -2, CH_RIGHT);
		break;
	case 'M':
	case 'm':
        toggle_mute(focus_control_index);
		break;
	case 'B':
	case 'b':
	case '=':
		//balance_volumes();
		break;
	case '<':
	case ',':
        change_control_relative(focus_control_index, -255, CH_LEFT);
		break;
	case '>':
	case '.':
        change_control_relative(focus_control_index, -255, CH_RIGHT);
		break;
    case 'r':
    case 'R':
        get_mixer_values(&fd_device);
        display_controls();
        break;
	}
}

static void on_window_size_changed(void)
{
	create_mixer_widget();
}

static void on_close(void)
{
	widget_free(&mixer_widget);
}

static void create_mixer_widget(void)
{
	static const char title[] = " Ampy Mixer ";

	widget_init(&mixer_widget, screen_lines, screen_cols, 0, 0,
		    attr_mixer_frame, WIDGET_BORDER);
	if (screen_cols >= (sizeof(title) - 1) + 2) {
		wattrset(mixer_widget.window, attr_mixer_active);
		mvwaddstr(mixer_widget.window, 0, (screen_cols - (sizeof(title) - 1)) / 2, title);
	}

	init_mixer_layout();
	display_card_info();
    display_view_mode();
}

static void finish(int sig)
{
  	if (curses_initialized) {
		clear();
		refresh();
		curs_set(1);
		endwin();
	}

    exit(0);
}

void change_control_to_percent(int ctrl, int value, unsigned int channels)
{
    float v_right = 2.55 * value;
    float v_left = 2.55 * value;

    mixer_set_vol_struct(ctrl+1, CH_RIGHT, (uint8_t) v_right);
    mixer_set_vol_struct(ctrl+1, CH_LEFT, (uint8_t) v_left);
    set_mixer_volume(&fd_device, ctrl+1, mixer_get_mute_struct(ctrl + 1), (uint8_t) v_right, (uint8_t) v_left);

	display_controls();
}

void change_control_relative(int ctrl, int delta, unsigned int channels)
{
    int v_left = mixer_get_vol_struct(ctrl + 1, CH_LEFT);
    int v_right = mixer_get_vol_struct(ctrl + 1, CH_RIGHT);
    int mute = mixer_get_mute_struct(ctrl + 1);

    if (channels & CH_LEFT) {
        if ((v_left < delta * -1) && (delta < 0)) {
            v_left = delta * -1;
        }
        if ((v_left > 255 - delta) && (delta > 0)) {
            v_left = 255 - delta;
        }
        v_left += delta;
    }

    if (channels & CH_RIGHT) {
        if ((v_right < delta * -1) && (delta < 0)) {
            v_right = delta * -1;
        }
        if ((v_right > 255 - delta) && (delta > 0)) {
            v_right = 255 - delta;
        }
        v_right += delta;
    }

    mixer_set_vol_struct(ctrl+1, CH_RIGHT, v_right);
    mixer_set_vol_struct(ctrl+1, CH_LEFT, v_left);
    set_mixer_volume(&fd_device, ctrl+1, mute, v_right, v_left);
	display_controls();
}

void toggle_mute(int ctrl)
{
    int v_left = mixer_get_vol_struct(ctrl + 1, CH_LEFT);
    int v_right = mixer_get_vol_struct(ctrl + 1, CH_RIGHT);
    int mute = mixer_get_mute_struct(ctrl + 1);

    if (mute) {
        mute = MUTE;
    } else {
        mute = UNMUTE;
    }

    mixer_set_mute_struct(ctrl + 1, mute);
    set_mixer_volume(&fd_device, ctrl+1, mute, v_right, v_left);
	display_controls();
}
