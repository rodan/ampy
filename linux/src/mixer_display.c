/*
 * mixer_display.c - handles displaying of mixer widget and controls
 * Copyright (c) 1874 Lewis Carroll
 * Copyright (c) 2009 Clemens Ladisch <clemens@ladisch.de>
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

#define _C99_SOURCE             /* lrint() */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <ncurses.h>
#include "utils.h"
#include "mem.h"
#include "colors.h"
#include "widget.h"
#include "widget.h"
#include "mixer_display.h"
#include "mixer_controls.h"
#include "mixer_widget.h"
#include "pga2311_helper.h"
#include "ampy_mixer.h"

enum align {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
};

static bool screen_too_small;
static bool has_info_items;

static int info_items_left;
static int info_items_width;

static int visible_controls;
static int first_visible_control_index;
static int first_control_x;
static int control_width;
static int control_name_width;

static int base_y;
static int volume_height;
static int cswitch_y;
static int values_y;
static int name_y;
static int channel_name_y;

static void display_string_in_field(int y, int x, const char *s, int width,
                                    enum align align)
{
    int string_width;
    const char *s_end;
    int spaces;
    int cur_y, cur_x;

    wmove(mixer_widget.window, y, x);
    string_width = width;
    s_end = mbs_at_width(s, &string_width, -1);
    if (string_width >= width) {
        waddnstr(mixer_widget.window, s, s_end - s);
    } else {
        if (align != ALIGN_LEFT) {
            spaces = width - string_width;
            if (align == ALIGN_CENTER)
                spaces /= 2;
            if (spaces > 0)
                wprintw(mixer_widget.window, "%*s", spaces, "");
        }
        waddstr(mixer_widget.window, s);
        if (align != ALIGN_RIGHT) {
            getyx(mixer_widget.window, cur_y, cur_x);
            if (cur_y == y) {
                spaces = x + width - cur_x;
                if (spaces > 0)
                    wprintw(mixer_widget.window, "%*s", spaces, "");
            }
        }
    }
}

void display_card_info(void)
{

    wattrset(mixer_widget.window, attr_mixer_active);
    display_string_in_field(1, info_items_left, card_name[view_mode],
                            info_items_width, ALIGN_LEFT);

    wattrset(mixer_widget.window, attr_mixer_active);
    display_string_in_field(2, info_items_left, chip_name[view_mode],
                            info_items_width, ALIGN_LEFT);
}

void display_view_mode(void)
{
    unsigned int widths[3];
    int i;

    for (i = 0; i < 3; ++i)
        widths[i] = get_mbs_width(view_mode_name[i]);
    if (4 + widths[0] + 6 + widths[1] + 6 + widths[2] + 1 <= info_items_width) {
        wmove(mixer_widget.window, 3, info_items_left);
        wattrset(mixer_widget.window, attr_mixer_text);
        for (i = 0; i < 2; ++i) {
            wprintw(mixer_widget.window, "F%c:", '3' + i);
            if (view_mode == i) {
                wattrset(mixer_widget.window, attr_mixer_active);
                wprintw(mixer_widget.window, "[%s]", view_mode_name[i]);
                wattrset(mixer_widget.window, attr_mixer_text);
            } else {
                wprintw(mixer_widget.window, " %s ", view_mode_name[i]);
            }
            if (i < 2)
                waddch(mixer_widget.window, ' ');
        }
    } else {
        wattrset(mixer_widget.window, attr_mixer_active);
        display_string_in_field(3, info_items_left,
                                view_mode_name[view_mode],
                                info_items_width, ALIGN_LEFT);
    }
}

void init_mixer_layout(void)
{
    const char *labels_left[4] = {
        "Card:",
        "Chip:",
        "View:",
        "Item:"
    };
    const char *labels_right[2] = {
        "F1:  Help",
        "Esc: Exit"
    };
    unsigned int label_width_left, label_width_right;
    unsigned int right_x, i;

    screen_too_small = screen_lines < 14 || screen_cols < 12;
    has_info_items = screen_lines >= 6;
    if (!has_info_items)
        return;

    label_width_left = get_max_mbs_width(labels_left, 4);
    label_width_right = get_max_mbs_width(labels_right, 2);
    if (2 + label_width_left + 1 + 28 + label_width_right + 2 > screen_cols)
        label_width_right = 0;
    if (2 + label_width_left + 1 + 28 + label_width_right + 2 > screen_cols)
        label_width_left = 0;

    info_items_left = label_width_left ? 3 + label_width_left : 2;
    right_x = screen_cols - label_width_right - 2;
    info_items_width = right_x - info_items_left;
    if (info_items_width < 1) {
        has_info_items = FALSE;
        return;
    }

    wattrset(mixer_widget.window, attr_mixer_text);
    if (label_width_left)
        for (i = 0; i < 4; ++i)
            display_string_in_field(1 + i, 2, labels_left[i],
                                    label_width_left, ALIGN_RIGHT);
    if (label_width_right)
        for (i = 0; i < 2; ++i)
            display_string_in_field(1 + i, right_x, labels_right[i],
                                    label_width_right, ALIGN_LEFT);
}

