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

/*创建共享内存*/

/***************************************************************************
Function:

Description:

Calls:

Called By:

Table Accessed:

Table Updated:

INput:

Output:

Return:

Others:
***************************************************************************/
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

/***************************************************************************
Function:

Description:

Calls:

Called By:

Table Accessed:

Table Updated:

INput:

Output:

Return:

Others:
***************************************************************************/
int deleteshm(int sid)
{
	void *p=NULL;
	return(shmctl(sid,IPC_RMID,p));
}


