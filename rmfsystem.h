/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/linghao\rmfsystem.h
#       @date         :2015-08-25 17:33
#       @algorithm    :
==========================================================================*/
#ifndef _RMFSYSTEM_H_
#define _RMFSYSTEM_H_

#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "sqlite3.h"

char *timerecord(void);
char *getconfigpath(char *);
char *getprogrammepath(char *);
#define NORMAL_TABLE "device_second_level_data"
#define ALARM_TABLE "device_alarm"
#define STATE_TABLE "device_running_statement"
#define REMOTE_PORT 13000
#define MSG_MAX 256
#define MSGID 2000
#define DB "/data/local.db"

#define DEV_CONF "/home/linaro/project/rmfsystem/configure/device.config"
#define PRO_CONF "/home/linaro/project/rmfsystem/configure/process.config"
#define CONFIG "/home/linaro/project/rmfsystem/configure/config"

#define FIFO_NAME "/tmp/my_fifo"
#define COM_PATH "/home/linaro/project/rmfsystem/comtest_pipe"
#define RMT_PATH "/home/linaro/project/rmfsystem/client_pipe"
/*
#define DEV_CONF getconfigpath("device.config")
#define PRO_CONF getconfigpath("process.config")
#define CONFIG   getconfigpath("config")

#define COM_PATH getprogrammepath("comtest_pipe")
#define RMT_PATH getprogrammepath("client_pipe")

#define FIFO_NAME "/tmp/my_fifo"
*/
#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("DATE: %s    FILE: "__FILE__", LINE: %04d: "format"\n",timerecord(),__LINE__,##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif


struct msg_local
{
	long mtype;
	int length;
	unsigned char data[MSG_MAX];
};

struct msg_remote
{
	long mtype;
	int length;
	char command[5];
	int number;
	char time[100];
	unsigned char data[100];
};


struct plc_struct 
{
	unsigned char org_id;
	unsigned char dbnr;
	unsigned char start_address_h;
	unsigned char start_address_l;
	unsigned char len_h;
	unsigned char len_l;
	struct plc_struct * next;
};

struct remote_data
{
	long long int number;
	char time[32];
	int length;
	unsigned char data[2*1024];
};

struct alarm_data
{
	long long int number;
	char time[32];
	int length;
	unsigned char data[256];
};

void gettime(char*);

int send_signal(int);


#endif
