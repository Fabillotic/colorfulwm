#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include "colorful.h"

#define MAX_ERR_LEN 200

Display *display;
Window root;
int (*xlib_err)(Display *, XErrorEvent *);

CLIENT *clients;


int main();
void run();
void scan_clients();
CLIENT *get_client_by_window(Window window);
CLIENT* create_client(Window window);
void update_client(CLIENT *client);
void maprequest(XMapRequestEvent ev);
void configure_request(XConfigureRequestEvent ev);
int catch_error(Display *d, XErrorEvent *e);
void check_other_wm();
int other_wm_error(Display *d, XErrorEvent *e);

int main() {
	printf("Welcome to colorfulwm!\n");
	printf("Connecting to X11...\n");
	display = XOpenDisplay(NULL);
	if(!display) {
		printf("Connection failed!\n");
		return -1;
	}
	
	printf("Gettting root window\n");
	root = XDefaultRootWindow(display);
	
	/* Find default Xlib error handler */
	printf("Finding default error handler\n");
	xlib_err = XSetErrorHandler(catch_error);
	XSetErrorHandler(xlib_err);
	
	/* Register SubstructureRedirect on root */
	check_other_wm();
	
	printf("Scanning for clients\n");
	clients = NULL;
	scan_clients();
	
	run();
	
	return 0;
}

void run() {
	XEvent e;
	while(true) {
		while(XPending(display)) {
			XNextEvent(display, &e);
			if(e.type == MapRequest) maprequest(e.xmaprequest);
		}
	}
}

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

CLIENT *create_client(Window window) {
	CLIENT *client = malloc(sizeof(CLIENT));
	client->window = window;
	client->title = NULL;
	
	update_client(client);
	
	if(!clients) clients = client;
	else {
		client->next = clients;
		clients = client;
	}
	return client;
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

void maprequest(XMapRequestEvent ev) {
	CLIENT *client;

	client = get_client_by_window(ev.window);
	if(client) {
		printf("MapRequest event emitted on existing client: %d\n", client->window);
	}
	else {
		client = create_client(ev.window);
		printf("New client: %d\n", client->window);
		XMapWindow(display, client->window);
	}
}

void configure_request(XConfigureRequestEvent ev) {
	CLIENT *client;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	printf("%d: ConfigureRequest!\n");
}

int catch_error(Display *d, XErrorEvent *e) {
	char err_text[MAX_ERR_LEN];
	XGetErrorText(d, e->error_code, err_text, MAX_ERR_LEN);
	printf("XLIB ERROR: %s\n", err_text);
}

void check_other_wm() {
	int (*old_err)(Display *, XErrorEvent *);
	XSetErrorHandler(other_wm_error);
	XSync(display, False);

	XSelectInput(display, root, SubstructureRedirectMask);
	XSync(display, False);

	XSetErrorHandler(old_err);
	XSync(display, False);
}

int other_wm_error(Display *d, XErrorEvent *e) {
	printf("Another window manager is already running!\n");
	exit(-1);
}
