/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\data_deal.c
#       @date         :2015-12-02 17:38
#       @algorithm    :
==========================================================================*/
#include "data_deal.h"

#include "rmfsystem.h"
#include <time.h>
#include <sys/time.h>
#include "sqlite3.h"
#include <unistd.h>

sqlite3 *db;
//int dbcount1=0;
//int dbcount2=0;

//char result[8][2048];



/***************************************************************************
Function: db_init

Description: to initialize database connection descriptor

Calls: NULL

Called By: comtest_pipe/client_pipe.main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: the global variable db

Return: NULL

Others: NULL
***************************************************************************/
void  db_init(void)
{
	char* errmsg;
	int rc;
	rc=sqlite3_open(DB,&db);
	if(rc!=SQLITE_OK)
	{
		DEBUG("cannot open db:%s",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	sqlite3_exec(db,"PRAGMA synchronous=OFF",0,0,0);
	return;
}



/***************************************************************************
Function: db_base_maintenence

Description: to check database capacity if the capacity is too big, delete some of records 

Calls: NULL

Called By: signal_handler

Table Accessed: device_second_level_data device_alarm device_running_statement

Table Updated: device_second_level_data device_alarm device_running_statement

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
/*
void db_base_maintenance(sqlite3 * dbase)
{
	int column;
	char db_table[3][32]={NORMAL_TABLE,ALARM_TABLE,STATE_TABLE};
	int rc;
	int count;
	char *errmsg=0;
	sqlite3_stmt* stmt=0;
	char sql[128];
	int i;
	for(i=0;i<3;i++)
	{
		sprintf(sql,"select count(id) from %s",db_table[i]);
		while((rc=sqlite3_prepare_v2(dbase,sql,-1,&stmt,0))!=SQLITE_OK)
		{
			if(rc==SQLITE_OK)
			{
				usleep(1000);
				continue;
			}
			if(stmt)
			{
				sqlite3_finalize(stmt);
			}
            DEBUG("PREPARE ERROR:%s",sqlite3_errmsg(dbase));
			exit(1);
		}
		count=sqlite3_column_count(stmt);
		rc=sqlite3_step(stmt);
		if(rc==SQLITE_ROW)
		{
             get the column number of the table 
			column=sqlite3_column_int(stmt,0);
			if(column>10000000)
			{	
                 if the num is beyond 10,000,000,about 20GB, delete the rest 
				column-=10000000;
				sprintf(sql,"delete from %s order by id limit %d",db_table[i],column);
				while(1)
				{
					rc=sqlite3_exec(dbase,sql,0,0,&errmsg);
					if(rc!=SQLITE_OK)
					{
						if(rc==SQLITE_BUSY)
						{
							usleep(1000);
							continue;
						}
						DEBUG("SQLITE3 DELETE:%s",errmsg);
						exit(1);
					}
					else
						break;
				}
			}
		}	
		sqlite3_finalize(stmt);
	}
	return;
}


*/
/***************************************************************************
Function:confirm

Description: when receive  the response, to update the table

Calls: NULL

Called By: NULL

Table Accessed: NULL

Table Updated: device_second_level_data

INput: 
	number: the number of data packet to update 

Output: NULL

Return: NULL

Others: UNUSED
***************************************************************************/
int confirm(int number)
{	
	int rc;
	sqlite3_stmt *stmt3=NULL;
	char * sql="update device_second_level_data set confirm=1 where id=?";
	while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt3,NULL)!=SQLITE_OK))
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		if(stmt3)
			sqlite3_finalize(stmt3);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_int(stmt3,1,number);
	while((rc=sqlite3_step(stmt3))!=SQLITE_DONE)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		sqlite3_finalize(stmt3);
		sqlite3_close(db);
		DEBUG("STEP ERROR!");
		exit(1);
	}
	sqlite3_reset(stmt3);
}

/*
int insert_second(unsigned char *data,int length)
{
	int rc;
	char*errmsg=0;
	if(dbcount2==0)
	{
		if(sqlite3_exec(db,"BEGIN",NULL,NULL,&errmsg)!=SQLITE_OK)
		{
			DEBUG("BEGIN ERROR");
			exit(1);
		}
		sqlite3_free(errmsg);
	}
	sqlite3_stmt *stmt=NULL;
	const char*sql="insert into device_second_level_data(id,data) values(NULL,?)";
	if(sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_blob(stmt,3,data,length,NULL);
	while(1)
	{
		if((rc=sqlite3_step(stmt))!=SQLITE_DONE)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("INSERT ERROR");
			exit(1);
		}
		else 
			break;
	}
	sqlite3_finalize(stmt);
	dbcount2++;
	if(dbcount2>=100)
	{
		if(sqlite3_exec(db,"COMMIT",NULL,NULL,&errmsg)!=SQLITE_OK)
		{

			DEBUG("COMMIT ERROR");
			exit(1);
		}
		sqlite3_free(errmsg);
		dbcount2=0;
	}
	return 1;
}

*/



