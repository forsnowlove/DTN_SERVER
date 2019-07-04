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


int process_dtn_msg(int client,struct sockaddr_rc rem_addr,dtn_req_msg *msg)
{
    int res = 0;
    if(client < 0)
    {
        printf("func process_dtn_msg err:client < 0!\n");
        exit(-3);
    }
    if(msg == NULL)
    {
        printf("func process_dtn_msg err: msg = NULL!\n");
        exit(-4);
    }
    if(msg->type == 0)
    {
        res = process_req_join(client,msg,rem_addr);
        if(res < 0)
        {
            printf("process_dtn_msg err: proces_req_join err!\n");
            exit(-5);
        }
    }
    if(msg->type == 4)
    {
        printf("pro:%d",msg->type);
        res = process_req_list(client,msg);
        if(res < 0)
        {
            printf("process_dtn_msg err: process_req_list err!\n");
            exit(-6);
        }
    }
    return 0;
}

int client_process(int client,struct sockaddr_rc rem_addr)
{   
    printf("process client...\n");
   
    dtn_req_msg *msg;
    msg = (dtn_req_msg*)malloc(sizeof(dtn_req_msg));
    if(msg == NULL)
    {
        printf("client_process err:malloc msg err!\n");
        return -1;
    }
    memset(msg,0,sizeof(*msg));
    int msg_len = sizeof(*msg);
    //addr num to str
    char blue_buf[BLUE_ADDR_LEN] = {0};
    int res = get_blue_addr(rem_addr,blue_buf);
    if(res < 0)
    {
        printf("client_peocess: get_blue_addr err!\n");
        return -2;
    }
    printf("Accepted connection from node blue addr: %s\n",blue_buf);

    //read data from client
   // char *buf = (char*)malloc(sizeof(dtn_req_msg));
    while(1)
    {
        memset(msg,0,sizeof(dtn_req_msg));
        int bytes_read = recv(client,msg,sizeof(dtn_req_msg),0);
        if(bytes_read > 0 )
        {
            //msg = (dtn_req_msg*)buf;
            process_dtn_msg(client,rem_addr,msg);
            //printf("received [%s] \n",buf);
        }
    }
    //test
    /*
    msg->version = 1;
    msg->type = 2;
   
    res = get_local_ipv6addr(msg->dest_ipv6_addr);//sent to server
    if(res < 0)
    {
        printf("client_peocess: get_local_ipv6addr err!\n");
        return -2;
    }
    char ipv6_source_buf[IPv6_ADDR_LEN] = {"fe80::961:7aed:d92a:7119"};//from node
    printf("Accept connection from node ipv6 addr:%s\n",ipv6_source_buf);
    strncpy(msg->source_ipv6_addr,ipv6_source_buf,IPv6_ADDR_LEN); 
    get_tstamp(&msg->create_time);
    msg->life_time = 86400;
    strncpy(msg->data_name,"test_data",sizeof("test_data"));
    msg->data_len = 100;
    */
    //process_dtn_msg(client,rem_addr,msg);
    printf("process client success!\n");
    return 0;
} 

int main01()
{
     check_node_list(NULL);

    struct sockaddr_rc loc_addr = {0};
    char local_addr[18] = {"90:00:4E:A7:10:55"};  
    loc_addr.rc_family = AF_BLUETOOTH;
    str2ba(local_addr,&loc_addr.rc_bdaddr);//set local addr
    loc_addr.rc_channel = 1;//set port
    //similar client 
    int res = mkfifo("test_fifo",0777);
    if(res == -1)
    {
        if(errno != EEXIST)
        {
            printf("create fifo err!\n");
            return -1;
        }
    }
    int fd = open("test_fifo",O_RDWR);
    if(fd == -1)
    {
        printf("open test_fifo err!\n");
        return -2;
    }
    client_process(fd,loc_addr);//test client = fd
    close(fd);
    printf("Test finished!\n");
    return 0;
}

int main()
{ Nodeinfo node;
 Nodeinfo node1;
int node_len = sizeof(node);

    char ipv6addr1[IPv6_ADDR_LEN];
    strncpy(ipv6addr1,"fe80::961:7aed:d92a:7119",sizeof("fe80::961:7aed:d92a:7119"));;//server
    char blue1[BLUE_ADDR_LEN];
    strncpy(blue1,"00:27:13:C9:42:6E",sizeof("00:27:13:C9:42:6E")); //server blue
    int datafd = open("./NodeList/nodelist.data",O_CREAT|O_RDWR,0777);
    int  res = set_node_info(&node,ipv6addr1,blue1);
  
    char *ipv6addr2 = "fe80::961:7aed:d92a:7118";//server
    char *blue2 = "90:00:4E:A7:10:55"; //server blue
    //int datafd = open("./NodeList/nodelist.data",O_CREAT|O_RDWR,0777);
    res = set_node_info(&node1,ipv6addr2,blue2);
    res = lseek(datafd,0,SEEK_END);
    res = write(datafd,(char*)(&node),node_len);
     res = lseek(datafd,0,SEEK_END);

    res = write(datafd,(char*)(&node1),node_len);
  close(datafd);
  printf("nema!");
    check_node_list(NULL);

    struct sockaddr_rc loc_addr = {0},rem_addr = {0};
    int s,client;
    unsigned int opt = sizeof(rem_addr);
    char local_addr[18] = {"00:27:13:C9:42:6E"};  
    //allocate socket
    s = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);
    
    //bind socket to port 1 of the first available bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    //loc_addr.rc_bdaddr = *BDADDR_ANY;
    str2ba(local_addr,&loc_addr.rc_bdaddr);//set local addr
    loc_addr.rc_channel = 1;//set port
    bind(s,(struct sockaddr*)&loc_addr,sizeof(loc_addr));

    //printf local addr
    memset(&local_addr,0,sizeof(local_addr));
    ba2str(&loc_addr.rc_bdaddr,local_addr);
    printf("local bluetooth addr:%s\n",local_addr);

    //put socket into listening mode 
    listen(s,1);
    printf("server running...\n");
    
    
    //set SIGCHILD process func--recycle child process
    if(signal(SIGCHLD,sig_child) == SIG_ERR)
    {
        printf("set sig_child func err!\n");
        exit(-1);
    }
    //process connect
    pid_t pid;
    while(1)
    {
        //rem_addr:to save connect addr
        memset(&rem_addr,0,sizeof(rem_addr));
        //accept
        client = accept(s,(struct sockaddr*)&rem_addr,&opt);
        if(client < 0)
        {
            if(errno = EINTR)
            {
                continue;//if process is interrupte try again
            }
            else
            {
                printf("accept err!\n");
                exit(-2);
            }
        }
        printf("new client!\n");
        if((pid = fork()) == 0)
        {//child process client
            close(s);//child not listen
            //process...
            client_process(client,rem_addr);
            //finish...
            close(client);
            exit(0);
        }
        //father
        close(client);//father not process
    }
    return 0;
}
    
