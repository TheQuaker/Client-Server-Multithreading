#include "functions.h"

void *thread_worker(void *data)
{
	int i,c,k,t,wordsTried=0,wordsNotInFilter=0;
	long long int oracleCalls=0;
	unsigned length,ch;
	data_struct *args = data;
	
	pthread_t thrID	= pthread_self();
	int seed	= time(NULL) * (thrID + 1);

	char* randomWord = NULL;
	char* arrayForNode=NULL;
	const char** arrayFromOracle;
	
	listPtr list = NULL;
	
	t = (int)rand_r(&seed)%10 + 21;//tuxaio t apo 21 ews 30
	
	for(c=0; c < args->l; c++)
	{//plh8os prospathiwn gia euresh lekshs
		oracleCalls = 0;
		length = (unsigned)rand_r(&seed)%6 + 5;
		randomWord = malloc(length + 1);
		for(k=0; k<length; k++)
		{//dhmiourgia thxaias lekshs megethous 5-10 xarakthrwn
			ch = (unsigned)rand_r(&seed)%95 + 32;
			*(randomWord+k) = (char)ch;
		}
		strcpy(randomWord+length, "\0");
		//printf("%s\n",randomWord);
		insertFront(&list, randomWord);
		free(randomWord);
		
		do
		{
			oracleCalls++;
			if((arrayFromOracle = oracle(list->word)) == NULL)
			{//h leksh brethike kai to thread apodesmeuei mnhmh kai termati
				printf("Thread %d found it\nThe word is: %s\n",(int)pthread_self(),list->word);
				while(!empty(list))
				{
					popFront(&list);
				}
				pthread_mutex_lock(&counter_lock);
				foundIt = 1;
				pthread_mutex_unlock(&counter_lock);
				
				pthread_mutex_lock(&logFile_lock);
				fprintf(args->file,"Thread: %d\nWords tried: %d\nPercentage of words allready in memory: %.2f\n\n",(int)pthread_self(),wordsTried,(float)(wordsTried-wordsNotInFilter)/wordsTried);
				pthread_mutex_unlock(&logFile_lock);
				pthread_exit(NULL);

			}
			else
			{	popFront(&list);
				for(i=0 ; arrayFromOracle[i] != NULL; i++)
				{
					wordsTried++;
					arrayForNode=(char*)malloc(strlen(arrayFromOracle[i])+1);
					strcpy(arrayForNode,arrayFromOracle[i]);

					if(searchFilter(arrayForNode, bloomFilter, args->k, args->size, args->bytes))
					{
						wordsNotInFilter++;
						insertFront(&list, arrayForNode);//eisagwgh sthn lista
					}
					free(arrayForNode);
				}
			}
			for(i=0 ; arrayFromOracle[i] != NULL; i++)
			{
				free((char **)arrayFromOracle[i]);
			}
			free(arrayFromOracle);
			
			if(oracleCalls%t == 0)
			{//elenxos kathe t klhseis an kapio thread brhke thn leksh
				int wordFound = 0;
				pthread_mutex_lock(&counter_lock);
				if (foundIt == 1)
				{
					wordFound = 1;
				}
				pthread_mutex_unlock(&counter_lock);
				
				if(wordFound == 1)
				{//h leksh exei brethei apo allo thread kai to thread apodesmeuei mnhmh kai termati
					while(!empty(list))
					{
						popFront(&list);
					}
					pthread_mutex_lock(&logFile_lock);
					fprintf(args->file,"Thread: %d\nWords tried: %d\nPercentage of words allready in memory: %.2f\n\n",(int)pthread_self(),wordsTried,(float)(wordsTried-wordsNotInFilter)/wordsTried);
					pthread_mutex_unlock(&logFile_lock);
					pthread_exit(NULL);
				}
			}
		}while(!empty(list));
		
		printf("search %d from thread %d failed\n",c,(int)pthread_self());
	}
	pthread_mutex_lock(&logFile_lock);
	fprintf(args->file,"Thread: %d\nWords tried: %d\nPercentage of words allready in memory: %.2f\n\n",(int)pthread_self(),wordsTried,(float)(wordsTried-wordsNotInFilter)/wordsTried);
	pthread_mutex_unlock(&logFile_lock);
	pthread_exit(NULL);
}

void addToFilter(char* arrayForNode, unsigned char * bloomFilter, int k,int size)
{//eisagwgh ths lekshs sto bloomfilter
	uint64_t ui64;
	int counterOfHashInsert,numOfByte,numOfBit;
	
	for(counterOfHashInsert = 0; counterOfHashInsert < k ;counterOfHashInsert++)
	{//allagh twn bit tou bloom filter se 1 
		ui64 = hash_by(counterOfHashInsert, arrayForNode);
		ui64 = ui64%(size*CHAR_BIT);
					
		numOfByte = ui64 / CHAR_BIT;
		numOfBit = ui64 % CHAR_BIT;
		bloomFilter[numOfByte] = bloomFilter[numOfByte] | (1<<(CHAR_BIT-numOfBit-1));
					
	}
}


int searchFilter(char* arrayForNode, unsigned char * bloomFilter, int k, int size, int memBlock)
{//elenxos an pithanon h leksh einai sto filtro
	
	uint64_t ui64;
	int notFound = 0,i,c,d;
	int lastLockedMutex=-1,mutexToLock=-1,lastUnlockedMutex=-1,mutexToUnlock=-1;
	long long int numOfByte,numOfBit, byte_array[k][2];
	for(i = 0; i < k; i++)
	{//ebresh twn bit tou filtrou pou antistixoun sthn leksh
		ui64 = hash_by(i, arrayForNode);
		ui64 = ui64%(size*CHAR_BIT);
		
		byte_array[i][0] = ui64 / CHAR_BIT;
		byte_array[i][1] = ui64 % CHAR_BIT;
		
	}

	for (c = 0 ; c < ( k - 1 ); c++)
	{//bublesort tou pinaka me auksousa seira
		for (d = 0 ; d < k - c - 1; d++)
		{
			if (byte_array[d][0] > byte_array[d+1][0]) 
			{
				numOfByte = byte_array[d][0];
				byte_array[d][0] = byte_array[d+1][0];
				byte_array[d+1][0] = numOfByte;
				
				numOfBit = byte_array[d][1];
				byte_array[d][1] = byte_array[d+1][1];
				byte_array[d+1][1] = numOfBit;
			}
		}
	}
	
	for(i=0; i<k; i++)
	{//kleidwnw ta aparaitita mutex
		mutexToLock = byte_array[i][0]/memBlock;
		if(mutexToLock != lastLockedMutex)
		{
			lastLockedMutex = mutexToLock;
			pthread_mutex_lock(&filterLocks[mutexToLock]);
		}
	}

	for(i = 0; i < k; i++)
	{//elenxos an h leksh einai sto filtro
		if((bloomFilter[byte_array[i][0]] & (1<<(CHAR_BIT-byte_array[i][1]-1))) == 0)
		{
			notFound = 1;
			addToFilter(arrayForNode, bloomFilter, k, size);//eisagwgh lekshs sto filtro
		}
	}
	
	for(i=0; i<k; i++)
	{//ksekleidwnw ta aparaitita mutex
		mutexToUnlock = byte_array[i][0]/memBlock;
		if(mutexToUnlock != lastUnlockedMutex)
		{
			lastUnlockedMutex = mutexToUnlock;
			pthread_mutex_unlock(&filterLocks[mutexToUnlock]);
		}
	}
	
	return notFound;
}
