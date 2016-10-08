/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\plc_test.c
#       @date         :2015/10/29 12:56
#       @algorithm    :
==========================================================================*/
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include "plc_simulate.h"

#define TCP_ADDR "192.168.0.1"
#define TCP_PORT 2000
#define UDP_LEN 100
#define PUBLIC_KEY "public.key"
#define PUBLIC_TXT "public.txt"

int tcp_connect(void);
void udp_communication(void);



int tcp_connect()
{
	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	client_sockfd=socket(AF_INET,SOCK_STREAM,0);
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr(TCP_ADDR);
	client_address.sin_port=htons(TCP_PORT);
	client_len=sizeof(client_address);
	result=connect(client_sockfd,(struct sockaddr*)&client_address,client_len);
	if(result==-1)
	{
		perror("connect error:");
		exit(1);
	}
	return client_sockfd;
}


void tcp_communication()
{
	struct timeval tpstart,tpend;
	float timeuse;
	int i;
	int rc;
	int sockfd;
	struct fetch request;
	unsigned char buff[2048];
	sockfd=tcp_connect();
	request.systemid_1=0x53;
	request.systemid_2=0x35;
	request.len_of_head=0x10;
	request.id_op_code=0x01;
	request.len_op_code=0x03;
	request.op_code=0x05;
	request.org_field=0x03;
	request.len_org_field=0x08;
	request.empty_field=0xff;
	request.len_empty_field=0x02;
	/*need fill in*/
	request.org_id=0x01;
	request.dbnr=0x02;
	request.start_address_h=0x00;
	request.start_address_l=0x00;
	request.len_h=0x00;
	request.len_l=0x64;
	while(1)
	{
		rc=write(sockfd,&request,sizeof(struct fetch));
		if(rc<16)
		{
			printf("send error\n");
			exit(1);
		}
		gettimeofday(&tpstart,NULL);
		rc=read(sockfd,buff,16);
		gettimeofday(&tpend,NULL);
		if(rc<16)
		{
			printf("tcp recv error\n");
			exit(1);
		}
		rc=read(sockfd,buff,2048);
		if(rc<=0)
		{
			printf("tcp recv error\n");
			exit(1);
		}
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
		timeuse/=1000000;
		printf("used time:%f sec\n",timeuse);
	}



void udp_communication()
{
	int i;
	int udp_socket;
	struct sockaddr_in server;
	struct sockaddr_in client;
	unsigned char buff[UDP_LEN];
	socklen_t sin_size;
	int num;
	udp_socket=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(udp_socket,(struct sockaddr*)&server,sizeof(struct sockaddr));
	sin_size=sizeof(struct sockaddr_in);
	while(1)
	{
		num=recvfrom(udp_socket,buff,UDP_LEN,0,(struct sockaddr*)&client,&sin_size);
		if(num<0)
		{
			perror("recv error:");
			exit(1);
		}
		else
		{
			printf("recv udp\n");
			for(i=0;i<num;i++)
			{
				printf("%02x\t",buff[i]);
				if(i%5==0)
					printf("\n");
			}
			printf("\nrecv over\n");
		}
	}
}

void tcp_test()
{
	int rc;
	int sock;
	sock=tcp_connect();
	char value[100];
	while(1)
	{
		rc=read(sock,value,100);
		if(rc<=0)
		{
			printf("recv error\n");
			exit(1);
		}

		rc=write(sock,value,100);
		if(rc<=0)
		{
			printf("send error\n");
			exit(1);
		}
	}
}

void main()
{
	tcp_communication();
	return;
}
