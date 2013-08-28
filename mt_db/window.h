typedef struct window {
	int in;
	int out;
	int pid;
} window_t;

window_t *window_create(char *);
void window_destroy(window_t *);
void serve(window_t *, char *, char *);
