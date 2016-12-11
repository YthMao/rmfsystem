/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\client_pipe.h
#       @date         :2015/12/02 17:51
#       @algorithm    :
==========================================================================*/

#ifndef _CLIENT_PIPE_H_
#define _CLIENT_PIPE_H_

#include "rmfsystem.h"


void *udp_test(void *);

int conn(void);

int order(void);

int msg_init(int);

int first_login(void);

int login(void);

void * signal_wait(void*);

int get_line(int,char*,int);

void *remote_recv(void*);

void * remote_send(void*);

int recv_error(void);

int send_signal_usr2(void);

void write_pid_remote(void);

void main(void);

void write_pid_deal(void);

int plc_online(void);

#endif
