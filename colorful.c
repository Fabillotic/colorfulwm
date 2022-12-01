#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "logger.h"
#include "colorful.h"
#include "clients.h"
#include "xinerama.h"

#define MAX_ERR_LEN 200
#define return_endlog {log_end_section(); return;}

Display *display;
Window root;
int (*xlib_err)(Display *, XErrorEvent *);


int main();
void run();
void init_client(CLIENT *client);
void arrange_all_clients();
void arrange_clients(SCREEN *screen);
void map_request(XMapRequestEvent ev);
void configure_request(XConfigureRequestEvent ev);
void configure_notify(XConfigureEvent ev);
void unmap_notify(XUnmapEvent ev);
void destroy_notify(XDestroyWindowEvent ev);
int catch_error(Display *d, XErrorEvent *e);
void check_other_wm();
int other_wm_error(Display *d, XErrorEvent *e);

int main() {
	printf("Welcome to colorfulwm!\n");
	/* Sorry for this huge blob haha */
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			"  ____   ",  " ______  ", " _       ", " ______  ", " ______  ",   " ______  ", " _    _  ", " _      ");
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			" /  ___\\ ", "|  __  | ", "| |      ", "|  __  | ", "|  __  | ",   "|  ____| ", "| |  | | ", "| |     ");
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			"|  |     ", "| |  | | ", "| |      ", "| |  | | ", "| |__| | ",   "| |____  ", "| |  | | ", "| |     ");
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			"|  |     ", "| |  | | ", "| |      ", "| |  | | ", "| |____| ",   "|  ____| ", "| |  | | ", "| |     ");
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			"|  |___  ", "| |__| | ", "| |____  ", "| |__| | ", "| | \\ \\  ", "| |      ", "| |__| | ", "| |____ ");
	printf("\033[38;5;196m%s\033[38;5;208m%s\033[38;5;220m%s\033[38;5;28m%s\033[38;5;21m%s\033[38;5;54m%s\033[38;5;196m%s\033[38;5;208m%s\033[0m\n",
			" \\_____/ ", "|______| ", "|______| ", "|______| ", "| |  \\_\\ ", "|_|      ", "|______| ", "|______|");
	
	log_init(LOG_ALL, NULL, true);
	
	log_print(INFO, "Connecting to X11...\n");
	display = XOpenDisplay(NULL);
	if(!display) {
		log_print(ERROR, "Connection failed!\n");
		return -1;
	}
	
	log_print(INFO, "Gettting root window\n");
	root = XDefaultRootWindow(display);
	
	/* Find default Xlib error handler */
	log_print(INFO, "Finding default error handler\n");
	xlib_err = XSetErrorHandler(catch_error);
	XSetErrorHandler(xlib_err);
	
	/* Find all the connected screens */
	log_print(INFO, "Xinerama active: %s\n", (check_xinerama_active() ? "true" : "false"));
	query_screens();
	
	/* Register SubstructureRedirect on root */
	check_other_wm();
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	
	log_print(INFO, "Scanning for clients\n");
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
	XWMHints *hints;
	
	log_start_section("MapRequest");
	client = get_client_by_window(ev.window);
	if(client) {
		log_print(INFO, "MapRequest event emitted on existing client: %d\n", client->window);
		client->iconic = false; /* Transition from Iconic to Normal requires mapping the window */
	}
	else {
		client = create_client(ev.window);
		init_client(client);
		log_print(INFO, "New client (MapRequest): %d\n", client->window);
		
		/* Figure out whether the window transitioned from Withdrawn to Normal or to Iconic */
		hints = XGetWMHints(display, client->window);
		if(hints) {
			client->iconic = hints->initial_state == IconicState;
			XFree(hints);
			if(client->iconic) log_print(INFO, "Client was started in an iconic state!\n");
		}
		else client->iconic = false; /* Assume its Normal otherwise */
	}
	XMapWindow(display, client->window);
	XSync(display, False);
	log_end_section();
}

void configure_request(XConfigureRequestEvent ev) {
	CLIENT *client;
	XWindowChanges wc;
	
	log_start_section("ConfigureRequest");
	client = get_client_by_window(ev.window);
	if(!client) return_endlog;
	
	log_print(INFO, "%d: ConfigureRequest!\n");
	
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
	log_end_section();
}

void configure_notify(XConfigureEvent ev) {
	CLIENT *client;
	
	log_start_section("ConfigureNotify");
	client = get_client_by_window(ev.window);
	if(!client) return_endlog;
	
	client->x = ev.x;
	client->y = ev.y;
	client->width = ev.width;
	client->height = ev.height;
	client->border_width = ev.border_width;
	client->override_redirect = ev.override_redirect;
	
	log_print(INFO, "%d: ConfigureNotify! x: %d, y: %d, w: %d, h: %d, or: %d\n", client->window, client->x, client->y, client->width, client->height, client->override_redirect);
	log_end_section();
}

void unmap_notify(XUnmapEvent ev) {
	CLIENT *client;
	SCREEN *screen;
	
	log_start_section("UnmapNotify");
	client = get_client_by_window(ev.window);
	if(!client) return_endlog;
	
	screen = get_screen_client(client);
	
	log_print(INFO, "%d: Unmapping...\n", client->window);
	delete_client(client);
	arrange_clients(screen);
	log_end_section();
}

void destroy_notify(XDestroyWindowEvent ev) {
	CLIENT *client;
	SCREEN *screen;
	
	log_start_section("DestroyNotify");
	client = get_client_by_window(ev.window);
	if(!client) return_endlog;
	
	screen = get_screen_client(client);
	
	log_print(INFO, "%d: Destroying...\n", client->window);
	delete_client(client);
	arrange_clients(screen);
	log_end_section();
}

int catch_error(Display *d, XErrorEvent *e) {
	char err_text[MAX_ERR_LEN];
	XGetErrorText(d, e->error_code, err_text, MAX_ERR_LEN);
	log_print(ERROR, "XLIB ERROR: %s\n", err_text);
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
	log_print(ERROR, "Another window manager is already running!\n");
	exit(-1);
}
