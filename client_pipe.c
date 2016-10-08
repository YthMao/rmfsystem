/*=========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\client_pipe.c
#       @date         :2015-12-02 17:51
#       @algorithm    :
=========================================================================*/
#include "client_pipe.h"
#include <python2.7/Python.h>
#include "data_deal.h"
#include "rmfsystem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "change_profile.h"
#include "read_file.h"
#include <sys/file.h>
#include <netdb.h>
#include <sys/wait.h>
#include <asm/ioctls.h>
#include "base64.h"
#include <sys/shm.h>
#include "md5.h"
#include "systemdetect.h"
//int k=0;
pthread_mutex_t mut;
/*the state of plc connection
* plc_connect_err=1 if error happened
* plc_connect_err=0 if NOT
*/
int plc_connect_err=0;
/*the running state of the facility
 * device_state=1 if the ficility is running
 * device_state=0 if the ficility is down
 */
volatile int  device_state=0;
//int datalevel2;
struct alarm_data alarm_data;
struct remote_data data;
int socketfd;
/*the start ID of the normal data packet */
int number=1;
/*the start ID of the alarm number packet */
int alarm_number=1;
/* the string to store username */
char username[100];


void set_timer()
{

    struct itimerval itv;
    itv.it_interval.tv_sec=30*10;
    itv.it_interval.tv_usec=0;
    itv.it_value.tv_sec=60;
    itv.it_value.tv_usec=0;
    setitimer(ITIMER_REAL,&itv,NULL);
}

/***************************************************************************
Function: udp_test

Description: the pthread function to send heart-beat packet 

Calls:Null

Called By: .main

Table Accessed: NULL 

Table Updated: NULL

INput: NULL

Output: Null

Return: NULL

Others: NULL
***************************************************************************/
void  udp_test(int m)
{
    char test[1024];
    int sockfd;
    int maxid,state;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;
    state=query_state();
    maxid=query_maxid();
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {
        DEBUG("establish socket error");
        perror("error");
    }
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(10000);
    server.sin_addr.s_addr=inet_addr("101.231.108.196");
    sin_size=sizeof(struct sockaddr_in);
    sprintf(test,"Local Time: %s\ndevice remains alive..\nthe current data id is %d..\nthe facility state is %d..\ntime interval is 5 min..",timerecord(),maxid,state);
    sendto(sockfd,test,strlen(test),0,(struct sockaddr*)&server,sin_size);
    close(sockfd);
}

/***************************************************************************
Function: signal_wait

Description: the pthread function to wait signal 
	SIGUSR1: show the statement of plc connection
	SIGUSR2: show the statement of device running

Calls: addroaltconfig

Called By: .main

Table Accessed: NULL 

Table Updated: NULL

INput: NULL

Output: change the global variable plc_connect_err and device_state

Return: NULL

Others: NULL
***************************************************************************/
void *signal_wait(void *arg)
{
//	devicehaha_state=1;
//	static int devicehaha_state=0;
	int result;
	char value[20];
    printf("DATE: %s remote: the signal thread begins!\n",timerecord());
	int err;
	int signo;
    /*set the waiting signal */
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset,SIGUSR1);
	sigaddset(&sigset,SIGUSR2);
    /*waiting for signal and solve them */
	while(1)
	{
//		devicehaha_state=1;
		err=sigwait(&sigset,&signo);
//		plc_connect_err=1;
//		devicehaha_state=1;
//		sleep(100);
		if(err!=0)
		{
			DEBUG("SIGNAL ERROR:%d",err);
			exit(1);
		}
        /* deal with plc connection problem */
		if(signo==SIGUSR1)
		{
	//		printf("plc error\n");
			plc_connect_err=plc_connect_err?0:1;
			DEBUG("remote:plc_connect_err=%d\n",plc_connect_err);
		}
        /* the facility running problem */
		if(signo==SIGUSR2)
		{
		//	plc_connect_err=1;
//			devicehaha_state=1;
			read_file("signal","setting",value);
			pthread_mutex_lock(&mut);
			device_state=atoi(value);
	//		addoraltconfig(DEV_CONF,"setting","setting=0");
			if(device_state==2)
				printf("device off\n");
			else if(device_state==1)
				printf("device on\n");
			else
				device_state=0;
			pthread_mutex_unlock(&mut);
				
		}
        printf("DATE: %s signal:device_state=%d\n",timerecord(),device_state);
	//	devicehaha_state=1;
	}
}

/*
void * test(void * arg)
{
	while(1)
	{
		printf("device_state=%d\n",device_state);
		sleep(1);
	}
}

void * signal_wait(void *arg)
{
//	static int device_state;
	int rc;
	char value[32];
	while(1)
	{
	//	printf("device_state=%d\n",device_state);
		read_file("signal","setting",value);
		rc=atoi(value);
		if(rc!=0)
		{
			device_state=rc;
			if(device_state==1)
				printf("device on\n");
			else if(device_state==2)
				printf("device off\n");
			addoraltconfig(DEV_CONF,"setting","setting=0");
		}
		read_file("signal","connect",value);
		rc=atoi(value);
		if(rc)
		{
			plc_connect_err=1;
			printf("plc connect error\n");
			addoraltconfig(DEV_CONF,"connect","connect=0");
		}
		usleep(100000);
	}
}
*/


