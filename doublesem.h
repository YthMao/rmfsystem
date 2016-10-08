/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem_test\doublesem.h
#       @date         :2015/12/17 22:43
#       @algorithm    :
==========================================================================*/

#ifndef _DOUBLESEM_H_
#define _DOUBLESEM_H_

#include<sys/ipc.h>
#include<sys/sem.h>

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short*array;
};

#define PUT 5000
#define GET 6000

int sem_get(int);
int init_sem(int,int);
int del_sem(int);
int sem_p(int);
int sem_v(int);

#endif 
