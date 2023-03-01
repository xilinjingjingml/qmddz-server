#ifndef _CHECK_PROCESS_H_
#define _CHECK_PROCESS_H_

#include <iostream>
#include "stdio.h"
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>


int _check_process()
{
    pid_t pid = getpid();
    int last_pid =0;

    FILE* file = fopen("pid.data","r+"); 
    if(file == NULL)
    {
        file = fopen("pid.data","a");
        fprintf(file,"%d",pid);
        fclose(file);
        return 0;
    }

    fscanf(file,"%d",&last_pid);
    if(last_pid == 0)
    {
        printf("process is not running\n"); 
        fprintf(file,"%d",pid);
        fclose(file);
        return 0;
    }

    //check this pid is running
    char process_buffer[100];
    sprintf(process_buffer,"/proc/%d",last_pid);
    DIR* dir = opendir(process_buffer);
    if(dir == NULL)
    {
        rewind(file);
        fprintf(file,"%d",pid);
    }else
    {
        closedir(dir); 
        return last_pid;
    }

    fclose(file);
    return 0;
}

void process_check()
{
    pid_t pid = _check_process();
    if(pid != 0)
    {
        printf("process is running kill -9 %d",pid); 
        assert(false);
    }
}
// 
// int main()
// {
//     int pid = check_process();
//     if(pid == 0)
//     {
//         printf("process is not running ,you can run this process");
//     }
//     else
//     {
//         printf("process is running ,can't run this process"); 
//         assert(false);
//     }
//     return 1;
// }
#endif