/***************************************************************************
Function: write_pid_remote

Description: to record the process ID of the current process 

Calls: NULL

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL 

Return: NULL

Others: NULL
***************************************************************************/
void write_pid_remote()
{
	char line[50];
	int pid;
	pid=getpid();
	sprintf(line,"remote_pid=%d",pid);
	addoraltconfig(PRO_CONF,"remote_pid",line);
//	printf("write process.config ok\n");
	return;
}
/*
int data_response()
{
	int rc;
	int level;
	int number;
	char*p;
//	printf("data response\n");
	char response[50];
	while(1)
	{
		rc=get_line(socketfd,response,50);
		if(rc>1)
		{
			if(strncmp(response,"datalevel",9)==0)
			{
				p=strchr(response,'=');
				p++;
				response[strlen(response)-1]='\0';
				level=atoi(p);
			}
			if(strncmp(response,"number",6)==0)
			{
				p=strchr(response,'=');
				p++;
				response[strlen(response)-1]='\0';
				number=atoi(p);
			}
		}
		else
			break;
	}
//	printf("level=%d,number=%d\n",level,number);
	if((level==1&&number==datalevel1)|(level==2&&number==datalevel2))
	{
		pthread_mutex_lock(&mutex);
		confirm=1;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
	}
	if(level==1)
		update_data_1(number);
	else if(level==2)
		update_data_2(number);
	else
		DEBUG("UPDATE ERROR\n");
//	printf("10\n");
	return 1;

}
*/
/*
int update_data_1(int number)
{
	int rc;
//	printf("number=%d\n",number);
	sqlite3_stmt * stmt3=NULL;
	char * sql="update device_first_level_data set confirm=1 where number=?";
	while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt3,NULL))!=SQLITE_OK)
	{
//		printf("tata\n");
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
*/
/*
int update_data_2(int number)
{
	int rc;
//	printf("number=%d\n",number);
	sqlite3_stmt * stmt3=NULL;
	const char * sql="update device_second_level_data set confirm=1 where number=?";
	while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt3,NULL))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		if(stmt3)
			sqlite3_finalize(stmt3);
	//	sqlite3_close(db);
		DEBUG("PREPARE ERROR:%s",sqlite3_errmsg(db));
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
	//	sqlite3_close(db);
		DEBUG("STEP ERROR:%s",sqlite3_errmsg(db));
		exit(1);
	}
	sqlite3_reset(stmt3);
}
*/
/*
int update_data_2(int number)
{
	sqlite3_stmt * stmt;
	printf("number=%d\n",number);
	const char *sql1="update device_second_level_data set confirm=1 where number=?";
	if(sqlite3_prepare_v2(db,sql1,strlen(sql1),&stmt,NULL)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		sqlite3_close(db);
		DEBUG("PREPARE ERROR!");
		exit(1);
	}
	sqlite3_bind_int(stmt,1,number);
	if(sqlite3_step(stmt)!=SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		DEBUG("STEP ERROR:%d",errno);
		exit(1);
	}
	sqlite3_reset(stmt);
}
*/
/*int update_data(int datalevel,int number)
{
	int rc;
	char sql[100];
	char *errmsg=0;
	if(datalevel==1)
	{
		sprintf(sql,"update device_first_level_data set confirm=1 where number=%d",number);
		while(1)
		{
			rc=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			if(rc!=SQLITE_OK)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("ERRMSG=%s",errmsg);
				sqlite3_free(errmsg);
				perror("update error:");
				return 0;
			}
			else
				break;
		}
	}
	else if(datalevel==2)
	{
		sprintf(sql,"update device_second_level_data set confirm=1 where number=%d",number);
		while(1)
		{
			rc=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			if(rc!=SQLITE_OK)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("UPDATE ERROR");
				sqlite3_free(errmsg);
				return 0;
			}
			else 
				break;
		}
	}
	else
		return 0;
	return 1;
}
*/
/*
int device_change()
{
	int n;
	int rc;
	char*p;
	char command[5]="CHAG";
	char time[50];
	char recvbuff[50];
	char commandline[50];
	char parameter[20];
	char value[20];
	char number[10];
	struct msg_remote msg_data;
	while(1) 
	{
		rc=get_line(socketfd,recvbuff,sizeof(recvbuff));
		if(rc>1)
		{
			if(strncmp(recvbuff,"parameter",9)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(parameter,p);
				p=strchr(parameter,'\n');
				*p='\0';
			}
			if(strncmp(recvbuff,"value",5)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(value,p);
				p=strchr(value,'\n');
				*p='\0';
			}
			if(strncmp(recvbuff,"number",6)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(number,p);
				p=strchr(number,'\n');
				*p='\0';
			}
		}
		else
			break;
	}
	n=atoi(number);
	commandnumber=n;
//	printf("n=%d\n",n);
	sprintf(commandline,"%s=%s",parameter,value);
	printf("remote:CHAG: %s\n",commandline);
	gettime(time);
	insert_command(n,time,command,commandline);
	addoraltconfig(DEV_CONF,"setting","setting=1");
	addoraltconfig(DEV_CONF,parameter,commandline);
	send_signal(1);
	msg_data.mtype=1;
	strcpy(msg_data.command,"2120");
	msg_data.number=commandnumber;
	strcpy(msg_data.time,time);
	msg_send(&msg_data);
	return 1;
}
*/
/*
int msg_send(struct msg_remote*data)
{
	if(msgsnd(remote_msgid,(void*)data,sizeof(struct msg_remote),0)==-1)
	{
		DEBUG("REMOTE MSGSND ERROR");
		return 0;
	}
	else
		return 1;
}
*/
/*
int insert_command(int number,char *time,char *command,char*context)
{
	int rc;
	char sql[200];
	char *errmsg=0;
	sprintf(sql,"insert into device_command values(%d,\"%s\",\"%s\",\"%s\",1)",number,time,command,context);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("REMOTE SQLITE3 INSERT");
			exit(1);
		}
		else
			break;
	}
	return 1;
}
*/
/*
void dbinit()
{
	db=NULL;
	char *errmsg;
	int rc;
	rc=sqlite3_open("local.db",&db);
	if(rc!=SQLITE_OK)
	{
		DEBUG("CANnot OPEN DB:%s",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	else
		printf("remote:open local.db successfully\n");
	return;
}
*/
/*
void msg_init(void)
{
	int msgid;
	msgid=msgget((key_t)MSGID_REMOTE,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("LOCAL MSG ERROR");
		exit(EXIT_FAILURE);
	}
	printf("remote:msg init ok\n");
	remote_msgid=msgid;
	return;
}
*/
/*
int msg_recv(struct msg_remote*data)
W{	
	int ret;
	int length;
	length=2*sizeof(struct msg_remote);
	if((ret=msgrcv(remote_msgid,(void*)data,length,0,IPC_NOWAIT))==-1)
	{
		if(errno!=ENOMSG)	
		{
			DEBUG("LOCAL MSGRCV ERROR");
			exit(1);
		}
		return 0;
	}
	else
	{
//		printf("length=%d\n",data->length);
		return 1;
	}
}
*/


/***************************************************************************
Function: remote_recv

Description: the pthread function to deal with the data received from center 

Calls: get_line:order:recv_error

Called By: .main

Table Accessed: NULL

Table Updated: NULL

Input: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void * remote_recv(void*arg)
{
//	int  msg;
//	msg=msg_init(MSGID);
	printf("remote:secv pthread begin\n");
	char recvbuff[128];
	int rc;
    /* loop to wait for receving from the center */ 
	while(1)
	{
		rc=get_line(socketfd,recvbuff,sizeof(recvbuff));
//		printf("recvbuff=%s\n",recvbuff);
		if(rc==0)
		{
			DEBUG("remote close");
			exit(1);
		}
		else if(rc<0)
		{
			DEBUG("recv error");
			exit(1);
		}
        /* change profile 
        * not use now
        * */
		if(strncmp(recvbuff,"CHAG",4)==0)
		{
            printf("DATE: %s change profile\n",timerecord());
	//		device_change();		
		}
        /* vpn setting */
		else if(strncmp(recvbuff,"REON",4)==0)
		{
            printf("DATE: %s remote:VPN online begin!\n",timerecord());
			rc=plc_online();
		}
        /* not use now */
		else if(strncmp(recvbuff,"2200",4)==0)
		{
		//	data_response();
		}
        /* data tranmission error */ 
		else if(strncmp(recvbuff,"4200",4)==0)
		{
			while(get_line(socketfd,recvbuff,sizeof(recvbuff))>1);
			close(socketfd);
            printf("DATE: %s data abnormal\n",timerecord());
			exit(1);
		}
        /* heart beat packet */ 
		else if(strncmp(recvbuff,"NOOP",4)==0)
		{
			while(get_line(socketfd,recvbuff,sizeof(recvbuff))>1);
		}
        /* the ID out of order */ 
		else if(strncmp(recvbuff,"4100",4)==0)
		{
			order();
		}
        /* the default solution*/
		else
		{
			rc=recv_error();
		}
	}
}



