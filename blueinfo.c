#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "blueinfo.h"

int get_blue_addr(struct sockaddr_rc blue_addr,char* buf)
{
    if(buf == NULL)
    {
        printf("get_blue_addr err:buf = NULL\n");\
        return -1;
    }
    ba2str(&blue_addr.rc_bdaddr,buf);
    return 0;
}




