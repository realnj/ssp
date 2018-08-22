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

GSList * list = NULL;

typedef struct _ST_DATA_
{
	char code;
	char pgm_nm[12];
	int  cycle;
	int  sec;
}ST_DATA;

void add_to_list(ST_DATA * in);
void update_list(ST_DATA * in);
void free_list(void);
void init_schedule_list(void);
void print_batch( void );
void free_batch(void);
int check_valid_schedule( ST_DATA * d );

void add_list(ST_DATA * in)
{
	ST_DATA * p = (ST_DATA *) malloc( sizeof(ST_DATA) );
	memcpy( p, in, sizeof( ST_DATA ) );
	list = g_slist_append(list, (ST_DATA *) p);
}

void update_list(ST_DATA * in)
{
	for( int i = 0; i < g_slist_length( list ); i++ )
	{
		ST_DATA * p = (ST_DATA *)g_slist_nth(list, i)->data;
		if( strcmp(p->pgm_nm, in->pgm_nm) == 0 )
		{
			p->cycle = in->cycle;
			p->sec   = in->sec;
			break;
		}
	}
}

void delete_list(ST_DATA * in)
{
	for( GSList *cur = list; cur; cur = cur-> next )
	{
		ST_DATA * p = (ST_DATA *) cur->data;
		if( strcmp(p->pgm_nm, in->pgm_nm) == 0 )
		{
			free( p );
			list = g_slist_remove(list, cur->data);
			break;
		}
	}
}

void free_list(void)
{
	for( GSList *cur = list; cur; cur = cur-> next )
	{
		ST_DATA * p = ( ST_DATA * ) cur->data;
		free( p );
	}

	g_slist_free( list );
}

#define RESULT_FILE_NM "RESULT.TXT"
void write_result_file( char * record)
{
	FILE *fp;

	fp = fopen( RESULT_FILE_NM, "a+" );
	if( fp == NULL )
	{
		perror( "error > result file" );
	}

	fputs( record, fp );
	fclose( fp );
}

void write_records_from_list(void)
{
	for(int i = 0; i < g_slist_length(list); i++)
	{
		ST_DATA * p = (ST_DATA *) g_slist_nth( list, i )->data;
		char rec[255];
		sprintf( rec, "%s#%d#%d\n", p->pgm_nm, p->cycle, p->sec);
		write_result_file( rec );
	}
}

int compare_func( void *p1, void *p2 )
{
	ST_DATA *i1 = (ST_DATA *)p1;
	ST_DATA *i2 = (ST_DATA *)p2;

	if( i1->cycle == i2->cycle )
		return strcmp( i2->pgm_nm, i1->pgm_nm );
	else
		return i2->cycle - i1->cycle;
}

void sort_list(void)
{
	list = g_slist_sort( list, (GCompareFunc) compare_func);
}

int directory_manipulation( char * fnm )
{
	DIR * dir_info;
	struct dirent *dir_entry;

	dir_info = opendir("./BATCH");
	if( dir_info != NULL )
	{
		while( (dir_entry = readdir( dir_info )) )
		{
			if( dir_entry->d_type == DT_DIR )
				continue;
			if( strcmp( dir_entry->d_name, fnm ) == 0 )
			{
				closedir( dir_info );
				return 0;
			}
		}

		closedir( dir_info );
	}
	else
	{
		perror( "error > folder is not exist" );
	}

	return -1;
}

int check_create_valid(ST_DATA * d)
{
	for( int i = 0; i < g_slist_length( list ); i++ )
	{
		ST_DATA * p = (ST_DATA *) g_slist_nth(list , i)->data;
		if( strcmp( p->pgm_nm, d->pgm_nm ) == 0  && p->code == d->code )
			return -1;
	}

	return 0;
}

