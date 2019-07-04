#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include "fileinfo.h"

int get_list_info(list_info **list/*out*/)
{
    list_info *myinfo;
    myinfo = (list_info*)malloc(sizeof(list_info));
    if(myinfo == NULL)
    {
        printf("get_list_info:malloc myinfo err!\n");
        return -1;
    }
    
    char **parent_list = NULL;
    char ***child_list = NULL;
    int  *cdir_num_buf = NULL;

    int dir_num = 0,child_dir_num = 0,i = 0,j = 0,k = 0;//j:malloc child_list k:save one child list
    char name[255] = {0};
    struct dirent **info = NULL;
    struct dirent **namelist = NULL;
    struct dirent **childnamelist = NULL;
    DIR *dirp = NULL;
    if( (dirp = opendir("./Data")) == NULL )
    {
        printf("get_list_info err: opendir ./Data err!\n");
        return -2;
    }
    //scan parent dir
    dir_num = scandir("Data",&namelist,0,0);//last param :sort func pointer : like-->alphasort
    //dir_num = scandir("./Data",&namelist,0,alphasort);
    //printf("parent dir num:%d\n",dir_num);
    if(dir_num < 2)
    {
        printf("get_list_info err:scandir err!\n");
        return -3;
    }
    /*
    for( i = 0;i < dir_num;i++)
    {
        printf("%s\n",namelist[i]->d_name);
    }
    */
    dir_num = dir_num -2;
    if(dir_num <= 0)
    {
        printf("get_list_info err: ./Data is empty!\n");
        return -4;
    }
    //save parent dir num
    myinfo->parent_dir_num = dir_num;
    //malloc parent_dir buf 
    parent_list = (char**)malloc(sizeof(char*)*dir_num);
    if(parent_list == NULL)
    {
        printf("get_list_info err:malloc parent_list err!\n");
        return -5;
    }
    for(i = 0;i < dir_num;i++)
    {
        parent_list[i] = (char*)malloc(sizeof(char)*255);//name_value
        if(parent_list[i] == NULL)
        {
            printf("get_list_info err:malloc parent_list[i] err!\n");
            goto myfree;
        }
    }
    for(i = 0;i < dir_num;i++)
    {
        memset(parent_list[i],0,sizeof(*parent_list[i]));//name_value = ""
    }
    //malloc child_dir buf
    child_list = (char***)malloc(sizeof(char**)*dir_num);
    if(child_list == NULL)
    {
        printf("get_list_info err: malloc child_list err!\n");
        goto myfree;
    }
    for(i = 0;i < dir_num;i++)
    {
        child_list[i] = NULL;
    }
    
    //malloc child_dir_num
    cdir_num_buf = (int*)malloc(sizeof(int)*dir_num); 
   
    
    //save all info to ***childlist
    //change dir to Data
     if( chdir("./Data") == -1 )
     {
        printf("get_list_info err:chdir err!\n");
        goto myfree;
     }
     int m = 0;//parent_list 
     for(i = 0 ;i < dir_num + 2;i++)
     {//child dir ingore "." and ".."
         //once process one child dir
         if( (strcmp(namelist[i]->d_name,".") == 0) || (strcmp(namelist[i]->d_name,"..") == 0) )
         {
             continue;
         }
         //save parent dir_name to parent_list
         strncpy(parent_list[m],namelist[i]->d_name,255);
         //printf("parent_list[%d] name:%s\n",m,parent_list[m]);
         m++;
         //scan child dir
         child_dir_num = 0;
         memset(&childnamelist,0,sizeof(childnamelist));
         //printf("child_list[%d]:%s\n",i,namelist[i]->d_name);
         child_dir_num = scandir(namelist[i]->d_name,&childnamelist,0,0);
         if(child_dir_num < 2)
         {
             printf("get_list_info: scan child dir err!\n");
             goto myfree;
         }
         child_dir_num -= 2;
         //printf("child_dir_num:%d\n",child_dir_num);
         if(child_dir_num == 0)
         {
             //printf("get_list_info: %s empty!\n",namelist[i]->d_name);
             cdir_num_buf[j] = 0;//this child_list dir NULL: child_list[j] = NULL
             child_list[j] = NULL;
             //printf("cdir_num_buf[%d]:0\n",j);
             j++;
             continue;
         }
         //save this child dir num
         cdir_num_buf[j] = child_dir_num;  
         //printf("cdir_num_buf[%d]:%d\n",j,child_dir_num);
         //malloc one child dir
         child_list[j] = (char**)malloc(sizeof(char*)*child_dir_num);
         if((child_list[j]) == NULL)
         {
             printf("get_list_info: malloc child_list[k] err!\n");
             goto myfree;
         }
         for( k = 0;k < child_dir_num;k++)
         {
             child_list[j][k] = (char*)malloc(sizeof(char)*255);
             if(child_list[j][k] == NULL)
             {
                 printf("get_list_info:malloc child_list[j][k] err!\n");
                 goto myfree;
             }
         }
         for(k = 0;k < child_dir_num;k++)
         {
             memset(child_list[j][k],0,255);
         }
         //save child dir to child_list[k]
         int h = 0;
         for(k = 0;k < child_dir_num + 2;k++)
         {
             if( (strcmp(childnamelist[k]->d_name,".") == 0) || 
                     (strcmp(childnamelist[k]->d_name,"..") == 0) )
             {
                 continue;
             }
             strncpy(child_list[j][h],childnamelist[k]->d_name,255); 
             h++;
         }
         //one child dir process finish
         j++;
     }
     myinfo->parent = parent_list;
     myinfo->child = child_list;
     myinfo->child_dir_num = cdir_num_buf;

     *list = myinfo;
     return 0;
myfree:
     if(parent_list != NULL)
     {
         for(i = 0;i < dir_num;i++)
         {
             if(parent_list[i] != NULL)
             {
                 free(parent_list[i]);
             }
         }
         free(parent_list);
     }
     if(child_list != NULL)
     {
         for(i = 0;i <child_dir_num;i++)
         {
             if(child_list[i] != NULL)
             {
                 for(j = 0;j < cdir_num_buf[i];j++)
                 {//each child_name_list
                     if(child_list[i][j] != NULL)
                     {
                         free(child_list[i][j]);
                     }
                 }
                 free(child_list[i]);
             }
         }
         free(child_list);
     }
     closedir(dirp);
     return -6;
}

