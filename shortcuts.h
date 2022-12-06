typedef struct Shortcut SHORTCUT;
struct Shortcut {
	SHORTCUT *next;
	bool is_button; /* default: keyboard shortcut */
	bool global;
	unsigned int detail;
	unsigned int state;
	
	/* CLIENT *client -> client that caused this event
	 * int x_root, y_root -> x and y position of the pointer relative to root window
	 * unsigned int detail -> key / button used to trigger this shortcut
	 * unsigned int state -> modifiers used to trigger this shortcut
	 * bool is_button -> whether this shortcut was triggered by a button event or a keyboard event
	 */
	void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool);
};

extern SHORTCUT *shortcuts;

void init_shortcuts();
void create_shortcut(bool is_button, bool global, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool));
void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool));
void create_keyboard_shortcut(bool global, unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int,unsigned int,unsigned int,bool));

void shortcut_move_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button);
void shortcut_resize_client(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button);
void shortcut_toggle_floating(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button);
void shortcut_spawn_dmenu(CLIENT *client, int x_root, int y_root, unsigned int detail, unsigned int state, bool is_button);