int check_update_valid(ST_DATA * d)
{
	for( int i = 0; i < g_slist_length( list ); i++ )
	{
		ST_DATA * p = (ST_DATA *) g_slist_nth(list , i)->data;
		if( strcmp( p->pgm_nm, d->pgm_nm ) == 0  && p->code == 'C' )
			return 0;
	}

	return -1;
}
void read_record_from_console( void )
{
	char buf[255] = { 0x00, };

	while ( 1 )
	{
		scanf( "%s", buf );
		if( strcmp( buf, "P") == 0 )
		{
			sort_list();
			write_records_from_list();
			break;
		}
		else
		{
			char *p = strchr( buf, '\r');
			if( p != NULL ) *p = 0x00;
			char *code   = strtok( buf, "#" );
			char *pgm_nm = strtok( NULL, "#" );
			char *cycle, *sec;
			if( code[0] != 'D')
			{
				cycle = strtok( NULL, "#" );
				sec   = strtok( NULL, "#" );
			}
			else
			{
				strcpy(cycle, "");
				strcpy(sec,   "");
			}

			ST_DATA d;
			memset( &d, 0x00, sizeof(d) );
			d.code = code[0];
			strcpy(d.pgm_nm, pgm_nm);
			d.cycle = atoi( cycle );
			d.sec   = atoi( sec );

			if( d.code == 'C' )
			{
				if ( check_create_valid( &d ) < 0 ) continue;
				add_list( &d );
			}
			else if( d.code == 'U' )
			{
				if ( check_update_valid( &d ) < 0 ) continue;
				update_list( &d );
			}
			else if( d.code == 'D' )
			{
				if ( check_update_valid( &d ) < 0 ) continue;
				delete_list( &d );
			}

			printf("%s %s %s %s \n", code, pgm_nm, cycle, sec);

		}
	}
}

#define INPUT_FILE_NM "INPUT.TXT"

void read_record_from_file( void )
{
	FILE * fp;

	fp = fopen( INPUT_FILE_NM, "r");
	if( fp == NULL )
	{
		perror(" error > input file open");
		exit( 0 );
	}

	char buf[255] = { 0x00, };
	while ( fgets( buf, sizeof( buf ), fp) != NULL )
	{
		char *p = strchr( buf, '\r');
		if( p != NULL ) *p = 0x00;

		char *code   = strtok( buf, "#" );
		char *pgm_nm = strtok( NULL, "#" );
		char *cycle, *sec;
		if( code[0] != 'D')
		{
			cycle = strtok( NULL, "#" );
			sec   = strtok( NULL, "#" );
		}
		else
		{
			strcpy(cycle, "");
			strcpy(sec,   "");
		}

		ST_DATA d;
		memset( &d, 0x00, sizeof(d) );
		d.code = code[0];
		strcpy(d.pgm_nm, pgm_nm);
		d.cycle = atoi( cycle );
		d.sec   = atoi( sec );

		add_list( &d );
	}

	fclose( fp );
}


int main(int argc, char *argv[])
{
	char buf[255];

	sprintf( buf, "rm -f %s", RESULT_FILE_NM );
	system( buf );

	init_schedule_list();

	printf("--------------------------------------------------\n");

	read_record_from_console();

	for(int i = 0; i < g_slist_length(list); i++)
	{
		ST_DATA * p = (ST_DATA *) g_slist_nth( list, i )->data;
		check_valid_schedule( p );
	}

	print_batch();

	free_list();

	free_batch();

	return EXIT_SUCCESS;
}

#define TOTAL_SEC_SLOT 20

GSList *schedule_list = NULL;
GSList *task_list[TOTAL_SEC_SLOT];

typedef struct _ST_TASK_
{
	char pgm_nm[12];
	int  sec;
	int  cycle;
}ST_TASK;

typedef struct _ST_BATCH_
{
	GSList * node;
}ST_BATCH;

void init_schedule_list(void)
{
	for(int i = 0; i < TOTAL_SEC_SLOT; i++ )
	{
		ST_BATCH * p = (ST_BATCH *) malloc( sizeof( ST_BATCH ) );
		memset( p, 0x00, sizeof( ST_BATCH ) );
		p->node = task_list[i];
		schedule_list = g_slist_append( schedule_list, (ST_BATCH *) p);
		printf("void_init_schedule_list : %d\n", g_slist_length( schedule_list ) );
	}
}

