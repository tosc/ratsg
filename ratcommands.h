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
	int nr;
	int next;
	int prev;
} group;

typedef struct ratsession_struct
{
	group *grouplist;
	group **sortedlist;
	int group_len;
	int current_screen;
} ratsession;

void* create_group(ratsession *session);

int current_frame();

int current_screen();

void free_windows(window *windowlist);

window* new_window();

ratsession* new_session();

void update_group(ratsession *session);

void update_session(ratsession *session);

void session_to_string(ratsession *session, char *group_string);

void screen_r(ratsession *session);

void screen_l(ratsession *session);
#endif
