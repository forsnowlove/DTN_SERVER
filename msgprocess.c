#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include "DTNmsg.h"
#include "msgprocess.h"
#include "ipv6info.h"
#include "blueinfo.h"
#include "fileinfo.h"

int get_local_time(time_t* stamp,char* timebuf)
{
    if(stamp == NULL)
    {
        printf("get_local_time err: stamp = NULL!\n");
        return -1;
    }
    if(timebuf == NULL)
    {
        printf("get_local_time err!\n");
        return -2;
    }
    struct tm* timeinfo;
    timeinfo = localtime(stamp);
    strftime(timebuf,24,"%Y-%m-%d %H:%M:%S",timeinfo);
    return 0;
}

int get_dead_time(time_t* stamp,int life_time,char* timebuf)
{   
    if(stamp == NULL || timebuf == NULL || life_time < 0)
    {
        printf("get_dead_time err:stamp = NULL");
        printf("|| timebuf = NULL || life_time < 0!\n");
    }
    int t = (int)*stamp + life_time;
    int res = get_local_time((time_t*)(&t),timebuf);
    if(res < 0)
    {
        printf("get_dead_time err!\n");
        return -1;
    }
    return 0;
}

int printf_dtn_msg(dtn_req_msg* msg)
{
    if(msg == NULL)
    {
        printf("printf_dtn_msg err!\n");
        return -1;
    }
    printf("Receive from:%s\n",msg->source_ipv6_addr);
    char timebuf[24] = {0};
    int res =get_local_time(&(msg->create_time),timebuf);
    if(res < 0)
    {
        printf("client err:get_local_time err!\n");
        return -2;
    }
    printf("Time:%s\n",timebuf);
    
    char deadtime[24] = {0};
    res = get_dead_time(&(msg->create_time),
                        msg->life_time,deadtime);
    if(res < 0)
    {
        printf("printf_dtn_msg err: get_dead_time err!\n");
        return -3;
    }
    printf("Deadline:%s\n",deadtime);
    printf("Data_name:%s\n",msg->data_name);
    printf("Data_len:%d\n",msg->data_len);
    return 0;
}

int get_tstamp(time_t *stamp)
{
    time_t tm;
    time(&tm);
    *stamp = tm;
    return 0;
}

int init_node_info(Nodeinfo* node)
{
    if(node == NULL)
    {
        printf("init_node_info err: node = NULL!\n");
        return -1;
    }
    memset(node->ipv6_addr,0,IPv6_ADDR_LEN);
    memset(node->blue_addr,0,BLUE_ADDR_LEN);
    node->create_tstamp = 0;
    return 0;
}

int set_node_info(Nodeinfo *node,char* ipv6addr,char* blueaddr)
{
    if(node == NULL || ipv6addr == NULL || blueaddr == NULL)
    {
        printf("set_node_info err: input para have NULL!\n");
        return -1;
    }
    strncpy(node->ipv6_addr,ipv6addr,IPv6_ADDR_LEN);
    strncpy(node->blue_addr,blueaddr,BLUE_ADDR_LEN);
    get_tstamp(&node->create_tstamp);
    return 0;
}

