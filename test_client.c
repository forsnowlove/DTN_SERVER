#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "sigprocess.h"
#include "DTNmsg.h"
#include "msgprocess.h"
#include "blueinfo.h"
#include "ipv6info.h"
#include "fileinfo.h"



int main()
{
    int fd = open("test_fifo",O_RDWR);
    if(fd == -1)
    {
        printf("client open test_fifo err!\n");
        return -1;
    }
    char* recv_buf = NULL;
    int dtn_msg_len = sizeof(dtn_req_msg);
    recv_buf = (char*)malloc(dtn_msg_len);
    if(recv_buf == NULL)
    {
        printf("client err: recv_buf = NULL!\n");
        return -2;
    }
    int rd_len = 0;
again:
    rd_len = read(fd,recv_buf,dtn_msg_len);
    if(rd_len == -1)
    {
        if(errno = EINTR)
        {
            goto again;
        }
        else
        {
            printf("client err:read err!\n");
            if(recv_buf != NULL)
            {
                free(recv_buf);
            }
            return -3;
        }
    }
    dtn_req_msg* msg  = (dtn_req_msg*)recv_buf; 
    printf_dtn_msg(msg);
    free(recv_buf);


    /***********************************/
    //list info
    char* data = (char*)malloc(sizeof(list_data));
    if(data == NULL)
    {
        printf("malloc data err!\n");
        return -4;
    }
    int data_len = sizeof(list_data);
    printf("list_data_len:%d\n",data_len);
    list_data *dir_data = NULL;
    rd_len = my_read(fd,data,data_len);
    if(rd_len != data_len)
    {
            printf("read list data err!\n");
            return -5;
    }
    dir_data = (list_data*)data;
    //first list_data child_dir_num
    printf("******%s*******\n",dir_data->dir_name);
    int i = 0,j = 0;
    list_data *child_dir = NULL;
    char dir_name[255] = {0};
    char *child_data = (char*)malloc(data_len);
    if(child_data == NULL)
    {
        printf("malloc child_data err!\n");
    }
    for(i = 0;i < dir_data->child_dir_num;i++)
    {
        memset(child_data,0,data_len); 
        rd_len = my_read(fd,child_data,data_len);
        if(rd_len != data_len)
        {
            printf("my_read err!\n");
            return -6;
        }
        child_dir = (list_data*)child_data;
        printf("%s\n",child_dir->dir_name);
        if(child_dir->child_dir_num > 0)
        {
            for(j = 0;j < child_dir->child_dir_num;j++)
            {
                rd_len = my_read(fd,dir_name,255);
                if(rd_len != 255)
                {
                    printf("my_read child_dir err!\n");
                    return -7;
                }
                printf("   %s\n",dir_name);
                memset(dir_name,0,255);
            }
        }
        else
        {
            printf("   empty\n");
        }
    }
    return 0;
}
