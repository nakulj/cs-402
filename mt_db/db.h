#include <pthread.h>

typedef struct {
	pthread_mutex_t rw_lock;
	pthread_cond_t 	ready_for_read;
	pthread_cond_t 	ready_for_write;

	int num_readers;
	int num_writers;
	int num_waiting_writers;
} lockunit_t;

typedef struct Node {
	char *name;
	char *value;
	struct Node *lchild;
	struct Node *rchild;
#ifdef FINE_LOCK
	lockunit_t node_lock;
#endif
} node_t;

extern node_t head;

void init_db();
void interpret_command(char *, char *, int);