static char *format_gain(uint8_t val)
{
    if (val) {
        return casprintf("%.2f", 31.5 - (0.5 * (255 - val)));
    } else {
        return cstrdup("mute");
    }
}

static void display_focus_item_info(void)
{
    struct control *control;
    char *dbs, *dbs2;
    char *value_info;
    char *item_info;

    if (!has_info_items)
        return;
    wattrset(mixer_widget.window, attr_mixer_active);
    if (!controls_count || screen_too_small) {
        display_string_in_field(4, info_items_left, "", info_items_width,
                                ALIGN_LEFT);
        return;
    }
    control = &controls[view_mode][focus_control_index];
    value_info = NULL;
    if (control->flags & TYPE_PVOLUME) {
        dbs = format_gain(*control->volume_left);
        dbs2 = format_gain(*control->volume_right);
        value_info = casprintf(" [dB gain: %s, %s]", dbs, dbs2);
        free(dbs);
        free(dbs2);
    } else if (control->flags & TYPE_PSWITCH) {
        value_info = casprintf(" [%s]", *control->pswitch ? "on" : "off");
    }
    item_info =
        casprintf("%s%s", control->full_name, value_info ? value_info : "");
    free(value_info);
    display_string_in_field(4, info_items_left, item_info, info_items_width,
                            ALIGN_LEFT);
    free(item_info);
}

static void clear_controls_display(void)
{
    int i;

    wattrset(mixer_widget.window, attr_mixer_frame);
    for (i = 5; i < screen_lines - 1; ++i)
        mvwprintw(mixer_widget.window, i, 1, "%*s", screen_cols - 2, "");
}

/*
static void center_string(int line, const char *s)
{
    int width = get_mbs_width(s);
    if (width <= screen_cols - 2)
        mvwaddstr(mixer_widget.window, line, (screen_cols - width) / 2, s);
}
*/

static void display_string_centered_in_control(int y, int col, const char *s,
                                               int width)
{
    int left, x;

    left = first_control_x + col * (control_width + 1);
    x = left + (control_width - width) / 2;
    display_string_in_field(y, x, s, width, ALIGN_CENTER);
}

static void display_control(unsigned int control_index)
{
    struct control *control;
    int col;
    int i, c;
    int left, frame_left;
    int bar_height;
    double volumes[2];

    control = &controls[view_mode][control_index];
    col = control_index - first_visible_control_index;
    left = first_control_x + col * (control_width + 1);
    frame_left = left + (control_width - 4) / 2;
    if (control->flags & IS_ACTIVE)
        wattrset(mixer_widget.window, attr_ctl_frame);
    else
        wattrset(mixer_widget.window, attr_ctl_inactive);
    if (control->flags & TYPE_PVOLUME) {

        // if volume control
        mvwaddch(mixer_widget.window, base_y - volume_height - 1, frame_left,
                 ACS_ULCORNER);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_URCORNER);
        for (i = 0; i < volume_height; ++i) {
            mvwaddch(mixer_widget.window, base_y - i - 1, frame_left,
                     ACS_VLINE);
            mvwaddch(mixer_widget.window, base_y - i - 1, frame_left + 3,
                     ACS_VLINE);
        }
        mvwaddch(mixer_widget.window, base_y, frame_left,
                 control->flags & TYPE_PSWITCH ? ACS_LTEE : ACS_LLCORNER);

        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window,
               control->flags & TYPE_PSWITCH ? ACS_RTEE : ACS_LRCORNER);
    } else if (control->flags & TYPE_PSWITCH) {
        mvwaddch(mixer_widget.window, base_y, frame_left, ACS_ULCORNER);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_URCORNER);
    }
    if (control->flags & TYPE_PSWITCH) {
        mvwaddch(mixer_widget.window, base_y + 1, frame_left, ACS_VLINE);
        mvwaddch(mixer_widget.window, base_y + 1, frame_left + 3, ACS_VLINE);
        mvwaddch(mixer_widget.window, base_y + 2, frame_left, ACS_LLCORNER);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_HLINE);
        waddch(mixer_widget.window, ACS_LRCORNER);
    }

    if (control->flags & TYPE_PVOLUME) {
        // volume bar
        volumes[0] = (*control->volume_left) / 255.0;
        volumes[1] = (*control->volume_right) / 255.0;

        for (c = 0; c < 2; c++) {
            bar_height = lrint(volumes[c] * volume_height);
            for (i = 0; i < volume_height; ++i) {
                chtype ch;
                if (i + 1 > bar_height)
                    ch = ' ' | attr_ctl_frame;
                else {
                    ch = ACS_CKBOARD;
                    if (i > volume_height * 8 / 10)
                        ch |= attr_ctl_bar_hi;
                    else if (i > volume_height * 4 / 10)
                        ch |= attr_ctl_bar_mi;
                    else
                        ch |= attr_ctl_bar_lo;
                }
                mvwaddch(mixer_widget.window, base_y - i - 1,
                         frame_left + c + 1, ch);
            }
        }

        wattrset(mixer_widget.window, attr_mixer_active);

        // print volume value
        mvwprintw(mixer_widget.window, values_y, frame_left - 2,
                  "%3d", (int)lrint(volumes[0] * 100));
        wattrset(mixer_widget.window, attr_ctl_frame);
        waddstr(mixer_widget.window, "<>");
        wattrset(mixer_widget.window, attr_mixer_active);
        wprintw(mixer_widget.window, "%-3d", (int)lrint(volumes[1] * 100));
    }

    if (control->flags & TYPE_PSWITCH) {
        // mute status
        wattrset(mixer_widget.window, 0);
        mvwaddch(mixer_widget.window, base_y + 1, frame_left + 1,
                 *control->
                 pswitch ? "O"[0] | attr_ctl_nomute : "M"[0] | attr_ctl_mute);
        waddch(mixer_widget.window,
               *control->
               pswitch ? "O"[0] | attr_ctl_nomute : "MM"[0] | attr_ctl_mute);
    }
    // channel names
    //wattrset(mixer_widget.window, attr_mixer_active);
    //display_string_centered_in_control(base_y + 4, col, ch_name[control_index], control_width);

    if (control_index == focus_control_index) {
        i = first_control_x + col * (control_width + 1) + (control_width -
                                                           control_name_width) /
            2;
        wattrset(mixer_widget.window, attr_ctl_mark_focus);
        mvwaddch(mixer_widget.window, name_y, i - 1, '<');
        mvwaddch(mixer_widget.window, name_y, i + control_name_width, '>');
        wattrset(mixer_widget.window, attr_ctl_label_focus);
    } else {
        wattrset(mixer_widget.window, attr_ctl_label);
    }

    display_string_centered_in_control(name_y, col,
                                       controls[view_mode][control_index].name,
                                       control_name_width);

    //wattrset(mixer_widget.window, attr_mixer_frame);
    //display_string_centered_in_control(channel_name_y, col, ch_name[control_index], control_name_width);
}

