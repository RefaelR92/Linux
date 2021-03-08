#include "Ex2.h"
#include <time.h>
#include <limits.h> // for INT_MAX

// all mutexes and condition variables declared and initialize in h file.

#define assert_if(errnum) if (errnum != 0){printf("Error in line #%u: %m\n",__LINE__);exit(EXIT_FAILURE);}

int main(int argc, char* argv[])
{
	int* intArrIDs;
	int lenOfPassword;
	pthread_t encrypter;
	pthread_t* decryptersArr;
	int decrypters;
	long int timeToDecryptPass;
	pthread_attr_t *attrP;

	validationCheck(argv, argc, &timeToDecryptPass, &decrypters, &lenOfPassword);
	fflush(stdout);
	initList();
	attrP = attrPriorityEncrypter();

	pthread_mutex_lock(&len_of_origin_password_mtx);
	g_LenOfOriginPassword = (unsigned int)lenOfPassword;
	pthread_mutex_unlock(&len_of_origin_password_mtx);
	
	// create the encyepter thread with real-time priority 
	pthread_create(&encrypter, attrP, do_encrypter, (void*)&timeToDecryptPass);
	// create the decrtpters threads and wait for them 
	createAndWaitForDecryptersThreads( decryptersArr, decrypters, intArrIDs);
	// main thread will wait for encrypter to finish
	pthread_join(encrypter, NULL);
	
	free(attrP);
	free(intArrIDs);
	free(decryptersArr);
	// free all mutexes and condition variables. 
	destory_resources();
	return 0;
}// end main

void validationCheck(char** argv, int argc,long int* timeToDecryptPass ,int* decrypters, int* lenOfPassword)
{
	if(argc < 5 || argc > 7)
	{
		printf("Too few/many arguments to function!\n");
		exit(1);
	}
	else
	{
		if(strcmp(argv[1],"-n")!= 0 ||strcmp(argv[3],"-l")!= 0)
		{
			printf("Wrong flag: please use '-n' for number of decrypters and '-l' for password length!\n");
			exit(1);
		}
		*decrypters = atoi(argv[2]);
		*lenOfPassword = atoi(argv[4]);

		if(decrypters <= 0)
		{
			printf("Number of decrypters must be a positive number!\n");
			exit(1);
		}
		if(*lenOfPassword <= 0 || *lenOfPassword % PASS_MULTI_LEN_OF != 0)
		{
			printf("length of password must be positive and divided by %d\n",PASS_MULTI_LEN_OF);
			exit(1);
		}
		// with timeOut
		if(argc>5)
		{
			if(strcmp(argv[5],"-t")!=0)
			{
				printf("Wrong Time out flage, please try again later!\n");
				exit(1);
			}
			*timeToDecryptPass = (long int)atoi(argv[6]);
			if(*timeToDecryptPass <= 0)
			{
				printf("Time out must be a positive number!\n");
				exit(1);
			}
			g_IsEnteredTime = TRUE;
		}else
		{
			g_IsEnteredTime = FALSE;
		}
	}	
}

void destory_resources()
{
	pthread_mutex_destroy(&list_mtx);
	pthread_mutex_destroy(&encrypted_mtx);
    pthread_mutex_destroy(&start_decrypting_mtx);
    pthread_mutex_destroy(&len_of_origin_password_mtx);
    pthread_mutex_destroy(&print_mtx);

	pthread_cond_destroy(&g_start_decrypting_cv);
  	pthread_cond_destroy(&g_available_nodes_cv);
}

