/* Modnet Dummy Module
 * Author: Sharvanath Pathak */

/* For setaffinity */
#define _GNU_SOURCE 
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* For inet_ntoa. */
#include <arpa/inet.h>
/* Required by event.h. */
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
/* Libevent. */
#include <event2/event.h>
#include <event2/thread.h>
#include "../include/modnet.h"

#define BUFF_SIZE (1024*10)
int NUM_WORKERS = 4;

struct event_base * main_base;
void on_read(evutil_socket_t fd, short ev, void *arg);
void on_write(evutil_socket_t fd, short ev, void *arg);

/**
 * A struct for client specific data, in this simple case the only
 * client specific data is the read event.
 */
struct client {
	int fd;
	struct event * ev_read;
	struct event * ev_write;
	struct client * other;
	int is_app;
	int closed;
	char buffer[BUFF_SIZE];
	char read_buffer[BUFF_SIZE];
	int off;
	int len;
	struct sockaddr_storage temp_addr;
	socklen_t addr_len;
};

/**
 * Set a socket to non-blocking mode.
 */
int setnonblock_fd(int fd)
{

	int flags;
	
	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		return -1;

	return 0;

}

struct client * alloc_client(int fd){

	setnonblock_fd(fd);
	struct client * client = (struct client *)malloc(sizeof(struct client));
	client->fd = fd;
	client->closed = 0;
	client->off = 0;
	client->len = 0;
	return client;	
}

void register_client_event(struct client * client){

	client->ev_read = event_new(
			main_base, 
			client->fd, 
			EV_READ|EV_PERSIST, 
			on_read, 
			client);
	event_add(client->ev_read, NULL);
	client->ev_write = event_new(
			main_base, 
			client->fd,
			EV_WRITE|EV_PERSIST, 
			on_write, 
			client);
}

void close_free_client(struct client * client){

	close(client->fd);
	event_free(client->ev_read);
	event_free(client->ev_write);
	free(client);
}


void close_client(struct client * client){

	close_free_client(client->other);
	close_free_client(client);
}


void on_write(evutil_socket_t fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	int wlen;

	if (client->len>0)
	{

		if(client->is_app == 1) {
			wlen = modnet_isock_send(
					client->fd, 
					client->buffer + client->off, 
					client->len, 
					MSG_NOSIGNAL, 
					&client->other->temp_addr, 
					client->other->addr_len);
		} 
		else {
			wlen = send(client->fd, 
					client->buffer + client->off, 
					client->len, 
					MSG_NOSIGNAL);
		}
		
		if(wlen<0) {
			fprintf(stderr, "on_write wlen=%d, errno=%d\n",wlen, errno);
		}
		
		if (wlen==-1 && errno==EAGAIN)
			wlen = 0;
		
		else if(wlen==-1)
		{
			perror("on_write");
			event_del(client->ev_write);
			if(client->other->len>0)
			{
				event_del(client->other->ev_write);
			}
			else
			{
				// either my read event or other's write event
				event_del(client->ev_read);
			}
			close_client(client);
			return;
		}

		client->len -= wlen;
		client->off += wlen;

	}

	if(client->len==0)
	{
		// either my read event or other's write event
		event_del(client->ev_write);
		event_add(client->other->ev_read, NULL);
	}

}

/**
 * This function will be called by libevent when the client socket is
 * ready for reading.
 */