int check_node_list_blue(char* dest_blue_addr,char* dest_ipv6_addr/*int out*/)
{
    //printf("check node list...\n");
    if(dest_blue_addr == NULL || dest_ipv6_addr == NULL)
    {
        printf("check_node_list err: dest_blue_addr = NULL || dest_ipv6_addr = NULL!\n");
        return -1;
    }
    //if( strcmp(dest_blue_addr,SERVER_BLUE_ADDR) == 0)
    //{
    //  strncpy(dest_ipv6_addr,SERVER_IPv6_ADDR,IPv6_ADDR_LEN);
    //    return 1;
    //}
    umask(0);
    int list_fd = open("./NodeList/nodelist.data",O_CREAT|O_RDWR,0777);
    if(list_fd == -1)
    {
        printf("check_node_list err:open nodelist.data failed!\n");
        return -1;
    }
    Nodeinfo *node = NULL;
    int node_len = sizeof(*node);
    //printf("node_len:%d\n",node_len);
    char *buf = NULL;
    buf  = (char*)malloc(node_len);
    if(buf == NULL)
    {
        printf("check_node_list err : molloc buf err!\n");
        close(list_fd);
        return -2;
    }
    int read_size = 0;
    int flag = 0;//flag = 0 not having this node flag = 1 this node already have
again:
    read_size = read(list_fd,buf,node_len); 
    if(read_size == 0)
    {
        if(errno == EINTR)
        {
            goto again;
        }

        else
        {
            printf("check_node_list:node_list empty!\n");
            if(buf != NULL)
            {
                free(buf);  
            }
            flag = 0;
            return list_fd;
        }
    }
    int i = 1;
    //printf("readsize:%d\n",read_size);
    while(read_size == node_len)
    {
        node = (Nodeinfo*)buf;
        printf("node %d:ipv6_addr:%s blue_addr:%s \n",i++,node->ipv6_addr,node->blue_addr);
        //printf("client_ipv6addr:%s\n",client_ipv6addr);
        if( strcmp(node->blue_addr,dest_blue_addr) == 0)
        {
            strncpy(dest_ipv6_addr,node->ipv6_addr,IPv6_ADDR_LEN);
            flag = 1;
            break;
        }
        memset(buf,0,node_len);
again_1:
        read_size = read(list_fd,buf,node_len);
        if(read_size == 0)
        {
            if(errno == EINTR)
            {
                goto again_1;
            }
            else
            {
                printf("check_node_list:read node_list finished!\n");
                break;
            }
        }
    }
    //printf("flag = %d\n",flag);
    if(flag == 0)
    {//not found node
        if(buf != NULL)
        {
            free(buf);  
        }
        return list_fd;
    }
    else
    {//already in node list flag == 1
        if(buf != NULL)
        {
            free(buf);  
        }
        close(list_fd);
        return 1;
    }
}

int check_node_list(char* client_ipv6addr)
{

    printf("check node list...\n");
    umask(0);
    int list_fd = open("./NodeList/nodelist.data",O_CREAT|O_RDWR,0777);
    if(list_fd == -1)
    {
        printf("check_node_list err:open nodelist.data failed!\n");
        return -1;
    }
    Nodeinfo *node = NULL;
    int node_len = sizeof(*node);
    //printf("node_len:%d\n",node_len);
    char *buf = NULL;
    buf  = (char*)malloc(node_len);
    if(buf == NULL)
    {
        printf("check_node_list err : molloc buf err!\n");
        close(list_fd);
        return -2;
    }
    int read_size = 0;
    int flag = 0;//flag = 0 not having this node flag = 1 this node already have
    //read_size = read(list_fd,buf,node_len); 

again:
    read_size = read(list_fd,buf,node_len); 

    if(read_size == 0)
    {
        if(errno == EINTR)
        {
            goto again;
        }
        else
        {
            printf("check_node_list:node_list empty!\n");
            if(buf != NULL)
            {
                free(buf);  
            }
            flag = 0;
            return list_fd;
        }
    }
    int i = 1;
    //printf("readsize:%d\n",read_size);
    while(read_size == node_len)
    {
        node = (Nodeinfo*)buf;
        printf("node %d:ipv6_addr:%s blue_addr:%s \n",i++,node->ipv6_addr,node->blue_addr);
        //printf("client_ipv6addr:%s\n",client_ipv6addr);
        //if( strcmp(node->ipv6_addr,client_ipv6addr) == 0)
        {
            flag = 1;
          //  break;
        }
        memset(buf,0,node_len);
again_1:
        read_size = read(list_fd,buf,node_len);
        if(read_size == 0)
        {
            if(errno == EINTR)
            {
                goto again_1;
            }
        }
    }
     printf("check_node_list:read node_list finished!\n");

    //printf("flag = %d\n",flag);
    if(flag == 0)
    {//not found node
        if(buf != NULL)
        {
            free(buf);  
        }
        return list_fd;
    }
    else
    {//already in node list flag == 1
        if(buf != NULL)
        {
            free(buf);  
        }
        close(list_fd);
        return 1;
    }
}

int add_new_node(char* ipv6addr,char* blueaddr,int datafd)
{
   printf("add new node...\n");
   if(ipv6addr == NULL)
   {
        printf("add_new_node err:ipv6addr = NULL!\n");
        return -1;
   }
   if(blueaddr == NULL)
   {
        printf("add_new_node err:blue = NULL!\n");
        return -2;
   }
   if(datafd < 0)
   {
        printf("add_new_node err: datafd < 0!\n");
        return -3;
   }
   Nodeinfo node;
   int node_len = sizeof(node);
   int res = init_node_info(&node);
   if(res < 0)
   {
       printf("addr_new_node err: init_node_info!\n");
       return -4;
   }
   res = set_node_info(&node,ipv6addr,blueaddr);
   if(res < 0)
   {
        printf("add_new_node err: set_node_info err!\n");
        return -5;
   }
   res = lseek(datafd,0,SEEK_END);
   res = write(datafd,(char*)(&node),node_len); 
   if(res != node_len)
   {
        printf("add_new_node err: write err!\n");
        return -6;
   }
   return 0;
}

