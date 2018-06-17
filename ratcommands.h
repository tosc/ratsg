#ifndef RATHELPER_H_
#define RATHELPER_H_

typedef struct window_struct
{
	int status;
	char name[256];
	struct window_struct *next;	
} window;

typedef struct group_struct
{
	window *windowlist;
	int x;
	int y;
	int width;
	int height;
	int screen_nr;
	int frame_nr;
	int next;
	int prev;
	int enabled;
} group;

typedef struct ratsession_struct
{
	group *grouplist;
	group **sortedlist;
	int screenX[10];
	int screenY[10];
	int group_len;
	int group_max;
	int current_screen;
	int current_frame;
} ratsession;

void* create_group(ratsession *session);

int current_frame();

int current_screen();

void free_windows(window *windowlist);

window* new_window();

ratsession* new_session();

void update_groups(ratsession *session);

void update_session(ratsession *session);

void session_to_string(ratsession *session, char *group_string);

void move_r(ratsession *session);

void move_l(ratsession *session);

void screen_r(ratsession *session);

void screen_l(ratsession *session);

void update_mouse(ratsession *session);
#endif
