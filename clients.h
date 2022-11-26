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

extern CLIENT *clients;

void scan_clients();
CLIENT *get_client_by_window(Window window);
void add_client(CLIENT *client);
CLIENT* create_client(Window window);
void remove_client(CLIENT *client);
void delete_client(CLIENT *client);
void update_client(CLIENT *client);
