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


int main()
{

   Nodeinfo node;
   int node_len = sizeof(node);
   //int res = init_node_info(&node);
   int res = 0;
   if(res < 0)
   {
       printf("addr_new_node err: init_node_info!\n");
       return -4;
   }
   char *ipv6addr1 = "fe80::961:7aed:d92a:7119";//server
   char *blue1 = "00:27:13:C9:42:6E"; //server blue
    int datafd = open("./NodeList/nodelist.data",O_CREAT|O_RDWR,0777);

  //res = set_node_info(&node,ipv6addr1,blue1);
   strncpy(node.ipv6_addr,ipv6addr1,IPv6_ADDR_LEN);
    strncpy(node.blue_addr,blue1,BLUE_ADDR_LEN);
    get_tstamp(&node.create_tstamp);

   if(res < 0)
   {
        printf("add_new_node err: set_node_info err!\n");
        return -5;
   }
   res = lseek(datafd,0,SEEK_END);
   res = write(datafd,(char*)(&node),node_len);
   return 0;
}
