
#include <stdlib.h>
#include <ctype.h>
#include <curses.h>
#include <panel.h>
#include <inttypes.h>
#include <signal.h>

#include "widget.h"
#include "pga2311_helper.h"
#include "amp_mixer.h"
#include "proj.h"
#include "version.h"
#include "colors.h"

WINDOW *curses_initialized;

static void finish(int sig);
static void resize(int sig);

volatile uint8_t first_run = 1;

int screen_lines;
int screen_cols;

static int cursor_visibility = -1;

static void on_handle_key(int key);
static void on_window_size_changed(void);
static void on_close(void);
static void create_mixer_widget(void);

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

void ncurses_main_w(void)
{
    struct widget *active_widget;
    int key;

    for (;;) {
		update_panels();
		doupdate();

		active_widget = get_active_widget();
		if (!active_widget)
			break;

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
        /*
		if (controls_changed) {
			controls_changed = FALSE;
			create_controls();
			control_values_changed = FALSE;
			display_controls();
		} else if (control_values_changed) {
			control_values_changed = FALSE;
			display_controls();
		}
        */

    }


    /*
    uint8_t i;
    int ch;

    while ((first_run) || ((ch = getch()) != 27)) {

        first_run = 0;

        get_mixer_values(&fd_device);
        attrset(COLOR_PAIR(5));
        mvprintw(5, 10, "ampy mixer build #%d", BUILD);

        for (i = 1; i < 6; i++) {
            mvprintw(2 * i + 7, 10, " %s  %3d %3d %s\n",
                     ch_name[i - 1],
                     mixer_get_vol_struct(i, CH_RIGHT),
                     mixer_get_vol_struct(i, CH_LEFT),
                     mixer_get_mute_struct(i) ? "live" : "mute");
        }

        mvprintw(19, 10, " %s  %3d   %s\n",
                 ch_name[5],
                 mixer_get_vol_struct(i, CH_RIGHT),
                 mixer_get_mute_struct(6) ? "live" : "mute");
        mvprintw(20, 10, " %s  %3d   %s\n",
                 ch_name[6],
                 mixer_get_vol_struct(i, CH_LEFT),
                 mixer_get_mute_struct(6) ? "live" : "mute");

        refresh();
    }
    */

    finish(0);
}

int ncurses_init(void)
{

    if ((curses_initialized = initscr()) == NULL) {
        fprintf(stderr, "Error initializing ncurses.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, finish);
    signal(SIGWINCH, resize);

    noecho();
    cbreak();
    set_escdelay(100);
    //keypad(mainwin, TRUE);

    window_size_changed();
	init_colors(1);

    create_mixer_widget();

    first_run = 1;

    return EXIT_SUCCESS;
}

static void on_handle_key(int key)
{
	switch (key) {
	case 27:
	case KEY_CANCEL:
	case KEY_F(10):
		mixer_widget.close();
        finish(0);
		break;
	case KEY_F(1):
	case KEY_HELP:
	case 'H':
	case 'h':
	case '?':
        exit(1);
		break;
        /*
	case KEY_F(2):
	case '/':
		create_proc_files_list();
		break;
	case KEY_F(3):
		set_view_mode(VIEW_MODE_PLAYBACK);
		break;
	case KEY_F(4):
		set_view_mode(VIEW_MODE_CAPTURE);
		break;
	case KEY_F(5):
		set_view_mode(VIEW_MODE_ALL);
		break;
	case '\t':
		set_view_mode((enum view_mode)((view_mode + 1) % VIEW_MODE_COUNT));
		break;
	case KEY_F(6):
	case 'S':
	case 's':
		create_card_select_list();
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
			refocus_control();
		}
		break;
	case KEY_RIGHT:
	case 'N':
	case 'n':
		if (focus_control_index < controls_count - 1) {
			++focus_control_index;
			refocus_control();
		}
		break;
	case KEY_PPAGE:
		change_control_relative(5, LEFT | RIGHT);
		break;
	case KEY_NPAGE:
		change_control_relative(-5, LEFT | RIGHT);
		break;
#if 0
	case KEY_BEG:
	case KEY_HOME:
		change_control_to_percent(100, LEFT | RIGHT);
		break;
#endif
	case KEY_LL:
	case KEY_END:
		change_control_to_percent(0, LEFT | RIGHT);
		break;
	case KEY_UP:
	case '+':
	case 'K':
	case 'k':
	case 'W':
	case 'w':
		change_control_relative(1, LEFT | RIGHT);
		break;
	case KEY_DOWN:
	case '-':
	case 'J':
	case 'j':
	case 'X':
	case 'x':
		change_control_relative(-1, LEFT | RIGHT);
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		change_control_to_percent((key - '0') * 10, LEFT | RIGHT);
		break;
	case 'Q':
	case 'q':
		change_control_relative(1, LEFT);
		break;
	case 'Y':
	case 'y':
	case 'Z':
	case 'z':
		change_control_relative(-1, LEFT);
		break;
	case 'E':
	case 'e':
		change_control_relative(1, RIGHT);
		break;
	case 'C':
	case 'c':
		change_control_relative(-1, RIGHT);
		break;
	case 'M':
	case 'm':
		toggle_mute(LEFT | RIGHT);
		break;
	case 'B':
	case 'b':
	case '=':
		balance_volumes();
		break;
	case '<':
	case ',':
		toggle_mute(LEFT);
		break;
	case '>':
	case '.':
		toggle_mute(RIGHT);
		break;
	case ' ':
		toggle_capture(LEFT | RIGHT);
		break;
	case KEY_IC:
	case ';':
		toggle_capture(LEFT);
		break;
	case KEY_DC:
	case '\'':
		toggle_capture(RIGHT);
		break;
    */
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
	//init_mixer_layout();
	//display_card_info();
	//set_view_mode(view_mode);
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

static void resize(int sig)
{
    /*
    int nh, nw;

    // new screen size
    getmaxyx(stdscr, nh, nw);

    first_run = 1;
    */
    window_size_changed();
    //create_mixer_widget();
    doupdate();
    //refresh();
}
