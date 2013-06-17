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
#include <time.h>
#include "hash.h"
#include "command.h"
#include "hippo.h"
#include "util.h"
#include "anet.h"
//find command
hippo_command *lookup_command(char *name) {
    int j = 0;
    while(cmd_table[j].name != NULL) {
        if (!strcmp(name,cmd_table[j].name)) return &cmd_table[j];
        j++;
    }
    return NULL;
}
// Send message to client
void send_info_to_client(int client_sock, char *msg, int length){
	int len;
	FILE *fp;
	time_t currtime_timestamp;
	char anet_info[200];
	len = length>0 ? length : strlen(msg);
	anet_write(client_sock, msg, len);
	getdate(server.currtime);
	currtime_timestamp = time((time_t *)NULL);
	sprintf(anet_info, "%s`%d`%s\n", server.currtime,currtime_timestamp,msg); 
	write_file_lock(anet_info);
}
//write file
void write_file_lock(char *msg){
	 FILE *fp;
	 char *currtime_timestamp;
	 char write_file_msg[1000];
	 if((fp = fopen(LOG_FILE, "a+"))!=NULL){
        fprintf(fp, "%s`%d`%s\n", server.currtime,currtime_timestamp,msg); 
        fclose(fp);
	 } 
}
//string insert
int set_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 5 && head_num != 4 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		// store variables
		short mode;
		char *key, *data;
		unsigned key_len,value_len, lifetime;
		/* pick every store item */
		if ( head_num == 5 ){
			lifetime = atoi(trim(head_arr[2]));
			value_len   = atoi(trim(head_arr[3]));
		}else {
			lifetime = atoi(trim(head_arr[2]));
			value_len	 = atoi(trim(head_arr[3]));
		}
		// data too large 
		if ( value_len > MAX_BUF_SIZE ){
			send_error_to_client(client->fd, E_CLIENT, "data too large, to exceed MAX_BUF_SIZE.\n" );
			return FALSE;
		}
		// item assign 
		key=head_arr[1];
		key_len=strlen(key)+1;
		data=(char *)malloc(value_len + 1);
		substr(head_arr[4],0,value_len,data);
		data[value_len]='\0';
		if(strlen(data)!=value_len){
			send_error_to_client(client->fd, E_CLIENT, "data's length is right.\n" );
			return FALSE;
		}
		lifetime = lifetime > MAX_LIFE_TIME ? MAX_LIFE_TIME : lifetime;
		mode=MODE_SET;
		/*if ( fread(data, 1, length + 1, fp) <= 0 ){
			send_error_to_client(client->fd, E_CLIENT, "data too short, not match LENGTH");
			return FALSE;
		}*/
		/* store data */
		if ( store_hash_data(key,key_len,data,value_len,lifetime,mode)){
			send_info_to_client(client->fd, "STORED\r\n", 0);
		}else {
			send_info_to_client(client->fd, "NOT_STORED\r\n", 0);
		}
		return TRUE;
}
//string modify
int modify_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 5 && head_num != 4 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		// store variables
		short mode;
		char *key, *data;
		unsigned key_len,value_len, lifetime;
		/* pick every store item */
		if ( head_num == 5 ){
			lifetime = atoi(trim(head_arr[2]));
			value_len   = atoi(trim(head_arr[3]));
		}else {
			lifetime = atoi(trim(head_arr[2]));
			value_len	 = atoi(trim(head_arr[3]));
		}
		// data too large 
		if ( value_len > MAX_BUF_SIZE ){
			send_error_to_client(client->fd, E_CLIENT, "data too large, to exceed MAX_BUF_SIZE.\n" );
			return FALSE;
		}
		// item assign 
		key=head_arr[1];
		key_len=strlen(key)+1;
		data=(char *)malloc(value_len + 1);
		substr(head_arr[4],0,value_len,data);
		data[value_len]='\0';
		if(strlen(data)!=value_len){
			send_error_to_client(client->fd, E_CLIENT, "data's length is right.\n" );
			return FALSE;
		}
		lifetime = lifetime > MAX_LIFE_TIME ? MAX_LIFE_TIME : lifetime;
		mode=MODE_REPLACE;
		/* store data */
		if ( store_hash_data(key,key_len,data,value_len,lifetime,mode)){
			send_info_to_client(client->fd, "STORED\r\n", 0);
		}else {
			send_info_to_client(client->fd, "NOT_STORED\r\n", 0);
		}
		return TRUE;
}
//string read
int get_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 2 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		char *key, buf[BUFFER_SIZE];
        HashNode* pNode;
		time_t currtime;
		key = trim(head_arr[1]);
		pNode = fetch_hash_item(key);
		/* item out expire time, remove it */
		currtime = time((time_t *)NULL);
		if(!pNode){
			send_info_to_client(client->fd, "NOT EXIST\r\n", 0);
			return FALSE;
		}
		if ( pNode->expired < currtime ){
			hash_table_remove( key );
			send_info_to_client(client->fd, "EXPIRED\r\n", 0);
			return FALSE;
		}
        if ( !pNode){
			send_info_to_client(client->fd, "END\r\n", 0);
            return FALSE;
        }
		// send data to client
		memset(buf, 0, BUFFER_SIZE);
		sprintf(buf, "VALUE %s %d\r\n", pNode->value, pNode->value_len);
		send_info_to_client(client->fd, buf, 0);
		send_info_to_client(client->fd, "END\r\n", 0);
		return TRUE;
}
//string delete
int del_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 2 && head_num != 3 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		char *key;

		key = trim(head_arr[1]);
		if (!remove_hash_item( key )){
			send_info_to_client(client->fd, "NOT_FOUND\r\n", 0);
			return FALSE;
		} else {
			send_info_to_client(client->fd, "DELETED\r\n", 0);
			return TRUE;
		}
}
//list insert
int set_list_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//list modify item
int modify_list_item_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//list read
int get_list_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//list delete
int del_list_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//clear string and list
int flush_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 1 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		if(flush_hash_table()){
			send_info_to_client(client->fd, "OK\r\n", 0);
		}
		return TRUE;
}
//clear string and list's expired item
int clear_expire_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//version show
int version_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 1 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		unsigned short version_len = strlen(VERSION) + 2 + 8;
		char version[version_len];
		sprintf(version, "VERSION %s\r\n", VERSION);
		send_info_to_client(client->fd, version, version_len);
		return TRUE;
}
//string and list's stats
int stats_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 1 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		if(hash_table_print(client->fd)){
			send_info_to_client(client->fd, "OK\r\n", 0);
			print_server_status();
		}else{
  			  send_info_to_client(client->fd, "Fail\r\n", 0);
  		}
		return TRUE;
}
//save string and list to file
int persist_command(hippo_client *client,char **head_arr,int head_num){
	return 1;
}
//quit
int quit_command(hippo_client *client,char **head_arr,int head_num){
	if ( head_num != 1 ){
			send_error_to_client(client->fd, E_GENERAL, "");
			return FALSE;
		}
		if(hash_table_release()){
            send_info_to_client(client->fd, "Quit,OK\r\n", 0);
		}
        return EXIT;
}


