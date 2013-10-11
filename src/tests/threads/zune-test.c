#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "devices/timer.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

static thread_func zune_thread_func;

void
zune_test (void) 
{
	char name[16] = "thread 1 name";
	int priority = 10;
	
	if (thread_mlfqs)
	{
		printf("mlfqs true\n");
	} else {
		printf("mlfqs false\n");
	}

	printf("Initialized. Calling get ready threads: %d \n", get_ready_threads_count());
	thread_create(name, priority, zune_thread_func, NULL);
	printf("1 Thread Created. Calling get ready threads: %d \n", get_ready_threads_count());
	
	printf("Initialized Load Avg: ");
	print_real(thread_get_load_avg());
	thread_calc_load_avg();
	printf("\nRecalculated Load Avg: ");
	print_real(thread_get_load_avg());
}

static void
zune_thread_func(void *aux UNUSED)  
{
	printf("thread created\n");
	return;
}