int process_req_join(int client,dtn_req_msg* msg,struct sockaddr_rc rem_addr)
{//type = 0 new join node
    printf("process new join node...\n");
    if(msg == NULL)
    {
        printf("process_req_join err: msg = NULL!\n");
        return -1;
    }
    if(client < 0)
    {
        printf("process_req_join err: client< 0!\n");
        return -2;
    }
    char ipv6addr_buf[IPv6_ADDR_LEN] = {0};
    char blueaddr_buf[BLUE_ADDR_LEN] = {0};
    int res = get_local_ipv6addr(ipv6addr_buf);
    if(res < 0)
    {
        printf("process_req_join err: get_local_ipv6addr err!\n");
        return -3;
    }
    if((strcmp(ipv6addr_buf,msg->dest_ipv6_addr)) == 0)
    {//the dest is server
     //add this node in system
        int list_fd = check_node_list(msg->source_ipv6_addr);
        if(list_fd == -3)
        {
            printf("node already in system!\n");
            return 0;
        }
        else if(list_fd < 0)
        {
            printf("process_req_join err: check_node_list!\n");
            return -4;
        }
        res = get_blue_addr(rem_addr,blueaddr_buf);
        if(res < 0)
        {
            printf("process_req_msg err: get_blue_addr err!\n");
            return -5;
        }
        printf("check node list finished!\n");
        res = add_new_node(msg->source_ipv6_addr,blueaddr_buf,list_fd);
        close(list_fd);
        if(res < 0)
        {
            printf("process_req_msg err: add_new_node err!\n");
            return -7;
        }
        printf("add new node success!\n");
        return 0;
    }
}

int check_dest_addr(char* dest_ipv6_addr)
{
    char local_ipv6_buf[IPv6_ADDR_LEN] = {0};
    int res = get_local_ipv6addr(local_ipv6_buf);
    if(res < 0)
    {
        printf("check_dest_addr err:get_local_ipv6addr err!\n");
        return -1;
    }
    if( (strcmp(local_ipv6_buf,dest_ipv6_addr)) == 0 )
    {
        return 0;
    }
    else
    {
        return -2;
    }
}

int send_msg(int client,dtn_req_msg *msg)
{
    if(client < 0)
    {
        printf("send_msg err:client < 0!\n");
        return -1;
    }
    if(msg == NULL)
    {
        printf("send_msg err: msg = NULL\n");
        return -2;
    }
    int wr_len = 0; 
again:
    wr_len = write(client,(char*)msg,sizeof(dtn_req_msg));
    if(wr_len == -1)
    {
        if(errno == EINTR)
        {
            goto again;
        }
        else
        {
            printf("send_msg err:write err!\n");
            return -3;
        }
    }
    return 0;
}

int send_node_list(int client,char* dest_ipv6_addr)
{
    if(client< 0)
    {
        printf("send_node_list err:client < 0!\n");
        return -1;
    }
    if(dest_ipv6_addr == NULL)
    {
        printf("send_node_list err:dest_ipv6_addr = NULL!\n");
        return -2;
    }
    list_info *list = NULL;
    int res = get_list_info(&list);
    if(res < 0)
    {
        printf("send_node_list err: get_list_info err!\n");
        return -3;
    }
    dtn_req_msg msg;
    msg.version = 1;
    msg.type = 3;
    res = get_local_ipv6addr(msg.source_ipv6_addr);
    if(res < 0)
    {
        printf("send_node_list err: get_loacl_ipv6addr err!\n");
        return -4;
    }
    strncpy(msg.dest_ipv6_addr,dest_ipv6_addr,IPv6_ADDR_LEN);
    res = get_tstamp(&msg.create_time);
    if(res < 0)
    {
        printf("send_node_list err:get_tstamp err!\n");
        return -5;
    }
    msg.life_time = 86400;
    strncpy(msg.data_name,"server_list",sizeof("server_list")); 
    msg.data_len = get_list_len(list);
    if(msg.data_len < 0)
    {
        printf("send_node_list err:list_len < 0!\n");
        return -6;
    }
    //send msg
    res = send_msg(client,(dtn_req_msg*)&msg);
    if(res < 0)
    {
        printf("send_node_list err:send_msg err!\n");
        return -7; 
    }
    //send list data
    res = send_list_info(client,list);
    if(res < 0)
    {
        printf("send_node_list err:send_list_info err!\n");
        return -8;
    }
    return 0;
}

