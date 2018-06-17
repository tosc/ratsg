#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "ratcommands.h"

#define FIFO_COMMAND "/tmp/ratstatus"
#define FIFO_OUTPUT "/tmp/ratoutput"
#define COMMAND_MAX 2048

/**
 * Write info to pipe.
 *
 * @param command The info you wish to write to the pipe.
 * @param fifo The name of the fifo-pipe where you will write the info.
 */
void send_info(char *command, char *fifo)
{
	int fd;
	mkfifo(fifo, 0666);

	fd = open(fifo, O_WRONLY);
	write(fd, command, strlen(command));
	close(fd);
	unlink(fifo);
}

/**
 * Receive info from pipe.
 *
 * @param command Where the info will be stored.
 * @param fifo The name of the fifo-pipe from where you get the info.
 * @return 1 if successful.
 */
int get_info(char *info, char *fifo)
{
	int status = 0;
	int fd;
	fd = open(fifo, O_RDONLY);
	if(read(fd, info, COMMAND_MAX) > 0)
	{
		status = 1;
	}
	else
	{
		strcpy(info, "");
	}
	close(fd);
	return status;
}

/**
 * Write to status file.
 */
 void write_status(ratsession *session)
 {
	FILE *f;
	f = fopen("/tmp/ratbar", "w");
	if(f == NULL)
	{
		printf("Error!");
		exit(1);
	}

	char session_string[COMMAND_MAX];
	session_to_string(session, session_string);
	fprintf(f, "%s", session_string);
	fclose(f);
}

/**
 * Starts a server daemon.
 */
void server()
{
	// Updates existing ratpoison group to follow my naming.
	system("ratpoison -c 'grename :0'");

	// Initialize the grouplist.
	ratsession *session = new_session();

	while(1)
	{
		// Update session information.
		update_session(session);
		update_mouse(session);

		// Check for new commands.
		char command[COMMAND_MAX];
		if(get_info(command, FIFO_COMMAND))
		{
			printf("%s\n", command);
			printf("%d\n", session->current_frame);

			if(strcmp(command, "move-l") == 0)
			{
				move_l(session);
			}
			else if(strcmp(command, "move-r") == 0)
			{
				move_r(session);
			}
			else if(strcmp(command, "scrn-l") == 0)
			{
				screen_l(session);
			}
			else if(strcmp(command, "scrn-r") == 0)
			{
				screen_r(session);
			}
		}
		else
		{
			write_status(session);
		}
	}
}

int main( int argc, char *argv[] )
{
	// Start server if program is called without arguments.
	if(argc == 1)
	{
		printf("Starting server....\n");
		
		server();
	}
	// If program is called with an arguement, send that arguement
	// to the currently running server.
	if(argc == 2)
	{
		if(strcmp(argv[1], "status") == 0)
		{
			// Wait for response from server and print it.
			FILE *f;
			f = fopen("/tmp/ratbar", "r");
			if(f == NULL)
			{
				printf("Error!");
				exit(1);
			}
			
			char ch;
			while((ch = fgetc(f)) != EOF)
			{
				printf("%c", ch);
			}
		}
		else
		{
			char info[COMMAND_MAX];
			strcpy(info, argv[1]);
			info[COMMAND_MAX - 1] = '\0';
			send_info(argv[1], FIFO_COMMAND);
		}
	}
	return 0;
}
