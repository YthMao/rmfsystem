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
#ifndef _INIT_H_
#define _INIT_H_

#include <sqlite3.h>


void db_initialize(void);

sqlite3* db_exist(void);

void db_table(sqlite3 *db);

void state_table_detect(sqlite3* );

int err_record(sqlite3 *,char*);

void profile_init(void);

void profile_config(void);

void profile_dev(void);

void profile_process(void);

void init(void);

#endif
