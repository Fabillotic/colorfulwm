#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "colorful.h"
#include "logger.h"
#include "clients.h"
#include "xinerama.h"
#include "shortcuts.h"

#define MAX_ERR_LEN 200
#define return_endlog {log_end_section(); return;}

Display *display;
Window root;
int (*xlib_err)(Display *, XErrorEvent *);
int focus_type = FocusClick;

int main();
void run();
void init_client(CLIENT *client);
void register_global_shortcuts();
void scan_clients();
void arrange_all_clients();
void arrange_clients(SCREEN *screen);
void map_request(XMapRequestEvent ev);
void configure_request(XConfigureRequestEvent ev);
void configure_notify(XConfigureEvent ev);
void unmap_notify(XUnmapEvent ev);
void destroy_notify(XDestroyWindowEvent ev);
void button_pressed(XButtonEvent ev);
void button_released(XButtonEvent ev);
void key_pressed(XKeyEvent ev);
void spawn(char** cmd);
void key_released(XKeyEvent ev);
void enter_window(XCrossingEvent ev);
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
	
	/* Find all the connected screens */
	log_print(INFO, "Xinerama active: %s\n", (check_xinerama_active() ? "true" : "false"));
	query_screens();
	
	/* Register SubstructureRedirect on root */
	check_other_wm();
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	
	XSetErrorHandler(catch_error);
	
	init_shortcuts();
	register_global_shortcuts();
	
	log_print(INFO, "Scanning for clients\n");
	clients = NULL;
	scan_clients();
	
	arrange_all_clients();
	
	focus_client(NULL, false);
	XDefineCursor(display, root, XCreateFontCursor(display, XC_arrow));
	
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
			else if(e.type == ButtonPress) button_pressed(e.xbutton);
			else if(e.type == ButtonRelease) button_released(e.xbutton);
			else if(e.type == EnterNotify) enter_window(e.xcrossing);
			else if(e.type == KeyPress) key_pressed(e.xkey);
			else if(e.type == KeyRelease) key_released(e.xkey);
		}
	}
}