int process_req_list(int client,dtn_req_msg* msg)
{//send list to the request node 
    printf("process_req_list...\n");
    //printf("request node ipv6 addr:%s\n",msg->source_ipv6_addr);
    int res = 0;
    if(client < 0)
    {
        printf("process_req_list err: client < 0!\n");
        return -1;
    }
    if(msg == NULL)
    { 
        printf("process_req_list: msg = NULL!\n");
        return -2;
    }
    
    res = check_dest_addr(msg->dest_ipv6_addr);
    if(res < 0)
    {
        printf("process_req_list err:check_dest_addr err!\n");
        return -3;
    }
    res = check_node_list(msg->source_ipv6_addr); 
    if(res != 1)
    {
        printf("process_req_list: node not in system!\n");
        return -4;
    }
    res = send_node_list(client,msg->source_ipv6_addr);
    if(res < 0)
    {
        printf("process_req_list:send_info_list err!\n");
        return -5;
    }
    printf("process_req_list finished...\n");
    return 0;
}

int mysend(int sockfd,char* buf,int len)
{
    if(sockfd < 0 || buf == NULL || len < 0)
    {
        printf("mysend err:sock < 0 || buf = NULL || len < 0!\n");
        return -1;
    }
    int send_len = 0;
again:
    send_len = send(sockfd,buf,len,0);
    if(send_len == -1)
    {
        if(errno == EINTR)
        {
            goto again;
        }
        else
        {
            printf("mysend err:send err!\n");
            return -2;
        }
    }
    return send_len;
}

int myrecv(int sockfd,char *buf,int len)
{
    if(sockfd < 0|| buf == NULL || len < 0 )
    {
        printf("myrecv err:sockfd<0||buf=NULL||len<0!\n");
        return -1;
    }
    int recv_len = 0;
again:
    recv_len = recv(sockfd,buf,len,0);
    if(recv_len == -1)
    {
        if(errno == EINTR)
        {
            goto again;
        }
        else
        {
            printf("myrecv err:recv err recv_len == -1!\n");
            return -2;
        }
    }
    return recv_len;
}

int process_make_friend(int server,dtn_req_msg *msg)
{
    if(msg == NULL)
    {
        printf("process_make_friend err:msg = NULL!\n");
        return -1;
    }
    printf("Make a new friend? y/n\n");
    char c;
    scanf("%c",&c);
    if(c == 'y')
    {//send request
       printf("sending make friend info...\n");
       int send_len = mysend(server,(char*)msg,sizeof(*msg));
       if(send_len < 0)
       {
           printf("process_make_friend err:mysend err!\n");
           return -2;
       }
    }
    else
    {
        return -3;
    }
    printf("send make friend info success!\n");
    return 0;
}

int init_dtn2_msg(dtn_req_msg *msg)
{//init send make friend msg
    if(msg == NULL)
    {
        printf("init_dtn7_msg err:msg = NULl!\n");
        return -1;
    }
    msg->type = 2;
    int res = get_local_ipv6addr(msg->source_ipv6_addr);
    if(res < 0)
    {
        printf("init_dtn7_msg err:get_ipv6_addr err!\n");
        return -2;
    }
    res = get_tstamp(&msg->create_time);
    if(res < 0)
    {
        printf("init_dtn7_msg err:get_tstamp err!\n");
    }
    msg->life_time = DAY_SECONDS;
    memset(msg->dest_ipv6_addr,0,IPv6_ADDR_LEN);
    memset(msg->data_name,0,255);
    msg->data_len = 0;
    return 0;
}

int init_dtn_msg(dtn_req_msg *msg)
{
    if(msg == NULL)
    {
        printf("init_dtn_msg err:msg = NULL!\n");
    }
    msg->type = -1;
    memset(msg->source_ipv6_addr,0,IPv6_ADDR_LEN);
    memset(msg->dest_ipv6_addr,0,IPv6_ADDR_LEN);
    msg->create_time = 0;
    msg->life_time = 0;
    memset(msg->data_name,0,255);
    msg->data_len = 0;
    return 0;
}

