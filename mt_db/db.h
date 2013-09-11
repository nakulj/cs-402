#include <pthread.h>

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
