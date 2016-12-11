/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\systemdetect.c
#       @date         :2015/12/07 10:03
#       @algorithm    :
==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include "systemdetect.h"


struct capacity  get_system_tf_free()
{
        struct capacity temp;
	struct statfs diskInfo;
	statfs("/data",&diskInfo);
	unsigned long long totalBlocks=diskInfo.f_bsize;
	unsigned long long totalSize=totalBlocks*diskInfo.f_blocks;
	size_t mbtotalsize=totalSize>>20;
	unsigned long long freeDisk=diskInfo.f_bfree*totalBlocks;
	size_t mbfreesize=freeDisk>>20;
        temp.total_cap=mbtotalsize;
        temp.free_cap=mbfreesize;
	return temp;
}

int file_size2(char*filename)
{
	struct stat statbuf;
	stat(filename,&statbuf);
	int size;
	size=statbuf.st_size;
	printf("%d\n",size);
}
/*
void main()
{
	double free;
	int rc;
	rc=get_system_tf_free();
	if(rc==-1)
		return;
	file_size2("/home/harry/rmfsystem.1.2/local.db");
	return;

}
*/
