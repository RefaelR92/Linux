#ifndef _EX2_H_
#define _EX2_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
// for priority 
#include <sched.h>

// gabi files
# include <ctype.h>
# include <mta_rand.h>
# include <mta_crypt.h>


#define bool int
#define TRUE 1
#define FALSE 0
#define MAX_BUFFER 500
#define PASS_MULTI_LEN_OF 8

typedef struct _decryptedDataNode{
	char* decryptedPassword;
	int passLen;
	char* key;
	int threadID;
	int iteration;
	struct _decryptedDataNode* next;
}decryptedDataNode;

typedef struct _decryptedDataList{
	decryptedDataNode* head;
	decryptedDataNode* tail;
	int size;
}decryptedDataList;

// mutexes 
pthread_mutex_t list_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t encrypted_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_decrypting_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t len_of_origin_password_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;
// condition variables 
pthread_cond_t g_start_decrypting_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_available_nodes_cv = PTHREAD_COND_INITIALIZER;

// global varibales 
char* g_encryptedPassword;
unsigned int g_lenOfEncryptedPassword;
unsigned int g_LenOfOriginPassword;
decryptedDataList g_DecryptedDataList;
bool g_startDecrypting = FALSE;
bool g_IsEnteredTime = FALSE; 

//new 
pthread_attr_t* attrPriorityEncrypter();
decryptedDataNode* checkIfTimeExpiredOrAvailableNodes(int* io_IsTimeoutExpired, struct timespec* i_Timeout);

//void validationCheck(char** i_Argv, int i_Argc, int* o_Decreypters, int* o_LenOfPassword, int* o_imeToDecryptPass);
void makeUpdatesBeforeEncrypt();
void printclient(decryptedDataNode* i_NodePtr, int i_PassLen, int i_KeyLen);
void validationCheck(char** argv, int argc,long int* timeToDecryptPass ,int* decrypters, int* lenOfPassword);
void createAndWaitForDecryptersThreads(pthread_t* decryptersArr, int decrypters, int* intArrIDs);

// testing 
pthread_mutex_t temp_mutex;
pthread_cond_t temp_cond;
//
// threads methods
void *do_encrypter(void *ptr);
void *do_decrypter(void *ptr);

// password methods 
void printNewPasswordGenerated(char* i_GeneratedPassword, unsigned int i_PassLen, char* i_Key, unsigned i_KeyLen, unsigned int i_EncryptedLength);
bool isPrintable(char* i_GeneratedPassword, int i_PassLen);
char* generateAndEncryptPassword(unsigned int i_LenOfPassword);
bool cmpPass(char* i_DecryptedPassword, unsigned int i_PassLen, char* i_GeneratedPassword, unsigned int i_LenOfPassword);

// time methods 
void resetTimer(struct timespec* i_Timeout, long int i_TimeToWait, bool* o_IsTimeoutExpired);

// end of program methods
void destory_resources();

// list methods 
void makeEmptyList();
void initList();
void addToTail(decryptedDataNode* i_Ptr);
decryptedDataNode* getNodeAndRemoveFromList();
decryptedDataNode* createNode(char* i_DecryptedPassword, unsigned int i_DecreyptedPassLen, char* i_Key,unsigned int i_KeyLength, int i_ID, int i_Iteration);
void deleteNode(decryptedDataNode* i_Ptr);



#endif