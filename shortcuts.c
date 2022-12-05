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
	create_keyboard_shortcut(false, XKeysymToKeycode(display, XK_f), Mod1Mask, shortcut_toggle_floating);
	create_keyboard_shortcut(true, XKeysymToKeycode(display, XK_d), Mod1Mask, shortcut_spawn_dmenu);
}

void create_shortcut(bool global, bool is_button, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int)) {
	SHORTCUT *sc, *tmp;
	
	sc = malloc(sizeof(SHORTCUT));
	sc->next = NULL;
	sc->global = global;
	sc->is_button = is_button;
	sc->detail = detail;
	sc->state = state;
	sc->callback = callback;
	
	for(tmp = shortcuts; tmp && tmp->next; tmp = tmp->next);
	if(tmp) tmp->next = sc;
	else shortcuts = sc;
}

void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int)) {
	create_shortcut(false, true, button, state, callback);
}

void create_keyboard_shortcut(bool global, unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int)) {
	create_shortcut(global, false, key, state, callback);
}
