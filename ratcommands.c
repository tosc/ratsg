#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "ratcommands.h"

/**
 * Create all the groups and add them to session
 *
 * @return Current ratsession.
 */
void create_groups(ratsession *session)
{
	// Get information about screens.
	char buffer[8192];
	FILE *fp = popen("ratpoison -c sdump", "r");
	fgets(buffer, sizeof(buffer) - 1, fp);
	pclose(fp);

	// Get the information about all the screens.
	char *pch = strtok(buffer, "() ,\n");
	int i = 0;
	int screen_nr = 0;
	while(pch != NULL)
	{
		if(i == 0)
		{
			screen_nr++;
		}
		else if(i == 2)
		{
			session->screenX[screen_nr - 1] = atoi(pch);
		}
		else if(i == 3)
		{
			session->screenY[screen_nr - 1] = atoi(pch);
		}
		pch = strtok(NULL, "() ,\n");
		i++;
		if(i == 7)
		{
			i = 0;
		}
	}

	// Make room for all the screens.
	session->group_max = 20;
	session->group_len = 0;
	group *groups = malloc(sizeof(group) * session->group_max);
	for(i = 0; i < session->group_max; i++)
	{
		groups[i].x = 0;
		groups[i].y = 0;
		groups[i].windowlist = NULL;
		groups[i].frame_nr = i;
		groups[i].screen_nr = 0;
		groups[i].width = 0;
		groups[i].height = 0;
		groups[i].enabled = 0;
		char command[128];
		sprintf(command, "ratpoison -c 'gnewbg :%d'", i);
		system(command);
	}
	session->grouplist = groups;
}

/**
 * Returns the current ratpoison frame.
 *
 * @return The current ratpoison frame.
 */
int current_frame()
{
	int c_frame = 0;
	// Variables used for piping.
	char buffer[256];
	FILE *fp;

	fp = popen("ratpoison -c curframe", "r");
	fgets(buffer, sizeof(buffer) - 1, fp);
	c_frame = buffer[0] - '0';
	pclose(fp);
	return c_frame;
}

/**
 * Returns the current ratpoison screen.
 *
 * @return The current ratpoison screen.
 */
int current_screen()
{
	// Get information about all frames.
	char buffer[8192];
	FILE *fp = popen("ratpoison -c sfdump", "r");
	fgets(buffer, sizeof(buffer) - 1, fp);
	pclose(fp);

	// Go through information about frames.
	char *pch;
	pch = strtok(buffer, "() ,\n");
	int i = 0;
	int c_frame = current_frame();
	int frame = -1;
	while(pch != NULL)
	{
		// pch is at the start of a new frame.
		if(strcmp(pch, "frame") == 0)
		{
			i = 0;
		}

		// pch is framenumber.
		if(i == 2)
		{
			frame = atoi(pch);
		}

		//pch is screennr.
		else if(i == 21)
		{
			if(frame == c_frame)
			{
				return atoi(pch);	
			}
		}

		pch = strtok(NULL, "() ,\n");
		i++;
	}
	return -1;
}

/**
 * Delete window list.
 *
 * @param The window list you wish to delete.
 */
void free_windows(window *windowlist)
{
	window *tmp;
	while(windowlist != NULL)
	{
		tmp = windowlist;
		windowlist = windowlist->next;
		free(tmp);
	}
}

/**
 * Create a new window.
 *
 * @return Returns a pointer to a new window.
 */
window* new_window()
{
	window *new_window = malloc(sizeof(window));
	new_window->next = NULL;
	new_window->status = 0;
	strcpy(new_window->name, "");
	return new_window;
}

/**
 * Create new ratsession.
 *
 * @return Returns the new ratsession.
 */
ratsession* new_session()
{
 	ratsession *new_session = malloc(sizeof(ratsession));
	new_session->group_len = 0;
	new_session->current_screen = 0;
	new_session->grouplist = NULL;
	new_session->sortedlist = NULL;
	create_groups(new_session);
	return new_session;
}

/**
 * Update the current group.
 */
