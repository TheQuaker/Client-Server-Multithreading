#ifndef __EXTERN
#define __EXTERN


extern int foundIt;
extern unsigned char * bloomFilter;
extern pthread_mutex_t *filterLocks;
extern pthread_mutex_t counter_lock;
extern pthread_mutex_t logFile_lock;

#endif
