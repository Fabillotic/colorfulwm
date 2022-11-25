typedef struct client CLIENT;
struct client {
	CLIENT *next;
	Window window;
	char *title;
	int x;
	int y;
	int width;
	int height;
	int border_width;
	bool override_redirect;
};
