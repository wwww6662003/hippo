/**
 * Hippo Memory Cached
 * Copyright (C) 2012 heaven. All rights reserved.
 *
 * Hippo is a mini open-source cache daemon, mainly used in dynamic data cache.
 *  
 * Use and distribution licensed under the BSD license.  See
 * the LICENSE file for full text.
 *
 * To learn more open-source code, visit: http://code.google.com/p/heavenmvc/
 * Email: wangwei(wwww6662003@163.com)
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#define _GNU_SOURCE
#include <pthread.h>
#include<malloc.h>


#include "anet.h"
#include "hash.h"
#include "event.h"
#include "util.h"
#include "command.h"
#include "hippo.h"

//close idle client
void close_time_out_client(void) {
    /*redisClient *c;
    listIter *li;
    listNode *ln;
    time_t now = time(NULL);

    li = listGetIterator(server.clients,AL_START_HEAD);
    if (!li) return;
    while ((ln = listNextElement(li)) != NULL) {
        c = listNodeValue(ln);
        if (now - c->lastinteraction > server.maxidletime) {
            redisLog(REDIS_DEBUG,"Closing idle client");
            freeClient(c);
        }
    }
    listReleaseIterator(li);*/
}
// Output environment and configure information
void print_server_config(){
	fprintf(stderr, "===================================\n");
	fprintf(stderr, " hippo Configure information\n");
	fprintf(stderr, "===================================\n");
	fprintf(stderr, "Is-Debug\t = %s\n", IS_DEBUG ? "Yes" : "No");
	fprintf(stderr, "Is-Daemon\t = %s\n", IS_DAEMON ? "Yes" : "No");
	fprintf(stderr, "Port\t\t = %d\n", SERVERPORT);
	fprintf(stderr, "Max-Client\t = %d\n", MAX_CLIENT_NUM);
	fprintf(stderr, "Max-MemSize\t = %d\n", MAX_MEM_SIZE);
	fprintf(stderr, "===================================\n\n");
}
//Print status
void print_server_status(){
	char buf[8192];
	int visit_total;
	visit_total = status->visit_add + status->visit_del + status->visit_get;

sprintf(buf, "=============Status===============\n\
STAT version %s\r\n\
STAT pid %d\r\n\
STAT start_time %d\r\n\
STAT run_time %d\r\n\
STAT mem_total %d\r\n\
STAT mem_used %d\r\n\
STAT item_total %d\r\n\
STAT visit_total %d\r\n\
STAT visit_add %d\r\n\
STAT visit_del %d\r\n\
STAT visit_get %d\r\n\
STAT is_debug %d\r\n\
STAT is_daemon %d\r\n\
STAT port %d\r\n\
STAT max_client %d\r\n\
STAT max_tablesize %d\r\n\
===============END================\n", 
status->version, status->pid, status->start_time, status->run_time, 
status->mem_total, status->mem_used, status->item_total, 
visit_total, status->visit_add, status->visit_del, status->visit_get
, status->is_debug, status->is_daemon, status->port
, status->max_client, /*status->max_mem_size,*/ status->max_tablesize);

		printf("%s", buf);
}
// Init status
void init_server_status(){
	status = (hippo_status *)malloc(sizeof(hippo_status));
	memset(status, 0, sizeof(hippo_status));
	sprintf(status->version, "%s", VERSION);
	status->pid			= getpid();	
	status->start_time	= time( (time_t *)NULL );
	status->run_time		= 0;
	status->mem_total		= MAX_MEM_SIZE;
	status->mem_used		= 0;
	status->item_total	= 0;
	status->visit_add		= 0;
	status->visit_del		= 0;
	status->visit_get		= 0;

	status->is_debug=IS_DEBUG;
	status->is_daemon=IS_DAEMON;
	status->port=SERVERPORT;
	status->max_client=MAX_CLIENT_NUM;
	//status->max_mem_size=MAX_MEM_SIZE;
	status->max_tablesize=HASH_TABLE_MAX_SIZE;

}
// Setting status
void set_server_status( unsigned mem_used, unsigned item_total, unsigned visit_add, unsigned visit_del, unsigned visit_get ){
	time_t currtime;

	currtime = time((time_t *)NULL);
	status->run_time		= currtime - status->start_time;
	status->mem_used		+= mem_used;
	status->item_total	+= item_total;
	status->visit_add		+= visit_add;
	status->visit_del		+= visit_del;
	status->visit_get		+= visit_get;
}
// Get status
void get_server_status(){
	time_t currtime;
	currtime = time((time_t *)NULL);
	status->run_time = currtime - status->start_time;
}
// Get memory use size
unsigned get_mem_used(){
	unsigned ret;
	ret = status->mem_used;
	return ret;
}

//free
 void free_client(int client_sock) {
    aeDeleteFileEvent(server.el,client_sock,AE_READABLE);
    aeDeleteFileEvent(server.el,client_sock,AE_WRITABLE);
    close(client_sock);
}

