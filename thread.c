/*
 ============================================================================
 Name        : thread.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define DEBUG_ON 1
#define debug(fmt, args...) do { if (DEBUG_ON) printf("%s:%d:%s> " fmt, __FILE__, __LINE__, __FUNCTION__, ##args); } while(0)

#define MAX_THREAD 10

typedef struct {
	char type[3];
	int  count;
	char time[20];
	char message[10];
	char converted[10];
	int  status; // 0:free, 1:using
}ST_DATA;

ST_DATA d[MAX_THREAD];
int exit_flag = 0;

int exe_r(char * in, char * out);
int save_r(ST_DATA * p);

int exe_r(char * in, char * out)
{
	FILE *fp;
	char cmd[50] = { 0x00, };
	char tmp[50] = { 0x00, };

	sprintf(cmd, "./CODECONV %s", in);

	fp = popen(cmd, "r");
	if(fp == NULL)
	{
		debug("error");
		return -1;
	}

	fgets(tmp, sizeof(tmp), fp);
	memcpy(out, tmp, 9);
	pclose(fp);

	return 0;
}

int save_r(ST_DATA * p)
{
	FILE * fp;
	char cmd[50] = { 0x00, };
	char tmp[50] = { 0x00, };

	sprintf(cmd, "TYPELOG_5_%s.txt", p->type);

	fp = fopen(cmd, "a+");
	if(fp == NULL)
	{
		debug("error");
		return -1;
	}

	sprintf(tmp, "%s#%s#%s\n", p->time, p->type, p->converted);
	fputs(tmp, fp);
	fclose(fp);

	return 0;
}

void * t_function(void * data)
{
	int index = 0;

	index = *(int *)data;
	while(!exit_flag)
	{
		if(d[index].status == 1)
		{
			exe_r(d[index].message, d[index].converted);
			save_r(&d[index]);
			d[index].status = 0;
		}
		else
		{
			usleep(100);
		}
	}

	return (void *)0;
}

int main(void) {
	pthread_t pthread[MAX_THREAD];
	int thread_id[MAX_THREAD];
	int thread_idx = 0;
	int t_idx[MAX_THREAD] = { 0x00, };

	do
	{
		t_idx[thread_idx] = thread_idx;
		thread_id[thread_idx] = pthread_create(&pthread[thread_idx], NULL, t_function, (void*)&t_idx[thread_idx]);
		if(thread_id[thread_idx] < 0)
		{
			debug("error");
			return -1;
		}
		thread_idx++;
	}
	while(thread_idx < MAX_THREAD);

	FILE * fp = fopen("LOGFILE_C.TXT", "r");
	if(fp == NULL)
	{
		debug("error");
		return -1;
	}

	char tmp[50] = { 0x00, };
	int cur = 0;

	while(fgets(tmp, 50, fp))
	{
		memset(&d[cur], 0x00, sizeof(ST_DATA));
		memcpy(d[cur].time,		tmp, 		19);
		memcpy(d[cur].type,		&tmp[20],	2);
		memcpy(d[cur].message,	&tmp[23],	9);

		int empty = 0;
		while(!empty)
		{
			for(int i=0; i<MAX_THREAD; i++)
			{
				if(d[i].status == 0)
				{
					empty = 1;
					cur = i;
					d[i].status = 1;
					break;
				}
			}

			if(empty != 1) usleep(10);
		}
	}
	fclose(fp);
	exit_flag = 1;

	for(int j=0; j<thread_idx; j++)
	{
		int status;
		pthread_join(pthread[j], (void **)&status);
	}

	return EXIT_SUCCESS;
}
