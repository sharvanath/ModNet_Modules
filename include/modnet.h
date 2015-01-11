/* Modnet Header File
 * Author: Sharvanath Pathak */

#ifndef MODNET_H
#define MODNET_H
#include <sys/uio.h>
#include "modnet_sys.h"

typedef struct modnet_module_operations {
	// TODO(sharva) support sg, multiple iovecs.
	struct iovec (*process_left)(struct iovec input, void *state_ptr);
	struct iovec (*process_right)(struct iovec input, void *state_ptr);
	/* This will be called when the connection is being deleted.*/
	void (*delete_connection)(void *state_ptr);
	/* Return the state_ptr, which is transparent for modnet, but
	 * will be passed to process_left, process_right and delete_connection.*/
	void * (*init_connection)();
	/* Should be set if the user wants the connections to be blocking */
	/* However the blocking case is not properly handled. */
	int blocking;
	/* For blocking case more than one worker per core is desirable, and user
	 * can configure it through this parameter.*/
	int workers_per_core;
	/* String representing module name */
	char * module_name;
} modnet_module_operations_t;

int modnet_main(modnet_module_operations_t * module);
#endif