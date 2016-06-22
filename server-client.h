

void *server(void * sock);
void *thread_server(void *newsock);
void client(int SIZE, int N, int L, int PORT, char *LOGFILE, int SEED, int NUM, char *ADDRESS,int clientPort,int numOfMutexes);
