int main();
void run();
void init_client(CLIENT *client);
void scan_clients();
void arrange_all_clients();
void arrange_clients(SCREEN *screen);
void map_request(XMapRequestEvent ev);
void configure_request(XConfigureRequestEvent ev);
void configure_notify(XConfigureEvent ev);
void unmap_notify(XUnmapEvent ev);
void destroy_notify(XDestroyWindowEvent ev);
void button_pressed(XButtonEvent ev);
void shortcut_move_client(CLIENT *client, int x_root, int y_root);
void shortcut_resize_client(CLIENT *client, int x_root, int y_root);
void button_released(XButtonEvent ev);
void key_pressed(XKeyEvent ev);
void shortcut_toggle_floating(CLIENT *client, int x_root, int y_root);
void key_released(XKeyEvent ev);
void enter_window(XCrossingEvent ev);
int catch_error(Display *d, XErrorEvent *e);
void check_other_wm();
int other_wm_error(Display *d, XErrorEvent *e);
