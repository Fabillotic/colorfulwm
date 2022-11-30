#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include "colorful.h"
#include "clients.h"
#include "xinerama.h"

#define MAX_ERR_LEN 200

Display *display;
Window root;
int (*xlib_err)(Display *, XErrorEvent *);


int main();
void run();
void init_client(CLIENT *client);
void arrange_all_clients();
void arrange_clients(SCREEN *screen);
void map_request(XMapRequestEvent ev);
void create_notify(XCreateWindowEvent ev);
void configure_request(XConfigureRequestEvent ev);
void configure_notify(XConfigureEvent ev);
void unmap_notify(XUnmapEvent ev);
void destroy_notify(XDestroyWindowEvent ev);
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
	
	/* Find all the connected screens */
	printf("Xinerama active: %s\n", (check_xinerama_active() ? "true" : "false"));
	query_screens();
	
	/* Register SubstructureRedirect on root */
	check_other_wm();
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	
	printf("Scanning for clients\n");
	clients = NULL;
	scan_clients();
	
	arrange_all_clients();
	
	run();
	
	return 0;
}

void run() {
	XEvent e;
	while(true) {
		while(XPending(display)) {
			XNextEvent(display, &e);
			if(e.type == MapRequest) map_request(e.xmaprequest);
			else if(e.type == ConfigureRequest) configure_request(e.xconfigurerequest);
			else if(e.type == UnmapNotify) unmap_notify(e.xunmap);
			else if(e.type == DestroyNotify) destroy_notify(e.xdestroywindow);
			else if(e.type == CreateNotify) create_notify(e.xcreatewindow);
			else if(e.type == ConfigureNotify) configure_notify(e.xconfigure);
		}
	}
}

void init_client(CLIENT *client) {
	SCREEN *screen;
	
	screen = get_screen_client(client);
	arrange_clients(screen);
}

void arrange_all_clients() {
	SCREEN *screen;
	
	for(screen = screens; screen; screen = screen->next) {
		arrange_clients(screen);
	}
}

void arrange_clients(SCREEN *screen) {
	CLIENT *tmp;
	int i, c, x, y, w, h;
	bool odd;
	
	if(!screen) return;
	
	c = 0;
	for(tmp = clients; tmp; tmp = tmp->next) {
		if(!tmp->override_redirect && get_screen_client(tmp) == screen) ++c;
	}
	
	if(c == 0) return;
	else if(c == 1) {
		move_client(clients, screen->x, screen->y);
		resize_client(clients, screen->width, screen->height);
		return;
	}
	
	odd = c % 2 == 1;
	
	w = screen->width / 2;
	h = screen->height / (c / 2);
	
	i = 0;
	for(tmp = clients; tmp; tmp = tmp->next) {
		if(tmp->override_redirect || get_screen_client(tmp) != screen) continue;
		x = screen->x + (i % 2 == 1) * w;
		y = screen->y + (i / 2) * h;
		
		if(odd && tmp->next && !tmp->next->next) { /* Second to last */
			printf("Second to last: %d\n", h / 2);
			move_client(tmp, x, y);
			resize_client(tmp, w, h / 2);
		}
		else if(odd && !tmp->next) { /* Last */
			move_client(tmp, x + w, y - (h / 2));
			resize_client(tmp, w, h / 2);
		}
		else {
			move_client(tmp, x, y);
			resize_client(tmp, w, h);
		}
		i++;
	}
}

void map_request(XMapRequestEvent ev) {
	CLIENT *client;

	client = get_client_by_window(ev.window);
	if(client) {
		printf("MapRequest event emitted on existing client: %d\n", client->window);
	}
	else {
		client = create_client(ev.window);
		init_client(client);
		printf("New client (MapRequest): %d\n", client->window);
	}
	XMapWindow(display, client->window);
	XSync(display, False);
}

void create_notify(XCreateWindowEvent ev) {
	CLIENT *client;

	client = get_client_by_window(ev.window);
	if(!client) {
		client = create_client(ev.window);
		init_client(client);
		printf("New client (CreateNotify): %d\n", client->window);
		XMapWindow(display, client->window);
		XSync(display, False);
	}
}

void configure_request(XConfigureRequestEvent ev) {
	CLIENT *client;
	XWindowChanges wc;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	printf("%d: ConfigureRequest!\n");
	
	wc = (XWindowChanges){};
	
	wc.x = ev.x;
	wc.y = ev.y;
	wc.width = ev.width;
	wc.height = ev.height;
	wc.border_width = ev.border_width;
	wc.sibling = ev.above;
	wc.stack_mode = ev.detail;
	
	XConfigureWindow(display, client->window, ev.value_mask, &wc);
	XSync(display, False);
}

void configure_notify(XConfigureEvent ev) {
	CLIENT *client;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	client->x = ev.x;
	client->y = ev.y;
	client->width = ev.width;
	client->height = ev.height;
	client->border_width = ev.border_width;
	client->override_redirect = ev.override_redirect;
	
	printf("%d: ConfigureNotify! x: %d, y: %d, w: %d, h: %d, or: %d\n", client->window, client->x, client->y, client->width, client->height, client->override_redirect);
}

void unmap_notify(XUnmapEvent ev) {
	CLIENT *client;
	SCREEN *screen;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	screen = get_screen_client(client);
	
	printf("%d: Unmapping...\n", client->window);
	delete_client(client);
	arrange_clients(screen);
}

void destroy_notify(XDestroyWindowEvent ev) {
	CLIENT *client;
	SCREEN *screen;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	screen = get_screen_client(client);
	
	printf("%d: Destroying...\n", client->window);
	delete_client(client);
	arrange_clients(screen);
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