int init_dtn3_msg(int server,dtn_req_msg *msg,int fd)
{
    if(server < 0 || msg == NULL || fd < 0)
    {
        printf("init_dtn3_msg err:ser<0||msg=NULL||fd<0!\n");
        return -1;
    }
    int res = init_dtn_msg(msg);
    if(res < 0)
    {
        printf("init_dtn3_msg err: init_dtn_msg err!\n");
        return -2;
    }
    res = myrecv(server,(char*)msg,sizeof(*msg));
    if(res < 0)
    {
        printf("init_dtn3_msg:myrecv err!\n");
        return -3;
    }
    msg->data_len = fd;//use data_len as file_fd
    return 0;
}

int init_dtn4_msg(dtn_req_msg *msg,char* dest_blue_addr)
{//init require server list msg
    if(msg == NULL)
    {
        printf("init_dtn4_msg err: msg = NULL!\n");
        return -1;
    }
    int res = init_dtn_msg(msg);
    if(res < 0)
    {
        printf("init_dtn3_msg err: init_dtn_msg err!\n");
        return -2;
    }
    msg->type = 4;
    res = get_local_ipv6addr(msg->source_ipv6_addr);
    if(res < 0)
    {
        printf("init_dtn4_msg err:get_loc_ipv6addr err!\n");
        return -3;
    }
    res =check_node_list_blue(dest_blue_addr,msg->dest_ipv6_addr);
    if(res != 1)
    {
        printf("init_dtn4_msg err:get dest ipv6 addr err!\n");
        return -4;
    }
    res = get_tstamp(&msg->create_time);
    if(res < 0)
    {
        printf("init_dtn4_msg err:get_tstamp err!\n");
        return -5;
    }
    msg->life_time = DAY_SECONDS;
    memset(msg->data_name,0,255);
    msg->data_len = 0;
    return 0;
}

int init_dtn5_msg(int server,dtn_req_msg *msg)
{//recv list dtn_msg from server
    if(server < 0 || msg == NULL)
    {
        printf("init_dtn5_msg err:server<0 || msg=NULL!\n");
        return -1;
    }
    int recv_len = 0;
    recv_len = myrecv(server,(char*)msg,sizeof(*msg));
    if(recv_len < 0)
    {
        printf("init_dtn5_msg:myrecv err!len=%d\n",recv_len);
        return -1;
    }
    return 0;
}

int process_send_reqlist(int server,dtn_req_msg *msg)
{//send requset to server to get list
    if(server < 0 || msg == NULL )
    {
        printf("pro_send_reqlist err:server<0 || msg = NULL");
        return -1;
    }
    printf("sending requset list to server...\n");
    int res = mysend(server,(char*)msg,sizeof(*msg));
    if(res < 0)
    {
        printf("pro_send_reqlist err: mysend err!\n");
        return -2;
    }
    printf("sending request list to server success!\n");
    return 0;
}

int process_add_new_friend(dtn_req_msg *msg,char *blue_addr)
{
    printf("process add new friend...\n");
    if(msg == NULL || blue_addr == NULL)
    {
        printf("prc_add_friend err:msg=NULL||addr=NULL!\n");
        return -1;
    }
    int filefd = 0; 
    if(msg->data_len > 0)
    {
        filefd = msg->data_len;
    }
    else
    {
        printf("pro_add_friend err:filefd(msg->len) <= 0\n!");
        return -2;
    }
    int res = 0;
    res = add_new_node(msg->source_ipv6_addr,blue_addr,filefd);
    if(res < 0)
    {
        printf("pro_add_friend:add_new_node err!res=%d\n",res);
        return -3;
    }
    printf("add new friend success!\n");
    return 0;
}

