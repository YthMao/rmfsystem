/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem_12_19\comtest_pipe.c
#       @date         :2015-12-23 17:19
#       @algorithm    :
==========================================================================*/
#include "comtest_pipe.h"
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <memory.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "rmfsystem.h"
#include "read_file.h"
#include "change_profile.h"
#include "plc_simulate.h"
#include "data_deal.h"
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>

pthread_mutex_t mutex;
/* facility running state
 * device_state=0 initial value
 * device_state=1 shutdown state
 * device_state=2 ON
 */
int device_switch=0;
/**/
static struct itimerval oldtv;
/* not use */
int netflag=0;
/* not use */
int plc_state=0;
/* not use */
int count2=0;
/* PLC configure struct pointer */
struct plc_struct * plc_head=NULL;

/* runningdetermine whether shutdown value is used
 * datalength datalocation dataoffset mean the location of the value
 */
int datalength;
int running;
int datalocation;
int dataoffset;



/***************************************************************************
Function: write_local_pid

Description: to keep the local process ID in record

Calls: NULL

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void write_pid_local()
{
	char line[50];
	int pid;
	pid=getpid();
	sprintf(line,"local_pid=%d",pid);
	addoraltconfig(PRO_CONF,"local_pid",line);
	printf("DATA: %s local:write process_config ok\n",timerecord());
	return;
}

/*
static void sig_cld(int signo)
{
	pid_t pid;
	int status;
	printf("SIGCLD received\n");
	if(signal(SIGCLD,sig_cld)==SIG_ERR)
	{
		perror("signal err");
		exit(1);
	}
	if((pid=wait(&status))<0)
		perror("wait error");
	printf("pid=%d\n",pid);
}
*/


/***************************************************************************
Function: signal_handler

Description: signal register function

Calls: db_base_maintenence

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: 
	m: the signal

Output: NULL

Return: NULL

Others: UNUSED
***************************************************************************/
/*
void signal_handler(int m)
{
	db_base_maintenance();	
}
*/

/***************************************************************************
Function: tcp_connect

Description: to establish the connection with the remote center

Calls: db_record_v2 send_signal

Called By: second_level_recv

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: return socket descriptor

Others: NULL
***************************************************************************/

