#include "wordList.h"


int empty(listPtr head)
{
	if(head == NULL)
		return 1;
	else
		return 0;
}

void popFront(listPtr* head)
{//remove first element
	listPtr tmp;
	
	tmp = *head;
	*head = (*head)->next;
	if (tmp->word != NULL)
		free(tmp->word);
	free(tmp);
}

void insertFront(listPtr *head, char *str)
{//add front
	listPtr tmp;
	
	tmp = *head;
	*head = malloc(sizeof(struct listNode));
	(*head)->word = malloc(strlen(str)+1);
	strcpy((*head)->word, str);
	(*head)->next = tmp;
}

char *getWord(listPtr *head)
{
	return (*head)->word;
}
