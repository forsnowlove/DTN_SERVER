#ifndef DTNMSG_H_
#define DTNMSG_H_
#define IPv6_ADDR_LEN 46
#define SERVER_IPv6_ADDR "fe80::961:7aed:d92a:7118"  
    typedef struct DTN_REQ_MSG{
        int     version;
        int     type;//0:recv msg: new node join system 
                     //1:send msg: node join success/node already join
                     //2:send msg: make friend 
                     //3:recv msg: add new friend
                     //4:require  list 
                     //5:recv server list
                     //6:require data
                     //7:recv data 
                     //8:make friend success
                     //9:save other nodes info
                     //10:send other nodes info
                     //11:disconnect 
        char    source_ipv6_addr[IPv6_ADDR_LEN];
        char    dest_ipv6_addr[IPv6_ADDR_LEN];
        time_t  create_time;//stamp
        int     life_time;//seconds
        char    data_name[255];
        int     data_len;
    }dtn_req_msg;
#endif

