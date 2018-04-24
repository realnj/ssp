/*
 ============================================================================
 Name        : socket.c
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#define DEBUG_ON 1
#define debug(fmt, args...) do { if (DEBUG_ON) printf("%s:%d:%s> " fmt, __FILE__, __LINE__, __FUNCTION__, ##args); } while(0)

int tcp_send(int sockfd, const char *buf, int len, int tm_sec);
int tcp_recv(int sockfd, char *buf, int len, int tm_sec);
int tcp_connect(int * psockfd, int tm_sec);
int write_list_file(const char *file_name, char *data, int data_length);
int recv_files_loop(const char *file_name, int sockfd);
int make_file_list_from_dir(const char *path, const char * fname);
int append_file_from_buf(const char *fname, char *d, int length);
int write_file_from_buf(const char *fname, char *d, int length);
int cmp_files(const char *result, const char *file1, const char *file2);
char cmp_str(char *p1, char *p2);
int file_operation(void);

int make_file_list_from_dir(const char *path, const char * fname)
{
	DIR *dir;
	struct dirent *ent;
	FILE *fp1, *fp2;
	char buf[50] = { 0x00, };

	sprintf(buf, "%s/%s", path, fname);

	fp1 = fopen(buf, "w");
	if(fp1 == NULL)
	{
		debug("error");
		return -1;
	}

	dir = opendir(path);
	if(dir != NULL)
	{
		while((ent = readdir(dir)) != NULL)
		{
			if(ent->d_type != DT_REG) continue;
			if(strcmp(ent->d_name, fname) == 0) continue;
			sprintf(buf, "%s/%s", path, ent->d_name);

			fp2 = fopen(buf, "r");
			if(fp2 == NULL)
			{
				debug("error");
				fclose(fp1);
				return -1;
			}
			char ver[3] = { 0x00, };
			fgets(ver, sizeof(ver), fp2);
			fclose(fp2);
			sprintf(buf, "%s_%s\n", ent->d_name, ver);
			fputs(buf, fp1);
		}
		closedir(dir);
	}
	else
	{
		fclose(fp1);
		debug("error");
		return -1;
	}
	fclose(fp1);

	return 0;
}

char cmp_str(char *p1, char *p2)
{
	char *token, *old;
	char c1[50] = { 0x00, };
	char c2[50] = { 0x00, };
	char t1[50] = { 0x00, };
	char t2[50] = { 0x00, };
	char v1[50] = { 0x00, };
	char v2[50] = { 0x00, };

	strcpy(c1, p1);
	strcpy(c2, p2);

	token = strtok_r(c1, "_.", &old);
	if(token != NULL)
	{
		strcpy(t1, token);
		token = strtok_r(NULL, "_.", &old);
		if(token != NULL) strcpy(v1, token);
	}

	token = strtok_r(c2, "_.", &old);
	if(token != NULL)
	{
		strcpy(t2, token);
		token = strtok_r(NULL, "_.", &old);
		if(token != NULL) strcpy(v2, token);
	}

	if(strcmp(t1, t2) != 0) return 'X';
	else
	{
		if(strcmp(v1, v2) > 0) return 'C';
		else return 'U';
	}
}

int recv_files_loop(const char *file_name, int sockfd)
{
	FILE *fp;
	char *ptr, *old;
	char *buf;
	const int ack_length = 5;

	fp = fopen(file_name, "r");
	if(fp == NULL)
	{
		perror("error : file open");
		return -1;
	}

	char file_nm[50] = { 0x00, };
	while(fgets(file_nm, sizeof(file_nm), fp) != NULL)
	{
		if((ptr = strchr(file_nm, '\r')) != NULL)
			*ptr = 0x00;
		if((ptr = strchr(file_nm, '\n')) != NULL)
			*ptr = 0x00;

		char *token = strtok_r(file_nm, "_", &old);

		if(tcp_send(sockfd, token, strlen(token), 1) < 0)
		{
			fclose(fp);
			return -1;
		}

		buf = malloc(ack_length+1);
		if(tcp_recv(sockfd, token, strlen(token), 1) < 0)
		{
			fclose(fp);
			return -1;
		}

		free(buf);
	}

	fclose(fp);

	return 0;

}

int append_file_from_buf(const char *fname, char *d, int length)
{
	FILE *fp;
	fp = fopen(fname, "a+");
	if(fp == NULL)
	{
		debug("error");
		return -1;
	}
	fwrite(d, length, 1, fp);
	fclose(fp);

	return 0;
}

int write_file_from_buf(const char *fname, char *d, int length)
{
	FILE *fp;
	fp = fopen(fname, "w");
	if(fp == NULL)
	{
		debug("error");
		return -1;
	}
	fwrite(d, length, 1, fp);
	fclose(fp);

	return 0;
}

int cmp_files(const char *result, const char *file1, const char *file2)
{
	FILE *s, *t;
	char s_buf[50] = { 0x00, };
	char t_buf[50] = { 0x00, };

	s = fopen(file1, "r");
	t = fopen(file2, "r");

	while(fgets(s_buf, sizeof(s_buf), s) != NULL)
	{
		char ch;
		int  search = 0;

		fseek(t, 0x00, SEEK_SET);
		while(fgets(t_buf, sizeof(t_buf), t) != NULL)
		{
			ch = cmp_str(s_buf, t_buf);
			if(ch=='U' || ch=='C')
			{
				search = 1;
				break;
			}
		}

		if(!search) ch = 'C';

		char tmp[50] = { 0x00, };
		char *token, *old;
		token = strtok_r(s_buf, "_", &old);
		sprintf(tmp, "%s_%c\n", token, ch);
		append_file_from_buf("./result.txt", tmp, strlen(tmp));
	}

	fseek(t, 0x00, SEEK_SET);
	while(fgets(t_buf, sizeof(t_buf), t) != NULL)
	{
		char ch;
		int search = 0;

		fseek(s, 0x00, SEEK_SET);
		while(fgets(s_buf, sizeof(s_buf), s) != NULL)
		{
			ch = cmp_str(t_buf, s_buf);
			if(ch=='U' || ch=='C')
			{
				search = 1;
				break;
			}
		}

		if(!search)
		{
			ch = 'D';
			char tmp[50] = { 0x00, };
			char *token, *old;
			token = strtok_r(t_buf, "_", &old);
			sprintf(tmp, "%s_%c\n", token, ch);
			append_file_from_buf("./result.txt", tmp, strlen(tmp));
		}
	}

	fclose(s);
	fclose(t);

	return 0;
}

int file_operation(void)
{
	FILE *fp;
	char b[50] = { 0x00, };

	fp = fopen("./result.txt", "r");
	if(fp == NULL)
	{
		debug("error\n");
		return -1;
	}

	while(fgets(b, sizeof(b), fp) != NULL)
	{
		char *token, *old;
		char tmp[50] = {0x00, };
		char file_name[50] = {0x00, };

		token = strtok_r(b, "_", &old);
		if(token != NULL)
		{
			strncpy(file_name, token, strlen(token));
			token = strtok_r(NULL, "_", &old);

			if(token != NULL)
			{
				switch(token[0])
				{
				case 'D' : 	sprintf(tmp, "rm -f ./target/%s", file_name);
							//system(tmp);
							debug("%s\n", tmp);
							break;
				case 'C' : 	sprintf(tmp, "cp ./source/%s ./target/%s", file_name, file_name);
							//system(tmp);
							debug("%s\n", tmp);
							break;
				default  :	break;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}


int write_list_file(const char *file_name, char *data, int len)
{
	FILE *fp;
	fp = fopen(file_name, "w");
	if( fp == NULL)
	{
		perror("error : file open");
		return -1;
	}

	fwrite(data, len, 1, fp);
	fclose(fp);
	return 0;
}

int tcp_send(int sockfd, const char *buf, int len, int tm_sec)
{
	fd_set write_fds;
	struct timeval timeout;
	int nLeft = len;			// 미송신 데이터 사이즈
	int nSent = 0;				// 송신 완료 데이터 사이즈

	timeout.tv_sec = tm_sec;
	timeout.tv_usec = 0;

	FD_ZERO(&write_fds);
	FD_SET(sockfd, &write_fds);

	while(nLeft > 0)
	{
		int ret = select(sockfd+1, NULL, &write_fds, NULL, (tm_sec == 0)? NULL : &timeout);
		if(ret == -1)
		{
			if(errno == EINTR) continue;
			else
			{
				perror("error : sock send select");
				return -1;
			}
		}
		else if(ret == 0)
		{
			perror("error : sock send select");
			return -1;
		}

		if(FD_ISSET(sockfd, &write_fds))
		{
			nSent = send(sockfd, buf, nLeft, 0);
			if(nSent == -1)
			{
				if(errno == EINTR) nSent = 0;
				else
				{
					perror("error : sock send");
					return -1;
				}
			}
			nLeft -= nSent;
			buf += nSent;
		}
	}
	return 0;
}

int tcp_recv(int sockfd, char *buf, int len, int tm_sec)
{
	fd_set read_fds;
	struct timeval timeout;
	int nLeft = len;			// 미송신 데이터 사이즈
	int nRecv = 0;				// 수신 완료 데이터 사이즈

	timeout.tv_sec = tm_sec;
	timeout.tv_usec = 0;

	while(nLeft > 0)
	{
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);

		int ret = select(sockfd+1, &read_fds, NULL, NULL, (tm_sec == 0)? NULL : &timeout);
		if(ret == -1)
		{
			if(errno == EINTR) continue;
			else
			{
				perror("error : sock recv select");
				return -1;
			}
		}
		else if(ret == 0)
		{
			perror("error : sock recv select");
			return -1;
		}

		if(FD_ISSET(sockfd, &read_fds))
		{
			nRecv = recv(sockfd, buf, nLeft, MSG_NOSIGNAL);
			if(nRecv == -1)
			{
				if(errno == EINTR) nRecv = 0;
				else
				{
					perror("error : sock recv select");
					return -1;
				}
			}
			nLeft -= nRecv;
			buf += nRecv;
		}
	}

	return 0;
}

int tcp_connect(int * psockfd, int tm_sec)
{
	int client_len;
	int sockfd;
	struct sockaddr_in clientaddr;
	fd_set fds;
	struct timeval timeout;
	const char *ip_addr = "12.0.0.1";
	const int port_no = 1234;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(ip_addr);
	clientaddr.sin_port = htons(port_no);
	client_len = sizeof(clientaddr);

	while( connect(sockfd, (struct sockaddr *)&clientaddr, client_len) < 0 )
	{
		if(errno != EINPROGRESS)
		{
			close(sockfd);
			printf("error : connect errno=%d[%s]\n", errno, strerror(errno));
			return -1;
		}

		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);

		timeout.tv_sec = tm_sec;
		timeout.tv_usec = 0;

		if(select(sockfd+1, NULL, &fds, NULL, &timeout) <= 0)
		{
			close(sockfd);
			printf("error : select errno=%d[%s]\n", errno, strerror(errno));
			return -1;
		}
	}

	*psockfd = sockfd;

	return 0;
}

int sock_main(void)
{
	int sockfd;
	char *buf;
	const int ack_list_length = 5;

	if(tcp_connect(&sockfd, 1) < 0)
		exit(-1);

	// cmd list process
	if(tcp_send(sockfd, "list", 4, 1) < 0)
		exit(-1);

	buf = (char *)malloc(ack_list_length+1);
	memset(buf, 0x00, ack_list_length+1);
	if(tcp_recv(sockfd, buf, ack_list_length, 1) < 0)
	{
		free(buf);
		exit(-1);
	}
	int data_len = atoi(buf);
	buf = realloc(buf, data_len+1);
	memset(buf, 0x00, data_len+1);
	if(tcp_recv(sockfd, buf, data_len, 1) < 0)
	{
		free(buf);
		exit(-1);
	}
	write_list_file("./file_list.txt", buf, data_len);
	free(buf);

	// receive each file
	recv_files_loop("./file_list.txt", sockfd);

	tcp_send(sockfd, "quit", 4, 1);
	close(sockfd);
	return 0;
}

int main(void) {
	system("rm -f ./file_list.txt");
	system("rm -f ./target/file_list.txt");
	system("rm -f ./result.txt");

	sock_main();
	make_file_list_from_dir("./source", "file_list.txt");
	make_file_list_from_dir("./target", "file_list.txt");
	cmp_files("./result.txt", "./source/file_list.txt", "./taret/file_list.txt");
	file_operation();
	return EXIT_SUCCESS;
}
