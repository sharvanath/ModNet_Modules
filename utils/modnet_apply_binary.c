#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../include/modnet.h"

int main(int argc, char *argv[]){

	if(argc < 4){
		fprintf(stderr,"usage: modnet_apply fname modname num_mods\n");
		return 1;
	}	

	int num_mods = atoi(argv[argc - 1]);
	if(num_mods < 0 || num_mods > argc - 3)
	{
		fprintf(stderr,"usage: modnet_apply fname modname num_mods\n");
		return 1;

	}

    // terminal arguments to the binary
	int num_args = argc - (num_mods + 2); 	
	char * args[num_args];
	int a;
	for(a = 0; a < num_args; a++)
		args[a]  = argv[a + 1];
	args[num_args] = NULL;

	int val = 0;
	
	val = modnet_apply(-1, argv + argc - num_mods - 1, num_mods);

	if(val >= 0)
		printf("successfully applied module, num_mods=%d, num_args=%d\n", 
				num_mods, num_args);
	else
	{
		printf("vnet_apply error, with num_mods=%d, err=\"%s\"\n", 
				num_mods, strerror(errno));
		return 1;
	}

	return execv(argv[1], args);	
}