void *do_encrypter(void *ptr)
{
	pthread_mutex_lock(&len_of_origin_password_mtx);
	unsigned int lenOfPassword = g_LenOfOriginPassword;
	pthread_mutex_unlock(&len_of_origin_password_mtx); 

	pthread_mutex_lock(&encrypted_mtx);
	char* g_encryptedPassword =(char*)malloc(sizeof(char));
	pthread_mutex_unlock(&encrypted_mtx); 
	
	decryptedDataNode* currNodePtr = NULL;
	bool isTimeoutExpired = FALSE;
	struct timespec timeout, cur_time;
	long int timeToWait = *((long int*)ptr);

	unsigned int keyLength = lenOfPassword/PASS_MULTI_LEN_OF;
	char* generatedPassword = (char*)malloc(sizeof(char)* lenOfPassword);
	generatedPassword = generateAndEncryptPassword(lenOfPassword);
	
	resetTimer(&timeout, timeToWait, &isTimeoutExpired);

	while(1)
	{		
		clock_gettime(CLOCK_REALTIME, &cur_time);
		
		currNodePtr = checkIfTimeExpiredOrAvailableNodes(&isTimeoutExpired, &timeout);
				
		if(isTimeoutExpired == TRUE || (g_IsEnteredTime == TRUE && difftime(timeout.tv_sec, cur_time.tv_sec) <= 0.0))
		{
			makeUpdatesBeforeEncrypt();			
			printf("[SERVER]	[ERROR]	no password recieved during the configured timeout period (%ld seconds), regenerating password\n", timeToWait);
			generatedPassword = generateAndEncryptPassword(lenOfPassword);
			resetTimer(&timeout, timeToWait, &isTimeoutExpired);
		}
		else
		{	
			if(cmpPass(currNodePtr->decryptedPassword, currNodePtr->passLen, generatedPassword, lenOfPassword))// if (decrypted) - check if method updated
			{	
				makeUpdatesBeforeEncrypt();
				printclient(currNodePtr, lenOfPassword, keyLength); // print client x decrypted
				generatedPassword = generateAndEncryptPassword(lenOfPassword);
				resetTimer(&timeout, timeToWait, &isTimeoutExpired);
			}
			else
			{
				pthread_mutex_lock(&print_mtx);
				printf("[SERVER]	[ERROR]	Wrong password received from client #%d : (%s) should be (%s), and iteration: %d \n", currNodePtr->threadID, currNodePtr->decryptedPassword, generatedPassword, currNodePtr->iteration);
				pthread_mutex_unlock(&print_mtx);
			}

			deleteNode(currNodePtr);
		}	
	}
	
	makeEmptyList();
	//free(generatedPassword); free somewhere else
	
	pthread_exit(NULL);  
}

 void *do_decrypter(void *ptr)
{
	pthread_mutex_lock(&len_of_origin_password_mtx);
	unsigned int lenOfPassword = g_LenOfOriginPassword;
	pthread_mutex_unlock(&len_of_origin_password_mtx);

	unsigned int ID = *((unsigned int*)ptr);
	unsigned int iterations = 0;
	unsigned int keyLength = lenOfPassword/PASS_MULTI_LEN_OF;

	char* key = (char*)calloc(keyLength, sizeof(char));
	unsigned int* plainDataLengthPtr = (unsigned int*)malloc(sizeof(unsigned int));
	
	bool printable;
	char plainData[MAX_BUFFER];
	MTA_CRYPT_RET_STATUS status;

	char* decreyptedPass = (char*) malloc(sizeof(char));
	
	while(1)
	{
		pthread_mutex_lock(&start_decrypting_mtx);
		
		while (g_startDecrypting == FALSE) 
		{
			iterations = 0;
			pthread_cond_wait(&g_start_decrypting_cv, &start_decrypting_mtx);
		}
		
		pthread_mutex_unlock(&start_decrypting_mtx);
		
		iterations++;
		MTA_get_rand_data(key, keyLength);

		pthread_mutex_lock(&encrypted_mtx);
		status = MTA_decrypt(key, keyLength, g_encryptedPassword,g_lenOfEncryptedPassword, plainData, plainDataLengthPtr);// try to decrypt the password
		assert_if(status)
		pthread_mutex_unlock(&encrypted_mtx);

		free(decreyptedPass);
		decreyptedPass = (char*)malloc(sizeof(char)*(*plainDataLengthPtr));
		memcpy(decreyptedPass, plainData, *plainDataLengthPtr);
		printable = isPrintable(decreyptedPass, *plainDataLengthPtr);// is printable ? add to list 	
		
		if(printable)
		{
			decryptedDataNode* newNode = createNode(decreyptedPass, *plainDataLengthPtr, key,keyLength, ID, iterations);
			pthread_mutex_lock(&list_mtx);
			addToTail(newNode);	
			pthread_cond_signal(&g_available_nodes_cv);
			pthread_mutex_unlock(&list_mtx);
		}
	}
	
	free(key);
	pthread_exit(NULL);  
}

pthread_attr_t* attrPriorityEncrypter()
{
	pthread_attr_t* attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
	//set all the attr priroty for the encrypter Thread. 
	int res = 0;
	struct sched_param max_prio = {sched_get_priority_max(SCHED_RR)};
	// struct of priority for the encypter thread 
	
	res = pthread_attr_init(attr);
	assert_if(res)

	//attr methods 
	res = pthread_attr_setschedpolicy(attr,SCHED_RR);
    assert_if(res)

	res = pthread_attr_setschedparam(attr,&max_prio);
    assert_if(res)

  	pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	
	return attr;
} 



