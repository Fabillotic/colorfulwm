#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include "logger.h"
#include "colorful.h"
#include "clients.h"

CLIENT *clients;

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
}

void configure_client(CLIENT *client, unsigned int value_mask, XWindowChanges *values) {
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
