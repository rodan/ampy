
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
#include "die.h"
#include "textbox.h"
#include "utils.h"
#include "mixer_widget.h"
#include "mixer_display.h"
#include "mixer_controls.h"

int focus_control_index;

static void on_handle_key(int key);
static void on_window_size_changed(void);
static void on_close(void);

bool control_values_changed;
bool controls_changed;

struct widget mixer_widget = {
	.handle_key = on_handle_key,
	.window_size_changed = on_window_size_changed,
	.close = on_close,
};

static void on_window_size_changed(void)
{
	create_mixer_widget();
}

static void on_close(void)
{
	widget_free(&mixer_widget);
}


void create_mixer_widget(void)
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

void show_help(void)
{
	const char *help[] = {
		"Esc        Exit",
		"F1 ? H     Help",
		"F3         Show mixer board controls",
		"F4         Show power amp controls",
		"R          Redraw screen",
		"",
		"Left       Move to the previous control",
		"Right      Move to the next control",
		"",
		"Up/Down    Change volume",
		"+ -        Change volume",
		"Page Up/Dn Change volume in bigger steps",
		"End        Set volume to 0%",
		"0-9        Set volume to 0%-90%",
		"Q W E      Increase left/both/right volumes",
		"Z X C      Decrease left/both/right volumes",
		"",
		"M          Toggle mute",
		"< >        Toggle left/right mute",
	};
	show_text(help, ARRAY_SIZE(help), "Help");
}

void refocus_control(void)
{

	display_controls();
}

void change_control_to_percent(int ctrl, int value, unsigned int channels)
{
    if (view_mode == VIEW_MIXER) {
        float v_right = 2.55 * value;
        float v_left = 2.55 * value;

        mixer_set_vol_struct(ctrl+1, CH_RIGHT, (uint8_t) v_right);
        mixer_set_vol_struct(ctrl+1, CH_LEFT, (uint8_t) v_left);
        set_mixer_volume(&fd_device, ctrl+1, mixer_get_mute_struct(ctrl + 1), (uint8_t) v_right, (uint8_t) v_left);
   
    	display_controls();
    }
}

void change_control_relative(int ctrl, int delta, unsigned int channels)
{
    int v_left, v_right, mute;

    if (view_mode == VIEW_MIXER) {
        v_left = mixer_get_vol_struct(ctrl + 1, CH_LEFT);
        v_right = mixer_get_vol_struct(ctrl + 1, CH_RIGHT);
        mute = mixer_get_mute_struct(ctrl + 1);

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
}

void toggle_mute(int ctrl)
{
    int v_left, v_right, mute;

    if (!(controls[view_mode][ctrl].flags & IS_ACTIVE)) {
        return;
    }

    if (view_mode == VIEW_MIXER) {
        v_left = mixer_get_vol_struct(ctrl + 1, CH_LEFT);
        v_right = mixer_get_vol_struct(ctrl + 1, CH_RIGHT);
        mute = mixer_get_mute_struct(ctrl + 1);

        if (mute) {
            mute = MUTE;
        } else {
            mute = UNMUTE;
        }

        mixer_set_mute_struct(ctrl + 1, mute);
        mixer_mute[ctrl] = mute;
        set_mixer_volume(&fd_device, ctrl+1, mute, v_right, v_left);
    	display_controls();
    } else if (view_mode == VIEW_AMP) {
        if (*controls[VIEW_AMP][ctrl].pswitch == UNMUTE) {
            *controls[VIEW_AMP][ctrl].pswitch = MUTE;
        } else {
            *controls[VIEW_AMP][ctrl].pswitch = UNMUTE;
        }
        if (focus_control_index == 0) {
            // send activation
        }

        display_controls();
    }
}

static void on_handle_key(int key)
{
	switch (key) {
    case KEY_F(1):
    case 'H':
    case 'h':
    case '?':
        show_help();
        break;
	case KEY_F(3):
        view_mode = VIEW_MIXER;
        focus_control_index = 0;
        create_controls();
        display_card_info();
		display_controls();
        break;
	case KEY_F(4):
        view_mode = VIEW_AMP;
        focus_control_index = 0;
        create_controls();
        display_card_info();
		display_controls();
        break;
	case 27:
	case KEY_CANCEL:
	case KEY_F(10):
		mixer_widget.close();
        //app_shutdown();
		break;
	case KEY_REFRESH:
	case 12:
	case 'L':
	case 'l':
	case 'R':
	case 'r':
        get_mixer_values(&fd_device);
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
/*
	case 'B':
	case 'b':
	case '=':
		//balance_volumes();
		break;
*/
	case '<':
	case ',':
        change_control_relative(focus_control_index, -255, CH_LEFT);
		break;
	case '>':
	case '.':
        change_control_relative(focus_control_index, -255, CH_RIGHT);
		break;
	}
}

