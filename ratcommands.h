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
} group;

int current_group();

void free_windows(window *windowlist);

group* new_group();

window* new_window();

void update_groups(group *grouplist);

void groups_to_string(group *grouplist, char *group_string);

#endif
