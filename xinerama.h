typedef struct screen SCREEN;
struct screen {
	SCREEN *next;
	int x;
	int y;
	int width;
	int height;
};

extern SCREEN *screens;

bool check_xinerama_active();
void query_screens();
SCREEN *get_screen_xy(int x, int y);
SCREEN *get_screen_client(CLIENT *client);
