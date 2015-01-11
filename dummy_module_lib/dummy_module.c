/* Modnet Dummy Module
 * Author: Sharvanath Pathak */
#include <stdio.h>
#include <stddef.h>
#include "../include/modnet.h"

struct iovec pass_content(struct iovec input, void *state_ptr) {
	//printf(input.iov_base);
	return input;
}

void delete_connection(void * ptr) {
	printf("dummy deleted connection\n");
}

void * init_connection() {
	printf("dummy module initialized connection\n");
	return NULL;
}

modnet_module_operations_t module_operations = {
		.process_left = &pass_content,
		.process_right = &pass_content,
		.delete_connection = &delete_connection,
		.init_connection = &init_connection,
		.blocking = 0,
		.workers_per_core = 1,
};

int main(int argc, char **argv) {

	if(argc < 2)
	{
		printf("usage: dummy_module <module name>\n");
		return 1;
	}

	module_operations.module_name = argv[1];
	
	return modnet_main(&module_operations);
}