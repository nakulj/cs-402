#ifndef THREADS_READY_LIST_H
#define THREADS_READY_LIST_H

#include "lib/kernel/list.h"
#include "threads/thread.h"

/*
typedef struct {
	struct list ready_list;
} ready_list_t;
*/

static bool
priority_less (const struct list_elem *a_, const struct list_elem *b_,
            void *aux UNUSED) 
{
  const struct thread *a = list_entry (a_, struct thread, elem);
  const struct thread *b = list_entry (b_, struct thread, elem);
  
  return get_priority(a) < get_priority(b);
}


#endif
