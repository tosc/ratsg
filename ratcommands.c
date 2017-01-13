#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "ratcommands.h"

/**
 * Returns the current group number.
 *
 * @return the current group number.
 */
int current_group()
{
	int c_group = 0;
	// Variables used for piping.
	char buffer[256];
	FILE *fp;

	// Runs external ratpoison command that lists all groups.
	fp = popen("ratpoison -c groups", "r");
	while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL)
	{
		c_group = buffer[0] - '0';
		if (buffer[1] == '*')
		{
			break;
		}
	}
	pclose(fp);
	return c_group;
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
 * Create a new group.
 *
 * @return Returns a pointer to a new group.
 */
group* new_group()
{
	group *new_group = malloc(sizeof(group));
	new_group->next = NULL;
	new_group->windowlist = NULL;
	return new_group;
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
	new_session->grouplist = new_group();
	new_session->current_frame = 0;
	new_session->current_group = 0;
	return new_session;
}

/**
 * Update the session.
 *
 * @param session The current ratsession.
 */
void update_session(ratsession *session)
{
	// Update framenr.
	session->current_frame = current_frame();

	// Change all current windows to inactive.
	group *c_group = session->grouplist;
	while(c_group != NULL)
	{
		window *c_window = c_group->windowlist;
		while(c_window != NULL)
		{
			c_window->status = 0;
			c_window = c_window->next;
		}
		c_group = c_group->next;
	}


	// Get current group. Creates more groups if needed.
	char command[128];
	int c_group_nr = session->current_frame;
	int i = 0;
	c_group = session->grouplist;
	while(i < c_group_nr)
	{
		if(c_group->next == NULL)
		{
			c_group->next = new_group();

			// Have ratpoison create this group also.
			sprintf(command, "ratpoison -c 'gnewbg :%d:'", i+1);
			system(command);
		}
		c_group = c_group->next;
		i++;
	}
	// Make ratpoison switch to the groups that should be
	// the current one.
	sprintf(command, "ratpoison -c 'gselect :%d:'", session->current_frame);
	system(command);

	// Run the external ratpoison command to get all the current windows.
	char buffer[256];
	FILE *fp;
	fp = popen("ratpoison -c windows", "r");

	free_windows(c_group->windowlist);
	c_group->windowlist = NULL;
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

		if(c_group->windowlist == NULL)
		{
			windowlist = w;
			c_group->windowlist = windowlist;
		}
		else
		{
			windowlist->next = w;
			windowlist = w;
		}
	}
	pclose(fp);
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
	group *c_group = session->grouplist;
	while(c_group != NULL)
	{
		// Add an orange | between each group.
		if(first == 0)
		{
			strcat(group_string, "<fc=#ee9a00>|</fc> ");
		}
		window *c_window = c_group->windowlist;
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
			first = 0;
			c_window = c_window->next;
		}
		c_group = c_group->next;
	}
	strcat(group_string, "\n\0");
}
