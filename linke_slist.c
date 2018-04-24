/*
 ============================================================================
 Name        : linkedlist.c
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

GSList * list = NULL;

typedef struct _ITEM_
{
	char item[10];
}ITEM;

int my_comp(void *p1, void *p2)
{
	ITEM *i1 = (ITEM *)p1;
	ITEM *i2 = (ITEM *)p2;

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
		printf("1. Add Item\n");
		printf("2. View Item\n");
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
			list = g_slist_append(list, (ITEM *)p);
		}
		else if(num == '2')
		{
			if(list != NULL)
			{
				for(GSList * cur = list; cur; cur = cur->next )
				{
					ITEM * ptr = (ITEM *)cur->data;
					printf("%s\n", (char *)ptr->item);
				}
			}

		}
		else if(num == '3')
		{
			if(list != NULL)
			{
				list = g_slist_sort(list, (GCompareFunc)my_comp);
			}
		}
		else if(num == 'q')
		{
			for(GSList * cur = list; cur->next != NULL; cur = cur->next )
			{
				ITEM * ptr = (ITEM *)cur->data;
				free(ptr);
			}
			g_slist_free(list);
			exit(0);
		}

	}


	return NULL;
}

int main(void) {
	pthread_t p_console;
	int status;

	if( pthread_create(&p_console, NULL, console1, NULL ) < 0)
		perror("Console Thread creation error!!");

	if( pthread_join( p_console, (void **) &status) != 0 )
		printf("status : %d\n", status );

	return EXIT_SUCCESS;
}