void
on_read(evutil_socket_t fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	client->addr_len = sizeof(client->temp_addr);	
	int len, wlen;
	
	//only when reading from nic
	if(client->is_app == 0)
	{
		len = recvfrom(fd, 
				client->read_buffer,
				sizeof(client->read_buffer),
				MSG_DONTWAIT,
				(struct sockaddr *) &client->temp_addr,
				&client->addr_len);
	}
	else
	{
		len = read(fd, client->read_buffer, sizeof(client->read_buffer));
	}

	if (len < 0 && errno==EAGAIN)
	{
		goto FREE_BUF;
	}
	else if (len == 0) {
		/* Client disconnected, remove the read event and the
		 * free the client structure. */
		if(shutdown(client->other->fd,SHUT_WR)==-1 && errno == ENOTCONN)
		{
			if(client->len==0)
				event_del(client->other->ev_read);
			else
				event_del(client->ev_write);

			client->other->closed = 1;
		}

		client->closed = 1;
		event_del(client->ev_read);
		
		// note that unless there is a connection break, this is the point 
		// where it will always end, since the last read event 
		// added will do this. the write event never closes anything unless 
		// it gets EPIPE.
		if(client->closed&&client->other->closed)
		{
			close_client(client);
		}

		goto FREE_BUF;
	}
	else if (len < 0) {
		/* Some other error occurred, close the socket, remove
		 * the event and free the client structure. */
		perror("disconnected in on_read");
		event_del(client->ev_read);

		if(client->len>0)
		{
			event_del(client->ev_write);
		}
		else
		{
			// either my read event or other's write event
			event_del(client->other->ev_read);
		}

		close_client(client);

		goto FREE_BUF;
	}


	// write the data, put into the buffer of other if it blocks
	if(client->is_app == 0 && client->addr_len!=0)
	{
		wlen = modnet_isock_send(client->other->fd, client->read_buffer, len,
				MSG_NOSIGNAL, &client->temp_addr, client->addr_len);
	}
	else
	{
		wlen = send(client->other->fd,client->read_buffer,len,MSG_NOSIGNAL);
	}

	if(wlen==-1 && errno==EPIPE)
	{
		if(client->is_app)
			printf("EPIPE from client side in on_read\n");
		else
			printf("EPIPE from nic side in on_read\n");


		event_del(client->ev_read);

		if(client->len>0)
		{
			event_del(client->ev_write);
		}
		else
		{
			// either my read event or other's write event
			event_del(client->other->ev_read);
		}

		close_client(client);
		goto FREE_BUF;
	}

	if (wlen==-1 && errno==EAGAIN)
		wlen = 0;

	if (wlen < len) {
		// We didn't write all our data.  

		void * x = memcpy(
				client->other->buffer,
				client->read_buffer+wlen,
				len - wlen);

		if(x!=client->other->buffer)
			printf("Error in memcpy. "
					"Short write, not all data echoed back to client. "
					"wlen %d, len %d\n", wlen, len);

		client->other->len = len - wlen;
		client->other->off = 0;

		event_del(client->ev_read);
		event_add(client->other->ev_write, NULL);

	}

	FREE_BUF: ;

}

void register_client(int fd_app, int fd_nic) {
	struct client *client_app, *client_nic;
	client_app = alloc_client(fd_app);
	client_nic = alloc_client(fd_nic);
	if(client_app == NULL || client_nic == NULL)
	{
		printf("Error in malloc\n");
		exit(1);
	}

	client_app->other = client_nic;
	client_nic->other = client_app;

	client_app->is_app = 1;
	client_nic->is_app = 0;

	register_client_event(client_app);
	register_client_event(client_nic);
}

void get_sockets(long * cpu_mask) {

	int num = 10000;
	int val = 1;
	int fd_app[num], fd_nic[num];

	val = 0;
	val = modnet_getsockets(fd_app, fd_nic, &num, *cpu_mask);
	printf("Dummy module stole %d socket(s)\n", val);
	int i = 0;
	for(i = 0; i < val; i++)
	{	
		register_client(fd_app[i], fd_nic[i]);
	}
}

void * dispatcher(long * cpu_mask) {
	int epfd_central = epoll_create(10);
	struct epoll_event ev, events[1];
	int nfds;

	ev.events = 0x0800;
	ev.data.fd = 0;
	if(epoll_ctl(epfd_central, EPOLL_CTL_ADD, -1, &ev)==-1){
		perror("epoll_ctl: failed");
	}

	while(1){

		nfds = epoll_wait(epfd_central, events, 1, -1);
		if(nfds != 1)
	      printf("woke from event but the ndfs=%d\n", nfds);

		get_sockets(cpu_mask);
	}
}

void * event_worker(void * ptr){

	main_base = event_base_new();
	event_base_loop(main_base, EVLOOP_NO_EXIT_ON_EMPTY);

	return NULL;
}

int get_cpu_count()
{
	cpu_set_t cs;
	CPU_ZERO(&cs);
	sched_getaffinity(0, sizeof(cs), &cs);

	int count = 0,i=0;
	for (i = 0; i < 8; i++)
	{
		if (CPU_ISSET(i, &cs))
			count++;
	}
	return count;
}

int
main(int argc, char **argv)
{

	if(argc < 2)
	{
		printf("usage: dummy_module <module name>\n");
		return 1;
	}

	int reg = modnet_register(argv[1]);

	if(reg < 0)
	{
		printf("vnet_register failed\n");
		return 1;
	}

	long mask = 1;
	int x = 0;

	// determining the number of logical CPUs, 
	// since we have one worker per logical cpu
	NUM_WORKERS = get_cpu_count();
	printf("Detected %d logical allowed CPUs\n", NUM_WORKERS);

	for(x = 0; x < NUM_WORKERS - 1; x++){
		if(fork() == 0)
			break;
		mask = mask << 1;
	}

	sched_setaffinity(0, sizeof(mask), (cpu_set_t *)&mask);

	int i  = evthread_use_pthreads();
	if(i != 0)
		perror("evthread_use_pthreads failed\n");

	pthread_t fishing_thread;
	int err = pthread_create(&fishing_thread, NULL, &event_worker, NULL);

	if(err < 0)
		perror("While creating the fishing thread\n");

	dispatcher(&mask);

	return 0;
}
