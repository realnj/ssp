/*
 ============================================================================
 Name        : queue.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

GQueue *q;

typedef struct _ITEM_
{
	char item[10];
}ITEM;


int my_comp(void * a, void * b, gpointer data)
{
	ITEM *i1 = (ITEM *)a;
	ITEM *i2 = (ITEM *)b;

	return(strcmp(i1->item, i2->item));
}


void * console1(void * data)
{
	while( 1 )
	{
		char buf[100] = {0x00, };
		char num;
		ITEM * p;

		printf("\n");
		printf("1. Enqueue Item\n");
		printf("2. Dequeue Item\n");
		printf("3. Sort Item\n");
		printf("Select Number:");
		scanf(" %c", &num);

		if( num == '1' )
		{
			printf("\n");
			printf("Input Item :");
			fflush(stdin);
			scanf("%s", buf);

			p = (ITEM *) malloc(sizeof(ITEM));

			strcpy(p->item, buf);
			g_queue_push_tail(q, (ITEM *)p);
		}
		else if(num == '2')
		{
			while((p = g_queue_pop_head(q)) != NULL)
			{
				printf("%s\n", (char *)p->item);
				free(p);
			}

		}

		else if(num == '3')
		{

			if(q != NULL)
			{
				g_queue_sort(q, (GCompareDataFunc)my_comp, NULL);
			}
		}

		else if(num == 'q')
		{
			while((p = g_queue_pop_head(q)) != NULL)
			{
				printf("%s\n", (char *)p->item);
				free(p);
			}
			g_queue_free(q);
			exit(0);
		}

	}


	return NULL;
}


int main(void) {
	pthread_t p_console;
	int status;

	q = g_queue_new();

	if( pthread_create(&p_console, NULL, console1, NULL ) < 0)
		perror("Console Thread creation error!!");

	if( pthread_join( p_console, (void **) &status) != 0 )
		printf("status : %d\n", status );

	return EXIT_SUCCESS;
}

