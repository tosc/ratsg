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
	// Variables used for piping.
	char buffer[256];
	FILE *fp;
	
	// Runs external ratpoison command that lists all groups.
	fp = popen("ratpoison -c groups", "r");
	while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL)
	{
		int te = buffer[0] - '0';
		if (buffer[1] == '*')
		{
			return te;
		}
	}
	pclose(fp);

	return 0;
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
 * Update the windows in the group supplied.
 *
 * @param grouplist The list of groups to update.
 */
void update_groups(group *grouplist)
{
	// Change all current windows to inactive.
	group *c_group = grouplist;
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
	int c_group_nr = current_group();	
	int i = 0;
	c_group = grouplist;
	while(i < c_group_nr)
	{
		if(c_group->next == NULL)
		{
			c_group->next = new_group();
		}
		c_group = c_group->next;
		i++;
	}

	// Run the external ratpoison command to get all the current windows.
	char buffer[256];
	FILE *fp;
	fp = popen("ratpoison -c windows", "r");

	if(c_group_nr == current_group())
	{
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
	}
	pclose(fp);
}

/**
 * Returns a string representing all groups and windows.
 *
 * @param groups The groups you want a string representation of.
 * @param group_string Where to store the string representation.
 * @return A string representation of all groups and windows.
 */
void groups_to_string(group *grouplist, char *group_string)
{
	int first = 1;
	strcpy(group_string, "");
	group *c_group = grouplist;
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
