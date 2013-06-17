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
typedef struct _hippo_client {
	int fd;
	time_t last_cmd_time; /* time of the last interaction, used for timeout */
}hippo_client;
typedef int command_proc(hippo_client *client,char **head_arr,int head_num);
typedef struct _hippo_command {
    char *name;
    command_proc *proc;
    int param_num;
}hippo_command;


hippo_command *lookup_command(char *name);

void write_file_lock(char *msg);
void send_info_to_client(int client_sock, char *msg, int length);

int store_hash_data(const char* skey,unsigned ken_len,char *data, unsigned value_len, unsigned lifetime, short mode);
HashNode *fetch_hash_item(const char* skey);
int remove_hash_item(const char* skey);
int flush_hash_table();
int flush_hash_table_expire_data();int store_hash_data(const char* skey,unsigned ken_len,char *data, unsigned value_len, unsigned lifetime, short mode);
HashNode *fetch_hash_item(const char* skey);
int remove_hash_item(const char* skey);
int flush_hash_table();
int flush_hash_table_expire_data();

/**
     * set key's value
	 * method: set 
	 * format: <cmd> <key> <expire> <length> <value>\r\n
	 *		   cmd: set
	 */
int set_command(hippo_client *client,char **head_arr,int head_num);
/**
     * set key's value
	 * method: set 
	 * format: <cmd> <key> <expire> <length> <value>\r\n
	 *		   cmd: modify
	 */
int modify_command(hippo_client *client,char **head_arr,int head_num);
/**
	 * get key's value
	 * method: get 
	 * format: <cmd> <key>\r\n
	 *		   cmd: get
	 */
int get_command(hippo_client *client,char **head_arr,int head_num);
int del_command(hippo_client *client,char **head_arr,int head_num);

int set_list_command(hippo_client *client,char **head_arr,int head_num);
int modify_list_item_command(hippo_client *client,char **head_arr,int head_num);
int get_list_command(hippo_client *client,char **head_arr,int head_num);
int del_list_command(hippo_client *client,char **head_arr,int head_num);
/**
     * clear all key and value
	 * method: flush
	 * format: <cmd>\r\n
	 *		   cmd: flush
	 */	
int flush_command(hippo_client *client,char **head_arr,int head_num);
int clear_expire_command(hippo_client *client,char **head_arr,int head_num);
/**
	 * method: version 
	 * format: <cmd>\r\n
	 *		   cmd: version
	 */	
int version_command(hippo_client *client,char **head_arr,int head_num);
/**
	 * method: stats 
	 * format: <cmd>\r\n
	 *	cmd: stats
	 */	
int stats_command(hippo_client *client,char **head_arr,int head_num);
int persist_command(hippo_client *client,char **head_arr,int head_num);
/**
	 * method: quit 
	 * format: <cmd>\r\n
	 *		   cmd: quit
	 */	
int quit_command(hippo_client *client,char **head_arr,int head_num);

static hippo_command cmd_table[] = {
	{"set",set_command,5},
	{"modify",modify_command,5},
    {"get",get_command,2},
	{"del",del_command,2},
	{"set_list",set_list_command,5},
	{"modify_list_item",modify_list_item_command,6},
    {"get_list",get_list_command,2},
	{"del_list",del_list_command,2},
	{"flush",flush_command,1},
	{"clear_expire",clear_expire_command,1},
	{"version",version_command,1},
	{"stats",stats_command,1},
	{"persist",persist_command,1},
	{"quit",quit_command,1},
    {"",NULL,0}
};