void addToTail(decryptedDataNode* i_Ptr)
{
	if(g_DecryptedDataList.head == NULL)
	{
		g_DecryptedDataList.head = i_Ptr;
		g_DecryptedDataList.tail = i_Ptr;
	}
	else
	{
		g_DecryptedDataList.tail->next = i_Ptr;
		g_DecryptedDataList.tail = i_Ptr;
	}
	
	g_DecryptedDataList.size++;
}

decryptedDataNode* createNode(char* i_DecryptedPassword, unsigned int i_DecreyptedPassLen, char* i_Key,unsigned int i_KeyLength, int i_ID, int i_Iteration)
{
	decryptedDataNode* ptr = (decryptedDataNode*)malloc(sizeof(decryptedDataNode));
	ptr->decryptedPassword = (char*)malloc(sizeof(char)*i_DecreyptedPassLen);
	ptr->key = (char*)malloc(sizeof(char)*i_KeyLength);
	memcpy(ptr->decryptedPassword, i_DecryptedPassword,i_DecreyptedPassLen);
	memcpy(ptr->key, i_Key, i_KeyLength);
	ptr->passLen = i_DecreyptedPassLen;
	ptr->threadID = i_ID;
	ptr->iteration = i_Iteration;
	ptr->next = NULL;
	
	return ptr;
}

void initList()
{
	g_DecryptedDataList.size = 0;
	g_DecryptedDataList.head = NULL;
	g_DecryptedDataList.tail = NULL;
}

void deleteNode(decryptedDataNode* i_Ptr)
{
	free(i_Ptr->decryptedPassword);
	free(i_Ptr->key);
	free(i_Ptr);
}


void makeEmptyList()
{
	decryptedDataNode* curr = g_DecryptedDataList.head;
	g_DecryptedDataList.size = 0;
	
	if(curr != NULL)
	{
		decryptedDataNode* next = curr->next;
		
		while(next != NULL)
		{
			deleteNode(curr);
			curr = next;
			next = next->next;
		}
		
		deleteNode(curr);
		g_DecryptedDataList.head = NULL;
		g_DecryptedDataList.tail = NULL;		
	}
}

void printNewPasswordGenerated(char* i_GeneratedPassword, unsigned int i_PassLen,char* i_Key, unsigned i_KeyLen, unsigned int i_EncryptedLength)
{
	char tPass[i_PassLen+1];
	char tKey[i_KeyLen+1];
	char c;
	int i;
	
	for(i = 0; i < i_PassLen; i++)
	{
		c = i_GeneratedPassword[i];
		tPass[i] = c;
	}
	
	tPass[i] = '\0';
	
	for(i = 0; i < i_KeyLen; i++)
	{
		c = i_Key[i];
		tKey[i] = c;
	}
	
	tKey[i] = '\0';
	
	pthread_mutex_lock(&print_mtx);		
	printf("[SERVER]	[INFO]	New password generated: %s, key: %s, After encryption: %s \n", tPass, tKey, g_encryptedPassword);
	pthread_mutex_unlock(&print_mtx);
}

decryptedDataNode* getNodeAndRemoveFromList()
{
	decryptedDataNode* nodePtr = NULL;
	nodePtr = g_DecryptedDataList.head;
	g_DecryptedDataList.head = g_DecryptedDataList.head->next;
	g_DecryptedDataList.size--;
	
	return nodePtr;
}

void printclient(decryptedDataNode* i_NodePtr, int i_PassLen, int i_KeyLen)
{
	char tPass[i_PassLen+1];
	char tKey[i_KeyLen+1];
	char c;
	int i;
	
	for(i = 0; i < i_PassLen; i++)
	{
		c = i_NodePtr->decryptedPassword[i];
		tPass[i] = c;
	}
	
	tPass[i] = '\0';
	
	for(i = 0; i < i_KeyLen; i++)
	{
		c = i_NodePtr->key[i];
		tKey[i] = c;
	}
	
	tKey[i] = '\0';
	
	pthread_mutex_lock(&encrypted_mtx);
	pthread_mutex_lock(&print_mtx);		
	printf("[CLIENT #%d]	[INFO]	After decryption: (%s) with encrypt password: (%s), key guessed(%s),sending to server after %d iterations \n", i_NodePtr->threadID,tPass, g_encryptedPassword, tKey, i_NodePtr->iteration);
	pthread_mutex_unlock(&print_mtx);
	pthread_mutex_unlock(&encrypted_mtx);

}//end printclient

bool isPrintable(char* i_GeneratedPassword, int i_PassLen)
{
	bool res = TRUE;
	int i;

	for(i = 0; i < i_PassLen; i++)
	{
		if(isprint(i_GeneratedPassword[i])== FALSE)
		{
			
			res = FALSE;
		}
	}
	
	return res;
}

