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
		err=sigwait(&sigset,&signo);
		if(err!=0)
		{
			DEBUG("SIGNAL ERROR:%d",err);
			exit(1);
		}
        /* deal with plc connection problem */
		if(signo==SIGUSR1)
		{
			plc_connect_err=plc_connect_err?0:1;
			DEBUG("remote:plc_connect_err=%d\n",plc_connect_err);
		}
        /* the facility running problem */
		if(signo==SIGUSR2)
		{
			read_file("signal","setting",value);
			pthread_mutex_lock(&mut);
			device_state=atoi(value);
			if(device_state==2)
				printf("device off\n");
			else if(device_state==1)
				printf("device on\n");
			else
				device_state=0;
			pthread_mutex_unlock(&mut);
				
		}
        printf("DATE: %s signal:device_state=%d\n",timerecord(),device_state);
	}
}



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
	return;
}


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
	printf("remote:secv pthread begin\n");
	char recvbuff[128];
	int rc;
    /* loop to wait for receving from the center */ 
	while(1)
	{
		rc=get_line(socketfd,recvbuff,sizeof(recvbuff));
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
	char buff[50];
	char *p;
	int number=0;
	while(get_line(socketfd,buff,50)>1)
	{
		if(strncmp(buff,"number",6)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			number=atoi(p);
		}
	}
	if(number<=0)
		number=0;
    printf("DATE: %s receive a unknown command\n",timerecord());
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
    /* establish the tcp connection */
	socketfd=conn();
	if(err==0)
	{
		err=pthread_create(&tid1,&attr,remote_recv,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,remote_send,NULL);
		if(err!=0)
			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)0);
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
	address.sin_addr.s_addr = inet_addr(value);
	address.sin_port = htons(REMOTE_PORT);
	len = sizeof(address);
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
		}
		while((i>20)&&(result==-1))
		{
			result=connect(fd,(struct sockaddr*)&address,len);
			sleep(i*3);
		}
	}
	if(result!=-1)
	{
		if(flag==1)
		{
			dbrecord_v2("NETWORK NORMAL");
		}
	}
	else
	{
		DEBUG("CONNECT ERROR\n");
		exit(1);
	}

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

    strcpy(enpyt,sn);
    strcpy(enpyt+1,key);
    strcpy(enpyt+6,sn+1);
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
	char time[32];
	int msgid;
	char sendbuf[1024];
	int rc;
	int init=1;
	unsigned char c;
	int flag=0;
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
        /* get the alarm data from db */
		rc=query_alarm(&alarm_data);
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
			alarm_data.number++;
		}
		usleep(20000);
	}	
}



