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
/* Static server configuration */
#include "anet.h"
#include "event.h"

#define SERVERPORT        4612    /* TCP port */
#define MAXIDLETIME       (60*5)  /* default client timeout */
#define REQUEST_MAX_SIZE		10240

#define MAX_CLIENT_NUM			2
#define MAX_MEM_SIZE			16777216	/* Max key data use memory size */

#define IS_DEBUG				0			/* Is open debug mode */
#define IS_DAEMON				0			/* Is daemon running */

#define SERVER_NAME				"hippo"
#define VERSION "0.1.0"

/* Return value define */
#define FALSE			0
#define TRUE			1
#define EXIT			2
#define MAX_BUF_SIZE			1048576		/* Key data max length bytes */

/* Anti-warning macro... */
#define HIPPO_NOTUSED(V) ((void) V)

typedef struct _hippo_server {
	char neterr[ANET_ERR_LEN];
	char currtime[32];
	int client_num;
	int maxidletime;
	int fd;
	//event
	aeEventLoop *el;
}hippo_server;

typedef struct _hippo_status {
	char version[32];		/* version */
	pid_t pid;				/* pid */
	time_t start_time;		/* start time (Unix timestamp) */
	time_t run_time;		/* run time (Unix timestamp) */
	unsigned mem_total;		/* allow use memory total size (Byte) */
	unsigned mem_used;		/* used memory size (byte) */
	unsigned item_total;	/* all item total */
	unsigned visit_add;		/* store visit total */
	unsigned visit_del;		/* remove visit total */
	unsigned visit_get;		/* fetch visit total */

	/* default global configure */
	unsigned short is_debug;
	unsigned short is_daemon;
	unsigned int port;
	unsigned int max_client;
	//unsigned int max_mem_size;
	unsigned int max_tablesize;
}hippo_status;
hippo_server server;
hippo_status *status;

void close_time_out_client(void);

void init_server_status();
void print_server_config();
void set_server_status( unsigned mem_used, unsigned item_total, unsigned visit_add, unsigned visit_del, unsigned visit_get );
void get_server_status();
unsigned get_mem_used();
void free_client(int client_sock);

int hipoo_proc_client( int client_sock );
void hipoo_handle_client( int client_sock);
void *hipoo_thread_callback(void *arg);
void hipoo_thread( int serversock, unsigned int max_client );
void current_client_num();
void init_server();
void sig_term(int signo);

