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
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
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
#include <dirent.h>
#include <time.h>
#include <signal.h> 

#ifndef NOFILE 
#define NOFILE 3 
#endif

//log path
#define LOG_FILE "./log"

 //Data error type
#define E_GENERAL		0
#define E_CLIENT		1
#define E_SERVER		2
#define BUFFER_SIZE				8192

// Die alert message
void die(char *mess);
void err_dump(char *mess);
void debug_log(short level, char *msg);

//substr - Sub string from pos to length 
char *substr( const char *s, int start_pos, int length, char *ret );

// explode -  separate string by separator 
void explode(char *from, char delim, char ***to, int *item_num);

//strtolower - string to lowner 
char *strtolower( char *s );

// strtoupper - string to upper 
char *strtoupper( char *s );

// strpos - find char at string position 
int strpos (const char *s, char c);

// strrpos - find char at string last position 
int strrpos (const char *s, char c);

//str_pad Pad a string to a certain length with another string 
int str_pad(char *s, int len, char c, char *to);

// str_repeat Repeat a string 
int str_repeat(char input, int len, char *to);

// trim - strip left&right space char 
char *trim( char *s );

// ltrim - strip left space char 
char *ltrim( char *s );

// ltrim - strip right space char 
char * rtrim(char *str);

//is_numeric - Check string is number 
int is_numeric( const char *s );

// Fetch current date tme 
void getdate(char *s);

// Set socket nonblock 
int socket_set_nonblock( int sockfd );

// File lock reister 
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);

// check file exists 
int file_exists (const char *path);

// filesize - get file size
long filesize(const char *filename);
// file_get_contents - read file contents
int file_get_contents( const char *filename, size_t filesize, char *ret, off_t length );
//is_dir - check file is directory
int is_dir(const char *filename);
// is_file - check file is regular file
int is_file(const char *filename);

// daemon 
void init_daemon();

// send error message
void send_error_to_client(int client_sock, short error_type, char *msg);

//All lock operate define

#define	read_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

/**
 * Disk io system call map
 */
#define hipoo_open(pathname, flags)				open((pathname), (flags))
#define hipoo_close(fd)							close((fd))
#define hipoo_read(fd, buf, count)				read((fd), (buf), (count))
#define hipoo_write(fd, buf, count)				write((fd), (buf), (count))
#define hipoo_sync(fd)							fsync((fd))
#define hipoo_seek(fildes, offset, whence)		lseek((fildes), (offset), (whence))
#define hipoo_lock(fd, operation)				flock((fd), (operation))


/**
 * Debug/Log level define
 */
#define LOG_LEVEL_DEBUG		1
#define LOG_LEVEL_TRACE		2
#define	LOG_LEVEL_NOTICE	3
#define LOG_LEVEL_WARNING	4
#define LOG_LEVEL_FATAL		5