void init_client(CLIENT *client) {
	SCREEN *screen;
	SHORTCUT *sc;
	
	screen = get_screen_client(client);
	arrange_clients(screen);
	
	XSelectInput(display, client->window, ButtonPressMask | ButtonReleaseMask | EnterWindowMask);
	if(client->sub != None) XSelectInput(display, client->sub, StructureNotifyMask);
	XGrabButton(display, AnyButton, AnyModifier, client->window, False, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
	
	for(sc = shortcuts; sc; sc = sc->next) {
		if(sc->is_button) {
			XGrabButton(display, sc->detail, sc->state, client->window, False, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
		}
		else if(!sc->global) {
			XGrabKey(display, sc->detail, sc->state, client->window, False, GrabModeSync, GrabModeSync);
		}
	}
}

void register_global_shortcuts() {
	SHORTCUT *sc;
	
	for(sc = shortcuts; sc; sc = sc->next) {
		if(!sc->is_button && sc->global) {
			XGrabKey(display, sc->detail, sc->state, root, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void scan_clients() {
	CLIENT *client;
	Window *children;
	XWindowAttributes att = (XWindowAttributes) {};
	Window r_root;
	Window r_parent;
	int n_children;
	int i;
	char *name;
	
	log_start_section("Scan Clients");
	XQueryTree(display, root, &r_root, &r_parent, &children, &n_children);
	for(i = 0; i < n_children; i++) {
		XGetWindowAttributes(display, children[i], &att);
		
		if(att.map_state == IsViewable) {
			client = create_client(children[i]);
			init_client(client);
			
			log_print(INFO, "Found client! id: %d, title: '%s', x: %d, y: %d, w: %d, h: %d, bw: %d, or: %d\n", client->window, client->title, client->x, client->y, client->width, client->height, client->border_width, client->override_redirect);
		}
	}
	log_end_section();
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
		if(!tmp->override_redirect && !tmp->floating && get_screen_client(tmp) == screen) ++c;
	}
	
	if(c == 0) return;
	else if(c == 1) {
		for(tmp = clients; tmp; tmp = tmp->next) {
			if(!tmp->override_redirect && !tmp->floating && get_screen_client(tmp) == screen) {
				move_client(tmp, screen->x, screen->y);
				resize_client(tmp, screen->width, screen->height);
				return;
			}
		}
	}
	
	odd = c % 2 == 1;
	
	w = screen->width / 2;
	h = screen->height / (c / 2);
	
	i = 0;
	for(tmp = clients; tmp; tmp = tmp->next) {
		if(tmp->override_redirect || tmp->floating || get_screen_client(tmp) != screen) continue;
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
		hints = XGetWMHints(display, client->window); //For some reason this raises a BadAccess when using st?
		if(hints) {
			client->iconic = hints->initial_state == IconicState;
			XFree(hints);
			if(client->iconic) log_print(INFO, "Client was started in an iconic state!\n");
		}
		else client->iconic = false; /* Assume its Normal otherwise */
	}
	XMapWindow(display, client->window);
	XSync(display, False);
	
	if(focus_type == FocusClick) focus_client(client, false);
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
	
	arrange_all_clients();
	log_end_section();
}

void configure_notify(XConfigureEvent ev) {
	CLIENT *client, *sclient;
	
	client = get_client_by_window(ev.window);
	if(!client) {
		for(sclient = clients; sclient; sclient = sclient->next) {
			if(sclient->sub == ev.window) {
				break;
			}
		}
		if(sclient) {
			sclient->width = ev.width + 20;
			sclient->height = ev.height + 20;
		}
		return_endlog;
	}
	
	client->x = ev.x;
	client->y = ev.y;
	client->width = ev.width;
	client->height = ev.height;
	client->border_width = ev.border_width;
	client->override_redirect = ev.override_redirect;
}

void unmap_notify(XUnmapEvent ev) {
	CLIENT *client, *sclient;
	SCREEN *screen;
	
	log_start_section("UnmapNotify");
	client = get_client_by_window(ev.window);
	if(!client) {
		for(sclient = clients; sclient; sclient = sclient->next) {
			if(sclient->sub == ev.window) {
				log_print(INFO, "Sub-Window was unmapped!\n");
				break;
			}
		}
		if(!sclient) return_endlog;
		
		XDestroyWindow(display, sclient->window);
		XSync(display, False);
		
		delete_client(sclient);
		arrange_all_clients();
		return_endlog;
	}
	
	screen = get_screen_client(client);
	
	log_print(INFO, "%d: Unmapping...\n", client->window);
	delete_client(client);
	arrange_clients(screen);
	
	if(clients && focus_type == FocusClick) focus_client(clients, false);
	
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

void button_pressed(XButtonEvent ev) {
	CLIENT *client;
	SHORTCUT *sc;
	bool is_shortcut;
	
	client = get_client_by_window(ev.window);
	if(!client) {
		XAllowEvents(display, ReplayPointer, CurrentTime);
		return;
	}
	
	focus_client(client, true);
	
	is_shortcut = false;
	for(sc = shortcuts; sc; sc = sc->next) {
		if(sc->is_button && ev.state == sc->state && ev.button == sc->detail) {
			is_shortcut = true;
			sc->callback(client, ev.x_root, ev.y_root, sc->detail, sc->state, true);
			XAllowEvents(display, SyncPointer, CurrentTime);
			break;
		}
	}
	
	if(!is_shortcut) XAllowEvents(display, ReplayPointer, CurrentTime);
}

void button_released(XButtonEvent ev) {
	CLIENT *client;
	
	client = get_client_by_window(ev.window);
	if(!client) {
		XAllowEvents(display, ReplayPointer, CurrentTime);
		return;
	}
	
	XAllowEvents(display, ReplayPointer, CurrentTime);
}

void key_pressed(XKeyEvent ev) {
	CLIENT *client;
	SHORTCUT *sc;
	bool is_shortcut;
	
	client = get_client_by_window(ev.window);
	if(!client && ev.window != root) {
		XAllowEvents(display, ReplayPointer, CurrentTime);
		return;
	}
	
	is_shortcut = false;
	for(sc = shortcuts; sc; sc = sc->next) {
		if(!sc->is_button && ev.state == sc->state && ev.keycode == sc->detail) {
			if(client && !sc->global) {
				is_shortcut = true;
				sc->callback(client, ev.x_root, ev.y_root, sc->detail, sc->state, false);
				XAllowEvents(display, SyncKeyboard, CurrentTime);
				break;
			}
			else if(!client && sc->global) {
				is_shortcut = true;
				sc->callback(NULL, ev.x_root, ev.y_root, sc->detail, sc->state, false);
				break;
			}
		}
	}
	
	if(!is_shortcut) XAllowEvents(display, ReplayKeyboard, CurrentTime);
}

void spawn(char** cmd) {
	if(fork() == 0) {
		if(display) close(ConnectionNumber(display));
		setsid();
		execvp(cmd[0], cmd);
		exit(0);
	}
}

void key_released(XKeyEvent ev) {
	CLIENT *client;
	
	client = get_client_by_window(ev.window);
	if(!client) {
		XAllowEvents(display, ReplayKeyboard, CurrentTime);
	}
	
	XAllowEvents(display, ReplayKeyboard, CurrentTime);
}

void enter_window(XCrossingEvent ev) {
	CLIENT *client;
	
	client = get_client_by_window(ev.window);
	if(!client) return;
	
	if(focus_type == FocusEnter) focus_client(client, false);
}

int catch_error(Display *d, XErrorEvent *e) {
	char err_text[MAX_ERR_LEN];
	XGetErrorText(d, e->error_code, err_text, MAX_ERR_LEN);
	log_print(ERROR, "XLIB ERROR: %s\n", err_text);
}

void check_other_wm() {
	int (*old_err)(Display *, XErrorEvent *);
	XSetErrorHandler(other_wm_error);

	XSelectInput(display, root, SubstructureRedirectMask);
	XSync(display, False);

	XSetErrorHandler(old_err);
}

int other_wm_error(Display *d, XErrorEvent *e) {
	log_print(ERROR, "Another window manager is already running!\n");
	exit(-1);
}