/***************************************************************************
Function: order

Description: when the data sent go wrong, the function will be called to set the true number

Calls: get_line

Called By: remote_recv

Table Accessed: NULL

Table Updated: NULL

Input: NULL

Output: set the true number

Return: if success return 0

Others:
***************************************************************************/
int	order(void)
{
//	struct msg_remote response;
	int rc;
	char buff[128];
	char *p;
	while(get_line(socketfd,buff,sizeof(buff))>1)
	{
        /* get the new ID */
		if(strncmp(buff,"index",5)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			rc=atoi(p);
/*			if(rc==0)
			{
				response.mtype=2;
				strcpy(response.command,"3100");
				gettime(response.time);
				if((rc=msgsnd(msg,&response,sizeof(struct msg_remote),0))==-1)
				{
					perror("msgsnd error");
					close(socketfd);
					DEBUG("RESPONSE ERROR");
					exit(1);
				}
			}
			else
*/
            /* set the new ID */
			data.number=rc;
		}
	}
	return 0;
}



/***************************************************************************
Function: recv_error

Description: when recv a wrong command, call the function to response the command

Calls: get_line

Called By: remote_recv

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: UNUSED
***************************************************************************/
int recv_error()
{
//	struct msg_remote errdata;
//	errdata.mtype=1;
	char buff[50];
//	char time[100];
	char *p;
//	char err[4]="err";
	int number=0;
	while(get_line(socketfd,buff,50)>1)
	{
		if(strncmp(buff,"number",6)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			number=atoi(p);
//			errdata.number=number;
		}
	}
	if(number<=0)
		number=0;
    printf("DATE: %s receive a unknown command\n",timerecord());
//	gettime(time);
//	strcpy(errdata.time,time);
//	sprintf(errdata.command,"3300");
//	insert_command(errdata.number,time,err,err);
//	msg_send(&errdata);
}


/***************************************************************************
Function: plc_online

Description: when recv plc online command, call python function post enable or disable router vpn function

Calls: get_line,post

Called By: remote_recv

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
int plc_online()
{
	char*p;
	PyObject * pName, *pModule,*pDict,*pFunc;
	int rc;
	int value;
	char buff[32];
	while(1)
	{
		rc=get_line(socketfd,buff,32);
		if(rc>1)
		{
            /* judge  to begin or close vpn function 
            * value=1 to start the vpn Function
            * value=0 to close the vpn function
            * */
			if(strncmp(buff,"begin",5)==0)
			{
				p=strchr(buff,'=');
				p++;
				buff[strlen(buff)-1]='\0';
				value=atoi(p);
			//	printf("value=%d\n",value);
			}
		}
		else 
			break;
	}

    /* initialize the python */ 
	Py_Initialize();
	if(!Py_IsInitialized())
	{
		DEBUG("python init error");
		exit(1);
	}
    /* load the python module */
	pName=PyString_FromString("post");
	pModule=PyImport_Import(pName);
	if(!pModule)
	{
		DEBUG("IMPORT PY MODULE ERROR");
		exit(1);
	}
    /* start the python function */
	pDict=PyModule_GetDict(pModule);
	if(value==1)
	{
		pFunc=PyDict_GetItemString(pDict,"post_disable");
		PyObject_CallObject(pFunc,NULL);
	}
	else
	{
		pFunc=PyDict_GetItemString(pDict,"post_enable");
		PyObject_CallObject(pFunc,NULL);
	}
	Py_Finalize();
	return 1;
}



/*
int send_signal_usr2()
{
	int ret;
	int local_pid;
	int deal_pid;
	char *p;
	char line[100];
	FILE*fp;
	if((fp=fopen(PRO_CONF,"r"))==NULL)
	{
		printf("open process.config error\n");
		return 0;
	}
	while((fgets(line,sizeof(line),fp))!=NULL)
	{
		if(strncmp(line,"local_pid",9)==0)
		{
			p=strchr(line,'=');
			p++;
			line[strlen(line)-1]='\0';
			local_pid=atoi(p);
		}
		if(strncmp(line,"deal_pid",8)==0)
		{
			p=strchr(line,'=');
			p++;
			line[strlen(line)-1]='\0';
			deal_pid=atoi(p);
		}
	}
//	printf("local=%d,deal=%d\n",local_pid,deal_pid);
	ret=kill((pid_t)local_pid,SIGUSR2);
	if(ret!=0)
		return 0;
	ret=kill((pid_t)deal_pid,SIGUSR2);
	if(ret!=0)
		return 0;
	printf("remote:send signal ok:SIGUSR2\n");
	return 1;
}
*/


/***************************************************************************
Function: main

Description: to start up the thread function: remote_recv remote_send signal_wait

Calls: db_init conn remote_recv remote_send signal_wait

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void main()
{
//	static int device_state=0;
	pthread_mutex_init(&mut,NULL);
	int err;
    signal(SIGALRM,udp_test);
    set_timer();
    /* initialize the db connection  */
	db_init();
    /* set the signal shield */
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);
	sigaddset(&set,SIGUSR2);
	err=pthread_sigmask(SIG_BLOCK,&set,NULL);
    /* keep the pid in profile */
	write_pid_remote();
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	pthread_t tid4;
	pthread_attr_t attr;
	err=pthread_attr_init(&attr);
	if(err!=0)
	{
		DEBUG("ERROR");
		exit(1);
	}
    /* set the thread detech */
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    printf("DATE: %s remote:local begin\n",timerecord());
	err=pthread_create(&tid3,&attr,signal_wait,NULL);
	if(err!=0)
		exit(1);
//	err=pthread_create(&tid4,&attr,test,NULL);
//	if(err!=0)
//		exit(1);
    /* establish the tcp connection */
	socketfd=conn();
//	devicehaha_state=1;
	if(err==0)
	{
	//printf("\n\ndevice_state=%d\n",device_state);
		err=pthread_create(&tid1,&attr,remote_recv,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,remote_send,NULL);
		if(err!=0)
			exit(1);
//		err=pthread_create(&tid3,&attr,signal_wait,NULL);
//		if(err!=0)
//			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)0);
/*
	char buffer[512]="username=okc\n";
	change_file(buffer);
	return;
*/
}