bool cmpPass(char* i_DecryptedPassword, unsigned int i_PassLen, char* i_GeneratedPassword, unsigned int i_LenOfPassword)
{
	bool res = TRUE;
	
	if(i_PassLen == i_LenOfPassword)
	{
		for(int i = 0; i < i_LenOfPassword; i++)
		{
			if(i_DecryptedPassword[i] != i_GeneratedPassword[i])
			{
				res = FALSE;
			}
		}
	}
	else
	{
		res = FALSE;
	}
	
	return res;
}

char* generateAndEncryptPassword(unsigned int i_LenOfPassword)
{
	char* generatedPassword = (char*)malloc(sizeof(char)* i_LenOfPassword);
	char passwordBuffer[MAX_BUFFER];
	MTA_CRYPT_RET_STATUS status;
	unsigned int encryptedLength;
	unsigned int keyLength = i_LenOfPassword/PASS_MULTI_LEN_OF;
	char* key = (char*)calloc(keyLength, sizeof(char));
	char currChar;
	int i=0; 
	
	while(i < i_LenOfPassword)
	{
		currChar = MTA_get_rand_char();
		
		if(isprint(currChar))
		{
			generatedPassword[i] = currChar;
			i++;
		}
	}

	MTA_get_rand_data(key, keyLength);
	
	status = MTA_encrypt(key, keyLength, generatedPassword, i_LenOfPassword, passwordBuffer, &encryptedLength); // encrypts it using a radomized key
	assert_if(status)
	
	pthread_mutex_lock(&encrypted_mtx);
	free(g_encryptedPassword);
	g_encryptedPassword = (char*)malloc(sizeof(char)*encryptedLength);
	memcpy(g_encryptedPassword, passwordBuffer, encryptedLength);
	g_lenOfEncryptedPassword = encryptedLength;
	pthread_mutex_unlock(&encrypted_mtx);
	
	printNewPasswordGenerated(generatedPassword, i_LenOfPassword, key, keyLength, encryptedLength);// need to update tomorrow

	pthread_mutex_lock(&start_decrypting_mtx);
	g_startDecrypting = TRUE;
	pthread_cond_broadcast(&g_start_decrypting_cv);
	pthread_mutex_unlock(&start_decrypting_mtx); 
	
	free(key); // for sure? 
	
	return generatedPassword;
}

void makeUpdatesBeforeEncrypt()
{
	pthread_mutex_lock(&start_decrypting_mtx);
	g_startDecrypting = FALSE;
	pthread_mutex_unlock(&start_decrypting_mtx);
	
	pthread_mutex_lock(&list_mtx);
	makeEmptyList(); // make list empty
	pthread_mutex_unlock(&list_mtx);		
}

void resetTimer(struct timespec* i_Timeout, long int i_TimeToWait, bool* o_IsTimeoutExpired)
{
	 if(g_IsEnteredTime == TRUE)
	 {
		 clock_gettime(CLOCK_REALTIME, i_Timeout);
		 i_Timeout->tv_sec += i_TimeToWait;
		 *o_IsTimeoutExpired = FALSE;
	 }
}

void createAndWaitForDecryptersThreads(pthread_t* decryptersArr,int decrypters, int* intArrIDs)
{
	decryptersArr = (pthread_t*)malloc(sizeof(pthread_t)* decrypters);
	intArrIDs = (int*)malloc(sizeof(int)*decrypters);

	for(int i = 0; i < decrypters; i++)
	{
		intArrIDs[i] = i;
		pthread_create(&decryptersArr[i], NULL, do_decrypter, (void *)&(intArrIDs[i]));
	}

	for (int i = 0; i < decrypters; i++) 
	{
		pthread_join(&decryptersArr[i], NULL);
	}
}

decryptedDataNode* checkIfTimeExpiredOrAvailableNodes(int* io_IsTimeoutExpired, struct timespec* i_Timeout)
{
	int res = 0;
	decryptedDataNode* currNodePtr = NULL;
	
	pthread_mutex_lock(&list_mtx);
	
	while (g_DecryptedDataList.size == 0 && *io_IsTimeoutExpired == FALSE)
	{
		res = pthread_cond_timedwait(&g_available_nodes_cv, &list_mtx, i_Timeout);
		
		if (g_IsEnteredTime == TRUE && res == ETIMEDOUT)
		{
			*io_IsTimeoutExpired = TRUE;
		}
	}
	
	if(*io_IsTimeoutExpired == FALSE)
	{
		currNodePtr = getNodeAndRemoveFromList(); // get node details from global list (for sure printable)
	}
	
	pthread_mutex_unlock(&list_mtx);
	
	return currNodePtr;
}