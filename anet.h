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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

#define MAX_LISTEN_CLIENT_NUM		5

//Send message to client
int anet_tcp_connect(char *err, char *addr, int port);
int anet_read(int fd, void *buf, int count);
int anet_resolve(char *err, char *host, char *ipbuf);
int anet_tcp_server(char *err, int port, char *bindaddr);
int anet_accept(char *err, int serversock, char *ip, int *port);
int anet_write(int fd, void *buf, int count);
int anet_non_block(char *err, int fd);
int anet_tcp_no_delay(char *err, int fd);
int anet_tcp_keep_alive(char *err, int fd);