// Process client request
int hippo_proc_client( int client_sock ){
	hippo_client client;
	hippo_command *cmd;
	int ret;
    char head[BUFFER_SIZE], **head_arr, *method;
	int head_num;
	FILE *fp;
	// read and explode cleent input
	fp = fdopen(client_sock, "r");
	memset(head, 0, sizeof(head));
	if ( fgets(head, BUFFER_SIZE, fp) == NULL ){
		send_error_to_client(client_sock, E_SERVER, "not recv client message");
		return FALSE;
	}
	explode(head, ' ', &head_arr, &head_num);
	method  = trim( strtolower( head_arr[0] ) );
	cmd=lookup_command(method);
	if (cmd){
		client.fd=client_sock;
		client.last_cmd_time=time( (time_t *)NULL );
		ret=cmd->proc(&client,head_arr,head_num);
		return ret;
	}else {	//exception
		send_error_to_client(client_sock, E_CLIENT, "That method is not implemented");
		return FALSE;
	}
}
// access a client and handle
void hippo_handle_client( int client_sock){
    int nread;
    int ret;
    while(1){
        anet_non_block(NULL,client_sock);
        anet_tcp_no_delay(NULL,client_sock);
        ioctl(client_sock, FIONREAD, &nread);
        if ( nread > 0 ){
            ret = hippo_proc_client( client_sock);
            if (ret == EXIT){
                free_client(client_sock);
				server.client_num--;
				current_client_num();
                break;
            }
        }
    }
}
// thread callback function
void *hippo_thread_callback(void *arg) {
    int *clientsock;
    clientsock = (int *)arg;
    hippo_handle_client(*clientsock);
    //close(*clientsock);
    pthread_exit(NULL);
    return NULL;
}

// Posix thread process new connection
void hippo_thread(aeEventLoop *el,int serversock,void *privdata, int mask){
    int clientsock, *arg;
    struct sockaddr_in client_addr;
    unsigned clientlen;
    pthread_attr_t thread_attr;
    void *thread_result;
    
	HIPPO_NOTUSED(el);
	HIPPO_NOTUSED(privdata);
	HIPPO_NOTUSED(mask);

    // setting pthread attribute 
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    // run until cancelled 
    while (1){
		int cport;
		char cip[128];
        pthread_t thread;
        unsigned int clientlen = sizeof(client_addr);
        memset(server.currtime, 0, sizeof(server.currtime));
        getdate(server.currtime);

        // wait for client connection 
        if ((clientsock = anet_accept(server.neterr, serversock, cip, &cport)) < 0){
            die("Failed to accept client connection");
        }else{
			server.client_num++;
		}
        // use thread process new connection 
		if(server.client_num>MAX_CLIENT_NUM){
			server.client_num--;
			free_client(clientsock);
		}else{
			current_client_num();
			arg = &clientsock;
			if (pthread_create(&thread, &thread_attr, hippo_thread_callback, (void *)arg) != 0){
				die("Create new thread failed");
			}
		}
    }
    // destory pthread attribute 
    (void)pthread_attr_destroy(&thread_attr);
}
void current_client_num(){
	getdate(server.currtime);
	fprintf(stdout, "[%s] current client num of connection is %d.\n", server.currtime,server.client_num);
}
void init_server(){
	signal(SIGCHLD, SIG_IGN); 
    signal(SIGTERM, sig_term);
	//signal(SIGSTOP,SIG_IGN);//ctrl+z
	//signal(SIGINT,SIG_IGN);//ctrl+c
	init_server_status();
	if(status->is_debug){
		print_server_config();
	}
	if(status->is_daemon){
		init_daemon();
	}else{
		getdate(server.currtime);
		fprintf(stdout, "[%s] Start server listening at port %d ...\n", server.currtime, status->port);
		fprintf(stdout, "[%s] Waiting client connection ...\n", server.currtime);
	}
	hash_table_init();
	server.el=aeCreateEventLoop();
	server.client_num=0;
	server.maxidletime=MAXIDLETIME;
	server.fd = anet_tcp_server(server.neterr, status->port, NULL);
	if(server.fd==-1){
		die("connect fail.\n");
		exit(1);
	}
	aeCreateTimeEvent(server.el, 1000, flush_hash_table_expire_data, NULL, NULL);
}
//  signal control 
void sig_term(int signo) {
     FILE *fp;
     char info[50];
     if(signo == SIGTERM){
		sprintf(info, "program terminated.\r\n");
		write_file_lock(info);
		//exit(0); 
     }
}
int main(){
	init_server();
    // multiplexing IO 
	if (aeCreateFileEvent(server.el, server.fd, AE_READABLE,hippo_thread, NULL, NULL) == AE_ERR) die("creating file event fail");
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
    return 1; 
}
 
 
