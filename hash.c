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
 
#include "hash.h"
// hash table init
unsigned int hash_table_init(){
	hashObject=(HashTable*)malloc(sizeof(HashTable));
    hashObject->hash_table_size = 0;
	memset(hashObject->hashTable, 0, sizeof(HashNode*) * HASH_TABLE_MAX_SIZE);
	return 1;
}
//	string hash function
unsigned int hash_table_hash_str(const char* skey){
    const signed char *p = (const signed char*)skey;
    unsigned int h = *p;
    if(h)
    {
        for(p += 1; *p != '\0'; ++p)
            h = (h << 5) - h + *p;
    }
    return h;
}
//	insert key-value into hash table
unsigned int hash_table_insert(const char* skey,unsigned key_len, char* nvalue,unsigned value_len, unsigned lifetime,short mode){
	unsigned int pos;
	HashNode* pHead,*pNewNode;
	time_t currtime;
    if(hashObject->hash_table_size >= HASH_TABLE_MAX_SIZE)
    {
        printf("out of hash table memory!\n");
        return;
    }
    pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    pHead =  hashObject->hashTable[pos];
    while(pHead)
    {
        if(strcmp(pHead->key, skey) == 0){
			if(mode==MODE_REPLACE){
				if(!hash_table_remove(skey)){
					printf("delete fail!\n");
				}
			}else{
				printf("%s already exists!\n", skey);
				return 0;
			}
        }
        pHead = pHead->pNext;
    }
    pNewNode = (HashNode*)malloc(sizeof(HashNode));
    memset(pNewNode, 0, sizeof(HashNode));
    pNewNode->key = (char*)malloc(sizeof(char) * (strlen(skey) + 1));
    strcpy(pNewNode->key, skey);
    pNewNode->value = (char*)malloc(sizeof(char) * (strlen(nvalue) + 1));
    strcpy(pNewNode->value, nvalue);
    pNewNode->value = nvalue;

	currtime = time( (time_t *)NULL );
	pNewNode->key_len = key_len;
	pNewNode->value_len = value_len;
	pNewNode->created = currtime;
	pNewNode->expired = currtime + lifetime;
    pNewNode->pNext = hashObject->hashTable[pos]!= NULL?hashObject->hashTable[pos]:NULL;
    hashObject->hashTable[pos] = pNewNode;
	if(mode!=MODE_REPLACE){
		hashObject->hash_table_size++;
	}
    return 1;
}
//	remove key-value frome the hash table
unsigned int hash_table_remove(const char* skey){
		HashNode* pHead,* pLast,* pRemove;
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    if(hashObject->hashTable[pos])
    {
        pHead = hashObject->hashTable[pos];
        pLast = NULL;
        pRemove = NULL;
        while(pHead)
        {
            if(strcmp(skey, pHead->key) == 0)
            {
                pRemove = pHead;
                break;
            }
            pLast = pHead;
            pHead = pHead->pNext;
        }
        if(pRemove)
        {
            if(pLast)
                pLast->pNext = pRemove->pNext;
            else
                hashObject->hashTable[pos] = NULL;

            free(pRemove->key);
			free(pRemove->value);
            free(pRemove);
        }
    }
    return 1;
}
//	lookup a key in the hash table
HashNode *hash_table_lookup(const char* skey){
	HashNode* pHead;
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    if(hashObject->hashTable[pos])
    {
        pHead = hashObject->hashTable[pos];
        while(pHead)
        {
            if(strcmp(skey, pHead->key) == 0)
                return pHead;
            pHead = pHead->pNext;
        }
    }
    return NULL;
}
//	print the content in the hash table
unsigned int hash_table_print(int client_sock){
	HashNode* pHead;
    printf("===========content of hash table=================\n");
    int i;
    char temp[200];
    for(i = 0; i < HASH_TABLE_MAX_SIZE; ++i)
        if(hashObject->hashTable[i])
        {
            pHead = hashObject->hashTable[i];
            printf("%d=>", i);
            sprintf(temp, "%d=>\r\n", i);
            send_info_to_client(client_sock, temp, 0);
            while(pHead)
            {
                printf("key=%s:value=%s", pHead->key, pHead->value);
                sprintf(temp, "key=%s:value=%s\r\n", pHead->key, pHead->value);
            	send_info_to_client(client_sock, temp, 0);
                pHead = pHead->pNext;
            }
        }
		//send_info_to_client(client_sock, "OK\r\n", 0);
		return 1;
}
//	free the memory of the hash table
unsigned int hash_table_release(){
    int i;
	HashNode *pHead,*pTemp;
    for(i = 0; i < HASH_TABLE_MAX_SIZE; i++)
    {
        if(hashObject->hashTable[i])
        {
            pHead = hashObject->hashTable[i];
            while(pHead)
            {
                pTemp = pHead;
                pHead = pHead->pNext;
                if(pTemp)
                {
                    free(pTemp->key);
					free(pTemp->value);
                    free(pTemp);
                }
            }
			hashObject->hashTable[i]=NULL;
        }
    }
    return 1;
}

