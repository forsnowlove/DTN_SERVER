#include <stdio.h>
#include <wait.h>
#include <pthread.h>
#include <signal.h>
#include "sigprocess.h"

void sig_child(int signo)
{//recycle child peocess
    pid_t pid;
    int stat;
    while(pid = waitpid(-1,&stat,WNOHANG) > 0)
    {
        printf("child %d terminated\n",pid);
    }
    return;
}
