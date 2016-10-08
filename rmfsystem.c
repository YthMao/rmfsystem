/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\rmfsystem.c
#       @date         :2015/12/04 16:11
#       @algorithm    :
==========================================================================*/
#include "rmfsystem.h"

#include "read_file.h"
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include "sqlite3.h"
#include <unistd.h>

/***************************************************************************
Function:send_signal

Description: to send signal to inform the process according Pid

Calls: read_file_v2

Called By: comtest_pipe.tcp_connect main.main comtest_pipe.second_level_recv 

Table Accessed: NULL

Table Updated: NULL

INput: 
	state: the signal to send SIGUSR1 SIGUSR2 

Output: NULL

Return: 
	if success return 1
	else return 0

Others: NULL
***************************************************************************/
int send_signal(int state)
{

	int ret;
	int pid;
	char *p;
	char line[100];
//	printf("state=%d\n",state);
//	if(state==1)
//	{
//		read_file_v2("pid","local_pid",line);
//		pid=atoi(line);
//	}
//	else if(state==2)
//	{
//		read_file_v2("pid","deal_pid",line);
//		pid=atoi(line);
//	}
//	else if(state==3)
//	{
	read_file_v2("pid","remote_pid",line);
	pid=atoi(line);
//	}
	printf("pid=%d\n",pid);
	if(state==1)
	{
		ret=kill((pid_t)pid,SIGUSR1);
		printf("local:send signal SIGUSR1\n");
	}
	else if(state==2)
	{
		ret=kill((pid_t)pid,SIGUSR2);
		printf("local:send signal SIGUSR2\n");
	}
	if(ret==0)
		return 1;
	else 
		return 0;
}
/*
void dbrecord_v2(char* state,sqlite3*db)
{
	int rc;
	char sql[256];
	char * errmsg=0;
	char time[100];
	gettime(time);
	sprintf(sql,"insert into device_running_statement values(\"%s\",\"%s\")",time,state);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("REMOTE RECORD ERROR");
			exit(1);
		}
		else
			break;
	}
	return;
}
*/


/***************************************************************************
Function: gettime

Description: to get system time

Calls: NULL

Called By: .

Table Accessed: NULL

Table Updated: NULL

INput:
	datatime: the buffer to store time

Output:	NULL

Return: NULL

Others: NULL
***************************************************************************/
void gettime(char *datetime)
{
	time_t now;
	char date[100];
	struct tm *tm_now;
	time(&now);
	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",(tm_now->tm_year+1900),(tm_now->tm_mon+1),tm_now->tm_mday,tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	memcpy(datetime,date,strlen(date));
	return;
}

/***************************************************************************
Function: timerecord

Description: to get system time

Calls: NULL

Called By: .

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output:	NULL

Return: 
	return the current time

Others: NULL
***************************************************************************/
char * timerecord(void)
{
	static char datetime[100];
	time_t now;
	char date[100];
	struct tm *tm_now;
	time(&now);
	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",(tm_now->tm_year+1900),(tm_now->tm_mon+1),tm_now->tm_mday,tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	memcpy(datetime,date,strlen(date));
	return datetime;
}



/***************************************************************************
Function: getconfigpath

Description: to get config path

Calls: NULL

Called By: .

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output:	NULL

Return: 
	return the config abstract path

Others: NULL
***************************************************************************/
char * getconfigpath(char *name)
{
	static char path[128];
	getcwd(path,128);
	strcat(path,"/configure/");
	strcat(path,name);
	return path;
}

/***************************************************************************
Function: getexecpath

Description: to get programme path

Calls: NULL

Called By: .

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output:	NULL

Return: 
	return the programme abstract path

Others: NULL
***************************************************************************/
char * getprogrammepath(char *name)
{
	static char path[128];
	getcwd(path,128);
	strcat(path,"/");
	strcat(path,name);
	return path;
}



