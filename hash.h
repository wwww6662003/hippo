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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HASH_TABLE_MAX_SIZE 100000
#define MAX_KEY_LEN 20
#define MIN_STR_LEN 10

/* Hash key operate method */
#define MODE_SET	0
#define MODE_REPLACE	1

#define MAX_LIFE_TIME			2592000
//data file
#define DATA_FILE "./hippo"

typedef struct _HashNode{
	char* key;
 	char* value;
	size_t key_len;
	size_t value_len;
	unsigned created;
	unsigned expired;
	struct _HashNode* pNext;
}HashNode;


typedef struct _HashTable {
	HashNode *hashTable[HASH_TABLE_MAX_SIZE];	/* hash table list (hash data struct array pointer) */
	unsigned hash_table_size;					/* table size */
}HashTable;

HashTable *hashObject;

unsigned int hash_table_init();
unsigned int hash_table_hash_str(const char* skey);
unsigned int hash_table_insert(const char* skey,unsigned key_len, char* nvalue,unsigned value_len, unsigned lifetime,short mode);
unsigned int hash_table_remove(const char* skey);
HashNode *hash_table_lookup(const char* skey);
unsigned int hash_table_release();
unsigned int hash_table_print(int client_sock);