/***************************************************************************
Function: conn

Description: before the recv/send thread begin, to build up the connection with remote center.

Calls: first_login login read_file_v1

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: socket descriptor

Others: NULL
***************************************************************************/
int conn()
{
	int init;
	int fd;
	char*p;
	char temp[32];
//	read_file_v1("login","init",temp);
//	init=atoi(temp);
//	printf("init=%d\n",init);
//	if(init==0)
//		first_login();
    /* login in the center */
	fd=login();
	if(fd<=0)
	{
		DEBUG("CONNECT ERROR");
		exit(1);
	}
    printf("DATE: %s remote:conn ok%d\n",timerecord(),fd);
	return fd;
}

/***************************************************************************
Function: first_login

Description: when the device is first to connect the remote center,this function will be called to carry on initial setting

Calls: get_line addoralrconfig

Called By: conn

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: if success return 0

Others: UNUSED
***************************************************************************/
/*
int first_login()
{
	char value[30];
	struct timeval timeo;
	timeo.tv_sec=5;
	timeo.tv_usec=0;
	int rc;
	int fd;
	int len,i=0,n=0;
	struct sockaddr_in address;
	int result;
	FILE *fp;
	char *p,ptr[4096];
	struct hostent *he;
	read_file("SERV_ADDR","servaddress",value);
	if(inet_addr(value)==INADDR_NONE)
	{
		he=gethostbyname(value);
		if(!he)
		{
			DEBUG("GETHOSTBYNAME ERROR");
			exit(1);
		}
		strcpy(value,he->h_addr_list[0]);
	}

//	printf("value=%s\n",value);

	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(value);
	address.sin_port = htons(13000);
	len = sizeof(address);
	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(struct timeval));
	result = connect(fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(fd, (struct sockaddr *)&address, len);
		sleep(++i*3);
	}
	if(result == -1) {
		DEBUG("CONNECT ERROR");
		exit(1);
	}
	printf("remote:local_first connection\n");
	snprintf(write_buf,4096,"RCON 002\r\nlength=0\r\n\r\n");
	write(fd,write_buf,strlen(write_buf));
//	printf("send ok**************\n%s\n",write_buf);
	n=get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
	if(n<=0)
	{
		perror("local_read error\n");
		exit(1);
	}
	if(strncmp(read_buf,"1010",4)!=0)//?
	{
		perror("local_RCON error");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	snprintf(write_buf,4096,"LOGI 002\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok**************\n%s\n",write_buf);
	//	write_buf="LOGI 002\r\n";
	fp=fopen(CONFIG,"r+");
	fgets(temp,4096,fp);
	fgets(temp,4096,fp);
	memcpy(write_buf,temp,4096); //write_buf=temp;
	fgets(temp,4096,fp);//!
	strcat(write_buf,temp);//!
	if(write_buf[strlen(write_buf)-1]=='\n')
		write_buf[strlen(write_buf)-1]='\0';
	strcat(write_buf,"\r\nlength=0\r\n\r\n");
	
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok**************\n%s\n",write_buf);
	n=get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
	if(strncmp(read_buf,"1020",4)!=0)
	{
		DEBUG("LOGIN ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	//write_buf="LGIN 002\r\n";
	snprintf(write_buf,4096,"LGIN 002\r\n");
	while(1)
	{
		fgets(temp,4096,fp);
		if(!feof(fp))
		{
			strcat(write_buf,temp);
		}
		else
			break;
	}
	fclose(fp);
	strcat(write_buf,"\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok***************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok***************\n%s\n",read_buf);
	if(strncmp(read_buf,"2010 002",8)!=0)
	{
		DEBUG("LOCAL LOGIN ERROR");
		exit(1);
	}
	addoraltconfig("config","init","init=1");
	get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
//	change_file(read_buf);
	while(get_line(fd,temp,4096)>1)
	{
//		change_file(temp);
//		printf("recv ok***************\n%s\n",temp);
	}
	//file_buf="1\r\n";
	
	strcat(file_buf,read_buf);
	for(p=file_buf,i=0;*p!='\0';p++)
		if(*p!='\r')
			ptr[i++]=*p;

	ptr[i-1]='\0';
	printf("%s\n",ptr);
	fp=fopen("config","w");
	fputs(ptr,fp);
	fclose(fp);
	//write_buf="QUIT 002\r\n";
	snprintf(write_buf,4096,"QUIT 002\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok******************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok******************\n%s\n",read_buf);
	if(strncmp(read_buf,"2000 002",8)!=0)
	{	
		DEBUG("LOCAL QUIT ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	close(fd);
	return 0;
}

*/

