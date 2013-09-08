#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "window.h"
#include "db.h"
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

/* the encapsulation of a client thread, i.e., the thread that handles
 * commands from clients */
typedef struct Client {
	pthread_t thread;
	window_t *win;
} client_t;

void *client_run(void *);
int handle_command(char *, char *, int len);

client_t *client_create(int ID)
{
	client_t *new_Client = (client_t *) malloc(sizeof(client_t));
	char title[16];

	if (new_Client == 0)
		return 0;

	sprintf(title, "Client %d", ID);

	/* Creates a window and set up a communication channel with it */
	new_Client->win = window_create(title);

	return new_Client;
}

void client_destroy(client_t *client)
{
	/* Remove the window */
	window_destroy(client->win);
	free(client);
}

/* Code executed by the client */
void *client_run(void *arg)
{
	client_t *client = (client_t *) arg;

	/* main loop of the client: fetch commands from window, interpret
	 * and handle them, return results to window. */
	char command[256];
	char response[256] = { 0 };	/* response must be null for the first call to serve */

	serve(client->win, response, command);
	while (handle_command(command, response, sizeof(response))) {
		serve(client->win, response, command);
	}
	client_destroy(client);
	return 0;
}

int handle_command(char *command, char *response, int len)
{
	if (command[0] == EOF) {
		strncpy(response, "all done", len - 1);
		return 0;
	}

	interpret_command(command, response, len);

	return 1;
}

int main(int argc, char *argv[])
{
	client_t *c;
	int clientCtr=0;
	char command[256];
	init_db();
	if (argc != 1) {
		fprintf(stderr, "Usage: server\n");
		exit(1);
	}
	
	// while(1) {
	// 	scanf("%s",command);
	// 	if(strcmp(command,"e")!=0) break;
	// 	c = client_create(clientCtr++);
	// 	pthread_t *theThread= &(c->thread);
	// 	pthread_create(theThread, NULL, client_run, (void *)c);
	// 	//client_run((void *)c);
	// }
	int i = 0;
	for(i = 0; i < 5; i++) {
		c = client_create(clientCtr++);
		pthread_t *theThread= &(c->thread);
		pthread_create(theThread, NULL, client_run, (void *)c);
	}
	
	return 0;
}
