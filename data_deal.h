/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\data_deal.h
#       @date         :2015/12/02 17:38
#       @algorithm    :
==========================================================================*/
#ifndef _DATA_DEAL_H_
#define _DATA_DEAL_H_

#include "rmfsystem.h"
#include <sqlite3.h>

int msg_recv(struct msg_local*,int);

void db_init(void);

int confirm(int );

int insert_alarm(unsigned char *,int);

int insert_second(unsigned char*,int);


void * second_level_deal(void*);

void db_base_maintenance(sqlite3 *);

void write_pid_deal(void);

int query(struct remote_data*);

int query_alarm(struct alarm_data *);

int query_state(void);

int query_maxid(void);

int record(char*);

void dbrecord_v2(char *);
#endif
