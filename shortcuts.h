typedef struct Shortcut SHORTCUT;
struct Shortcut {
	SHORTCUT *next;
	bool is_button; /* default: keyboard shortcut */
	unsigned int detail;
	unsigned int state;
	void (*callback)(CLIENT*,int,int);
};

extern SHORTCUT *shortcuts;

void init_shortcuts();
void create_shortcut(bool is_button, unsigned int detail, unsigned int state, void (*callback)(CLIENT*,int,int));
void create_button_shortcut(unsigned int button, unsigned int state, void (*callback)(CLIENT*,int,int));
void create_keyboard_shortcut(unsigned int key, unsigned int state, void (*callback)(CLIENT*,int,int));
