#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "logger.h"
#include "clients.h"
#include "colorful.h"

/* Implemented through colorful.c, necessary for mapping the frame clients */
void map_request(XMapRequestEvent ev);

CLIENT *clients;
CLIENT *active;

CLIENT *get_client_by_window(Window window) {
	CLIENT *r;
	
	for(r = clients; r; r = r->next) {
		if(r->window == window) return r;
	}
	return NULL;
}

void add_client(CLIENT *client) {
	if(!clients) clients = client;
	else {
		client->next = clients;
		clients = client;
	}
}

CLIENT *create_client(Window window) {
	CLIENT *client = malloc(sizeof(CLIENT));
	client->window = window;
	client->sub = None;
	client->next = NULL;
	client->title = NULL;
	client->iconic = false;
	client->floating = false;
	
	update_client(client);
	add_client(client);
	
	return client;
}

void remove_client(CLIENT *client) {
	CLIENT *temp;
	if(client == clients) {
		clients = client->next;
	}
	else {
		for(temp = clients; temp; temp = temp->next) {
			if(temp->next == client) {
				temp->next = client->next;
			}
		}
	}
}

void delete_client(CLIENT *client) {
	remove_client(client);
	if(client->title) free(client->title);
	free(client);
}

void update_client(CLIENT *client) {
	XWindowAttributes att;
	XSizeHints hints;
	long supplied;
	
	if(client->title) XFree(client->title);
	XFetchName(display, client->window, &client->title);
	
	att = (XWindowAttributes) {};
	XGetWindowAttributes(display, client->window, &att);
	
	client->x = att.x;
	client->y = att.y;
	client->width = att.width;
	client->height = att.height;
	client->border_width = att.border_width;
	client->override_redirect = att.override_redirect;
	
	client->min_width = 0;
	client->min_height = 0;
	client->base_width = 0;
	client->base_height = 0;
	client->max_width = -1;
	client->max_height = -1;
	client->inc_width = 1;
	client->inc_height = 1;
	
	if(client->sub) XGetWMNormalHints(display, client->sub, &hints, &supplied);
	else XGetWMNormalHints(display, client->window, &hints, &supplied);
	if(supplied & PMinSize) {
		client->min_width = hints.min_width;
		client->min_height = hints.min_height;
	}
	if(supplied & PMaxSize) {
		client->max_width = hints.max_width;
		client->max_height = hints.max_height;
	}
	if(supplied & PResizeInc) {
		client->inc_width = hints.width_inc;
		client->inc_height = hints.height_inc;
	}
	if(supplied & PBaseSize) {
		client->base_width = hints.base_width;
		client->base_height = hints.base_height;
	}
	
	if((supplied & PMinSize) && !(supplied & PBaseSize)) {
		client->base_width = client->min_width;
		client->base_height = client->min_height;
	}
	else if(!(supplied & PMinSize) && (supplied & PBaseSize)) {
		client->min_width = client->base_width;
		client->min_height = client->base_height;
	}
	
	log_print(DEBUG, "inc_width: %d, inc_height: %d\n", client->inc_width, client->inc_height);
}

void configure_client(CLIENT *client, unsigned int value_mask, XWindowChanges *values) {
	XWindowChanges sub_values;
	
	if(!client->override_redirect) {
		if(value_mask & CWX) {
			client->x = values->x;
		}
		else if(value_mask & CWY) {
			client->y = values->y;
		}
		else if(value_mask & CWWidth) {
			client->width = values->width;
		}
		else if(value_mask & CWHeight) {
			client->height = values->height;
		}
		else if(value_mask & CWBorderWidth) {
			client->border_width = values->border_width;
		}
		
		XConfigureWindow(display, client->window, value_mask, values);
		XSync(display, False);
		
		if(client->sub != None && value_mask & (CWWidth | CWHeight)) {
			sub_values = (XWindowChanges) {.width = values->width - 20, .height = values->height - 20};
			XConfigureWindow(display, client->sub, value_mask & (CWWidth | CWHeight), &sub_values);
			XSync(display, False);
		}
	}
	else log_print(WARN, "An attempt was made to configure an override redirect window!\n");
}

void move_client(CLIENT *client, int x, int y) {
	XWindowChanges wc;
	
	wc = (XWindowChanges) {.x = x, .y = y};
	configure_client(client, CWX | CWY, &wc);
}

void resize_client(CLIENT *client, int width, int height) {
	XWindowChanges wc;
	
	wc = (XWindowChanges) {.width = width, .height = height};
	configure_client(client, CWWidth | CWHeight, &wc);
}

void frame_client(CLIENT *client) {
	CLIENT *nclient;
	Window window, nwindow;
	XMapRequestEvent map_event;
	XUnmapEvent unmap_event;
	bool floating;
	int x, y, w, h;
	
	floating = client->floating;
	x = client->x;
	y = client->y;
	w = client->width;
	h = client->height;
	window = client->window;
	
	nwindow = XCreateSimpleWindow(display, root, x, y, w + 20, h + 20, client->border_width, 0, 0);
	
	XReparentWindow(display, client->window, nwindow, 10, 10);
	XSync(display, False);
	
	map_event = (XMapRequestEvent) {.type = MapRequest, .serial = 0, .send_event = False, .display = display, .parent = root, .window = nwindow};
	map_request(map_event);
	
	nclient = get_client_by_window(nwindow);
	nclient->floating = floating;
	nclient->sub = window;
	
	move_client(nclient, x, y);
	resize_client(nclient, w, h);
	
	update_client(nclient);
}

void unframe_client(CLIENT *client) {
	log_print(DEBUG, "Unframing client!\n");
	if(client->sub != None) {
		XReparentWindow(display, client->sub, root, client->x, client->y);
		XMapWindow(display, client->sub);
		XDestroyWindow(display, client->window);
		XSync(display, False);
		
		client->window = client->sub;
		client->sub = None;
	}
}

void focus_client(CLIENT *client, bool raise_window) {
	active = client;
	
	if(!client) {
		XSetInputFocus(display, None, RevertToNone, CurrentTime);
		return;
	}
	
	if(client->sub) XSetInputFocus(display, client->sub, RevertToNone, CurrentTime);
	else XSetInputFocus(display, client->window, RevertToNone, CurrentTime);
	
	if(raise_window) XRaiseWindow(display, client->window);
}
