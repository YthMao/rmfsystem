/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2016
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem_1_15\main.c
#       @date         :2016-01-18 10:38
#       @algorithm    :
==========================================================================*/
#include "main.h"

#include "data_deal.h"
#include "rmfsystem.h"
#include <sys/types.h>
#include <signal.h>
#include "init.h"
#include <sqlite3.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>


static pid_t com=-1;
static pid_t deal=-1;
static pid_t local=-1;
static sqlite3 *db;

/***************************************************************************
Function:main

Description: to start up the whole system

Calls: init dbrecord

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/

int main()
{	
	int rc;
	int status;
//	DEBUG("TEST");
	init();
	dbrecord("START");
	pid_t temp;
	while(1)
	{
		if(local==-1)
		{
			temp=fork();
			local=temp;
			if(temp<0)
				printf("error in remote\n");
			else if(temp==0)
				if((rc=execl(RMT_PATH,"client_pipe",NULL))<0)
				{
					printf("main rmt err\n");
				}
		}
		sleep(1);
//		if(deal==-1)
//		{
//			temp=fork();
//			deal=temp;
//			if(temp<0)
//				printf("error in com\n");
//			else if(temp==0)
//				if((rc=execl("/home/lhw4d4/project/git/rmfsystem_test/data_deal","data_deal",NULL))<0)
//				{
//					printf("main deal err:%d\n",errno);
//				}
//		}
//		sleep(2);
		if(com==-1)
		{
			temp=fork();
			com=temp;
			if(temp<0)
				printf("error in local\n");
			else if(temp==0)
				if((rc=execl(COM_PATH,"comtest_pipe",NULL))<0)
				{
					printf("main local err\n");
				}
		}
		temp=waitpid(-1,&status,0);
		if(temp==com)
		{
			com=-1;
			kill(temp,SIGKILL);
			printf("main com error\n");
		}
		if(temp==deal)
		{
			deal=-1;
			kill(temp,SIGKILL);
			printf("main deal error\n");
		}
		if(temp==local)
		{
			local=-1;
			kill(temp,SIGKILL);
			printf("main local error\n");
		}
	}
}


/***************************************************************************
Function: dbinit

Description: to initialize the database connection

Calls: NULL

Called By: dbrecord

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void dbinit(void)
{
	int count=0;
	db=NULL;
	char* errmsg=0;
	int rc;
	while(1)
	{
		rc=sqlite3_open(DB,&db);
		if(rc)
		{
			count++;
			if(count==10)
			{
				DEBUG("DB init error!");
				fprintf(stderr,"		cannot open database:%s\n",sqlite3_errmsg(db));
				count=0;
			}
			sqlite3_close(db);
			sleep(1);
		}
		else
			break;
	}
	printf("main:db open successfully\n");
	return;
}



/***************************************************************************
Function: dbrecord

Description: to keep the startup infomation in databse

Calls: dbinit

Called By: main

Table Accessed: NULL

Table Updated: device_running_statement

INput: the string to store

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
static void dbrecord(char * state)
{
	dbinit();
//	char time[100];
//	gettime(time);
	int count=0;
	int rc;
	char*errmsg=0;
	sqlite3_stmt *stmt=NULL;
	const char*sql="insert into device_running_statement(id,statement) values(NULL,?)";
	while(1)
	{	
		rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			if(stmt)
				sqlite3_finalize(stmt);
			count++;
			if(count==10)
			{	
				DEBUG("PREPARE ERROR:%s",sqlite3_errmsg(db));
				count=0;
			}
			sqlite3_close(db);
			sleep(1);
		}
		else
			break;
	}
//	sqlite3_bind_text(stmt,1,time,strlen(time),SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt,1,state,strlen(state),SQLITE_TRANSIENT);
	count=0;
	while(1)
	{
		if(rc=sqlite3_step(stmt)!=SQLITE_DONE)
		{
			if(rc==SQLITE_BUSY)
				continue;
			count++;
			if(count==10)
			{
				DEBUG("INSERT ERROR");
				count=0;
			}
			sqlite3_close(db);
			sleep(1);
		}
		else
			break;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return;	
}



/***************************************************************************
Function: gettime

Description: to get the system time

Calls: NULL

Called By: dbrecord

Table Accessed: NULL

Table Updated: NULL

INput: 
	datatime:the buffer to store time

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
/*void gettime(char *datetime)
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

*/
