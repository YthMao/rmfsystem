/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\init.c
#       @date         :2015/11/02 09:38
#       @algorithm    :
==========================================================================*/
#include "init.h"

#include "rmfsystem.h"
#include "mini-ntpclient.h"
#include "data_deal.h"
#include "read_file.h"
#include "change_profile.h"
#include "scan.h"
#include <unistd.h>

/***************************************************************************
Function: db_init

Description: to check the statement of database

Calls: db_exit db_table

Called By: init

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void db_initialize()
{
	sqlite3 * db;
    printf("DATE: %s database: sqlite3 detecting!......\n",timerecord());
    /* check the existence of db */
	db=db_exist();
    /* check the existence of tables */
	db_table(db);
    /* delete some records in db at regular time(every day) */ 
    printf("DATE: %s database ok!......\n",timerecord());
	return;
}


/***************************************************************************
Function: db_exist

Description: to check whether the database exist

Calls: NULL

Called By: db_init

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
sqlite3* db_exist()
{
	int rc;
	sqlite3 * db;
	rc=sqlite3_open_v2(DB,&db,SQLITE_OPEN_READWRITE,NULL);
	if(rc==SQLITE_OK)
	{
        printf("DATE: %s database: db exist detecting ok!......\n",timerecord());
		return db;
	} 
    printf("DATE: %s database: db not exist!......\ncreate database!......\n",timerecord());
	sqlite3_close(db);
	while(1)
	{
		rc=sqlite3_open(DB,&db);
		if(rc!=SQLITE_OK)
		{
			DEBUG("create database fail!......\n");
			sqlite3_close(db);
			sleep(3);
		}
		else
		{
            printf("DATE: %s create database successfully!......\n",timerecord());
			return db;
		}
	}
}



/***************************************************************************
Function: db_table

Description: to check the db table if not exist,to establish it 

Calls: NULL

Called By: db_init

Table Accessed: device_running_statement device_second_level_data device_alarm

Table Updated: NULL

INput: 
	db: the db decsriptor

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void db_table(sqlite3 * db)
{
	char **result;
	int nrow,ncolumn;
	int rc;
	char * errmsg;
	char * sql="SELECT count(id) FROM device_alarm";
	char * sql1="SELECT count(id) FROM device_second_level_data";
	char * sql2="SELECT count(id) FROM device_running_statement";
	rc=sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_alarm fail!......\n%s",errmsg);
		sqlite3_close(db);
		return;
	}
    printf("DATE: %s database: table device_alarm detected......\n",timerecord());
	rc=sqlite3_get_table(db,sql1,&result,&nrow,&ncolumn,&errmsg);
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_second_level_data fail!......\n%s",errmsg);
	    sqlite3_close(db);
		return;
	}
    printf("DATE: %s database: table device_second_level_data detected......\n",timerecord());
	rc=sqlite3_get_table(db,sql2,&result,&nrow,&ncolumn,&errmsg);
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_running_statement fail!......\n%s",errmsg);
		sqlite3_close(db);
		return;
	}
    printf("DATE: %s database: table device_running_statement detected......\n",timerecord());
	state_table_detect(db);
	sqlite3_close(db);
}

/***************************************************************************
Function: profile_init

Description: to check the profile

Calls: profile_config profile_process profil_dev

Called By: init 

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL
 
Return: NULL

Others: NULL
***************************************************************************/
void state_table_detect(sqlite3* db)
{
	int flg=0;
	char state[32];
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
		return;
	}
	ncolumn=sqlite3_column_count(stmt);
	rc=sqlite3_step(stmt);
	if(rc==SQLITE_ROW)
	{
		strcpy(state,sqlite3_column_text(stmt,2));
		sqlite3_finalize(stmt);
		if(strcmp(state,"DEVICE ON")==0|strcmp(state,"START")==0)
			flg=1;
		else
			flg=0;
	}
	else
	{
		sqlite3_finalize(stmt);
		flg=0;
	}
	if(flg==1)
	{
		err_record(db,"ERROR CLOSE");
	}
	else
		return;
}



/***************************************************************************
Function: profile_init

Description: to check the profile

Calls: profile_config profile_process profil_dev

Called By: init 

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL
 
Return: NULL

Others: NULL

***************************************************************************/
int err_record(sqlite3* db,char *state)
{
	char * errmsg;
	int rc,count=0;
	char sql[1024];
	sprintf(sql,"insert into device_running_statement(id,statement) values(NULL,\"%s\")",state );
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			count++;
			if(rc==SQLITE_BUSY)
				continue;
			while(count==10)
			{
				DEBUG("RECORD ERROR:%s",errmsg);
				count=0;
			}
			sqlite3_close(db);
			sleep(1);
		}
		else
			break;
		usleep(1000);
	}
	return 1;
}


/***************************************************************************
Function: profile_init

Description: to check the profile

Calls: profile_config profile_process profil_dev

Called By: init 

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL
 
Return: NULL

Others: NULL
***************************************************************************/
void profile_init()
{
    /* check config */
	profile_config();
    /* check device_config */
	profile_dev();
    /* check process.config */
	profile_process();
}



/***************************************************************************
Function: profile_config

Description: to check the profile: config

Calls: NULL

Called By: profile_init

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void profile_config()
{
    int rc;
    while(1)
    {
	rc=access(CONFIG,0);
	if(rc!=0)
	{
	    DEBUG("profile: %s not exist!......\n",CONFIG);
	    sleep(1);
	}
	else
	    break;
    }
    printf("DATE: %s profile: %s detect ok!......\n",timerecord(),CONFIG);
}



/***************************************************************************
Function: profile_dev

Description: to check profile device_config

Calls: NULL

Called By: profile_init

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void profile_dev()
{
	int rc;
        while(1)
        {
	    rc=access(DEV_CONF,0);
	    if(rc!=0)
	    {
		DEBUG("profile: %s not exist!......\n",DEV_CONF);
		sleep(1);
	    }
	    else 
		break;
	}
    printf("DATE: %s profile: %s detect ok!......\n",timerecord(),DEV_CONF);
}



/***************************************************************************
Function: profile_process 

Description: to check profile_process

Calls: NULL

Called By: profile_init

Table Accessed: NULL

Table Updated: NULL
 
INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void profile_process()
{
    int rc;
    while(1)
    {
        rc=access(PRO_CONF,0);
	if(rc!=0)
	{
	    DEBUG("profile: %s not exist!......\n",PRO_CONF);
	    sleep(1);
	}
	else
	    break;
    }
    printf("DATE: %s profile: %s detect ok!......\n",timerecord(),PRO_CONF);

}



/***************************************************************************
Function: init

Description: to check database and profile

Calls: db_init profile_init scan ntp

Called By: main.mian

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void init()
{
	int rc;
    /* DB check */
	db_initialize();
    /* profile check */
	profile_init();
    printf("DATE: %s initial finished \n",timerecord());
}
