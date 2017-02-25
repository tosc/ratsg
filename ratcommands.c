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
	// Run the external ratpoison command to get all the current windows.
	char buffer[8192];
	FILE *fp;
	fp = popen("ratpoison -c sdump", "r");

	// Add a new window for each line.
	fgets(buffer, sizeof(buffer) - 1, fp);
	pclose(fp);

	// Backup the information from the ratpoison command.
	char bufferold[8192];
	strcpy(bufferold, buffer);

	// Get the number of screens active and save it in the variable screen.
	char *pch;
	pch = strtok(buffer, "() ,\n");
	int i = 0;
	int screen_nr = 0;
	while(pch != NULL)
	{
		if(i == 0)
		{
			screen_nr++;
		}
		pch = strtok(NULL, "() ,\n");
		i++;
		if(i == 6)
		{
			i = 0;
		}
	}
	session->group_len = screen_nr;

	// Make room for all the screens.
	group *groups = malloc(sizeof(group) * screen_nr);

	// Get the information about all the screens.
// Get current group. Creates more groups if needed.
	strcpy(buffer, bufferold);
	pch = strtok(buffer, "() ,\n");
	i = 0;
	screen_nr = 0;
	while(pch != NULL)
	{
		if(i == 0)
		{
			screen_nr++;
		}
		else if(i == 1)
		{
			char command[128];
			sprintf(command, "ratpoison -c 'gnewbg :%d'", screen_nr - 1);
			system(command);
			groups[screen_nr - 1].x = atoi(pch);
			groups[screen_nr - 1].sorted_nr = screen_nr - 1;
			groups[screen_nr - 1].windowlist = NULL;
		}
		pch = strtok(NULL, "() ,\n");
		i++;
		if(i == 6)
		{
			i = 0;
		}
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
	create_groups(new_session);
	return new_session;
}

/**
 * Update the current group.
 */
void update_group(ratsession *session)
{
	char buffer[8192];
	// Run the external ratpoison command to get all the current windows.
	FILE *fp = popen("ratpoison -c windows", "r");

	group c_group = session->grouplist[session->current_screen];
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
	session->grouplist[session->current_screen] = c_group;
	pclose(fp);
}

/**
 * Update the session.
 *
 * @param session The current ratsession.
 */
void update_session(ratsession *session)
{
	// Update screennr.
	session->current_screen = current_screen();

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
	sprintf(command, "ratpoison -c 'gselect :%d'", session->current_screen);
	system(command);

	// Update current groups information.
	update_group(session);
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
	int lastX = -1;
	for(i = 0; i < session->group_len; i = i + 1)
	{
		// Sort screens by X.
		group c_group;
		int j;
		for(j = 0; j < session->group_len; j = j + 1)
		{
			group t_group = session->grouplist[j];
			if(t_group.x > lastX)
			{
				c_group = t_group;
				break;
			}
		}
		for(j = 0; j < session->group_len; j = j + 1)
		{
			group t_group = session->grouplist[j];
			if(t_group.x > lastX)
			{
				if(t_group.x < c_group.x)
				{
					c_group = t_group;
				}
			}
		}
		lastX = c_group.x;
		window *c_window = c_group.windowlist;

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
