/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\shm_mem.h
#       @date         :2015-12-02 10:59
#       @algorithm    :
==========================================================================*/
#ifndef _SHM_MEM_H_
#define _SHM_MEM_H_

#define SHM_SIZE (20*1024)
#define SHMID 1000
#define SHMID2 3000
int creatshm(int);

int deleteshm(int);
#endif
