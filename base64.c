/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\base64.c
#       @date         :2015/12/02 16:25
#       @algorithm    :
==========================================================================*/
#include "base64.h"

#include "rmfsystem.h"
/*maptable*/
char base64_index[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";



/***************************************************************************
Function: base64_encode_v1

Description: to encrypt the password by base64 method

Calls: NULL

Called By: client_pipe.c:login

Table Accessed: NULL

Table Updated: NULL

Input: 
	input: the string to be encrypted

	length: the length of the string to be encrypted

Output:
	output: (string)the result after encryption

Return:
	the result after encryption

Others: USING
***************************************************************************/
char* base64_encode_v1(char * input,int length,char* output)
{
	*output = '\0';
	if(input == NULL||length<1)
		return output;
	char*p = (char*)input;
	char*p_dst = (char*)output;
	char*p_end = (char*)input+length;
	int loop_count = 0;
	while(p_end-p >= 3)
	{
		*p_dst++=base64_index[(p[0] >> 2)];
		*p_dst++=base64_index[((p[0] << 4) & 0x30) | (p[1] >> 4)];
		*p_dst++=base64_index[((p[1] << 2) & 0x3c) | (p[2] >> 6)];
		*p_dst++=base64_index[p[2] & 0x3f];
		p += 3;
	}
	if (p_end-p > 0)
	{
		*p_dst++= base64_index[(p[0] >> 2)];
		if(p_end-p==2)
		{
			*p_dst++ = base64_index[((p[0] << 4) & 0x30) | (p[1] >>4)];
			*p_dst++ = base64_index[(p[1] << 2) & 0x3c];
			*p_dst++ = '=';
		}
		else if(p_end-p==1)
		{
			*p_dst++ = base64_index[(p[1] << 4) & 0x30];
			*p_dst++ = '=';
			*p_dst++ = '=';
		}
		*p_dst = '\0';
		return output;
	}
}



/***************************************************************************
Function: base64_encode_v2

Description: the second version to encrypt the string

Calls: NULL

Called By: NULL

Table Accessed: NULL

Table Updated: NULL

INput:
	binData:the string to be encrypted 

	binLength: the length of the input string

Output:
	base64: the result after encryption

Return:
	the result after encryption

Others: UNUSED
***************************************************************************/
char* base64_encode_v2(char* binData,char*base64,int binLength)
{
	int i=0;
	int j=0;
	int current=0;
	for(i=0;i<binLength;i+=3)
	{
		current=(*(binData) >> 2) & 0x3f;
		*(base64+j++)=base64_index[current];
		current=(*(binData+i)<<4) & 0x30;
		if(binLength<=(i+1))
		{
			*(base64+j++)=base64_index[current];
			*(base64+j++)='=';
			*(base64+j++)='=';
			break;
		}
		current|=(*(binData+i+1)>>4) & 0xf;
		*(base64+j++)=base64_index[current];
		current=(*(binData+i+1)<<2) & 0x3c;
		if(binLength<=(i+2))
		{
			*(base64+j++)=base64_index[current];
			*(base64+j++)='=';
			break;
		}
		current|=(*(binData+i+2)>>6)&0x03;
		*(base64+j++)=base64_index[current];
		current=*(binData+i+2)&0x3f;
		*(base64+j++)=base64_index[current];
	}
	*(base64+j)='\0';
	return base64;
}
/*
void main()
{
	char *origin = "are you sb";
	char after[128];
	//base64_encode(origin,strlen(origin),after);
	base64_encode_v2(origin,after,strlen(origin));
	printf("after encrypt:%s\n",after);
	return;
}
*/
