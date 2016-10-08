/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\scan.c
#       @date         :2015-12-02 10:57
#       @algorithm    :
==========================================================================*/
#include "scan.h"

#include "rmfsystem.h"
#include<fcntl.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include"read_file.h"
#include"change_profile.h"
#include <unistd.h>



/***************************************************************************
Function:sock_connect

Description: to connect the appointed address

Calls: NULL

Called By: scanip

Table Accessed: NULL

Table Updated: NULL

INput: 
	address: the address to connect

Output: NULL

Return: 
	if connect success return 1
	else if connot connect return 0
	else return -1

Others: NULL
***************************************************************************/
int sock_connect(char *address)
{
	int flag;
	struct sockaddr_in servaddr;
	int  sockfd;
	fd_set west;
	int maxfd;
	int error;
	struct timeval timeout;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("create scoket error");
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(TCP_PORT);
	int val=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,val| O_NONBLOCK);
//	printf("%s\n",address);
	servaddr.sin_addr.s_addr=inet_addr(address);
	int connect_flag=connect(sockfd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr_in));
	if(connect_flag>=0)
	{
		close(sockfd);
		return 1;
	}
	else if(errno==EINPROGRESS)
	{
		timeout.tv_sec=0;
		timeout.tv_usec=1000*200;
//		FD_ZERO(&rest);
		FD_ZERO(&west);
//		FD_SET(sockfd,&rest);
		FD_SET(sockfd,&west);
		maxfd=sockfd+1;
		flag=select(maxfd,NULL,&west,NULL,&timeout);
		if(flag<0)
		{
			close(sockfd);
			printf("select error\n");
			return 0;
		}
		if(FD_ISSET(sockfd,&west))
		{
			socklen_t len=sizeof(error);
			if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)<0)
			{
				close(sockfd);
				return 0;
			}
			if(error!=0)
			{
				close(sockfd);
				return 0;
			}
			else 
			{
				close(sockfd);
				return 1;
			}
		}
		else
		{
			close(sockfd);
			return 0;
		}
	}
	else 
	{
		printf("connect error\n");
		close(sockfd);
		return 0;
	}
}


/***************************************************************************
Function: scanip

Description: to choose the ip to connect and return the connection result

Calls: sock_connect read_file addoraltconfig

Called By: scan

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: 
	if success return socket descriptor
	else return 0
Others:
***************************************************************************/
int scanip()
{
	int sockfd,n,length,maxfd,flag;
	int i=1;
	int my;
	int rc;
	char address[20];
	char temp[30];
	struct timeval timeout;
	fd_set rest,west;
//	struct sockaddr_in servaddr,peersock;
//	length=sizeof(peersock);
	int  error=0;
	read_file("PLC_INFO","plcaddress",address);
	printf("address=%s\n",address);
	if(strlen(address)>7)
	{
		rc=sock_connect(address);
		if(rc>0)
			goto done;
	}
	/*quite important!timeout must be set in the loop
	 because every loop will flush  the structure*/
//	timeout.tv_sec=2;
//	timeout.tv_usec=0;
	printf("scan begin!~~~~~~~~~~~\n");	
	for(;i<255;i++)
	{
		sprintf(address,"192.168.0.%d",i);
//		printf("%s\n",address);
		rc=sock_connect(address);
		if(rc>0)
		{
		//	printf("1\n");
			goto done;
		}
	}
done:
	if(i<254)
	{
//		flag=fcntl(sockfd,F_GETFL,0);
//		fcntl(rc,F_SETFL,flag&~O_NONBLOCK);
		printf("scan successfully!\naddr:%s\nport:%d\n",address,TCP_PORT);
		sprintf(temp,"plcaddress=%s",address);
		addoraltconfig(DEV_CONF,"plcaddress",temp);
		return rc;
	}
	else
		return 0;
}


/***************************************************************************
Function:scan

Description: the interface function to connect the PLC

Calls: scanip

Called By: init.init

Table Accessed: NULL

Table Updated: NULL 

INput: NULL

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/
int scan(void)
{
	int rc;
	while(1)
	{
	
		rc=scanip();
		if(rc<=0)
		{
			printf("cannot match\n");
		}
		else 
		{
			printf("scan success\n");
			break;
		}
		sleep(60);
	}

}