void update_groups(ratsession *session)
{
	// Get information about all frames.
	char buffer[8192];
	FILE *fp = popen("ratpoison -c sfdump", "r");
	fgets(buffer, sizeof(buffer) - 1, fp);
	pclose(fp);

	// Disable all groups
	int i;
	for(i = 0; i < session->group_max; i++)
	{
		session->grouplist[i].enabled = 0;
	}

	// Get the information about all the screens.
	char *pch = strtok(buffer, "() ,\n");
	i = 0;
	int frame_nr = 0;
	int len = 0;
	while(pch != NULL)
	{
		if(strcmp(pch, "frame") == 0)
		{
			i = 0;
		}
		else if(i == 2)
		{
			frame_nr = atoi(pch);
			session->grouplist[frame_nr].enabled = 1;
			len++;
		}
		else if(i == 4)
		{
			session->grouplist[frame_nr].x = atoi(pch);
		}
		else if(i == 6)
		{
			session->grouplist[frame_nr].y = atoi(pch);
		}
		else if(i == 8)
		{
			session->grouplist[frame_nr].width = atoi(pch);
		}
		else if(i == 10)
		{
			session->grouplist[frame_nr].height = atoi(pch);
		}
		else if(i == 21)
		{
			int sNr = atoi(pch);
			session->grouplist[frame_nr].screen_nr = sNr;
			session->grouplist[frame_nr].x += session->screenX[sNr];
			session->grouplist[frame_nr].y += session->screenY[sNr];
		}
		pch = strtok(NULL, "() ,\n");
		i++;
	}

	session->group_len = len;

	// Sort the sorted groups
	group **sortedgroups = malloc(sizeof(group *) * session->group_len);
	int lastX = -1;
	int lastY = -1;
	int j = 0;
	for(i = 0; i < session->group_max; i++)
	{
		group *c_group = &session->grouplist[i];
		if(c_group->enabled)
		{
			sortedgroups[j] = c_group;
			j++;
		}
	}
	while(1)
	{
		int sorted = 1;
		group *p_group = sortedgroups[0];
		for(i = 1; i < session->group_len; i++)
		{
			group *c_group = sortedgroups[i];
			if(p_group->x > c_group->x || (p_group->x == c_group->x && p_group->y > c_group->y))
			{
				sortedgroups[i - 1] = c_group;
				sortedgroups[i] = p_group;
				sorted = 0;
				break;
			}
			p_group = c_group;
		}
		if(sorted)
		{
			break;
		}
	}
	session->sortedlist = sortedgroups;

	// Add links to next and previous screen.
	group *p_group = NULL;
	for(i = 0; i < session->group_len; i = i + 1)
	{
		if(p_group == NULL)
		{
			p_group = sortedgroups[session->group_len - 1];
		}
		group *c_group = sortedgroups[i];	
		c_group->prev = p_group->frame_nr;
		p_group->next = c_group->frame_nr;
		p_group = c_group;
	}

	// Run the external ratpoison command to get all the current windows.
	fp = popen("ratpoison -c windows", "r");

	group c_group = session->grouplist[session->current_frame];
	free_windows(c_group.windowlist);
	c_group.windowlist = NULL;
	window *windowlist;
	// Add a new window for each line.
	while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL)
	{
		window *w = new_window();
		w->status= 1;
		if(buffer[1] == '*')
		{
			w->status= 2;
		}
		buffer[strcspn(buffer, "\n")] = 0;
		buffer[1] = '-';
		if(buffer[0] != 'N')
		{
			strcpy(w->name, buffer);
		}
		else
		{
			strcpy(w->name, "-");
			w->status= 2;
		}

		if(c_group.windowlist == NULL)
		{
			windowlist = w;
			c_group.windowlist = windowlist;
		}
		else
		{
			windowlist->next = w;
			windowlist = w;
		}
	}
	session->grouplist[session->current_frame] = c_group;
	pclose(fp);
}

/**
 * Update the session.
 *
 * @param session The current ratsession.
 */
void update_session(ratsession *session)
{
	// Update screennr and framenr
	session->current_screen = current_screen();
	session->current_frame = current_frame();

	// Change all current windows to inactive.
	int i;
	for(i = 0; i < session->group_len; i = i + 1)
	{
		window *c_window = session->grouplist[i].windowlist;
		while(c_window != NULL)
		{
			c_window->status = 0;
			c_window = c_window->next;
		}
	}

	// Make ratpoison switch to the current group.
	char command[128];
	sprintf(command, "ratpoison -c 'gselect :%d'", session->current_frame);
	system(command);

	// Update current groups information.
	update_groups(session);
}

/**
 * A string representation of the current session.
 *
 * @param session The ratsession you wish to print.
 * @param group_string Where to store the string representation.
 * @return A string representation of all groups and windows.
 */