/***************************************************************************
Function: msg_recv

Description: to receive the data from message queue

Calls:  NULL

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput: 
	msg: the msg descriptor 

Output:
	data:the received data to be store in the buff
	

Return: 
	if success return 1
	else return 0

Others:UNUSED
***************************************************************************/
int msg_recv(struct msg_local*data,int msg)
{
	int ret;
	int length;
	length=sizeof(struct msg_local);
	if((ret=msgrcv(msg,(void*)data,length,0,IPC_NOWAIT))==-1)
	{
		if(errno!=ENOMSG)
		{
			DEBUG("remote msgrecv error");
			exit(1);
		}
		return 0;
	}
	else 
		return 1;
}

/*
int insert_alarm(unsigned char *data,int length)
{
	int rc;
	char *errmsg;
	struct timeval tpstart,tpend;
	float timeuse;
	char sql[512];
	sprintf(sql,"insert into device_alarm(length,data) values(%d,\"%s\")",length,data);
	gettimeofday(&tpstart,NULL);
	rc=sqlite3_exec(db,sql,0,0,&errmsg);
	gettimeofday(&tpend,NULL);
	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
	timeuse/=1000000;
	printf("used time : %f sec\n",timeuse);
	if(rc==SQLITE_OK)
		return 1;
	else 0;
}
*/


/***************************************************************************
Function: insert_alarm

Description: where receive alarm data, insert the data into db

Calls: NULL

Called By: alarm_recv

Table Accessed: NULL

Table Updated: device_alarm

INput: 
	data: the alarm data to store
	length: the length of data

Output: NULL

Return: 
	if success return 1
	else return -1

Others: NULL
***************************************************************************/
int insert_alarm(unsigned char *data,int length)
{
	int rc;
	char *errmsg=0;
//	struct timeval tpstart,tpend;
//	float timeuse;
	sqlite3_stmt *stmt=NULL;
	char *sql="insert into device_alarm(id,length,data) values(NULL,?,?)";
//	gettimeofday(&tpstart,NULL);
	if(length>0)
	{
		while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0))!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
			{
				usleep(10000);
				continue;
			}
			if(stmt)
				sqlite3_finalize(stmt);
			DEBUG("PREPARE ERROR:%s",sqlite3_errmsg(db));
			exit(1);
		}
//		gettimeofday(&tpstart,NULL);
		sqlite3_bind_int(stmt,1,length);
		sqlite3_bind_blob(stmt,2,data,length,NULL);
//		gettimeofday(&tpend,NULL);
		while(1)
		{
			if((rc=sqlite3_step(stmt))!=SQLITE_DONE)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("INSERT ERROR");
				exit(1);
			}
			else 
				break;
		}
//		gettimeofday(&tpend,NULL);
		sqlite3_finalize(stmt);
//		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
//		timeuse/=1000000;
//		printf("used time:%f sec\n",timeuse);
		return 1;
	}
	else
		return -1;
}



/***************************************************************************
Function: insert_second

Description: when receive data, store it

Calls: NULL

Called By: second_level_recv

Table Accessed: NULL

Table Updated: device_second_level_data

INput:
	data: the received data
	length: the length of the data

Output: NULL

Return:
	if success return 1
	else return -1

Others: NULL
***************************************************************************/
int insert_second(unsigned char *data,int length)
{
	int rc;
	char*errmsg=0;
	if(length>0)
	{
//		if(sqlite3_exec(db,"BEGIN",NULL,NULL,&errmsg)!=SQLITE_OK)
//		{
//			DEBUG("BEGIN ERROR");
//			exit(1);
//		}
//		sqlite3_free(errmsg);
		sqlite3_stmt *stmt=NULL;
		const char*sql="insert into device_second_level_data(id,length,data) values(NULL,?,?)";
		while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0))!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
			{
				usleep(20000);
				continue;
			}
			if(stmt)
				sqlite3_finalize(stmt);
			DEBUG("PREPARE ERROR");
			exit(1);
		}
		sqlite3_bind_int(stmt,1,length);
		sqlite3_bind_blob(stmt,2,data,length,NULL);
		while(1)
		{
			if((rc=sqlite3_step(stmt))!=SQLITE_DONE)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("INSERT ERROR");
				exit(1);
			}
			else 
				break;
		}
		sqlite3_finalize(stmt);
