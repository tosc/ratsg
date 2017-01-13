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
	struct group_struct *next;	
	int nr;
	int x;
	int y;
} group;

typedef struct ratsession_struct
{
	group *grouplist;
	int current_frame;
} ratsession;

typedef struct screen_struct
{
	int x;
	int y;
} screen;

screen* get_screens();

int current_frame();

void free_windows(window *windowlist);

group* new_group();

window* new_window();

ratsession* new_session();

void update_group(group *c_group, screen *screens);

void sort_session(ratsession *session);

void update_session(ratsession *session, screen *screens);

void session_to_string(ratsession *session, char *group_string);

#endif
