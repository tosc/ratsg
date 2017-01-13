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
	char buf[COMMAND_MAX];
	fd = open(fifo, O_RDONLY);
	if(read(fd, buf, COMMAND_MAX) > 0)
	{
		strcpy(info, buf);
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
 * Starts a server daemon.
 */
void server()
{
	// Initialize the grouplist.
	ratsession *session = new_session();
	
	// Updates existing ratpoison group to follow my naming.
	system("ratpoison -c 'grename :0'");

	while(1)
	{
		// Gets all the ratpoison information.
		update_session(session);

		// Check for new commands.
		char command[COMMAND_MAX];
		if(get_info(command, FIFO_COMMAND))
		{
			printf("%s\n", command);

			// Print session
			if(strcmp(command, "status") == 0)
			{
				char session_string[COMMAND_MAX];
				session_to_string(session, session_string);
				send_info(session_string, FIFO_OUTPUT);
			}
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
		send_info(argv[1], FIFO_COMMAND);

		if(strcmp(argv[1], "status") == 0)
		{
			// Wait for response from server and print it.
			while(1)
			{
				char output[COMMAND_MAX];
				if (get_info(output, FIFO_OUTPUT))
				{
					printf("%s", output);
					break;
				}
			}
		}
	}
	return 0;
}
