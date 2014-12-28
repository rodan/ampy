#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <panel.h>

#define WIDGET_BORDER		0x1
#define WIDGET_SUBWINDOW	0x2
#define WIDGET_CURSOR_VISIBLE	0x4

#define SCREEN_CENTER	-1

int ncurses_init(void);
void ncurses_mainloop(void);

struct widget {
	WINDOW *window;
	WINDOW *subwindow; /* optional: contents without border */
	PANEL *panel;
	int cursor_visibility;

	void (*handle_key)(int key);
	void (*window_size_changed)(void);
	void (*close)(void);
};

extern struct widget mixer_widget;
extern int screen_lines;
extern int screen_cols;
extern int focus_control_index;

void change_control_to_percent(int ctrl, int value, unsigned int channels);
void change_control_relative(int ctrl, int delta, unsigned int channels);
void toggle_mute(int ctrl);

#endif
