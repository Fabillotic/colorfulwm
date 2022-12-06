typedef struct Shortcut SHORTCUT;
struct Shortcut {
	SHORTCUT *next;
	bool is_button; /* default: keyboard shortcut */
	bool global;
	unsigned int detail;
	unsigned int state;
	void (*callback)(CLIENT*,int,int,unsigned int,unsigned int);
};

extern SHORTCUT *shortcuts;

void init_shortcuts();
void create_shortcut(bool is_button, bool global, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int));
void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int));
void create_keyboard_shortcut(bool global, unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int));

void shortcut_move_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state);
void shortcut_resize_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state);
void shortcut_toggle_floating(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state);
void shortcut_spawn_dmenu(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state);