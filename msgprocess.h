#include "blueinfo.h"
#include "DTNmsg.h"
#include "ipv6info.h"
#include "fileinfo.h"

#ifndef MSGPROCESS_H_
#define MSGPROCESS_H_
#define SERVER_BLUE_ADDR "90:00:4E:A7:10:55"
#define DAY_SECONDS 86400
typedef struct NODEINFO{
    char    ipv6_addr[IPv6_ADDR_LEN];
    char    blue_addr[BLUE_ADDR_LEN];
    time_t  create_tstamp;
}Nodeinfo;

int get_local_time(time_t* stamp,char* timebuf);
int get_dead_time(time_t* stamp,int life_time,char* timebuf);
int printf_dtn_msg(dtn_req_msg* msg);
int get_tstamp(time_t *stamp);
int init_node_info(Nodeinfo* node);
int set_node_info(Nodeinfo* node,char* ipv6addr,char* blueaddr);
int check_node_list_blue(char* dest_blue_addr,char* dest_ipv6_addr/*int out*/);
int check_node_list(char* client_ipv6addr);
int add_new_node(char* ipv6addr,char* blueaddr,int datafd);
int process_req_join(int client,dtn_req_msg *msg,struct sockaddr_rc rem_addr);
int check_dest_addr(char* dest_ipv6_addr);
int send_msg(int client,dtn_req_msg* msg);
int send_node_list(int client,char* dest_ipv6_addr);
int process_req_list(int client,dtn_req_msg* msg);
int mysend(int sockfd,char* buf,int len);
int myrecv(int sockfd,char* buf,int len);
int process_make_friend(int server,dtn_req_msg* msg);
int init_dtn_msg(dtn_req_msg *msg);
int init_dtn2_msg(dtn_req_msg* msg);
int init_dtn3_msg(int server,dtn_req_msg *msg,int fd);
int init_dtn4_msg(dtn_req_msg* msg,char* dest_blue_addr);
int init_dtn5_msg(int server,dtn_req_msg* msg);
int init_dtn6_msg(dtn_req_msg *msg,char *blueaddr); 
int process_send_reqlist(int server,dtn_req_msg *msg);
int process_add_new_friend(dtn_req_msg* msg,char* blue_addr);
int process_recv_list(int server,dtn_req_msg *msg,list_info **list);
int process_req_data(int server,dtn_req_msg *msg,list_info *list,char *blueaddr);
int process_recv_data(int server,dtn_req_msg *msg);
int init_dtn11_msg(dtn_req_msg *msg,char *blueaddr);
int process_send_data(int client,dtn_req_msg* msg);
int process_node_quit(int server,dtn_req_msg *msg,char* blueaddr);
#endif
