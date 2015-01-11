/* Modnet Header File
 * Modnet system calls, and related structures.
 * Author: Sharvanath Pathak */

#ifndef MODNET_SYS_H
#define MODNET_SYS_H

#include <stdint.h>
#include <sys/socket.h>

#define	__NR_modnet_getsockets		400 	
#define	__NR_modnet_register    	401
#define __NR_modnet_apply       	402
#define __NR_modnet_isock_send		403
#define __NR_modnet_isock_yank		404
#define __NR_modnet_map_last_sock 	405
#define __NR_modnet_yank			406
#define __NR_modnet_yankputdata		407
#define __NR_modnet_yankdata		408
#define __NR_modnet_interception	409
#define __NR_modnet_module_end		410
#define __NR_modnet_yield			411

struct tcp_sock_stats {
	uint32_t snd_cwnd;
	uint32_t snd_una;
	uint32_t write_seq;  
	uint32_t mss_size;
	uint32_t srtt;
	uint32_t snd_wnd;
};

int modnet_register(char * module_name);

int modnet_apply(int pid, char * modules[], int num_mods);

int modnet_getsockets(int left_fds[], int right_fds[], 
		int * array_length, long cpu_mask);

int modnet_isock_send(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr_storage *dest_addr, socklen_t addrlen);

#endif