int check_valid_schedule( ST_DATA * d )
{
	int sec = d->sec;
	int cycle = d->cycle;

	for( int j = 0; j <= TOTAL_SEC_SLOT/(sec + cycle); j++ )
		for( int i = 0; i < sec; i++ )
		{
			int index = i+j*(sec+cycle);
			if( index >= TOTAL_SEC_SLOT ) break;

			ST_BATCH * parent = (ST_BATCH *)g_slist_nth(schedule_list, index)->data;
			if ( g_slist_length ( parent->node ) > 5 ) return -1;
		}

	for( int j = 0; j <= TOTAL_SEC_SLOT/(sec + cycle); j++ )
		for( int i = 0; i < sec; i++ )
		{
			int index = i+j*(sec+cycle);
			if( index >= TOTAL_SEC_SLOT ) break;

			ST_BATCH * parent = (ST_BATCH *)g_slist_nth(schedule_list, index)->data;
			ST_TASK  * child  = (ST_TASK *)malloc( sizeof( ST_TASK ) );

			strcpy( child->pgm_nm, d->pgm_nm );
			child->sec = d->sec;
			child->cycle = d->cycle;
			parent->node = g_slist_append( parent->node, (ST_TASK *)child);
		}

	return 0;
}

void free_batch(void)
{
	for( GSList * p_cur = schedule_list; p_cur; p_cur = p_cur->next )
	{
		ST_BATCH * parent = (ST_BATCH *) p_cur->data;
		for( GSList * c_cur = parent->node; c_cur; c_cur = c_cur->next)
		{
			ST_TASK * child = (ST_TASK *) c_cur->data;
			free( child );
		}
		g_slist_free( parent->node );
		free( parent);
	}
	g_slist_free( schedule_list );
}

void print_batch( void )
{
	printf("--------------------------------------------------\n");

	for( GSList * p_cur = schedule_list; p_cur; p_cur = p_cur->next )
	{
		ST_BATCH * parent = (ST_BATCH *) p_cur->data;
		printf("PARENT SEQ : %d\n", g_slist_index(schedule_list, p_cur->data));

		for( GSList * c_cur = parent->node; c_cur; c_cur = c_cur->next)
		{
			ST_TASK * child = (ST_TASK *) c_cur->data;
			printf("\t CHILD SEQ:%d, PGM_NM:%s, SEC:%d, CYCLE:%d\n",
					g_slist_index(parent->node, c_cur->data), child->pgm_nm, child->sec, child->cycle );
		}
	}
}

// 리스트의 첫 전체 노드의 개수 리턴
void usage1_g_slist_length(void)
{
	int i = g_slist_length( list );
}
// 리스트의 첫 노드 앞에 데이터 삽입
void usage2_g_slist_prepend(void)
{
	ST_DATA * p = (ST_DATA *) malloc( sizeof(ST_DATA) );
	strcpy( p->pgm_nm, "BATCH90.EXE");
	p->sec = 1;
	p->sec = 2;
	list = g_slist_prepend(list, (ST_DATA *)p);
}

void usage3_g_slist_nth(void)
{
	for(int i = 0; i <g_slist_length(list); i++)
	{
		ST_DATA * p = (ST_DATA *) g_slist_nth(list , i);
	}
}

void usage4_g_slist_index(void)
{
	for( GSList * cur = list; cur; cur = cur->next )
	{
		int i = g_slist_index(list, cur->data);
	}
}

void usage5_g_slist_position(void)
{
	for( GSList * cur = list; cur; cur = cur->next )
	{
		int i = g_slist_position(list, cur);
	}
}

void usage6_g_slist_insert(void)
{
	ST_DATA * p = (ST_DATA *) malloc( sizeof(ST_DATA) );
	strcpy( p->pgm_nm, "BATCH91.EXE");
	p->sec = 1;
	p->sec = 2;
	list = g_slist_insert(list, (ST_DATA *)p, g_slist_length(list)/2);
}

void usage7_g_slist_insert_sorted(void)
{
	ST_DATA * p = (ST_DATA *) malloc( sizeof(ST_DATA) );
	strcpy( p->pgm_nm, "BATCH91.EXE");
	p->sec = 1;
	p->sec = 2;
	list = g_slist_insert_sorted(list, (ST_DATA *)p, (GCompareFunc) compare_func);
}

void usage8_g_slist_remove(void)
{
	for( GSList * cur = list; cur; cur = cur->next )
	{
		ST_DATA * p = (ST_DATA *) cur->data;
		free( p );
		list = g_slist_remove(list, cur->data);
	}
}