int printf_list(list_info *list)
{
    if(list == NULL)
    {
        printf("printf_list err:list = NULL!\n");
        return -1;
    }
    printf("*******************list********************\n");
    int i = 0,j = 0;
    for(i = 0; i < list->parent_dir_num;i++)
    {
        printf("%d. %s\n",i+1,list->parent[i]);
        if(list->child[i] == NULL)
        {
            printf("   empty\n");
        }
        else
        {
            for(j = 0;j < list->child_dir_num[i];j++)
            {
                if(list->child[i][j] != NULL)
                {
                    printf("   %d. %s\n",j+1,list->child[i][j]);
                }
            }
        } 
    }
    printf("*******************************************\n");
    return 0;
}

int free_list(list_info *list)
{
    if(list == NULL)
    {
        printf("free_list err: list = NULL!\n");
        return -1;
    }
    int i = 0,j = 0;
    int parent_len = list->parent_dir_num;
    for(i = 0; i < parent_len; i++)
    {
        if(list->parent[i] != NULL)
        {
            free(list->parent[i]);
            list->parent[i] = NULL;
        }
        for(j = 0;j < list->child_dir_num[i];j++)
        {   
            if(list->child[i] != NULL)
            {
                if(list->child[i][j] != NULL)
                {
                    free(list->child[i][j]);
                    list->child[i][j] = NULL;
                }
                free(list->child[i]);
                list->child[i] = NULL;
            }
        }
    }
    if(list->parent != NULL)
    {
        free(list->parent);
    }
    if(list->child != NULL)
    {
        free(list->child);
    }
    if(list->child_dir_num != NULL)
    {
        free(list->child_dir_num);
    }
    if(list != NULL)
    {
        free(list);
    }
    list = NULL;
    return 0;
}