int process_recv_list(int server,dtn_req_msg* msg,
                      list_info **outlist/*out*/)
{
    if(msg == NULL)
    {
        printf("peo_recv_list:msg=NULL!\n");
        return -1;
    }
    int res = printf_dtn_msg(msg);
    if(res < 0)
    {
        printf("pro_recv_list:printf_msg err!res=%d\n",res);
        return -2;
    }
    //list info
    char* data = (char*)malloc(sizeof(list_data));
    if(data == NULL)
    {
        printf("pro_recv_list:malloc data err!\n");
        return -3;
    }
    int data_len = sizeof(list_data);
    list_data *dir_data = NULL;
    //recv first data
    int recv_len = 0; 
    recv_len = myrecv(server,data,data_len);
    if(recv_len < 0)
    {
            printf("pro_recv_list:read list data err!recv_len=%d\n",recv_len);
            if(data != NULL)
                free(data);
            return -4;
    }
    dir_data = (list_data*)data;
    int child_num = dir_data->child_dir_num;
    list_info *list = NULL;
    if(child_num > 0)
    {
        list = (list_info*)malloc(sizeof(list_info));
        if(list == NULL)
        {
            printf("pro_recv_list:malloc list err!\n");
            return -2;
        }
        memset(list,0,sizeof(list_info));
        list->parent_dir_num = child_num;
        list->parent = (char**)malloc(child_num*sizeof(char*));
        list->child = (char***)malloc(child_num*sizeof(char**));
        list->child_dir_num = (int*)malloc(child_num*sizeof(int));
        if(list->parent == NULL || list->child == NULL||list->child_dir_num == NULL)
        {
            printf("pro_recv_list:malloc list para err!\n");
            free_list(list);
            return -2;
        }
    }
    else
    {
        printf("list empty!\n");
        return -5;
    }
    //printf("******%s******\n",dir_data->dir_name);//just"list"
    int i = 0,j = 0;
    for(i = 0;i < child_num;i++)
    {
        list->parent[i] = (char*)malloc(255*sizeof(char));
        if(list->parent[i] == NULL)
        {
            //printf("pro_recv_list err:malloc list err!\n");
            free_list(list);
        }
        memset(list->parent[i],0,255);
        list->child_dir_num[i] = 0;
    }
    list_data *child_dir = NULL;
    char dir_name[255] = {0};
    char *child_data = (char*)malloc(data_len);
    //child_data : child_dir_name and its children
    if(child_data == NULL)
    {
        printf("pro_recv_list:malloc child_data err!\n");
        return -2;
    }
    for(i = 0;i < dir_data->child_dir_num;i++)
    {
        memset(child_data,0,data_len); 
        recv_len = myrecv(server,child_data,data_len);
        if(recv_len < 0)
        {
            //printf("pro_recv_list:my_read err!\n");
            goto free;
            return -6;
        }
        child_dir = (list_data*)child_data;
        //printf("%s\n",child_dir->dir_name);
        strncpy(list->parent[i],child_dir->dir_name,255);
        //child_dir_name like "picture" 
        if(child_dir->child_dir_num > 0)
        {//grandson dir like "aaaaa.mp3"
            list->child_dir_num[i] = child_dir->child_dir_num;
            list->child[i] = (char**)malloc(child_dir->child_dir_num*sizeof(char*));
            if(list->child[i] == NULL)
            {
                printf("pro_recv_list:malloc list->child[i] err!\n");
                goto free;
                return -2;
            }
            for(j = 0;j < child_dir->child_dir_num;j++)
            {
                recv_len = myrecv(server,dir_name,255);
                if(recv_len < 0)
                {
                    printf("pro_recv_list my_recv err!\n");
                    goto free;
                    return -7;
                }
                list->child[i][j] = (char*)malloc(255*sizeof(char));
                if(list->child[i][j] == NULL)
                {
                    printf("pro_recv_list:malloc child[i][j] err\n");
                    goto free;
                    return -2;
                }
                //printf("   %s\n",dir_name);
                strncpy(list->child[i][j],dir_name,255);
                memset(dir_name,0,255);
            }
        }
        else
        {
            //printf("   empty\n");
            list->child[i] = NULL;
        }
    }
    *outlist = list;
    if(data != NULL)
    {
        free(data);
    }
    if(child_data != NULL)
    {
        free(child_data);
    }
    return 0;
free:
    if(list != NULL)
    {
        free_list(list);
    }
    if(data != NULL)
    {
        free(data);
    }
    if(child_data != NULL)
    {
        free(child_data);
    }
    return -1;
}

