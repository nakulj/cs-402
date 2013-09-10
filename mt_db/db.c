#include "db.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
	pthread_mutex_t rw_lock;
	pthread_cond_t 	ready_for_read;
	pthread_cond_t 	ready_for_write;

	int num_readers;
	int num_writers;
	int num_waiting_writers;
} lockunit_t;

#ifdef COARSE_LOCK
lockunit_t db_lockunit;

void init_db() {
	db_lockunit.num_readers = 0;
	db_lockunit.num_writers = 0;
	db_lockunit.num_waiting_writers = 0;

	if (pthread_mutex_init(&db_lockunit.rw_lock, NULL) != 0) {
		printf("\n db_rw_lock mutex init failed\n");
		return;
	}

	if (pthread_cond_init(&db_lockunit.ready_for_read, NULL) != 0) {
		printf("\n read condition variable init failed\n");
		return;
	}

	if (pthread_cond_init(&db_lockunit.ready_for_write, NULL) != 0) {
		printf("\n write condition variable init failed\n");
		return;
	}
}
#else
void init_db() {}
#endif

void start_write(lockunit_t* lockunit) {
	pthread_mutex_lock(&lockunit->rw_lock);
		lockunit->num_waiting_writers ++;
		while( lockunit->num_readers > 0 || lockunit->num_writers > 0 ) {
			pthread_cond_wait(&lockunit->ready_for_write, &lockunit->rw_lock);
		}
		lockunit->num_waiting_writers --;
		lockunit->num_writers ++;
	pthread_mutex_unlock(&lockunit->rw_lock);
}

void end_write(lockunit_t* lockunit) {
	pthread_mutex_lock(&lockunit->rw_lock);
	lockunit->num_writers --;
	if (lockunit->num_waiting_writers > 0) {
		pthread_cond_signal(&lockunit->ready_for_write);	
	} else {
		pthread_cond_broadcast(&lockunit->ready_for_read);
	}
	pthread_mutex_unlock(&lockunit->rw_lock);
}

void start_read(lockunit_t* lockunit) {
	pthread_mutex_lock(&lockunit->rw_lock);
		while( lockunit->num_writers > 0 && lockunit->num_waiting_writers > 0 ) {
			pthread_cond_wait(&lockunit->ready_for_read, &lockunit->rw_lock);
		}
		lockunit->num_readers ++;
	pthread_mutex_unlock(&lockunit->rw_lock);
}

void end_read(lockunit_t* lockunit) {
	pthread_mutex_lock(&lockunit->rw_lock);
	lockunit->num_readers --;
	pthread_cond_signal(&lockunit->ready_for_write);
	pthread_mutex_unlock(&lockunit->rw_lock);
}

node_t *search(char *, node_t *, node_t **);

node_t head = {"", "", 0, 0};

node_t *node_create(char *arg_name, char *arg_value,
		node_t * arg_left, node_t * arg_right) {
	node_t *new_node;

	new_node = (node_t *) malloc(sizeof (node_t));
	if (new_node == 0)
		return 0;

	if ((new_node->name = (char *) malloc(strlen(arg_name) + 1)) == 0) {
		free(new_node);
		return 0;
	}

	if ((new_node->value = (char *) malloc(strlen(arg_value) + 1)) == 0) {
		free(new_node->name);
		free(new_node);
		return 0;
	}

	strcpy(new_node->name, arg_name);
	strcpy(new_node->value, arg_value);
	new_node->lchild = arg_left;
	new_node->rchild = arg_right;

	return new_node;
}

void node_destroy(node_t * node) {
	if (node->name != 0)
		free(node->name);
	if (node->value != 0)
		free(node->value);
	free(node);
}

void query(char *name, char *result, int len) {
#ifdef COARSE_LOCK
	start_read(&db_lockunit);
#endif
	node_t *target;

	target = search(name, &head, 0);

#ifdef COARSE_LOCK
	end_read(&db_lockunit);
#endif
	if (target == 0) {
		strncpy(result, "not found", len - 1);
		return;
	} else {
		strncpy(result, target->value, len - 1);
		return;
	}
}

int add(char *name, char *value) {
#ifdef COARSE_LOCK
	start_write(&db_lockunit);
#endif
	node_t *parent;
	node_t *target;
	node_t *newnode;

	if ((target = search(name, &head, &parent)) != 0) {
#ifdef COARSE_LOCK
		end_write(&db_lockunit);
#endif
		return 0;
	}

	newnode = node_create(name, value, 0, 0);

	if (strcmp(name, parent->name) < 0)
		parent->lchild = newnode;
	else
		parent->rchild = newnode;
#ifdef COARSE_LOCK
	end_write(&db_lockunit);
#endif
	return 1;
}

