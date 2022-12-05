#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "clients.h"
#include "colorful.h"
#include "colorful_shortcuts.h"
#include "shortcuts.h"

SHORTCUT *shortcuts;

void init_shortcuts() {
	create_button_shortcut(Button1, Mod1Mask, shortcut_move_client);
	create_button_shortcut(Button3, Mod1Mask, shortcut_resize_client);
	create_keyboard_shortcut(XKeysymToKeycode(display, XK_f), Mod1Mask, shortcut_toggle_floating);
}

void create_shortcut(bool is_button, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int)) {
	SHORTCUT *sc, *tmp;
	
	sc = malloc(sizeof(SHORTCUT));
	sc->next = NULL;
	sc->is_button = is_button;
	sc->detail = detail;
	sc->state = state;
	sc->callback = callback;
	
	for(tmp = shortcuts; tmp && tmp->next; tmp = tmp->next);
	if(tmp) tmp->next = sc;
	else shortcuts = sc;
}

void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int)) {
	create_shortcut(true, button, state, callback);
}

void create_keyboard_shortcut(unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int)) {
	create_shortcut(false, key, state, callback);
}
