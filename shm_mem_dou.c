/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\shm_mem.c
#       @date         :2015-12-02 10:59
#       @algorithm    :
==========================================================================*/
#include "shm_mem.h"

#include "rmfsystem.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/*创建信号量函数*/
int creatsem(int i)
{
	int index,sid;
	union semun semopts;
	semopts.val=1;
	if(i==1)
	{
		if((sid=semget((key_t)SEMID,1,IPC_CREAT|0666))==-1)
		{
			DEBUG("SEMGET ERROR:%d",errno);
			return -1;
		}
	}
	if(i==2)
	{
		if((sid=semget((key_t)SEMID2,1,IPC_CREAT|0666))==-1)
		{
			DEBUG("SEMGET ERROR:%d",errno);
			return -1;
		}
	}
	semctl(sid,0,SETVAL,semopts);
	return sid;
}

int opensem(int i)
{
	key_t msgkey;
	int sid;
	if(i==1)
	{
		if((sid=semget((key_t)SEMID,0,0666))==-1)
		{
			DEBUG("SEMGET ERROR:%d",errno);
			return -1;
		}
	}
	if(i==2)
	{
		if((sid=semget((key_t)SEMID2,0,0666))==-1)
		{
			DEBUG("SEMGET ERROR:%d",errno);
			return -1;
		}
	}
	return sid;
}

/*p操作，获取信号量*/
int sem_p(int semid)
{
	struct sembuf sbuf={0,-1,SEM_UNDO};
	if(semop(semid,&sbuf,1)==-1)
	{
		DEBUG("SEMOP ERROR:%d",errno);
		return -1;
	}
	return 0;
}

/*int sem_p_no(int semid)
{
	struct sembuf sbuf={0,-1,IPC_NOWAIT};
	if(semop(semid,&sbuf,1)==-1)
	{
		if()
		printf("a wrong operation to semaphare occured\n");
		return -1;
	}
	return 0;
}
*/
/*v操作 释放信号量*/
int sem_v(int semid)
{
	struct sembuf sbuf={0,1,SEM_UNDO};
	if(semop(semid,&sbuf,1)==-1)
	{
		DEBUG("SEMOP ERROR:%d",errno);
		return -1;
	}
	return 0;
}

int getsem(int semid)
{
	if(semctl(semid,0,GETVAL,0)!=0)
		return 1;
	else
		return 0;
}
/*
int sem_v_no(int semid)
{
	struct sembuf sbuf={0,1,IPC_NOWAIT};
	if(semop(semid,&sbuf,1)==-1)
	{
		perror("a wrong operation to semaphore occured\n");
		return -1;
	}
	return 0;
}
*/
int sem_delete(int semid)
{
	return (semctl(semid,0,IPC_RMID));
}

int wait_sem(int semid)
{
	while(semctl(semid,0,GETVAL,0)==0)
	{
		usleep(500);
	}
	return 1;
}

/*创建共享内存*/
int creatshm(int i)
{
	key_t shmkey;
	int sid;
	if(i==1)
	{
		if((sid=shmget((key_t)SHMID,SHM_SIZE,IPC_CREAT|0666))==-1)
		{
			DEBUG("SHMGET ERROR:%d",errno);
			return -1;
		}
	}
	if(i==2)
	{
		if((sid=shmget((key_t)SHMID2,SHM_SIZE,IPC_CREAT|0666))==-1)
		{
			DEBUG("SHMGET ERROR:%d",errno);
			return -1;
		}
	}
	return sid;
}

/*删除共享内存*/
int deleteshm(int sid)
{
	void *p=NULL;
	return(shmctl(sid,IPC_RMID,p));
}


