#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "logger.h"
#include "clients.h"
#include "colorful.h"
#include "shortcuts.h"

SHORTCUT *shortcuts;

/* From colorful.c */
void spawn(char **cmd);
void arrange_all_clients();

void init_shortcuts() {
	create_button_shortcut(Button1, Mod1Mask, shortcut_move_client);
	create_button_shortcut(Button3, Mod1Mask, shortcut_resize_client);
	create_keyboard_shortcut(false, XKeysymToKeycode(display, XK_f), Mod1Mask, shortcut_toggle_floating);
	create_keyboard_shortcut(false, XKeysymToKeycode(display, XK_m), Mod1Mask, shortcut_move_client); /* Should not work! Just left here for testing */
	create_keyboard_shortcut(true, XKeysymToKeycode(display, XK_d), Mod1Mask, shortcut_spawn_dmenu);
	create_keyboard_shortcut(true, XKeysymToKeycode(display, XK_q), ShiftMask | Mod1Mask, shortcut_shutdown);
}

void create_shortcut(bool global, bool is_button, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool)) {
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

void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool)) {
	create_shortcut(false, true, button, state, callback);
}

void create_keyboard_shortcut(bool global, unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool)) {
	create_shortcut(global, false, key, state, callback);
}

void shortcut_move_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button) {
	XEvent event;
	int x, y;
	
	if(!client) {
		log_print(ERROR, "shortcut_move_client has to be registered non-globally!\n");
		return;
	}
	if(!is_button) {
		log_print(ERROR, "shortcut_move_client only supports button action!\n");
		return;
	}
	if(!client->floating) return;
	
	x = client->x - x_root;
	y = client->y - y_root;
	
	XGrabPointer(display, client->window, False, ButtonReleaseMask | PointerMotionMask, GrabModeSync, GrabModeSync, None, XCreateFontCursor(display, XC_fleur), CurrentTime);
	while(true) {
		XAllowEvents(display, SyncPointer, CurrentTime);
		XMaskEvent(display, ButtonReleaseMask | PointerMotionMask, &event);
		if(event.type == MotionNotify) {
			move_client(client, x + event.xmotion.x_root, y + event.xmotion.y_root);
		}
		else if(event.type == ButtonRelease && (event.xbutton.state & state) && (event.xbutton.button == detail)) {
			break;
		}
	}
	XUngrabPointer(display, CurrentTime);
}

void shortcut_resize_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button) {
	XEvent event;
	int x, y, w, h, nw, nh;
	
	if(!client) {
		log_print(ERROR, "shortcut_resize_client has to be registered non-globally!\n");
		return;
	}
	if(!is_button) {
		log_print(ERROR, "shortcut_resize_client only supports button action!\n");
		return;
	}
	if(!client->floating) return;
	
	x = x_root;
	y = y_root;
	w = client->width;
	h = client->height;
	
	XGrabPointer(display, client->window, False, ButtonReleaseMask | PointerMotionMask, GrabModeSync, GrabModeSync, None, XCreateFontCursor(display, XC_sizing), CurrentTime);
	while(true) {
		XAllowEvents(display, SyncPointer, CurrentTime);
		XMaskEvent(display, ButtonReleaseMask | PointerMotionMask, &event);
		if(event.type == MotionNotify) {
			nw = w + event.xmotion.x_root - x;
			nh = h + event.xmotion.y_root - y;
			
			if(nw < client->min_width) nw = client->min_width;
			if(nh < client->min_height) nh = client->min_height;
			if(client->max_width > 0 && nw > client->max_width) nw = client->max_width;
			if(client->max_height > 0 && nh > client->max_height) nh = client->max_height;
			
			nw -= client->base_width;
			nh -= client->base_height;
			
			nw -= nw % client->inc_width;
			nh -= nh % client->inc_height;
			
			nw += client->base_width;
			nh += client->base_height;
			
			resize_client(client, nw, nh);
		}
		else if(event.type == ButtonRelease && (event.xbutton.state & state) && (event.xbutton.button == detail)) {
			break;
		}
	}
	XUngrabPointer(display, CurrentTime);
}

void shortcut_toggle_floating(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button) {
	if(!client) {
		log_print(ERROR, "shortcut_toggle_floating has to be registered non-globally!\n");
		return;
	}
	
	client->floating = !client->floating;
	XRaiseWindow(display, client->window);
	
	if(client->floating) {
		frame_client(client);
	}
	else {
		unframe_client(client);
	}
	arrange_all_clients();
}

void shortcut_spawn_dmenu(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button) {
	char* cmd[] = {"dmenu_run", NULL};
	spawn(cmd);
}

void shortcut_shutdown(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button) {
	close(ConnectionNumber(display));
	exit(0);
}