static void display_scroll_indicators(void)
{
    int y0, y1, y;
    chtype left, right;

    if (screen_too_small)
        return;
    y0 = screen_lines * 3 / 8;
    y1 = screen_lines * 5 / 8;
    left = first_visible_control_index > 0 ? ACS_LARROW : ACS_VLINE;
    right = first_visible_control_index + visible_controls < controls_count
        ? ACS_RARROW : ACS_VLINE;
    wattrset(mixer_widget.window, attr_mixer_frame);
    for (y = y0; y <= y1; ++y) {
        mvwaddch(mixer_widget.window, y, 0, left);
        mvwaddch(mixer_widget.window, y, screen_cols - 1, right);
    }
}

void display_controls(void)
{
    unsigned int i;

    compute_controls_layout();

    if (first_visible_control_index > controls_count - visible_controls)
        first_visible_control_index = controls_count - visible_controls;
    if (first_visible_control_index > focus_control_index)
        first_visible_control_index = focus_control_index;
    else if (first_visible_control_index <
             focus_control_index - visible_controls + 1 && visible_controls)
        first_visible_control_index =
            focus_control_index - visible_controls + 1;

    clear_controls_display();

    display_focus_item_info();

    if (controls_count > 0) {
        if (!screen_too_small) {
            for (i = 0; i < visible_controls; ++i) {
                display_control(first_visible_control_index + i);
            }
        }
        display_scroll_indicators();
    }
}

void compute_controls_layout(void)
{
    bool any_volume, any_pswitch;
    int max_width, name_len;
    int height, space;
    unsigned int i;

    if (controls_count == 0 || screen_too_small) {
        visible_controls = 0;
        return;
    }

    any_volume = TRUE;
    any_pswitch = TRUE;

    max_width = 8;
    for (i = 0; i < controls_count; ++i) {
        name_len = 8;
        if (name_len > max_width)
            max_width = name_len;
    }
    max_width = (max_width + 1) & ~1;

    control_width = (screen_cols - 3 - (int)controls_count) / controls_count;
    if (control_width < 8)
        control_width = 8;
    if (control_width > max_width)
        control_width = max_width;
    if (control_width > screen_cols - 4)
        control_width = screen_cols - 4;

    visible_controls = (screen_cols - 3) / (control_width + 1);
    if (visible_controls > controls_count)
        visible_controls = controls_count;

    first_control_x =
        2 + (screen_cols - 3 - visible_controls * (control_width + 1)) / 2;

    if (control_width < max_width)
        control_name_width = control_width;
    else
        control_name_width = max_width;

    height = 2;
    if (any_volume)
        height += 2;
    if (any_pswitch)
        height += 2;
    if (any_volume) {
        space = screen_lines - 6 - height;
        if (space <= 1)
            volume_height = 1;
        else if (space <= 10)
            volume_height = space;
        else
            volume_height = 10 + (space - 10) / 2;
        height += volume_height;
    }

    space = screen_lines - 6 - height;
    channel_name_y = screen_lines - 2 - space / 2;
    name_y = channel_name_y;
    values_y = name_y - any_volume;
    cswitch_y = values_y;
    base_y = cswitch_y - 1 - 2 * any_pswitch;
}

