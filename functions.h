#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "oracle.h"
#include "hash.h"
#include "global.h"
#include "wordList.h"
#include "server-client.h"

#define perror2(s,e) fprintf(stderr , "%s: %s\n", s, strerror(e))

typedef struct {
	int k;
	int size;
	int l;
	int bytes;
	int port;
	int seed;
	int n;
	int socket;
	int mutexCount;
	char *logfile;
	
	FILE *file;
}data_struct;

void *thread_worker(void *data);
void addToFilter(char* arrayForNode, unsigned char * bloomFilter, int k,int size);
int searchFilter(char* arrayForNode, unsigned char * bloomFilter, int k, int size, int memBlock);