int init_dtn6_msg(dtn_req_msg *msg,char *blueaddr)
{
    if(msg == NULL || blueaddr == NULL)
    {
        printf("init_dtn6_msg err:server < 0 || msg = NULL!\n");
        return -1;
    }
    int res = init_dtn_msg(msg);
    if(res < 0)
    {
        printf("init_dtn6_msg err: init_dtn_msg err!\n");
        return -2;
    }
    msg->type = 6;
    res = get_local_ipv6addr(msg->source_ipv6_addr);
    if(res < 0)
    {
        printf("init_dtn6_msg err:get_loc_ipv6addr err!\n");
        return -3;
    }
    res =check_node_list_blue(blueaddr,msg->dest_ipv6_addr);
    if(res != 1)
    {
        printf("init_dtn6_msg err:get dest ipv6 addr err!\n");
        return -4;
    }
    res = get_tstamp(&msg->create_time);
    if(res < 0)
    {
        printf("init_dtn6_msg err:get_tstamp err!\n");
        return -5;
    }
    msg->life_time = DAY_SECONDS;
    strncpy(msg->data_name,"data_info",10);
    msg->data_len = 10;
    return 0;
}

int init_data_info(data_info *info)
{
    if(info == NULL)
    {
        printf("init_data_info:info = NULL!\n");
        return -1;
    }
    memset(info->parent_name,0,255);
    memset(info->data_name,0,255);
    info->data_len = 0;
    return 0;
}

int set_data_info(data_info *info,list_info *list)
{
    if(info == NULL || list == NULL)
    {
        printf("set_data_info:para err!\n");
        return -1;
    }
    int a = 0, b = 0;
again:
    //printf_list(list);
    printf("Please input your select data(-1 = quit!):\n");
    printf("dir  num:");
    scanf("%d",&a);
    printf("data num:");
    scanf("%d",&b);
    if(a == -1)
    {
        return 1;
    }
    int p_num = list->parent_dir_num;
    int c_num = list->child_dir_num[a-1];
    if( a < 0|| a == 0 || a > p_num || b < 0 || b == 0 || b>c_num )
    {
        printf("Input dir num err!Please try again!\n");
        goto again;
    }
    strncpy(info->parent_name,list->parent[a-1],255);
    strncpy(info->data_name,list->child[a-1][b-1],255);
    info->data_len = 0;
    return 0;
}

int process_req_data(int server,dtn_req_msg *msg,list_info *list,
                    char *blueaddr)
{
    if(server < 0 || msg == NULL || list == NULL)
    {
        printf("pro_req_data:para err!\n");
        return -1;
    }
    init_dtn6_msg(msg,blueaddr);
    int res = mysend(server,(char*)msg,sizeof(dtn_req_msg));
    if(res != sizeof(dtn_req_msg))
    {
        printf("pro_req_data:mysend err!\n");
        return -2;
    }
    data_info info;
    res = init_data_info(&info);
    if(res < 0)
    {
        printf("pro_req_data:init_data_info err:res=%d!\n",res);
        return -3;
    }
    res = set_data_info(&info,list);
    if(res == 1)
    {
        return 1;
    }
    if(res < 0)
    {
        printf("pro_req_data:set_data_info err!\n");
        return -4;
    }
    res = mysend(server,(char*)(&info),sizeof(data_info));
    if(res != sizeof(data_info))
    {
        printf("pro_req_data:mysend err:res=%d!\n",res);
        return -5;
    }
    return 0;
}

int process_send_data(int client,dtn_req_msg* msg)
{
	printf_dtn_msg(msg);
	data_info data;
	int res = myrecv(client,(char*)(&data),sizeof(data_info));
	if(res != sizeof(data_info))
	{
		printf("receive req data info err!\n");
		return -1;
	}
	printf("%s requset file %s:%s\n",msg->source_ipv6_addr,data.parent_name,data.data_name);
	char path[255] = "./Data/";
    strncpy(path+strlen(path),data.parent_name,
            strlen(data.parent_name));
    strncpy(path+strlen(path),"/",1);
    strncpy(path+strlen(path),data.data_name,
            strlen(data.data_name));
    path[strlen(path)] = '\0';
    printf("path:%s\n",path);
    int fd = -1;
    fd = open(path,O_RDWR,0777);
    if(fd == -1)
    {
        printf("pro recv data:open %s err!\n",path);
        return -2;
    }
	int file_len =lseek(fd,OFFSET,SEEK_END);
	if(file_len < 0)
	{
		return -3;
	}
	lseek(fd,OFFSET,SEEK_SET);
	data.data_len = file_len;
	int res = mysend(clent,(char*)(&data),sizeof(data_info));
	int read_len = 0;
	char buf[READ_SIZE] = {0};
	read_len = read(fd,buf,READ_SIZE);
	while(read_len > 0)
	{
		mysend(client,buf,read_len);
		print("sending size:%d\n",read_len);
		read_len = 0;
		memset(buf,0,READ_SIZE);
		read_len = read(fd,buf,READ_SIZE);
	}
	return 0;
}

