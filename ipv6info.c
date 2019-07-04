#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int get_local_ipv6addr(char *buf/*out*/)
{//success return 0 and local ipv6 addr save in buf  failed return -1
    if(NULL == buf)
    {
        printf("get_local_ipv6info err:buf == NULL\n");
        return -1;
    }
    /*
    struct sockaddr_in6 local_addr;
    bzero(&local_addr,sizeof(local_addr));
    local_addr.sin6_addr = in6addr_any;
    inet_ntop(AF_INET6,&local_addr.sin6_addr,buf,sizeof(buf));
    return 0;
    */
    strncpy(buf,"fe80::961:7aed:d92a:7119",sizeof("fe80::961:7aed:d92a:7119"));
    return 0;
}

/*
int main()
{
    char buf[46];
    int res = get_local_ipv6addr(buf);
    if(res == -1)
    {
        printf("get addr err!\n");
        return -1;
    }
    printf("IPv6 ADDR:%s\n",buf);
    printf("hello\n");
    return 0;
}
*/
