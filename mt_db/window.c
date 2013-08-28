#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "window.h"

int window_count = 0;

window_t *window_create(char *label)
{
	window_t *new_window = (window_t *) malloc(sizeof(window_t));
	char ififo[] = "/tmp/twiniYXXXXXX";
	char ofifo[] = "/tmp/twinoYXXXXXX";
	static char itoa[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
	};

	if (new_window == 0)
		return 0;

	/* mktemp only produces 26 unique names.  By adding another letter, we
	 * allow 26 times as many. */

	ififo[10] = ofifo[10] = itoa[(window_count++) / sizeof(itoa)];
	if (window_count >= sizeof(itoa) * sizeof(itoa)) {
		fprintf(stderr, "could not create FIFO name\n");
		free(new_window);
		return 0;
	}
	mkstemp(ififo);
	mkstemp(ofifo);

	/* cleanup after past failures */
	unlink(ififo);

	/* create the fifo (named pipe) for receiving data from the window */
	if (mkfifo(ififo, 0600) == -1) {
		perror("mkfifo");
		exit(1);
	}

	unlink(ofifo);

	/* create the fifo for transmitting data to the window */
	if (mkfifo(ofifo, 0600) == -1) {
		perror("mkfifo");
		free(new_window);
		return 0;
	}

	if ((new_window->in = open(ififo, O_RDWR)) == -1) {
		free(new_window);
		fprintf(stderr, "cannot open ififo\n");
		return 0;
	}

	if ((new_window->out = open(ofifo, O_RDWR)) == -1) {
		free(new_window);
		fprintf(stderr, "cannot open ofifo\n");
		return 0;
	}

	new_window->pid = -1;
	switch (fork()) {
	case -1:
		fprintf(stderr, "could not create process for new window\n");
		free(new_window);
		return 0;

	case 0:
		/* run xterm in the child */
		if (execlp("xterm", "xterm", "-T", label, "-n", label, "-ut",
			   "-geometry", "35x20",
			   "-e", "./interface",
			   ififo, ofifo, (char *)NULL) == -1) {
			perror("exec of xterm failed");
			exit(1);
		}
		perror("exec somehow failed");
		exit(1);

	default:
		break;
	}

	read(new_window->in, &new_window->pid, 4);

	return new_window;
}

void window_destroy(window_t * win)
{
	kill(win->pid, SIGTERM);
}

void serve(window_t * window, char *response, char *query)
{
	int rlen = strlen(response) + 1;
	int qlen;
	int ret;

	/* a null response must be sent, and only sent, on the first call
	 * to serve */
	if (rlen > 1) {
		/* send the response in two parts: first the length,
		 * then the string */
		const struct iovec vec[2] = { (void *)&rlen, sizeof(rlen),
			(void *)response, rlen
		};
		if (writev(window->out, (const struct iovec *)&vec, 2) == -1) {
			perror("writev");
			exit(1);
		}
	}

	while (1) {
		/* receive the length of the next query */
		if ((ret = read(window->in, &qlen, sizeof(qlen))) == -1) {
			/* Handle interrupted system calls */
			if (errno == EINTR)
				continue;
			perror("read");
			exit(1);
		}
		break;
	}

	if (ret != sizeof(qlen)) {
		fprintf(stderr,
			"Wrong count returned from read of window: %d\n", ret);
		exit(1);
	}

	if (qlen < 0) {
		query[0] = EOF;
		return;
	}

	if (qlen > 256)
		qlen = 256;

	/* receive the string portion of the query */
	while (1) {
		if ((ret = read(window->in, query, qlen)) == -1) {
			if (errno == EINTR)
				continue;
			perror("I/O error on read from window");
			exit(1);
		}
		break;
	}

	if (ret != qlen) {
		perror("I/O error");
		exit(1);
	}
}
