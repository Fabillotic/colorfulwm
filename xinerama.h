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
