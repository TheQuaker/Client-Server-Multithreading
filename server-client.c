//to megalutero meros tou kwdika einai apo tis shmeiwseis tou ma8hmatos
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "functions.h"

#define BUF_SIZE 8192


void perror_exit(char *message)
{
	perror (message);
	exit (EXIT_FAILURE);
}

void *server(void *data)
{
	data_struct *info = data;
	int port = info->port;
	pthread_t thr_server;
	int sock, newsock,retValue =-1;
	int optval;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;
    
    /* Create socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);      /* The given port */
    
    optval = 1;//make the socket reuseable fast
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
        
    /* Listen for connections */
    if (listen(sock, 5) < 0)
		perror_exit("listen");
    printf("Listening for connections to port %d\n", port);
    
    while (1)
    {	clientlen = sizeof(client);
        /* accept connection */
    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
			perror_exit("accept");
	
    	printf("Accepted connection\n");
		
		info->socket = newsock;
		//thread pou tha eksyphrethsei thn aithsh
		retValue = pthread_create(&thr_server, NULL, thread_server, (void *)info);
		if(retValue == 1)
			perror2("pthread_create", retValue);
			

    	
    	//close(newsock); /* parent closes socket to client */
    }
}

void *thread_server(void* info)
{
	data_struct * data = info;
	char buf[BUF_SIZE]={0};
    int i,newsocket = data->socket;
    
	//long long int size,n,l,seed,num;
	uint32_t size,n,l,seed,num;
	if(read(newsocket, &size, sizeof(long long int)) < 0)
		perror_exit("read");
	if(read(newsocket, &n, sizeof(long long int))<0)
		perror_exit("read");
    if(read(newsocket, &l, sizeof(long long int))<0)
		perror_exit("read");
	if(read(newsocket, &seed, sizeof(long long int))<0)
		perror_exit("read");
	if(read(newsocket, &num, sizeof(long long int))<0)
		perror_exit("read");
		
	size = ntohl(size); n = ntohl(n); l = ntohl(l); seed = ntohl(seed); num = ntohl(num);
	if(size==data->size && n==data->n && l==data->l && seed==data->seed && num==data->k)
    {printf("parameters match!\n");//elenxos omoiotas parametrwn
		buf[0] = 1;
		write(newsocket, buf, 1);

		for(i=0; i<data->mutexCount;i++)
		{//kleidwma olwn twn mutexes
			pthread_mutex_lock(&filterLocks[i]);
		}
		for(i=0; i< data->size; i++)
		{
			buf[i%BUF_SIZE] = bloomFilter[1];
			if((i%BUF_SIZE == 0) && (i != 0))
			{//apostolh bloom filter
				if (write(newsocket, buf, BUF_SIZE) < 0)
					perror_exit("write");		
				memset(buf,'0', BUF_SIZE);
			}
			if(i == (data->size -1))
			{
				if (write(newsocket, buf, i%BUF_SIZE) < 0)
					perror_exit("write");
			}
		}

		for(i=0; i<data->mutexCount;i++)
		{//ksekleidwma olwn twn mutexes
			pthread_mutex_unlock(&filterLocks[i]);
		}
	}
	else
	{
		printf("parameters DO NOT match!\n");
		buf[0]=0;
		write(newsocket, buf, 1);
	}
	printf("Closing connection.\n");
	close(newsocket);	  /* Close socket */
	pthread_exit(NULL);
}

void client(int SIZE, int N, int L, int PORT, char *LOGFILE, int SEED, int NUM, char *ADDRESS, int clientPort, int numOfMutexes)
{//printf("trexei o client\n");
	int sock, i, bytesread, notAmatch=0, byteCounter = 0;
	char *logfile;
	char array[32];
    char buf[BUF_SIZE];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
   
	/* Create socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    	perror_exit("socket");

    server.sin_family = AF_INET;       /* Internet domain */
    
    server.sin_port = htons(clientPort);         /* Server port */
    
	/* Find server address */
    if ((rem = gethostbyname(ADDRESS)) == NULL) {	
	   herror("gethostbyname"); exit(1);
    }
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);

    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0)
	   perror_exit("connect");
    printf("Connecting to %s port %d\n", ADDRESS, clientPort);
    
    //long long int size, n, l, seed, num;
    uint32_t size, n, l, seed, num;
    //size = SIZE; n= N; l=L; seed = SEED; num = NUM;
    size = htonl(SIZE); n = htonl(N); l = htonl(L); seed = htonl(SEED); num = htonl(NUM);
    //apostolh orismatwn gia elegxo isothtas
    if(write(sock, &size, sizeof(long long int))<0)
		perror_exit("write");
    if(write(sock, &n, sizeof(long long int))<0)
		perror_exit("write");
    if(write(sock, &l, sizeof(long long int))<0)
		perror_exit("write");
    if(write(sock, &seed, sizeof(long long int))<0)
		perror_exit("write");
    if(write(sock, &num, sizeof(long long int))<0)
		perror_exit("write");

    if (read(sock, buf, 1) < 0)
		perror_exit("read");
    if(buf[0] == 1)
    {printf("Both programms have the same parameteres.\n");
		for(i=0; i<numOfMutexes;i++)
		{//kleidwma olou to bloomfilter
			pthread_mutex_lock(&filterLocks[i]);
		}
		do
		{//antigrafh tou bloom filter
			bytesread = read(sock, buf, BUF_SIZE);
			for(i=0; i<BUF_SIZE; i++)
			{
				if(byteCounter < SIZE)
				{
					bloomFilter[byteCounter] = buf[i];
					byteCounter++;
				}
			}
		}while(bytesread != 0); //termatizei otan diabasei ola ta byte
		for(i=0; i<numOfMutexes;i++)
		{//ksekleidwma olou to bloomfilter
			pthread_mutex_unlock(&filterLocks[i]);
		}
    }
    else
		printf("Programms don't have the same parameteres.\n");
	
	close(sock);	  /* Close socket */
}
