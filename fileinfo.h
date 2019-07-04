#ifndef FILEINFO_H_
#define FILEINFO_H_
typedef struct LIST_INFO{
    char    **parent;
    int     parent_dir_num;
    char    ***child;
    int     *child_dir_num;
}list_info;

typedef struct LIST_DATA{
    char dir_name[255];
    int  child_dir_num;
}list_data;

typedef struct DATA_INFO{
    char parent_name[255];
    char data_name[255];
    int  data_len;
}data_info;

#define READ_SIZE 1008
int my_read(int fd,char* buf,int len);
int my_write(int fd,char* data,int len);
int init_list_data(list_data *data);
int send_list_info(int client,list_info* list);
int get_list_info(list_info **list/*in out*/);
int printf_list(list_info *list);
int free_list(list_info *list);
int get_list_len(list_info *list);
#endif
