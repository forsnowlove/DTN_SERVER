#include "stdio.h"
#include "stdlib.h"
#include "bluetooth/bluetooth.h"
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include <unistd.h>
#include <sys/socket.h>
#include "bluetooth/hci.h"
#include "bluetooth/hci_lib.h"

int register_service()
{
    uint32_t svc_uuid_int[] = { 0 , 0 , 0 , 0xABCD } ;
    uint8_t rfcomm_channel = 11 ;
    const char * service_name = "DTN_UUID_Server" ;
    const char * svc_dsc = "Info of DTN Server" ;
    const char * service_prov = "DTN Server" ;
    uuid_t root_uuid, l2cap_uuid , rfcomm_uuid , svc_uuid ,svc_class_uuid ;
    sdp_list_t  *l2cap_list = 0 ,
                *rfcomm_list = 0 ,
                *root_list = 0 ,
                *proto_list = 0 ,
                *access_proto_list = 0 ,
                *svc_class_list = 0 ,
                *profile_list = 0 ;
    sdp_data_t  *channel = 0 ;
    sdp_profile_desc_t profile ;
    sdp_record_t record = { 0 } ;
    sdp_session_t * session = 0 ;
    // set the general service ID
    sdp_uuid128_create(&svc_uuid , &svc_uuid_int) ;//set server UUID
    sdp_set_service_id( &record,svc_uuid);//set server UUID into SDP record
    // set the service class
    sdp_uuid16_create(&svc_class_uuid , SERIAL_PORT_SVCLASS_ID) ;//reserved serial_port_class
    svc_class_list = sdp_list_append(0 ,&svc_class_uuid) ;//server class list of UUID  
    //first: 0  create a new list
    sdp_set_service_classes(&record , svc_class_list ) ;//add server class list to SDP record
    // set the Bluetooth profile information
    sdp_uuid16_create(&profile.uuid,SERIAL_PORT_PROFILE_ID) ;//profile uuid
    profile.version = 0x0100 ;//profile version
    profile_list = sdp_list_append( 0 , &profile ) ;//profile list
    sdp_set_profile_descs(&record , profile_list ) ;//add profile to SDP record
    // make the service record publicly browsable  //gong kai infomation
    sdp_uuid16_create(&root_uuid , PUBLIC_BROWSE_GROUP ) ;
    root_list = sdp_list_append( 0 , &root_uuid ) ;
    sdp_set_browse_groups( &record , root_list);
    // set l2cap information
    sdp_uuid16_create(&l2cap_uuid,L2CAP_UUID ) ;
    l2cap_list = sdp_list_append( 0 , &l2cap_uuid ) ;
    proto_list = sdp_list_append( 0 , l2cap_list ) ;
    // register the RFCOMM channel for RFCOMM sockets
    sdp_uuid16_create(&rfcomm_uuid , RFCOMM_UUID ) ;
    channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel ) ;
    rfcomm_list = sdp_list_append( 0 , &rfcomm_uuid ) ;
    sdp_list_append( rfcomm_list , channel ) ;
    sdp_list_append( proto_list , rfcomm_list );
    access_proto_list = sdp_list_append( 0 , proto_list ) ;
    sdp_set_access_protos( &record , access_proto_list ) ;
    // set the name, provider, and description
    sdp_set_info_attr(&record , service_name , service_prov , svc_dsc ) ;
    // connect to the local SDP server, register the service record,
    // and disconnect
    //char* m_addr = "00:27:13:C9:42:6E";
    //bdaddr_t local_addr;
    //str2ba(m_addr,&local_addr);
    session = sdp_connect( BDADDR_ANY , BDADDR_LOCAL , SDP_RETRY_IF_BUSY ) ;
    //char addr[19] = {0};
    //ba2str(&local_addr,addr);
    //printf("%s\n",addr);
    //ba2str(BDADDR_LOCAL,addr);
    //printf("%s\n",addr);
    //sdp_connect_local(session);
    if(session == NULL)
    {
        //fprintf(stderr,"error code %d: %s\n",errno,strerror(errno));
        printf("session = NULL\n");
        return -1;
    }
    //printf("session\n");
    sdp_record_register(session, &record,0);
    //printf("11\n");
// cleanup
    sdp_data_free (channel) ;
    sdp_list_free (l2cap_list , 0 ) ;      
    sdp_list_free (rfcomm_list,0) ; 
    sdp_list_free (root_list , 0 ) ;
    sdp_list_free (access_proto_list , 0) ; 
    sdp_list_free (svc_class_list , 0 );
    sdp_list_free (profile_list , 0 ) ;
  
	sdp_session_t * session = register_service();
	sleep(100);
	printf("sdp works!\n");
	sdp_close(session);
	return 0;
}
int main ( )
{
    sdp_session_t * session = register_service();
    sleep ( 100 ) ;
    printf("sdp works!\n");
    sdp_close( session ) ;
    return 0 ;
}
