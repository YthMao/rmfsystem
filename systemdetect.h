#ifndef _SYSTEMDETECT_H_
#define _SYSTEMDETECT_H_


struct capacity
{
    int total_cap;
    int free_cap;
};

struct capacity get_system_tf_free(void);
int file_size2(char *);
#endif