int tcp_connect()
{
	int client_sockfd;
	char value[20];
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	read_file("PLC_INFO","plcaddress",value);
	if((client_sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket error:");
		exit(1);
	}
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr(value);
	client_address.sin_port=htons(TCP_PORT);
	client_len=sizeof(client_address);
	printf("DATE: %s value=%s port=%d\n",timerecord(),value,TCP_PORT);
/*
	int keepAlive=1;
	int keepIdle=5;
	int keepInterval=3;
	int keepCount=2;
	if(setsockopt(client_sockfd,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive))<0)
	{
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(client_sockfd,SOL_TCP,TCP_KEEPIDLE,(void*)&keepIdle,sizeof(keepIdle))<0)
	{
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(client_sockfd,SOL_TCP,TCP_KEEPINTVL,(void*)&keepInterval,sizeof(keepInterval))<0)
	{
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
	if(setsockopt(client_sockfd,SOL_TCP,TCP_KEEPCNT,(void*)&keepCount,sizeof(keepCount))<0)
	{
		DEBUG("SET KEEP ERROR:%d",errno);
		exit(1);
	}
*/
	do
	{
		result=connect(client_sockfd,(struct sockaddr*)&client_address,client_len);
		if(result!=-1)
			break;
		printf("    plc conn try:%d times\n",i);
		sleep(i*3);
		i++;
	}while(i<=20);
	if(result==-1)
	{
		DEBUG("CONNECT ERROR");
		dbrecord_v2("PLC CONNECT ABNORMAL");
	//	addoraltconfig(DEV_CONF,"connect","connect=1");
		send_signal(1);
	}
	while(i>20)
	{
		if(result==0)
			break;
		result=connect(client_sockfd,(struct sockaddr*)&client_address,client_len);
	//	if(result!=-1)
	//		break;
		sleep(i*3);
	}

//	if(result==-1)
//	{
//		DEBUG("connect error");
//		exit(1);
//	}
    /* set timeout for tcp receiving
     * may not be useful
     */
	struct timeval tv;
	tv.tv_sec=5;
	tv.tv_usec=0;
	if(setsockopt(client_sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		DEBUG("set sendtimeout error");
		perror("timeout");
		close(client_sockfd);
		exit(1);
	}
	printf("DATE: %s local:connect PLC OK!\n",timerecord());
	return client_sockfd;
}



/***************************************************************************
Function: read_plcfile

Description: read the parameter from the profile include PLC data infomation device state infomation. 

Calls: read_file

Called By: second_level_recv

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: global variables

Return: if success return 1

Others: NULL
***************************************************************************/
int read_plcfile()
{
	char value[32];
	int i;
//	struct plc_struct *q=NULL;
	int number;
	int length;
	char part1[32];
	char part2[32];
	unsigned char org_id;
	unsigned char dbnr;
	unsigned char start_address_h;
	unsigned char start_address_l;
	unsigned char len_h;
	unsigned char len_l;
	struct plc_struct *p;
	p=plc_head;
	strcpy(part1,"collect_area");
	strcpy(part2,"org_id");
	org_id=read_profile(part1,part2);
	strcpy(part1,"number_area");
	strcpy(part2,"number");
	number=(int)read_profile(part1,part2);
    /* read the plc configure information */
	for(i=1;i<=number;i++)
	{
		struct plc_struct* newnode;
		newnode=(struct plc_struct*)malloc(sizeof(struct plc_struct));
		newnode->org_id=org_id;
		sprintf(part1,"%d",i);
		strcpy(part2,"area");
		dbnr=read_profile(part1,part2);
		newnode->dbnr=dbnr;
		strcpy(part2,"start_address_h");
		start_address_h=read_profile(part1,part2);
		newnode->start_address_h=start_address_h;
		strcpy(part2,"start_address_l");
		start_address_l=read_profile(part1,part2);
		newnode->start_address_l=start_address_l;
		strcpy(part2,"len_h");
		len_h=read_profile(part1,part2);
		newnode->len_h=len_h;
		strcpy(part2,"len_l");
		len_l=read_profile(part1,part2);
		newnode->len_l=len_l;
		if(plc_head==NULL)
		{
			p=newnode;
			plc_head=p;
			p->next=NULL;
		}
		else
		{
			p->next=newnode;
			p=p->next;
			p->next=NULL;
		}
	}
    /* set the shutdown value */
	read_file("DEV-STAT","running",value);
	running=atoi(value);
	read_file("DEV-STAT","datalocation",value);
	datalocation=atoi(value);
	read_file("DEV-STAT","dataoffset",value);
	dataoffset=atoi(value);
	read_file("DEV-STAT","datalength",value);
	datalength=atoi(value);
//	device_state=query_state();
	
	return 1;
}



/***************************************************************************
Function: msg_init

Description: to initial the message queue

Calls: NULL

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput: 
	id:message ID

Output: NULL

Return: msg descriptor

Others: UNUSED
***************************************************************************/
/*
int msg_init(int id)
{
	int msgid;
	msgid=msgget((key_t)id,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("LOCAL MSG ERROR");
		exit(1);
	}
	printf("local:msg init ok\n");
	return msgid;
}

*/

/***************************************************************************
Function: alarm_recv

Description: the thread function to recv the alarm data from udp and judge whether there is alarm data, then to store the alarm data

Calls: insert_alarm

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput:  NULL

Output: NULL

Return: NULL
 
Others: NULL
***************************************************************************/
void *alarm_recv(void *arg)
{
	int i;
	struct timeval tpstart,tpend;
	float timeuse;
//	int msgid;
	int rc;
//	struct msg_local alarm_data;
//	unsigned char data[MSG_MAX];
	unsigned char cmp[MSG_MAX],data[MSG_MAX];
	int udp_socket;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t sin_size;
	int num;
//	msgid=msg_init(MSGID);
    /* set udp connection */
	udp_socket=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(udp_socket,(struct sockaddr*)&server,sizeof(struct sockaddr));
	sin_size=sizeof(struct sockaddr_in);
//	alarm_data.mtype=1;
	printf("DATE: %s local:alarm_recv pthread start!~\n",timerecord());
	memset(cmp,0x00,sizeof(cmp));
    /* recv data from udp connection */
	while(1)
	{
//		gettimeofday(&tpstart,NULL);
		num=recvfrom(udp_socket,data,MSG_MAX,0,(struct sockaddr *)&client,&sin_size);
		if(num<=0)
		{
			DEBUG("RECV ERROR");
			close(udp_socket);
			perror("    recv error:");
			exit(1);
		}
//		sleep(2);
	//	gettimeofday(&tpstart,NULL);
//		for(i=0;i<num;i++)
//			printf("%02x ",data[i]);
//		for(i=0;i<num;i++)
//			printf("%02x ",cmp[i]);
		if(memcmp(cmp,data,num))
		{
		//	alarm_data.length=num;
		//	if(~netflag)
		//	{
//again:
		//		if((rc=msgsnd(msgid,&alarm_data,(MSG_MAX+4),0))==-1)
		//		{
		//			if(errno==EINTR)
		//				goto again;
		//			DEBUG("LOCAL MSG ERROR:%d",errno);
	//
	//				perror("mmsgsnd error:");
	//				close(udp_socket);
	//				exit(1);				}
		//	}
		//	gettimeofday(&tpstart,NULL);
			pthread_mutex_lock(&mutex);
		//	gettimeofday(&tpstart,NULL);
			insert_alarm(data,num);
	//		gettimeofday(&tpend,NULL);
	//		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
	//		timeuse/=1000000;
	//		printf("used time:%f sec\n",timeuse);
			pthread_mutex_unlock(&mutex);
		//	gettimeofday(&tpend,NULL);
		//	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
		//	timeuse/=1000000;
		//	printf("used time:%f sec\n",timeuse);
			memcpy(cmp,data,num);
		}
		usleep(20);
	//	gettimeofday(&tpend,NULL);
	//	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
	//	timeuse/=1000000;
	//	printf("used time:%f sec\n",timeuse);

	}
	exit(1);
}


/***************************************************************************
Function: second_level_recv

Description: the thread function to request for data, judge whether the device work, then store the data

Calls: tcp_connect read_plcfile packet_set tcp_receice detection

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
void *second_level_recv(void *arg) 
{
	int rc;
	int i;
	int fd;
    char timeget[16];
    int fast_time=500000;
    int slow_time=5;
	fd=tcp_connect();
//	pid_t pid;
	struct fetch request;
	struct timeval tpstart,tpend;
	float timeuse;
//	int totlen=0;
//	int count=0;
//	unsigned char data[100*2*1024];
	int length=0;
	unsigned char second_data[2*1024];
	struct plc_struct *p;
	printf("DATE: %s local:second level recv pthread\n",timerecord());
	read_plcfile();
    read_file("TIME","fast_time",timeget);
    rc=atoi(timeget);
    if(rc!=0)
    {
        fast_time=rc;
    }
    read_file("TIME","slow_time",timeget);
    rc=atoi(timeget);
    if(rc!=0)
    {
        slow_time=rc;
    }

//	if(signal(SIGCLD,SIG_IGN)==SIG_ERR)
//	{
//		perror("signal error");
//		exit(1);
//	}
    /* set the request packet */ 
	packet_set(&request);
    /* request data from plc */
	while(1)
	{
	//	gettimeofday(&tpstart,NULL);
		if((length=tcp_receive(&request,fd,second_data))<0)
		{
//			goto err;
			DEBUG("tcp_recv error");
			perror("    tcp recv error:");
			exit(1);
		}
	//	gettimeofday(&tpend,NULL);
	//	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
	//	timeuse/=1000000;
	//	printf("time use：%f sec\n",timeuse);
        /* judge the facility state */
		rc=detection(second_data);
//		for(i=0;i<length;i++)
//			printf("%02x ",second_data[i]);
//		printf("%02x %02x ",second_data[200],second_data[201]);
//		printf("%02x %02x\n",second_data[202],second_data[203]);
//		printf("length=%d\n",length);
//		printf("rc=%d\n",rc);
		if(rc)
		{
			if(device_switch==0||device_switch==2)
			{
				dbrecord_v2("DEVICE ON");
				addoraltconfig(DEV_CONF,"setting","setting=1");
				send_signal(2);
				device_switch=1;
			}
	//		gettimeofday(&tpstart,NULL);
			pthread_mutex_lock(&mutex);
			insert_second(second_data,length);
			pthread_mutex_unlock(&mutex);
	//		gettimeofday(&tpend,NULL);
	//		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
	//		timeuse/=1000000;
	//		printf("time use:%f secs\n",timeuse);
	        usleep(fast_time);
		}
		else
		{
			if(device_switch==0||device_switch==1)
			{
				device_switch=2;
				addoraltconfig(DEV_CONF,"setting","setting=2");
				dbrecord_v2("DEVICE OFF");
				send_signal(2);
			}
            pthread_mutex_lock(&mutex);
            insert_second(second_data,length);
            pthread_mutex_unlock(&mutex);
            sleep(slow_time);
		}
//        usleep(timeinterval);
//		sleep(5);
	}
}
//		gettimeofday(&tpend,NULL);
//		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
//		timeuse/=1000000;
//		printf("used time:%f sec \n",timeuse);
//		if(count<50)
//		{
//			memcpy(data+totlen,second_data,length);
//			totlen+=length;
//		}
//		else
//		{
//			gettimeofday(&tpstart,NULL);
//err:
//			pid=fork();
//			if(pid==0)
//			{
//				close(fd);
//				printf("totlen=%d\n",totlen);
//				insert_second(data,totlen);
//				exit(1);
//			}
//			else if(pid>0)
//			{
//				gettimeofday(&tpend,NULL);
//				timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
//				timeuse/=1000000;
//				printf("used time:%f sec \n",timeuse);
//
//				count=0;
//				totlen=0;
//			}
//			else
//			{
//				DEBUG("fork error");
//			}
//		}
//	}
//}



/***************************************************************************
Function: detection

Description: to check the poweroff infomation of the device 

Calls: NULL

Called By: second_level_recv

Table Accessed: NULL

Table Updated: NULL

INput: 
	data: the pointer of the received data 

Output: NULL

Return: return whether the device is on
	if on return 1
	else return 0

Others: NULL
***************************************************************************/
int detection(unsigned char *data)
{
	unsigned char int0[2];
	memset(int0,0x00,2);
	unsigned char real[4];
	memset(real,0x00,4);
	unsigned char bito[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
	if(running==0)
		return 1;
	switch(datalength)
	{
		case 1:
			if(*(data+datalocation)&bito[dataoffset])
				return 1;
			else 
				return 0;
			break;
		case 16:
			if(memcmp(data+datalocation,int0,2)!=0)
				return 1;
			else
				return 0;
			break;
		case 32:
			if(memcmp(data+datalocation,real,4)!=0)
				return 1;
			else
				return 0;
			break;
		default:
			return 1;
	}
}


/***************************************************************************
Function: packet_set

Description: to set plc request package according to global variable

Calls: NULL

Called By: second_level_recv

Table Accessed: NULL

Table Updated: NULL

INput: 
	retuest: the request structure pointer

Output: the set structure

Return: if success return 1

Others: NULL
***************************************************************************/
int packet_set(struct fetch *request)
{
	struct plc_struct *p;
	p=plc_head;
	request->systemid_1=0x53;
	request->systemid_2=0x35;
	request->len_of_head=0x10;
	request->id_op_code=0x01;
	request->len_op_code=0x03;
	request->op_code=0x05;
	request->org_field=0x03;
	request->len_org_field=0x08;
	request->empty_field=0xff;
	request->len_empty_field=0x02;
	request->org_id=p->org_id;
	request->dbnr=p->dbnr;
	request->start_address_h=p->start_address_h;
	request->start_address_l=p->start_address_l;
	request->len_h=p->len_h;
//	printf("request->len_h=%d\n",request->len_l);
	request->len_l=p->len_l;
//	printf("request->len_h=%d\n",request->len_l);
	return 1;
}



/***************************************************************************
Function: tcp_receive
 
Description: send the set structure packet and receive from plc 

Calls: NULL

Called By: second_level_recv

Table Accessed: NULL

Table Updated: NULL

INput: 
	request: the set structure
	fd: the socket descriptor

Output:
	data:the received data

Return: 
	if success return the actual length
	else return -1

Others:
***************************************************************************/
int tcp_receive(struct fetch * request,int fd,unsigned char *data)
{
	struct timeval tpstart,tpend;
	float timeuse;
//	gettimeofday(&tpstart,NULL);
	int count;
	int i;
	int rc;
	int length=0;
	struct fetch_res *q;
	unsigned char temp[PLC_LEN];
	count=2*((int)(request->len_h<<8)+(int)request->len_l);
	rc=write(fd,request,sizeof(struct fetch));
	if(rc!=sizeof(struct fetch))
	{
		DEBUG("write error");
		return -1;
	}
//	gettimeofday(&tpend,NULL);
//	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
//	timeuse/=1000000;
//	printf("used time:%f sec\n",timeuse);
//	gettimeofday(&tpstart,NULL);
//again:
	rc=read(fd,temp,REQUEST);
//	gettimeofday(&tpend,NULL);
	if(rc==16)
	{			
		i=0;
		q=(struct fetch_res*)temp;
		if(q->error_field==0x00)
		{
		//	gettimeofday(&tpstart,NULL);
			while(1)
			{
			//	memset(temp,'\0',sizeof(temp));
				rc=read(fd,temp,PLC_LEN);
				if(rc!=0)
				{
					memcpy(data+i,temp,rc);
					i=i+rc;
					count-=rc;
				}
				else
				{
					DEBUG("READ ERROR");
					return -1;
				}
				if(count==0)
					break;
//				usleep(10000);
			}
		}
		else
			return -1;
	}
	else
	{
//		if(errno==EINTR)
//			goto again;
		DEBUG("recv error:%d",rc);
		perror("    read error:");
		return -1;
	}
//	gettimeofday(&tpend,NULL);
//	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
//	timeuse/=1000000;
//	printf("used time:%f sec\n",timeuse);
	return i;
}
/*
void * signal_wait(void * arg)
{

	int err;
	int result;
	int signo;
	char value[20];
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset,SIGUSR1);
	while(1)
	{
		err=sigwait(&sigset,&signo);
		if(err!=0)
			exit(1);
		if(signo==SIGUSR1)
		{
			read_file("setting","setting",value);
			result=atoi(value);
			if(result!=2)
				netflag=0;
			else
				netflag=2;
	
		}
	}
}
*/


/***************************************************************************
Function: set_timer

Description: to set the timer value

Calls: NULL

Called By: main

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: NULL

Others: UNUSED
***************************************************************************/
void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec=30;
	itv.it_interval.tv_usec=0;
	itv.it_value.tv_sec=60*60*24;
	itv.it_value.tv_usec=0;
	setitimer(ITIMER_REAL,&itv,&oldtv);
}



/***************************************************************************
Function: main

Description: to start plc connection module

Calls: set_timer write_local_pid db_init

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
//	signal(SIGALRM,signal_handler);
//	set_timer();
	write_pid_local();
	int err;
	db_init();
	FILE*fp;
	sigset_t set;
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	pthread_attr_t attr;
	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);
	err=pthread_sigmask(SIG_BLOCK,&set,NULL);
	if(err!=0)
		exit(1);
	pthread_mutex_init(&mutex,NULL);
	err=pthread_attr_init(&attr);
	if(err!=0)
	{
		DEBUG("ATTR ERROR");
		exit(1);
	
	}
    /* set thread detech */
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(err==0)
	{
		err=pthread_create(&tid1,&attr,alarm_recv,NULL);
		if(err!=0)
		{
			DEBUG("PTHRAD_CREATE ERROR");
			exit(1);
		}
		err=pthread_create(&tid2,&attr,second_level_recv,NULL);
		if(err!=0)
		{
			DEBUG("PTHREAD_CREATE ERROR");
			exit(1);
		}
//		err=pthread_create(&tid3,&attr,signal_wait,NULL);
//		if(err!=0)
//			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)1);
}