/***************************************************************************
Function: login

Description: every time the device connect to the remote center,this function will be called to get essential infomation

Calls: dbrecord_v2 read_file_v1 base64_encode_v2 addoraltconfig

Called By: conn

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: the socket descriptor

Others: USING
***************************************************************************/
int login()
{
        struct capacity cap;
	int datalocation=0;
	int dataoffset=0;
	int datalength=0;
	char value[30];
	int flag=0;
	struct timeval timeo;
	int rc;
	int i=0;
	int len,length,result;	
	int fd;
	struct sockaddr_in address;
	FILE *fp;
	char sn[32],enpyt[64];
        unsigned char  enpyt_temp[16];
	char *p,*ptr;
        char key[5];
	timeo.tv_sec=5;
	timeo.tv_usec=0;
	struct hostent *he;
	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	if((fd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		perror("socket error:");
		exit(1);
	}
	address.sin_family = AF_INET;
    /* get server IP address or domain name */
	read_file("SERV_ADDR","servaddress",value);
    /* translate the domain name to IP */
	if(inet_addr(value)==INADDR_NONE)
	{
		he=gethostbyname(value);
		if(!he)
		{
			close(fd);
			DEBUG("GETHOSTBYNAME ERROR");
			exit(1);
		}
		strcpy(value,he->h_addr_list[0]);
	}
//	printf("value=%s\n",value);
	address.sin_addr.s_addr = inet_addr(value);
	address.sin_port = htons(REMOTE_PORT);
	len = sizeof(address);
/*
	int keepAlive=1;
	int keepIdle=5;
	int keepInterval=3;
	int keepCount=2;

	if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive))<0)
	{
		close(fd);
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(fd,SOL_TCP,TCP_KEEPIDLE,(void*)&keepIdle,sizeof(keepIdle))<0)
	{
		close(fd);
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,(void*)&keepInterval,sizeof(keepInterval))<0)
	{
		close(fd);
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(fd,SOL_TCP,TCP_KEEPCNT,(void*)&keepCount,sizeof(keepCount))<0)
	{
		close(fd);
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
*/

//	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(struct timeval));
	result = connect(fd, (struct sockaddr *)&address, len);
    /* try to connect to the center */
	while(1)
	{
		if(result!=-1)
		{
			break;
		}
		while(i<=20)
		{
			printf("    try: %d times\n",i);
			result = connect(fd, (struct sockaddr *)&address, len);
			if(result!=-1)
				break;
			sleep(++i);
		}
        /* if exceed the limit, keep it in db */
		if(result==-1)
		{
			flag=1;
			DEBUG("CONNECT ERROR");
			dbrecord_v2("NETWORK ABNORMAL");
	//		addoraltconfig(DEV_CONF,"setting","setting=2");	
	//		send_signal(2);
		}
	//	exit(1);//fuck
		while((i>20)&&(result==-1))
		{
			result=connect(fd,(struct sockaddr*)&address,len);
			sleep(i*3);
		}
	}
	if(result!=-1)
	{
//		dbrecord_v2("NETWORK NORMAL",db);
//		read_file("setting","setting",value);
//		result=atoi(value);
//		if(result!=0)
//		{
		if(flag==1)
		{
			dbrecord_v2("NETWORK NORMAL");
		}

	//		addoraltconfig(DEV_CONF,"setting","setting=0");
	//		send_signal(2);
//		}
	}
	else
	{
		DEBUG("CONNECT ERROR\n");
		exit(1);
	}

//	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(struct timeval));
//	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(struct timeval));
//	int keepalive=1;
//	int keepidle=2;
//	int keepinternal=5;
//	int keepcount=3;
//	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepalive,sizeof(keepalive));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPIDLE,(void *)&keepidle,sizeof(keepidle));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPINTVL,(void *)&keepinternal,sizeof(keepinternal));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPCNT,(void *)&keepcount,sizeof(keepcount));
//	printf("remote:local CONNECTION!xx\n");
	struct timeval tv;
	tv.tv_sec=5;
	tv.tv_usec=0;
	if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		DEBUG("set sendtimeout error");
		perror("timeout");
		close(fd);
		exit(1);
	}
	snprintf(write_buf,4096,"RCON 002\r\n");
    read_file_v1("login","username",value);
    snprintf(temp,4086,"username=%s\r\n\r\n",value);
    strcat(write_buf,temp);
	printf("write_buf=%s",write_buf);
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		close(fd);
		DEBUG("WRITE ERROR");
		exit(1);
	}
	get_line(fd,read_buf,4096);
	if(strncmp(read_buf,"1010",4)!=0)
	{
		close(fd);
		DEBUG("LOCAL RCON ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1)
    {
        printf("%s",read_buf);
        if(strncmp(read_buf,"salt",4)==0)
        {
            p=strchr(read_buf,'=');
            p++;
            strcpy(key,p);
        }   
    }
	snprintf(write_buf,4096,"LOGI 002\r\n");
    read_file_v1("device_info","serialno",sn);
    MD5_CTX md5;
    MD5Init(&md5);

    printf("key=%s\n",key);
    printf("0=%s\n",sn);
    strcpy(enpyt,sn);
    printf("1=%s\n",enpyt);
    strcpy(enpyt+1,key);
    printf("2=%s\n",enpyt);
    strcpy(enpyt+6,sn+1);
    printf("3=%s\n",enpyt);
    MD5Update(&md5,enpyt,strlen(enpyt));
    memset(enpyt,'\0',sizeof(enpyt));
    MD5Final(&md5,enpyt_temp);
    for(i=0;i<16;i++)
    {
        sprintf((enpyt+2*i),"%02x",enpyt_temp[i]);
    }
    cap=get_system_tf_free();
    snprintf(temp,4096,"username=%s\r\nsn=%s\r\nfree_sp=%d\r\ntotal_sp=%d\r\n\r\n",value,enpyt,cap.free_cap,cap.total_cap);
    strcat(write_buf,temp);
    printf("%s",write_buf);
    



//    read_file_v1("login","username",name);
//	read_file_v1("login","password",password);
    /* nosense */
//	base64_encode_v2(password,encrypt,strlen(password));
//	sprintf(temp,"username=%s\r\npassword=%s\r\nlength=0\r\n\r\n",name,encrypt);
//	strcpy(username,name);
//	strcat(write_buf,temp);

	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		close(fd);
		DEBUG("WRITE ERROR");
		exit(1);
	}

	get_line(fd,read_buf,4096);

	if(strncmp(read_buf,"2000 002",8)!=0)
	{
		close(fd);
		DEBUG("LOCAL LOGI ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1)
	{
    /*
		if(strncmp(read_buf,"length",6)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			len=atoi(p);
		}
    */
        /* normal packet ID */
		if(strncmp(read_buf,"index",5)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			number=atoi(p);	
			if(number==0)
				number=1;
		}
        /* alarm packet ID */
		if(strncmp(read_buf,"alarm_index",11)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			alarm_number=atoi(p);
			if(alarm_number==0)
				alarm_number=1;
		}
        /* facility power-off  */
		if(strncmp(read_buf,"datalocation",12)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			datalocation=atoi(p);
		}
		if(strncmp(read_buf,"dataoffset",10)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			dataoffset=atoi(p);
		}
		if(strncmp(read_buf,"datalength",10)==0)
		{
			p=strchr(read_buf,'=');
			p++;
			read_buf[strlen(read_buf)-1]='\0';
			datalength=atoi(p);
		}
	}
    /* keep the power-off in profile */
	if(datalength)
	{
		addoraltconfig(DEV_CONF,"running","running=1");
		sprintf(temp,"datalength=%d",datalength);
		addoraltconfig(DEV_CONF,"datalength",temp);
		sprintf(temp,"dataoffset=%d",dataoffset);
		addoraltconfig(DEV_CONF,"dataoffset",temp);
		sprintf(temp,"datalocation=%d",datalocation);
		addoraltconfig(DEV_CONF,"datalocation",temp);
	}
	else
	{
		addoraltconfig(DEV_CONF,"running","running=0");
	}
    printf("DATE: %s remote:local connection ok\n",timerecord());
	return fd;
}



/***************************************************************************
Function: get_line

Description: recv a line ending with "\r\n" or "\n" from the tcp buffer 

Calls: NULL 

Called By: remote_send remote_recv

Table Accessed: NULL

Table Updated: NULL

INput: 
	sock: socket descriptor 
	buf: the buffer to store the line
	size: the max length of the line

Output: 
	buf:the received line

Return: return the length received in reality

Others: NULL
***************************************************************************/
int get_line(int sock,char*buf,int size)
{
	int m;
	int i=0;
	char c='\0';
	int n;
	while((i<size-1)&&(c!='\n'))
	{
        /* read  character one by one */ 
		n=recv(sock,&c,1,0);
		while(1)
		{
			if(n>0)
			{
				if(c=='\r')
				{
                    /* judge the next character,if this one is '\n' get it */
					n=recv(sock,&c,1,MSG_PEEK);
					if((n>0)&&(c=='\n'))
					{
						m=recv(sock,&c,1,0);
						if(m<0)
						{
							close(sock);
							DEBUG("ERROR");
							exit(1);
						}
					}
					else if(n==0)
						c='\n';
					else
					{
						perror("error");
						DEBUG("ERROR");
						close(sock);
						exit(1);
					}
				}
				buf[i]=c;
				i++;
				break;
			}
			else if(n==0)
			{
				c='\n';
				break;
			}
			else
			{
				if(errno==11)
					break;
				close(sock);
				perror("error");
				DEBUG("ERROR");
				exit(1);
			}
		}
	}
	buf[i]='\0';
	return(i);
}
/*
int change_file_single_value(char* buf)
{
	char linebuffer[512]={0};
	char buffer1[512]={0};
	char buffer2[512]={0};
	int line_len=0;
	int len=0,length=0;
	int res;
	char*p;
	p=strchr(buf,'=');
	length=p-buf;
	FILE *fp=fopen(CONFIG,"r+");
	if(fp==NULL)
	{
		DEBUG("LOCAL OPEN ERROR");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		line_len=strlen(linebuffer);
		len+=line_len;
		sscanf(linebuffer,"%[^=]=%[^=]",buffer1,buffer2);
		if(!strncmp(buf,buffer1,length))
		{
			len-=strlen(linebuffer);
			res=fseek(fp,len,SEEK_SET);
			if(res<0)
			{
				DEBUG("LOCAL FSEEK ERROR");
				exit(1);
			}
			memcpy(buffer1,buf,strlen(buf));
			fprintf(fp,"%s",buffer1);
			fclose(fp);
			return;
		}
	}
	return 0;
}
*/
/*
int change_file(char *buf)
{
	char linebuffer[512]={0};
	char buffer1[512]={0};
	char buffer2[512]={0};
	int line_len=0;
	int len=0,length=0;
	int res;
	char*p;
	p=strchr(buf,'=');
	length=p-buf;
	FILE *newfp=fopen("config.new","a");
	FILE *fp=fopen(CONFIG,"r+");
	if(fp==NULL)
	{
		printf("local_open error\n");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		if(strncmp(linebuffer,buf,length)!=0)
		{
			fputs(linebuffer,newfp);
		}
		else
			fputs(buf,newfp);		
	}
	fclose(fp);
	fclose(newfp);
	remove(CONFIG);
	rename("config.new",CONFIG);
	return 0;
}
*/
/*	
int change_config(char *parameter,char* value)
{
	char linebuffer[512]={0};
	char newline[100];
	int len=0,length=0;
	char*p;
	length=strlen(parameter);
	sprintf(newline,"%s=%c\n",parameter,*value);
	printf("newline=%s\n",newline);
	FILE *newfp=fopen("device.config.new","a");
	FILE *fp=fopen(DEV_CONF,"r+");
	if(fp==NULL)
	{
		printf("remote_open error\n");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		if(strncmp(linebuffer,parameter,length)!=0)
		{
			fputs(linebuffer,newfp);
		}
		else
			fputs(newline,newfp);		
	}
	fclose(fp);
	fclose(newfp);
	remove(DEV_CONF);
	rename("device.config.new",DEV_CONF);
	return 0;
}

*/
/*
int send_fork()
	{
		pid_t pid;
		pid=fork();
		if(pid<0)
	{
		perror("local fork error\n");
		exit(1);
	}
	else if(pid==0)
	{
		struct itimerval value,ovalue;
		signal(SIGALRM,handler);
		value.it_value.tv_sec=5;
		value.it_value.tv_usec=0;
		value.it_interval.tv_sec=5;
		value.it_interval.tv_usec=0;
		setitimer(ITIMER_REAL,&value,&ovalue);

		while(1)
		{
		//	send_data();
			fflush(stdout);
		}
	}
	close(socketfd);
	return pid;
}
*/
/*void handler()
{
	signal(SIGALRM,handler);
	send_data();
	return;
}
*/
/*
int send_level1(struct msg_remote data)
{
	int rc;
	char sendbuf[512];
	sprintf(sendbuf,"%s 002\r\nlength=0\r\nnumber=%d\r\n\r\n",data.command,data.number);
	rc=write(socketfd,sendbuf,strlen(sendbuf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	printf("remote:send response number=%d\n",data.number);
	return 1;
}
*/
/*
int send_level2(struct msg_remote data)
{
	char sendbuf[512];
	sprintf(sendbuf,"2130 002\r\nlength=%d\r\nnumber=%d\r\n\r\n",data.length,commandnumber);
	write(socketfd,sendbuf,strlen(sendbuf));
	write(socketfd,data.data,data.length);
	return 1;
}
*/
/*
int send_level3(struct msg_remote data)
{
	int rc;
	char sendbuf[2048];
	sprintf(sendbuf,"DATA 002\r\nusername=%s\r\ndate=%s\r\nlength=%d\r\nnumber=%d\r\ndatatype=plc\r\ndatalevel=1\r\n\r\n",username,data.time,data.length,data.number);
//	printf("%s\r\n",sendbuf);
	rc=write(socketfd,sendbuf,strlen(sendbuf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	rc=write(socketfd,data.data,data.length);
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	printf("remote:send a first data number=%d\n",data.number);
//	datalevel1=data.number;
//	pthread_mutex_lock(&mutex);
//	while(!confirm)
//		pthread_cond_wait(&cond,&mutex);
//	confirm=0;
//	pthread_mutex_unlock(&mutex);
	return 1;
}
*/


/***************************************************************************
Function:msg_init

Description: to establish the message pipe

Calls: NULL

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput: the ID of the message pipe

Output: the msg descriptor

Return: return the msg descriptor

Others: UNUSED
***************************************************************************/
/*
int msg_init(int id)
{
	int msgid;
	msgid=msgget((key_t)id,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("REMOTE MSG ERROR");
		exit(1);
	}
	printf("remote:msg init ok\n");
	return msgid;
}
*/


/***************************************************************************
Function: remote_send

Description: the thread function to send the data and response

Calls: get_time query query_alarm

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void *remote_send(void *arg)
{
	unsigned char rawbuf[2048];
    int currentid;
//	static int device_state;
	char time[32];
	int msgid;
//	struct remote_data data;
	char sendbuf[1024];
//	struct  msg_local alarm_data;
	int rc;
	int init=1;
	unsigned char c;
	int flag=0;
//	msgid=msg_init(MSGID);
    printf("DATE: %s remote:send pthread begin\n",timerecord());
    printf("DATE: %s send:device_state=%d\n",timerecord(),device_state);
	int length;
	data.number=number;
	alarm_data.number=alarm_number;
    currentid=query_maxid();

    printf("DATE: %s number=%lld alarm_number=%lld\n",timerecord(),data.number,alarm_data.number);
    /* loop to send data */
	while(1)
	{
        /* judge the plc connection state */
		if(plc_connect_err)
		{
			gettime(time);
			sprintf(sendbuf,"RUNS 002\r\nstate=PLC CONNECT ERROR\r\n\r\n");
			rc=write(socketfd,sendbuf,strlen(sendbuf));
			if(rc<=0)
			{
				DEBUG("WRITE ERROR");
				close(socketfd);
				exit(1);
			}
			plc_connect_err=0;
		}
//		printf("device_state=%d\n",devicehaha_state);
        /* deal with facility running state */
		if(device_state==1|device_state==2)
		{
			gettime(time);
			if(device_state==1)
			{
				sprintf(sendbuf,"RUNS 002\r\nstate=DEVICE ON\r\n\r\n");
			}
			else if(device_state==2)
			{
				sprintf(sendbuf,"RUNS 002\r\nstate=DEVICE OFF\r\n\r\n");
			}
			rc=write(socketfd,sendbuf,strlen(sendbuf));
			if(rc<=0)
			{
				DEBUG("WRITE ERROR");
				close(socketfd);
				exit(1);
			}
			device_state=0;
		}
//		pthread_mutex_unlock(&mut);
		
        /* get normal data from db  */
		rc=query(&data);
		if(rc<0)
		{
			close(socketfd);
			DEBUG("QUERY ERROR\n");
			exit(1);
		}
		else if(rc>0)
		{
			sprintf(sendbuf,"DATA 002\r\nusername=%s\r\ndate=%s\r\nlength=%d\r\nnumber=%lld\r\n\r\n",username,data.time,data.length,data.number);
			memcpy(rawbuf,sendbuf,strlen(sendbuf));
			memcpy(rawbuf+strlen(sendbuf),data.data,data.length);
			rc=write(socketfd,rawbuf,strlen(sendbuf)+data.length);
			if(rc<=0)
			{
				close(socketfd);
				DEBUG("WRITE ERROR");
				exit(1);
			}
	//		printf("remote:send a second data number=%lld,length=%d\n",data.number,rc);

            /*when the id in device can not match the id in monitor center
            * then speed up to send the data in order to catch up with the newest data 
            * the current speed is 10x:
            * */
            if (currentid>(data.number+10))
            {
                data.number+=10;
            }
            else
			    data.number++;
		}
//		confirm(data.number++);
/*		
		if(msg_recv(&alarm_data,msgid))
		{
//			printf("haha\n");
			gettime(time);
			sprintf(sendbuf,"DATA 002\r\nalarm=True\r\nlength=%d\r\ndate=%s\r\n\r\n",alarm_data.length,time);
			rc=write(socketfd,sendbuf,strlen(sendbuf));
			if(rc<=0)
			{
				close(socketfd);
				DEBUG("WRITE ERROR");
				exit(1);
			}
			rc=write(socketfd,alarm_data.data,alarm_data.length);
			if(rc<=0)
			{
				close(socketfd);
				DEBUG("WRITE ERROR");
				exit(1);
			}
w			printf("remote:send a alarm data length=%d\n",rc);
		}

*/		
        /* get the alarm data from db */
		rc=query_alarm(&alarm_data);
//		printf("number=%lld\n",alarm_data.number);
		if(rc<0)
		{
			close(socketfd);
			DEBUG("QUERY ERROR\n");
			exit(1);
		}
		else if(rc>0)
		{
			sprintf(sendbuf,"DATA 002\r\nalarm=True\r\nusername=%s\r\ndate=%s\r\nlength=%d\r\nnumber=%lld\r\n\r\n",username,alarm_data.time,alarm_data.length,alarm_data.number);
			memcpy(rawbuf,sendbuf,strlen(sendbuf));
			memcpy(rawbuf+strlen(sendbuf),alarm_data.data,alarm_data.length);
			rc=write(socketfd,rawbuf,strlen(sendbuf)+alarm_data.length);
			if(rc<=0)
			{
				close(socketfd);
				DEBUG("WRITE ERROR");
				exit(1);
			}
	//		printf("remote:send a alarm data number=%lld,length=%d\n",alarm_data.number,rc);
			alarm_data.number++;
		}
		usleep(20000);
	}	
}

//	printf("n_s=%c,value1=%s,e_w=%c,value2=%s\n",gpsdata.n_s,gpsdata.value1,gpsdata.e_w,gpsdata.value2);
	
//	sprintf(da,"%c,%s,%c,%s",gpsdata.n_s,gpsdata.value1,gpsdata.e_w,gpsdata.value2);
//	printf("da=%s\n",da);
/*	i=strlen(da);
	strcpy(data_buf,"DATA 002\r\n");
	strcat(data_buf,"datatype=gps\r\n");
	sprintf(temp,"length=%d\r\n",i);
	strcat(data_buf,temp);
	memset(temp,'\0',4096);
	gettime(time_temp);
	sprintf(time,"date=%s\r\n",time_temp);
	sprintf(name,"username=%s\r\n",username);
	strcat(data_buf,name);
	strcat(data_buf,time);
	count=INC(count);
	snprintf(temp,4096,"number=%d\r\n",count);
	strcat(data_buf,temp);
	strcat(data_buf,"\r\n");
	printf("local send:\n");
	printf("%s",data_buf);
	write(socketfd,data_buf,strlen(data_buf));
	write(socketfd,da,strlen(da));
//	printf("send data ok*************\n");
	printf("%s\n",da);
	printf("local send over!\n");
	return;
	*/



/*
void dbrecord(char* state)
{
	char time[100];
	gettime(time);
	int rc;
	char* errmsg=0;
	sqlite3_stmt *stmt=NULL;
	const char* sql="insert into device_running_statement values(?,?)";
	if(sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_text(stmt,1,time,strlen(time),SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt,2,state,strlen(state),SQLITE_TRANSIENT);
	while(1)
	{
		if(rc=sqlite3_step(stmt)!=SQLITE_DONE)
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
//	sqlite3_close(db);
	return;
}
*/
/*int main()
{	
    FILE *file;
	char buf[4096];
	file=fopen("config","r");
	fgets(buf,4096,file);//get the first line of config
	fclose(file);
	if(buf[5]=='0')
	{
		printf("initialization start!");
		authenticate();
		printf("initialization OK!");
	}
	login_in();
	exit(0);
}

int authenticate()
{
	
	int fd;
	int len,i=0;
	struct sockaddr_in address;
	int result;
	FILE *fp;
	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	result = connect(fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}
	printf("CONNECTION OK!\n");
	snprintf(write_buf,4096,"RCON");
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"1010",4)!=0)//?
	{
		perror("authenticate error");
		exit(1);
	}
    printf("get answer for the RCON\n");
	write_buf="LOGI oo2\r\n";
	fp=fopen("config","r+");
	fgets(temp,4096,fp);
	strcat(write_buf,temp);
	fgets(temp,4096,fp);
	strcat(write_buf,temp);
	strcat(write_buf,"\r\n");
	printf("%s\n",write_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"1020",4)==0)
	{
		
	}

*	fgets(write_buf,4096,fp);//get the first line of config
	fgets(write_buf,4096,fp);//get the second line of config
	char *p;
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
			//strcat(file_buf,4096,write_buf+1);
			//strchr(file_buf,':') rreturn the first pointer
	}//get the user name

	snprintf(write_buf,4096,"USER %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"331",3)!=0)
	{
		perror("error");
		exit(1);
	}
  //get third line of config
   fgets(write_buf,4096,fp);
   p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
 
//send passwd	
	snprintf(write_buf,4096,"PASS %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"212",3)!=0)
	{
		perror("error");
		exit(1);
	}
	//get the fourth line of fp 
	fgets(write_buf,4096,fp);
    p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
	printf("%s\n",file_buf);
	// receive new name and passwd
	snprintf(write_buf,4096,"CONF %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
    //
	i=read(fd,read_buf,4096);
	printf("%d\n",i);
	printf("%s\n",read_buf);
	char *p1,name[4096],passwd[4096];/name no memory
	//p1=read_buf;
    if(strncmp(read_buf,"231",3)==0)
	{
		printf("debug!1\n");
		p1=strchr(read_buf,' ');
		printf("%s\n",p1);
		for(i=0;*(++p1)!=' ';)
		{
			name[i]=*p1;
			i++;
		}
		name[i]='\0';
		printf("%s\n",name);
		for(i=0;*(++p1)!='\0';)// '/n' cause bug
		{
			passwd[i++]=*p1;
		}
		passwd[i]='\0';
	
	//sscanf(read_buf,"231 %s %s",name,passwd);
		printf("%s\n",passwd);
	}
	//write to config file
    p=name;p1=passwd;
	fseek(fp,0,SEEK_SET);//rewind(fp);
	printf("debug2!\n");
	fclose(fp);
	fp=fopen("config","w");
	fprintf(fp,"init:1\nusername:%s\npasswd:%s\nserial:0001",p,p1);
	//quit
	snprintf(write_buf,4096,"QUIT\r\n");
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"221",3)!=0)
	{
		perror("error");
		exit(1);
	}
	fclose(fp);
	close(fd);
}

int login_in()
{ 
	int ctr_fd,data_fd;
	int len,i=0;
	struct sockaddr_in address;
	int result,result1;
	FILE *fp;
	char *p;
	char write_buf[4096],read_buf[4096],file_buf[4096];
	ctr_fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	result = connect(ctr_fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{

		if(result!=-1)
			break;
		result = connect(ctr_fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1\n");
		exit(1);
	}
	printf("CONNECTION OK!\n");
	//
	snprintf(write_buf,4096,"RCON");
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("%s\n",write_buf);
	read(ctr_fd,read_buf,4096);
	printf("RCON REPLY:%s\n",read_buf);
	if(strncmp(read_buf,"211",3)!=0)//?
	{
		perror("login error\n");
		exit(1);
	}
    fp=fopen("config","r+");
    fgets(write_buf,4096,fp);//get the first line of config
    fgets(write_buf,4096,fp);//get the second line of config
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
			//strcat(file_buf,4096,write_buf+1);
			//strchr(file_buf,':') rreturn the first pointer
	}
	}
	snprintf(write_buf,4096,"USER %s\r\t",file_buf);
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("send username:%s\n",write_buf);
	read(ctr_fd,read_buf,4096);
	printf("Send user reply:%s\n",read_buf);
	if(strncmp(read_buf,"331",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
 
    fgets(write_buf,4096,fp);//get the third line of config
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
	snprintf(write_buf,4096,"PASS %s\r\n",file_buf);
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("send pass name");
	read(ctr_fd,read_buf,4096);
	printf("passwd reply:");
	if(strncmp(read_buf,"230",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
	printf("login in\n");
	snprintf(write_buf,4096,"TRAN");
	write(ctr_fd,write_buf,strlen(write_buf));
	read(ctr_fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"125",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
	printf("data tran\n");
	char newaddr[4096];
	char p0[6],p1[6];
    for(i=0;i<strlen(read_buf);i++)
	{
		read_buf[i]=read_buf[i+9];
		
	
	}
	read_buf[i]='\0';//read_buf[i-1]='\0';change \n to \0
	int j=0,m=0;
	char arr[6][4096];
	for(i=0;i<strlen(read_buf);i++)
	{
		if(read_buf[i]==',')
			{
				j++;m=0;
			}
		else
		{
			arr[j][m++]=read_buf[i];
		}
	}
	printf("%s\n",read_buf);
	for(i=0;i<3;i++)
	{
	strcat(newaddr,arr[i]);
	strcat(newaddr,".");
	}
	strcat(newaddr,arr[3]);
	printf("%s",newaddr);
		//	sscanf(read_buf,"%s,%s,%s,%s,%s,%s",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
	//printf("%s%s%s%s%s%s\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
	// USE DATA PORT 
	sprintf(newaddr,"%s.%s.%s.%s\n",arr[0],arr[1],arr[2],arr[3]);//?
	printf("%s\n",newaddr);
	data_fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	int port;
	port=256*atoi(arr[4])+atoi(arr[5]);
        printf("%d\n",port);
	address.sin_addr.s_addr = inet_addr(newaddr);
	address.sin_port = htons(port);
	len = sizeof(address);
    result1=connect(data_fd, (struct sockaddr *)&address, len);
	int local_fd;
	local_fd=local_comm();
	printf("local_fd:%d\n",local_fd);
	while(i<=20)
	{
		if(result1!=-1)
			break;
		result1 = connect(data_fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result1 == -1) {
		perror("oops: client1\n");
		exit(1);
	}
	while(1)
	{
		read(local_fd,write_buf,21);
		sleep(5);
//	sprintf(write_buf,"voltage:%dcurrent:%d",random()%450,random()%100);
		printf("snddata:%s\n",write_buf);
		write(data_fd,write_buf,strlen(write_buf));
	}
}
int local_comm()
{

	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	client_address.sin_family = AF_INET;
	client_address.sin_addr.s_addr = inet_addr(PLC_ADDRESS);
	client_address.sin_port =htons(13000);
	client_len = sizeof(client_address);
	result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}
	printf("LOCAL_COMM CONNECTION OK!\n");
	return client_sockfd;
}
*/