//		if(sqlite3_exec(db,"COMMIT",NULL,NULL,&errmsg)!=SQLITE_OK)
//		{
//			DEBUG("COMMIT ERROR");
//			exit(1);
//		}
//		sqlite3_free(errmsg);
	}
	else
		return 0;
	return 1;
}



/***************************************************************************
Function: query

Description: get the data from device_second_level_data 

Calls: NULL

Called By: remote_seond

Table Accessed: device_second_level_data

Table Updated: NULL

INput: 
	data: the buffer to store the data

Output: NULL

Return: 
	if have data return 1
	else no data return 0 
	else error -1

Others: NULL
***************************************************************************/
int query(struct remote_data* data)
{
	int result;
	int len;
	sqlite3_stmt* stmt=0;
	int ncolumn=0;
	int vtype,i;
	int rc;
	char sql[1024];
	sprintf(sql,"select * from device_second_level_data where id=%lld",data->number);
	while((rc=sqlite3_prepare_v2(db,sql,-1,&stmt,0))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(1000);
			continue;
		}
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		DEBUG("PREPARE ERROR");
		return -1;
	}
	ncolumn=sqlite3_column_count(stmt);
//	printf("ncolumn=%d\n",ncolumn);
	rc=sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
	{
		data->number=sqlite3_column_int(stmt,0);
		data->length=sqlite3_column_int(stmt,1);
		strcpy(data->time,sqlite3_column_text(stmt,2));
		len=sqlite3_column_bytes(stmt,3);
		memcpy(data->data,sqlite3_column_blob(stmt,3),len);
		sqlite3_finalize(stmt);
		return 1;
	}
	else
	{
		sqlite3_finalize(stmt);
		return 0;
	}
}




/***************************************************************************
Function: query_state

Description: to get data from device_running_statement

Calls: NULL

Called By: NULL

Table Accessed: device_running_statement

Table Updated: NULL

INput: NULL

Output: NULL

Return:
	if device is on return 1
	else return 0  

Others: NULL
***************************************************************************/
int query_state()
{
	char state[20];
	int result;
	int len;
	sqlite3_stmt* stmt=0;
	int ncolumn=0;
	int vtype,i;
	int rc;
	char sql[1024];
	sprintf(sql,"select * from device_running_statement order by id desc limit 0,1");
	while((rc=sqlite3_prepare_v2(db,sql,-1,&stmt,0))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(1000);
			continue;
		}
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		DEBUG("PREPARE ERROR");
		return -1;
	}
	ncolumn=sqlite3_column_count(stmt);
//	printf("ncolumn=%d\n",ncolumn);
	rc=sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
	{
//		data->number=sqlite3_column_int(stmt,0);
//		data->length=sqlite3_column_int(stmt,1);
		strcpy(state,sqlite3_column_text(stmt,2));
//		len=sqlite3_column_bytes(stmt,3);
//		memcpy(data->data,sqlite3_column_blob(stmt,3),len);
	//	printf("len=%d\n",len);
		sqlite3_finalize(stmt);
		if(strcmp(state,"DEVICE ON")==0)
			return 1;
		else if(strcmp(state,"DEVICE OFF")==0)
			return 0;
	}
	else
	{
		sqlite3_finalize(stmt);
		return 0;
	}
}
/***************************************************************************
Function: query_maxid

Description: to get maxid from device_second_level_data

Calls: NULL

Called By: NULL

Table Accessed: device_second_level_data

Table Updated: NULL

INput: NULL

Output: NULL

Return:
	the max id

Others: NULL
***************************************************************************/
int query_maxid()
{
	int maxid=0;
	int result;
	int len;
	sqlite3_stmt* stmt=0;
	int ncolumn=0;
	int vtype,i;
	int rc;
	char sql[1024];
	sprintf(sql,"select id from device_second_level_data order by id desc limit 1");
	while((rc=sqlite3_prepare_v2(db,sql,-1,&stmt,0))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(1000);
			continue;
		}
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		DEBUG("PREPARE ERROR");
		return -1;
	}
//	ncolumn=sqlite3_column_count(stmt);
//	printf("ncolumn=%d\n",ncolumn);
	rc=sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
	{
		maxid=sqlite3_column_int(stmt,0);
//		data->length=sqlite3_column_int(stmt,1);
//		strcpy(state,sqlite3_column_text(stmt,2));
//		len=sqlite3_column_bytes(stmt,3);
//		memcpy(data->data,sqlite3_column_blob(stmt,3),len);
	//	printf("len=%d\n",len);
		sqlite3_finalize(stmt);
		return maxid;
	}
	else
	{
		sqlite3_finalize(stmt);
		return 0;
	}
}



