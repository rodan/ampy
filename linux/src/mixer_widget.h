#ifndef __MIXER_WIDGET_H__
#define __MIXER_WIDGET_H__

#include <stdbool.h>

extern struct widget mixer_widget;

extern int view_mode;

extern int focus_control_index;

extern bool control_values_changed;
extern bool controls_changed;

void create_mixer_widget(void);
void refocus_control(void);
void change_control_to_percent(int ctrl, int value, unsigned int channels);
void change_control_relative(int ctrl, int delta, unsigned int channels);
void toggle_mute(int ctrl);
void show_help(void);

#endif
