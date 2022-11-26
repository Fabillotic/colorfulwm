#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include "colorful.h"
#include "clients.h"

CLIENT *clients;

void scan_clients() {
	CLIENT *client;
	Window *children;
	Window r_root;
	Window r_parent;
	int n_children;
	int i;
	char *name;
	
	XQueryTree(display, root, &r_root, &r_parent, &children, &n_children);
	for(i = 0; i < n_children; i++) {
		client = create_client(children[i]);
		
		printf("Found client! id: %d, title: '%s', x: %d, y: %d, w: %d, h: %d, bw: %d, or: %d\n", client->window, client->title, client->x, client->y, client->width, client->height, client->border_width, client->override_redirect);
	}
}

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
	free(client);
}

void update_client(CLIENT *client) {
	XWindowAttributes att = (XWindowAttributes) {};
	
	if(client->title) XFree(client->title);
	XFetchName(display, client->window, &client->title);
	
	XGetWindowAttributes(display, client->window, &att);
	
	client->x = att.x;
	client->y = att.y;
	client->width = att.width;
	client->height = att.height;
	client->border_width = att.border_width;
	client->override_redirect = att.override_redirect;
}
