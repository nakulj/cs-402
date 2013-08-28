#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

/*
   Program to run in an xterm window to interact with the capital cities
   database program.
 */

main(int argc, const char *argv[])
{
	char rbuf[256];
	char qbuf[256];
	int rlen;
	int qlen;
	int ret;
	int ifd;
	int ofd;
	int err;
	struct iovec vec[2];
	int pid = getpid();
	void terminate();
	struct sigaction sa;
	FILE *infile;

	sa.sa_handler = terminate;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, 0);

#ifdef __RUN_WINDOW_SCRIPT__
	sa.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sa, 0);
#endif

	if (argc != 3) {
		fprintf(stderr, "Usage: interface infile outfile\n");
		exit(1);
	}

	if ((ifd = open(argv[2], O_RDONLY)) == -1) {
		fprintf(stderr, "%s", argv[2]);
		perror("open ififo");
		sleep(10);
		exit(1);
	}

	if ((ofd = open(argv[1], O_WRONLY)) == -1) {
		fprintf(stderr, "%s", argv[1]);
		perror("open ofifo");
		sleep(10);
		exit(1);
	}

	if ((err = write(ofd, &pid, 4)) < 0) {
		fprintf(stderr, "write error code: %d\n", err);
		sleep(10);
		exit(1);
	}

	/* now that we've opened the FIFOs, make them go away when the
	   process terminates */
	unlink(argv[2]);
	unlink(argv[1]);

	rbuf[0] = '\0';

#ifdef __RUN_WINDOW_SCRIPT__
	infile = fopen("WindowScript", "r");
	if (infile == 0)
		terminate();
#else
	infile = stdin;
#endif

	for (;;) {
		/* print the previous response (if any) and the prompt */
		printf("%s\n >> ", rbuf);
		fflush(stdout);

		if (fgets(qbuf, sizeof(qbuf), infile) == NULL) {
			/* EOF means to terminate */
			qlen = -1;
			if (write(ofd, &qlen, sizeof(qlen)) == -1) {
				perror("write");
				sleep(10);
				exit(1);
			}
			printf("\nGoodbye\n");
			continue;	/* force the window to be terminated by the control pgm */
		} else {
#ifdef __RUN_WINDOW_SCRIPT__
			fputs(qbuf, stdout);
#endif

			/* we have a query, send it across in two parts:
			   the length and the string */
			qlen = strlen(qbuf);
			qbuf[qlen - 1] = '\0';	/* strip off newline */
			vec[0].iov_len = sizeof(qlen);
			vec[0].iov_base = (char *)&qlen;
			vec[1].iov_len = qlen;
			vec[1].iov_base = qbuf;

			if (writev(ofd, (const struct iovec *)&vec, 2) == -1) {
				perror("writev");
				sleep(10);
				exit(1);
			}
		}

		/* get the length of the response string */
		if ((ret = read(ifd, &rlen, sizeof(rlen))) == -1) {
			perror("read");
			sleep(10);
			exit(1);
		}

		if (ret != sizeof(rlen)) {
			fprintf(stderr, "read1 fetched %d bytes\n", ret);
			sleep(10);
			exit(1);
		}

		/* get the response string */
		if ((ret = read(ifd, rbuf, rlen)) == -1) {
			perror("read");
			sleep(10);
			exit(1);
		}

		if (ret != rlen) {
			fprintf(stderr, "read3 fetched %d bytes\n", ret);
			sleep(10);
			exit(1);
		}
	}
}

void terminate()
{
	fprintf(stderr, "\nGoodbye\n");
	exit(0);
}
