#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include "colorful.h"
#include "xinerama.h"

SCREEN *screens = NULL;

bool check_xinerama_active() {
	return XineramaIsActive(display);
}

void query_screens() {
	int root_width, root_height, screen_count, i;
	SCREEN *tmp, *screen;
	XineramaScreenInfo *xin_screens;
	XWindowAttributes att = (XWindowAttributes) {};
	
	XGetWindowAttributes(display, root, &att);
	root_width = att.width;
	root_height = att.height;
	
	for(tmp = screens; tmp;) {
		screen = tmp->next;
		free(tmp);
		tmp = screen;
	}
	screens = NULL;
	
	if(check_xinerama_active()) {
		xin_screens = XineramaQueryScreens(display, &screen_count);
		for(i = 0; i < screen_count; i++) {
			for(tmp = screens; tmp && tmp->next; tmp = tmp->next);
			screen = malloc(sizeof(SCREEN));
			screen->next = NULL;
			screen->x = xin_screens[i].x_org;
			screen->y = xin_screens[i].y_org;
			screen->width = xin_screens[i].width;
			screen->height = xin_screens[i].height;
			if(tmp) tmp->next = screen;
			else screens = screen;
			
			printf("Screen %d -> x: %d, y: %d, w: %d, h: %d\n", i, screen->x, screen->y, screen->width, screen->height);
		}
	}
	else {
		screens = malloc(sizeof(SCREEN));
		screens->next = NULL;
		screens->x = 0;
		screens->y = 0;
		screens->width = root_width;
		screens->height = root_height;
		
		printf("Screen %d -> x: %d, y: %d, w: %d, h: %d\n", 0, screens->x, screens->y, screens->width, screens->height);
	}
}