int xremove(char *name) {
#ifdef COARSE_LOCK
	start_write(&db_lockunit);
#endif
	node_t *parent;
	node_t *dnode;
	node_t *next;
	node_t **pnext;

	/* first, find the node to be removed */
	if ((dnode = search(name, &head, &parent)) == 0) {
		/* it's not there */
#ifdef COARSE_LOCK
		end_write(&db_lockunit);
#endif
		return 0;
	}

	/* we found it.  Now check out the easy cases.  If the node has no
	 * right child, then we can merely replace its parent's pointer to
	 * it with the node's left child. */
	if (dnode->rchild == 0) {
		if (strcmp(dnode->name, parent->name) < 0)
			parent->lchild = dnode->lchild;
		else
			parent->rchild = dnode->lchild;

		/* done with dnode */
		node_destroy(dnode);
	} else if (dnode->lchild == 0) {
		/* ditto if the node had no left child */
		if (strcmp(dnode->name, parent->name) < 0)
			parent->lchild = dnode->rchild;
		else
			parent->rchild = dnode->rchild;

		/* done with dnode */
		node_destroy(dnode);
	} else {
		/* So much for the easy cases ...
		 * We know that all nodes in a node's right subtree have lexicographically
		 * greater names than the node does, and all nodes in a node's left subtree
		 * have lexicographically smaller names than the node does. So, we find
		 * the lexicographically smallest node in the right subtree and replace
		 * the node to be deleted with that node. This new node thus is
		 * lexicographically smaller than all nodes in its right subtree, and
		 * greater than all nodes in its left subtree. Thus the modified tree
		 * is well formed. */

		/* pnext is the address of the pointer which points to next (either parent's
		 * lchild or rchild) */
		pnext = &dnode->rchild;
		next = *pnext;
		while (next->lchild != 0) {
			/* work our way down the lchild chain, finding the smallest node
			 * in the subtree. */
			pnext = &next->lchild;
			next = *pnext;
		}
		strcpy(dnode->name, next->name);
		strcpy(dnode->value, next->value);
		*pnext = next->rchild;
		node_destroy(next);
	}
#ifdef COARSE_LOCK
	end_write(&db_lockunit);
#endif
	return 1;
}

node_t *search(char *name, node_t * parent, node_t ** parentpp) {
	/* Search the tree, starting at parent, for a node containing
	 * name (the "target node").  Return a pointer to the node,
	 * if found, otherwise return 0.  If parentpp is not 0, then it points
	 * to a location at which the address of the parent of the target node
	 * is stored.  If the target node is not found, the location pointed to
	 * by parentpp is set to what would be the the address of the parent of
	 * the target node, if it were there.
	 *
	 * Assumptions:
	 * parent is not null and it does not contain name */

	node_t *next;
	node_t *result;

	if (strcmp(name, parent->name) < 0) {
		next = parent->lchild;
	} else {
		next = parent->rchild;
	}

	if (next == 0) {
		result = 0;
	} else {
		if (strcmp(name, next->name) == 0) {
			result = next;
		} else {
			/* "We have to go deeper!" */
			result = search(name, next, parentpp);
			return result;
		}
	}

	if (parentpp != 0)
		*parentpp = parent;

	return (result);
}

void interpret_command(char *command, char *response, int len) {
	char value[256];
	char ibuf[256];
	char name[256];

	if (strlen(command) <= 1) {
		strncpy(response, "ill-formed command", len - 1);
		return;
	}

	switch (command[0]) {
		case 'q':
			/* Query */
			sscanf(&command[1], "%255s", name);
			if (strlen(name) == 0) {
				strncpy(response, "ill-formed command", len - 1);
				return;
			}

			query(name, response, len);
			if (strlen(response) == 0) {
				strncpy(response, "not found", len - 1);
			}

			return;

		case 'a':
			/* Add to the database */
			sscanf(&command[1], "%255s %255s", name, value);
			if ((strlen(name) == 0) || (strlen(value) == 0)) {
				strncpy(response, "ill-formed command", len - 1);
				return;
			}

			if (add(name, value)) {
				strncpy(response, "added", len - 1);
			} else {
				strncpy(response, "already in database", len - 1);
			}

			return;

		case 'd':
			/* Delete from the database */
			sscanf(&command[1], "%255s", name);
			if (strlen(name) == 0) {
				strncpy(response, "ill-formed command", len - 1);
				return;
			}

			if (xremove(name)) {
				strncpy(response, "removed", len - 1);
			} else {
				strncpy(response, "not in database", len - 1);
			}

			return;

		case 'f':
			/* process the commands in a file (silently) */
			sscanf(&command[1], "%255s", name);
			if (name[0] == '\0') {
				strncpy(response, "ill-formed command", len - 1);
				return;
			}

		{
			FILE *finput = fopen(name, "r");
			if (!finput) {
				strncpy(response, "bad file name", len - 1);
				return;
			}
			while (fgets(ibuf, sizeof (ibuf), finput) != 0) {
				interpret_command(ibuf, response, len);
			}
			fclose(finput);
		}
			strncpy(response, "file processed", len - 1);
			return;

		default:
			strncpy(response, "ill-formed command", len - 1);
			return;
	}
}
