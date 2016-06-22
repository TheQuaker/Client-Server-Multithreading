#include "functions.h"

#define perror2(s,e) fprintf(stderr , "%s: %s\n", s, strerror(e))
#define MAX_MUTEX_COUNT 15000

int foundIt=0;
unsigned char *bloomFilter;
pthread_mutex_t *filterLocks = NULL;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logFile_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	pthread_t *thread_ids, server_thread;
	int SIZE,N,L,PORT,NUM=3,SEED,clientPort= -1;
	int i,fdLogFile,memBlock=64,numOfMutexes=15001;
	const char** arrayFromOracle;
	char *buf = NULL;
	char *LOGFILE = NULL;
	char *ADDRESS = NULL;
	int err,retValue = -1;
	FILE *fName;
	mode_t mode = 0644;
	

	if(argc !=7 && argc !=9 && argc !=11)
	{
		printf("SIZE N L PORT LOGFILE SEED [-k NUM] [-h ADDRESS:serverPort]\n");
		return 0;
	}
	else
	{//apodosh orismatwn stis antistoixes metablhtes
		SIZE = atoi(argv[1]);
		N = atoi(argv[2]);
		L = atoi(argv[3]);
		PORT = atoi(argv[4]);
		LOGFILE = malloc(strlen(argv[5])+1);
		strcpy(LOGFILE, argv[5]);
		SEED = atoi(argv[6]);
		
		if(argc >= 9)
		{
			if(!strcmp(argv[7], "-k"))
			{
				NUM = atoi(argv[8]);
			}
			else if(!strcmp(argv[7], "-h"))
			{
				buf = malloc(strlen(argv[8])+1);
				strcpy(buf, argv[8]);
				for(i=0; i<strlen(argv[8]); i++)
				{
					if(buf[i] == ':')
					{
						ADDRESS = malloc(i+1);
						strncpy(ADDRESS, buf, i);
						ADDRESS[i] = '\0';
						strcpy(buf, buf+i+1);
						clientPort = atoi(buf);
					}
				}
			}
			else
			{
				printf("check your arguments\n");
				return 0;
			}
			if (argc == 11)
			{
				if(!strcmp(argv[9], "-k"))
				{
					NUM = atoi(argv[10]);
				}
				else if(!strcmp(argv[9], "-h"))
				{
					buf = malloc(strlen(argv[10])+1);
					strcpy(buf, argv[10]);
					for(i=0; i<strlen(argv[10]); i++)
					{
						if(buf[i] == ':')
						{
							ADDRESS = malloc(i+1);
							strncpy(ADDRESS, buf, i);
							ADDRESS[i] = '\0';
							strcpy(buf, buf+i+1);
							clientPort = atoi(buf);
						}
					}
				}
				else
				{
					printf("check your arguments\n");
					return 0;
				}
			}
		}
	}
	//printf("PORT = %d\n NUM = %d\n SEED = %d\n ADDRESS = %s\n clientPort = %d\n",PORT,NUM,SEED,ADDRESS,clientPort);
	
	setEasyMode();
	//setHardMode();
	initSeed(SEED); //arxikopoihsh oracle
	initAlloc(malloc);
	
	bloomFilter = malloc(SIZE); //desmeush mnhmhs gia to bloom filter
	for(i=0;i<SIZE;i++)  //arxikopoihsh olwn twn byte se 0
	{
		bloomFilter[i]=0;
	}
	
	for(i=1; numOfMutexes > MAX_MUTEX_COUNT; i++)
	{//upologismos plu8ous mutexes
		memBlock = memBlock*i;
		numOfMutexes = SIZE/memBlock;
		
	}//printf("memBlock = %d\n",memBlock);printf("numOfMutexes %d\n",numOfMutexes);
	
	filterLocks = malloc(numOfMutexes * sizeof(pthread_mutex_t));
	for(i=0; i<numOfMutexes; i++)
	{
		pthread_mutex_init(&filterLocks[i], NULL);
	}
	
	if(argc == 9 || argc == 11)
	{//ekinish aithmatos client
		if(argc > 9)
		{
			if((!strcmp(argv[7], "-h")) || (!strcmp(argv[9], "-h")))
				client(SIZE, N, L, PORT, LOGFILE, SEED, NUM, ADDRESS, clientPort, numOfMutexes);
		}
		else if((!strcmp(argv[7], "-h")))
			client(SIZE, N, L, PORT, LOGFILE, SEED, NUM, ADDRESS, clientPort, numOfMutexes);
	}
	
	fName = fopen(LOGFILE, "w");//make/open/turcate file
	if (fName == NULL)
	{
		printf("Error opening file!\n");
	}
	
	data_struct *data = malloc(sizeof(data_struct));
	
	data->size = SIZE;
	data->k = NUM;
	data->l = L;
	data->port = PORT;
	data->n = N;
	data->seed = SEED;
	data->mutexCount = numOfMutexes;
	data->logfile = malloc(sizeof(LOGFILE));
	strcpy(data->logfile, LOGFILE);
	data->bytes = memBlock;
	data->file = fName;
	
	
	if((thread_ids = malloc(N*sizeof(pthread_t))) == NULL)
	{//desmeush mnhmhs gia ta threads
		perror("malloc");
		exit(1);
	}
	
	retValue = pthread_create(&server_thread, NULL, server,(void*) data);//server thread
	if (retValue == 1)
	{
		perror2("pthread_create", retValue);
	}
	
	for(i=0; i<N; i++)
	{//create N thread to find the word
		retValue = pthread_create(thread_ids+i, NULL, thread_worker, (void*) data);
		if (retValue == 1)
		{
			perror2("pthread_create", retValue);
		}
	}
	for(i=0; i<N; i++)
	{//wait for N threads to finish
		if (err = pthread_join(*(thread_ids+i), NULL))
		{
			perror2("pthread_join",err);
			exit(1);
		}
	}
	//printf("Ola ta threads teliwsan\n");
	for(i=0; i<numOfMutexes; i++)
	{//apodesmeush mutexes (edw mallon mono koitaei an einai unlocked)
		pthread_mutex_destroy(&filterLocks[i]);
	}
	
	if(foundIt == 0)
		printf("The word was not found\n");
	
	fclose(fName);
	
	//free memory
	free(ADDRESS);
	free(buf);
	free(bloomFilter);
	free(filterLocks);
	free(LOGFILE);
	free(thread_ids);
	free(data->logfile);
	free(data);


	exit(0);
}
