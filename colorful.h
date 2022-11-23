typedef struct client CLIENT;
struct client {
	CLIENT *next;
	Window window;
	int x;
	int y;
	int width;
	int height;
	int border_width;
	bool override_redirect;
};
