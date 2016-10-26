/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\comtest_pipe.h
#       @date         :2015/12/02 16:36
#       @algorithm    :
==========================================================================*/
#ifndef _COMTEST_PIPE_H_
#define _COMTEST_PIPE_H_

#include <limits.h>
#include <sqlite3.h>
#include "plc_simulate.h"

#define INC(x) (((++x)>65536)?1:x)
#define TCP_PORT 2000
#define UDP_PORT 2002
#define PLC_LEN (2*1024)
#define REQUEST 16
#define REAL_TIME_NUM 20
#define BUFF_SIZE PIPE_BUF

struct plc_loc
{
	int used;
	int offset;
	int location;
	int length;
};

//sqlite3 * db_init(void);

int tcp_connect(void);

int msg_init(int);

void * first_level_recv(void*);

void * second_level_recv(void*);

void * signal_wait(void*);

void second_rate_change(void);

void real_time_deal(void);

int detection(unsigned char*);

void write_pid_local(void);

void xml_read(int);

int read_plcfile(void);

int real_time_select(unsigned char *,unsigned char*);

int tcp_receive(struct fetch *,int,unsigned char*);

void main(void);

static void sig_cld(int);

int packet_set(struct fetch *);
#endif