/***************************************************************************
Function: query_alarm

Description: to get the data from device_alarm

Calls: NULL

Called By: remote_send

Table Accessed: device_alarm

Table Updated: NULL

INput: NULL

Output:
	data: the buffer to store the data 

Return:
	if have data return 1
	else if no data return 0
	else return -1

Others: NULL
***************************************************************************/
int query_alarm(struct alarm_data* data)
{
	int result;
	int len;
	sqlite3_stmt* stmt=0;
	int ncolumn=0;
	int vtype,i;
	int rc;
	char sql[1024];
	sprintf(sql,"select * from device_alarm where id=%lld",data->number);
	while((rc=sqlite3_prepare_v2(db,sql,-1,&stmt,0))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(1000);
			continue;
		}
		if(stmt)
		{
			sqlite3_finalize(stmt);
		}
		DEBUG("PREPARE ERROR");
		return -1;
	}
	ncolumn=sqlite3_column_count(stmt);
//	printf("ncolumn=%d\n",ncolumn);
	rc=sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
	{
//		data->number=sqlite3_column_int(stmt,0);
		data->length=sqlite3_column_int(stmt,2);
		strcpy(data->time,sqlite3_column_text(stmt,1));
		len=sqlite3_column_bytes(stmt,3);
		memcpy(data->data,sqlite3_column_blob(stmt,3),len);
		sqlite3_finalize(stmt);
		return 1;
	}
	else
	{
		sqlite3_finalize(stmt);
		return 0;
	}
}
/*
int query(struct remote_data* data)
{
	int i=0;
	int j=0;
	int nrow=0,ncolumn=0;
	char**result;
	char *errmsg=0;
	int rc;
	int len=0;
	char sql[100];
	sprintf(sql,"select * from device_second_level_data where id=%lld",data->number);
	while(1)
	{
		result=NULL;
	//	printf("xx\n");
		rc=sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
	//	printf("xm\n");
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
			{
				usleep(1000);
				continue;
			}
			DEBUG("query error:%s",sqlite3_errmsg(db));
			if(errmsg)
				printf("errmsg=%s\n",errmsg);
			sqlite3_free_table(result);
			sqlite3_close(db);
			return -1;
		}
		else
			break;
	}
//	printf("nrow=%d ncolumn=%d\n",nrow,ncolumn);
	if(nrow==1&&ncolumn==4)
	{
//		printf("4:%s\n5:%s\n6:%s\n7:%s\n",result[4],result[5],result[6],result[7]);
	//	printf("xx\n");
		data->number=atoi(result[4]);
	//	printf("ld\n");
		data->length=atoi(result[5]);
	//	printf("xx\n");
		strcpy(data->time,result[6]);
		printf("ld\n");
//		printf("result=%d\n",sizeof(result[7]));
//		if(result[7]!=NULL)
//		{
		memcpy(data->data,result[7],data->length);
//		}
		printf("xx\n");
	}
	else if(nrow==0&&ncolumn==0)
	{
		printf("no data\n");
	//	sqlite3_free_table(result);
		return 0;
	}
	else
	{
		DEBUG("QUERY ERROR!\n");
		return -1;
	}
//	memset(result,0,sizeof(result));
	printf("xmxs\n");
	sqlite3_free_table(result);
	printf("dhakjwh\n");
	return 1;
}
*/


/***************************************************************************
Function: dbrecord_v2

Description: to keep the device statement in device_running_statement

Calls: NULL

Called By: comtest_pipe.tcp_connect client_pipe.login main.main

Table Accessed: NULL

Table Updated: device_running_statement

INput: the string to store in the table

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void dbrecord_v2(char * state)
{
	int rc;
	char sql[1024];
	char *errmsg=0;
	sprintf(sql,"insert into device_running_statement(id,statement) values(NULL,\"%s\")",state);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("REMOTE RECORD ERROR:%s",errmsg);
			exit(1);
		}
		else
			break;
	}
	return;
}



/***************************************************************************
Function: dbrecord

Description: to keep the device running statement in device_running_statement

Calls: NULL

Called By: NULL

Table Accessed: NULL

Table Updated: device_running_statement

INput: 
	state: the string to store the infomation 

Output: NULL

Return: if success return 1

Others: UNUSED
***************************************************************************/
int dbrecord(char *state)
{
	char *errmsg;
	int rc;
	sqlite3 *db;
	char  sql[64];
	rc=sqlite3_open(DB,&db);
	if(rc!=SQLITE_OK)
	{
		DEBUG("cannot open db:%s",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	sprintf(sql,"insert into device_running_statement(id,statement) values(NULL,\"%s\")",state);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("RECORD ERROR:%s",errmsg);
			exit(1);
		}
		else 
			break;
	}
	sqlite3_close(db);
	return 1;
}