int get_list_len(list_info *list)
{
    if(list == NULL)
    {
        printf("get_list_len err:list = NULL\n");
        return -1;
    }
    int i = 0,j = 0;
    int list_data_len = sizeof(list_data);
    int len = list_data_len;
    for(i = 0;i < list->parent_dir_num;i++)
    {
        len = len + list_data_len;
        for(j = 0;j < list->child_dir_num[i];j++)
        {
            len = len + 255;
        }
    }
    return len;
}

int init_list_data(list_data* data)
{
    if(data == NULL)
    {
        printf("init_list_data err:data = NULL!\n");
        return -1;
    }
    memset(data->dir_name,0,255);
    data->child_dir_num = 0;
    return 0;
}

int my_read(int fd,char* buf,int len)
{
    if(fd < 0 || buf == NULL || len < 0)
    {
        printf("my_read err: fd <0 || buf = NULL || len < 0!\n");
        return -1;
    }
    int read_len = 0;
again:
    read_len = read(fd,buf,len);
    if(read_len == -1)
    {
        if(errno == EINTR)
        {
            goto again;      
        }
        else
        {
            printf("my_read err!\n");
            return -2;
        }
    }
    return read_len;
}

int my_write(int fd,char *data,int len)
{
    if(fd < 0)
    {
        printf("my_write err:client < 0!\n");
        return -1;
    }
    if(data == NULL)
    {
        printf("my_write err: data = NULL");
        return -2;
    }
    if(len < 0)
    {
        printf("my_write err:len < 0!\n");
        return -3;
    }
    int write_len = 0;
again:
    write_len = write(fd,data,len);
    if(write_len == -1)
    {
        if(errno == EINTR)
        {
            goto again;      
        }
        else
        {
            printf("send_list err:write err!\n");
            return -4;
        }
    }
    return write_len;
}

int send_list_info(int client,list_info* list)
{
    if(client < 0)
    {
        printf("send_list_info err:client < 0!\n");
        return -1;
    }
    if(list == NULL)
    {
        printf("send_list_info err:list = NULL!\n");
        return -2;
    }
    list_data data;
    init_list_data(&data);
    strncpy(data.dir_name,"list",sizeof("list"));
    data.child_dir_num = list->parent_dir_num;
    int list_data_len = sizeof(data);
    int send_len = 0;
    send_len = my_write(client,(char*)(&data),list_data_len);
    if(send_len != list_data_len)
    {
        printf("send_list_info err:mywrite err len!\n");
        return -3;
    }
    int i = 0;
    int j = 0;
    int res = 0;
    for(i = 0;i < list->parent_dir_num;i++)
    {
        init_list_data(&data);
        strncpy(data.dir_name,list->parent[i],255);
        data.child_dir_num = list->child_dir_num[i];
        send_len = my_write(client,(char*)(&data),list_data_len);
        if(send_len != list_data_len)
        {
            printf("send_list_info err:mywrite err len!\n");
            return -4;
        }
        if(data.child_dir_num > 0)
        {
            for(j = 0;j < data.child_dir_num;j++)
            {
                send_len = my_write(client,list->child[i][j],255);
                if(send_len != 255)
                {
                    printf("send_list_info err:mywrite err len!\n");
                    return -5;
                }
            }
        }
    }
    return 0;
}

int main()
{
    list_info *list = NULL;
    if(get_list_info(&list) < 0)
    {
        printf("failed!\n");
        return -1;
    }
    printf("list len:%d\n",sizeof(*list));
    
    if(printf_list(list) < 0)
    {
        printf("printf err!\n");
    }
    int res = get_list_len(list);
    if(res < 0)
    {
        printf("get list len err!\n");
    }
    printf("list len:%d\n",res);
    free_list(list);
    return 0;
}