void session_to_string(ratsession *session, char *group_string)
{
	int first = 1;
	strcpy(group_string, "");
	int i;
	for(i = 0; i < session->group_len; i = i + 1)
	{
		group *c_group = session->sortedlist[i];
		window *c_window = c_group->windowlist;

		// Add an orange | between each group.
		if(first == 0)
		{
			strcat(group_string, "<fc=#ee9a00>|</fc> ");
		}
		first = 0;

		if(c_window == NULL)
		{
			strcat(group_string, "<fc=#696665>-");
			strcat(group_string, "</fc> ");
		}
		while(c_window != NULL)
		{
			// If the window is inactive.
			if(c_window->status == 0)
			{
				// Grey
				strcat(group_string, "<fc=#696665>");
			}
			// If the group of this window is focused.
			else if(c_window->status == 1)
			{
				// Red
				strcat(group_string, "<fc=#F08080>");
			}
			// If the window is focused.
			else if(c_window->status == 2)
			{
				// Orange
				strcat(group_string, "<fc=#ee9a00>");
			}

			strcat(group_string, c_window->name);
			strcat(group_string, "</fc> ");
			c_window = c_window->next;
		}
	}
	strcat(group_string, "\n\0");
}

/**
 * Move the current window to the screen to the right.
 */
void move_r(ratsession *session)
{
	group c_group = session->grouplist[session->current_frame];
	char command[100];
	sprintf(command, "ratpoison -c 'gmove :%d'", c_group.next);
	system(command);
	system("ratpoison -c 'select -'");
	update_session(session);
	sprintf(command, "ratpoison -c 'sselect %d'", c_group.next);
	system(command);
	update_session(session);
	system("ratpoison -c other");
}

/**
 * Move the current window to the screen to the left.
 */
void move_l(ratsession *session)
{
	group c_group = session->grouplist[session->current_frame];
	char command[100];
	sprintf(command, "ratpoison -c 'gmove :%d'", c_group.prev);
	system(command);
	system("ratpoison -c 'select -'");
	update_session(session);
	sprintf(command, "ratpoison -c 'sselect %d'", c_group.prev);
	system(command);
	update_session(session);
	system("ratpoison -c other");
}

/**
 * Select the screen to the right.
 */
void screen_r(ratsession *session)
{
	// Switch focus to right screen.
	group c_group = session->grouplist[session->current_frame];
	c_group = session->grouplist[c_group.next];
	char command[100];
	sprintf(command, "ratpoison -c 'fselect %d'", c_group.frame_nr);
	system(command);

	// Move mouse there.
	int x = c_group.x + c_group.width/2;
	int y = c_group.y + c_group.height/2;
	sprintf(command, "ratpoison -c 'ratwarp %d %d'", x, y);
	system(command);

	update_session(session);
}

/**
 * Select the screen to the left.
 */
void screen_l(ratsession *session)
{
	// Switch focus to left screen.
	group c_group = session->grouplist[session->current_frame];
	c_group = session->grouplist[c_group.prev];
	char command[100];
	sprintf(command, "ratpoison -c 'fselect %d'", c_group.frame_nr);
	system(command);

	// Move mouse there.
	int x = c_group.x + c_group.width/2;
	int y = c_group.y + c_group.height/2;
	sprintf(command, "ratpoison -c 'ratwarp %d %d'", x, y);
	system(command);

	update_session(session);
}

/**
 * Checks what screen the mouse is on. Focuses that screen.
 */
void update_mouse(ratsession *session)
{
	// Get information about all frames.
	char buffer[8192];
	FILE *fp = popen("ratpoison -c ratinfo", "r");
	fgets(buffer, sizeof(buffer) - 1, fp);
	pclose(fp);

	// Go through information about frames.
	char *pch;
	pch = strtok(buffer, " ");
	int x = atoi(pch);
	pch = strtok(NULL, " ");
	int y = atoi(pch);
	group c_group = session->grouplist[session->current_frame];

	int i = 0;
	for(i = 0; i < session->group_len; i = i + 1)
	{
		group *o_group = session->sortedlist[i];
		if(o_group->enabled)
		{
			if(x >= o_group->x && x < o_group->x + o_group->width && y >= o_group->y && y < o_group->y + o_group->height)
			{
				if(o_group->frame_nr != c_group.frame_nr)
				{
					printf("Mouse on unfocused frame-> Changing focus to %d->\n", o_group->frame_nr);
					char command[100];
					sprintf(command, "ratpoison -c 'fselect %d'", o_group->frame_nr);
					system(command);
					update_session(session);
				}
				break;
			}
		}
	}
}
