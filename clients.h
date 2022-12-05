typedef struct client CLIENT;
struct client {
	CLIENT *next;
	Window window;
	Window sub;
	
	char *title;
	int x;
	int y;
	int width;
	int height;
	
	int min_width;
	int min_height;
	int base_width;
	int base_height;
	int max_width;
	int max_height;
	int inc_width;
	int inc_height;
	
	int border_width;
	bool override_redirect;
	bool iconic;
	bool floating;
};

extern CLIENT *clients;
extern CLIENT *active;

CLIENT *get_client_by_window(Window window);
void add_client(CLIENT *client);
CLIENT* create_client(Window window);
void remove_client(CLIENT *client);
void delete_client(CLIENT *client);
void update_client(CLIENT *client);
void configure_client(CLIENT *client, unsigned int value_mask, XWindowChanges *values);
void move_client(CLIENT *client, int x, int y);
void resize_client(CLIENT *client, int width, int height);
void frame_client(CLIENT *client);
void unframe_client(CLIENT *client);
void focus_client(CLIENT *client, bool raise_window);