// Store data
int store_hash_data(const char* skey,unsigned key_len,char *data, unsigned value_len, unsigned lifetime, short mode){
	int ret;

	/* current memory use size exceed MAX_MEM_SIZE, remove last node from queue, remove key from hash table */
	if ( (get_mem_used()+key_len+value_len) > status->mem_total ){
		return 2;
	}

	/* insert data to hashtable */
	ret = hash_table_insert(skey, key_len,data, value_len, lifetime, mode );
    if (status->is_debug){
        printf("key:%s,key_len:%d, data:%s, value_len:%d, lifetime:%d, mode:%d\n", skey,key_len,data, value_len, lifetime, mode);
        printf("Store Result: %d\n", ret);
    }
	
	/* use add method, if key exist return SUCCESS */
	if (!ret){
		return 0;
	}

	/* set status */
	set_server_status( key_len+value_len, 1, 1, 0, 0 );
	return 1;
}


// Fetch data
HashNode *fetch_hash_item(const char* skey){
	HashNode *hnode;

	hnode = hash_table_lookup(skey);
	set_server_status(0, 0, 0, 0, 1);
	return hnode?hnode:NULL;
}

// Remove data
int remove_hash_item(const char* skey){
	int ret;
	int length;
	HashNode *hnode;

	/* remove data from hash table, set status */
	hnode=hash_table_lookup(skey);
	if (!hnode){
		return 0;
	}
	length = 0 - hnode->key_len- hnode->value_len;
	set_server_status( length, -1, 0, -1, 0 );	
	ret = hash_table_remove(skey);
	return 1;
}
//Flush all data
int flush_hash_table(){
	/* Destroy queue & hash table */
	hash_table_release();
	/* Init hash table & queue */
	hash_table_init();
	init_server_status();
	return 1;
}

//Flush all expired data
int flush_hash_table_expire_data(){
    int i;
    time_t currtime;
    char *key;
    /* Remove expired (after date) data */
    currtime = time( (time_t *)NULL );
	for (i=0; i<hashObject->hash_table_size; i++){
		if (hashObject->hashTable[i] != NULL && hashObject->hashTable[i]->expired < currtime){
            remove_hash_item( hashObject->hashTable[i]->key );
        }
	}
    return 1000;
}
