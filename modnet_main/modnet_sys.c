/* Modnet system calls implementation.
 * Author: Sharvanath Pathak */
#include "../include/modnet.h"
#include <unistd.h>
#include <sys/syscall.h>

int modnet_register(char * module_name) {
	return syscall(__NR_modnet_register, module_name);
}

int modnet_apply(int pid, char * modules[], int num_mods) {
	return syscall(__NR_modnet_apply, pid, modules, num_mods);
}

int modnet_getsockets(int left_fds[], int right_fds[], 
		int * array_length, long cpu_mask) {
	return syscall(__NR_modnet_getsockets, left_fds, right_fds, 
			array_length, cpu_mask);
}

int modnet_isock_send(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr_storage *dest_addr, socklen_t addrlen) {
	return syscall(__NR_modnet_isock_send, sockfd, buf, len, flags, dest_addr, 
			addrlen);
}
