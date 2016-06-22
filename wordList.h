#include <stdlib.h>
#include <string.h>


typedef struct listNode *listPtr;

struct listNode {
	char * word;
	listPtr next;
};

int empty(listPtr);
void insertFront(listPtr *head, char *str);
void popFront(listPtr *head);
