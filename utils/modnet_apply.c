#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../include/modnet.h"

int main(int argc, char *argv[]){

	if(argc < 3){
		fprintf(stderr,"usage: modnet_apply pid modnames\n");
		return 1;
	}	

    // terminal arguments to the binary
	
	int val = 0;
	int pid = atoi(argv[1]);	
	int num_mods = argc - 2;
	printf("applying %d modules to pid=%d\n", num_mods, pid);
	val = modnet_apply(pid, argv + 2, argc - 2);

	if(val >= 0)
	{
		printf("successfully applied module, num_mods=%d\n", 
				num_mods);
		return 0;
	}
	else
	{
		printf("modnet_apply error, with num_mods=%d, err=\"%s\"\n", 
				num_mods, strerror(errno));
		return 1;
	}
}