int process_recv_data(int server,dtn_req_msg *msg)
{
    printf("Receiving data...\n");
    if(server < 0 || msg == NULL)
    {
        printf("pro_recv_data: para err!\n");
        return -1;
    } 
    int res = -1;
    data_info data;
   
    res = init_data_info(&data);
    if(res < 0)
    {
        printf("pro_recv_data:init_data_info err!\n");
        return -2;
    }
    res = myrecv(server,(char*)&data,sizeof(data));
    if(res < 0)
    {
        printf("pro_recv_data:myrecv err!\n");
        return -2;
    }
    printf("\n");
    printf("****************recv data*******************\n");
    printf("%s:%s  len = %d\n",data.parent_name,data.data_name,data.data_len);
    printf("********************************************\n");
    printf("\n");
    if(res < 0)
    {
        printf("pro_recv_data:myrecv data info err!\n");
        return -3;
    }
    char path[255] = "./Data/";
    strncpy(path+strlen(path),data.parent_name,
            strlen(data.parent_name));
    strncpy(path+strlen(path),"/",1);
    strncpy(path+strlen(path),data.data_name,
            strlen(data.data_name));
    path[strlen(path)] = '\0';
    printf("path:%s\n",path);
    int fd = -1;
    fd = open(path,O_CREAT|O_RDWR,0777);
    if(fd == -1)
    {
        printf("pro recv data:open %s err!\n",path);
        return -4;
    }
    int total = data.data_len;
    char buf[READ_SIZE] = {0};
    int recv_len = 0;
    recv_len = myrecv(server,buf,READ_SIZE);
    if(recv_len < 0)
    {
        printf("pro_recv_data:myrecv err!\n");
        close(fd);
        return -4;
    }
    while(recv_len > 0)
    {
        printf("recv_len:%d\n",recv_len);
       if(recv_len == 8)
       {
           if(strcmp(buf,"Success") == 0)
           {
                goto close;
           }
       }
       res = my_write(fd,buf,recv_len);
       if(res != recv_len)
        {
            printf("pro_recv_data:my_write err!res=%d",res);
            close(fd);
            return -5;
        }   
        memset(buf,0,READ_SIZE);
       
        recv_len = myrecv(server,buf,READ_SIZE);
        if(recv_len < 0)
        {
            printf("pro_recv_data:myrecv last err!res=%d",res);
            close(fd);
            return -6;
        }  
    }
close:
    close(fd);
    printf("Recv data successed!\n");
    return 0;
}

int init_dtn11_msg(dtn_req_msg *msg,char *blueaddr)
{
    if(msg == NULL || blueaddr == NULL)
    {
        printf("init_dtn11_info:para err!\n");
        return -1;
    }
    int res = init_dtn_msg(msg);
    if(res < 0)
    {
        printf("init_dtn6_msg err: init_dtn_msg err!\n");
        return -2;
    }
    msg->type = 11;
    res = get_local_ipv6addr(msg->source_ipv6_addr);
    if(res < 0)
    {
        printf("init_dtn6_msg err:get_loc_ipv6addr err!\n");
        return -3;
    }
    res =check_node_list_blue(blueaddr,msg->dest_ipv6_addr);
    if(res != 1)
    {
        printf("init_dtn6_msg err:get dest ipv6 addr err!\n");
        return -4;
    }
    res = get_tstamp(&msg->create_time);
    if(res < 0)
    {
        printf("init_dtn6_msg err:get_tstamp err!\n");
        return -5;
    }
    msg->life_time = DAY_SECONDS;
    strncpy(msg->data_name,"quit",5);
    msg->data_len = 0;
    return 0;
}

int process_node_quit(int server,dtn_req_msg *msg,char *blueaddr)
{
    if(server < 0 || msg == NULL || blueaddr == NULL)
    {
        printf("pro_node_quit:para err!\n");
        return -1;
    }
    int res = init_dtn11_msg(msg,blueaddr);
    if(res < 0)
    {
        printf("pro_node_quit:init_dtn11_msg err! res=%d\n",res);
        return -2;
    }
    res = mysend(server,(char*)msg,sizeof(dtn_req_msg));
    if(res < 0)
    {
        printf("pro_node_quit:mysend err!\n");
        return -3;
    }
    return 0;
}
