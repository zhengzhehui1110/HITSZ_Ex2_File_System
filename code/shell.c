#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r'){
      break;
    }
  }
  buf[i] = '\0';
  return buf;
}

int getcmd(char *buf, int nbuf) //read a command line
{
  printf("EXT2 FILE SYS $");
  fflush(stdout);
  //putchar('$');
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int read_data_block(unsigned int datablock_num, char* buf){ //read a data block
    
    char disk_block_buf[DEVICE_BLOCK_SIZE];  //one data block include two disk blocks
    int flag = 0;
    flag += disk_read_block(datablock_num*2,disk_block_buf); //read the first disk block
    memcpy(buf,disk_block_buf,DEVICE_BLOCK_SIZE);
    
    flag += disk_read_block(datablock_num*2+1,disk_block_buf);//read the second disk block
    memcpy(buf+DEVICE_BLOCK_SIZE,disk_block_buf,DEVICE_BLOCK_SIZE);
    return (flag==0)?0:-1; //if succeed return 0
}

int write_data_block(unsigned int datablock_num, char* buf){ //write a data block
    char disk_block_buf[DEVICE_BLOCK_SIZE];
    int flag = 0;
    memcpy(disk_block_buf,buf,DEVICE_BLOCK_SIZE);  //divide data block into 2 part
    flag += disk_write_block(datablock_num*2,disk_block_buf);//write the first disk block
    memcpy(disk_block_buf,buf+DEVICE_BLOCK_SIZE,DEVICE_BLOCK_SIZE);
    flag += disk_write_block(datablock_num*2+1,disk_block_buf); //write the second disk block
    return (flag==0)?0:-1; //if succeed return 0
}

void reset_disk(){  //reset file system
    close_disk();
    open_disk();
    create_disk();//reset all to zero
    char disk_buf[DEVICE_BLOCK_SIZE*2]; //data_block buf
    if(read_data_block(0,disk_buf)==0){ //start reset super block
      sp_block *super_block_buf = (sp_block *)disk_buf; //super block struct
      super_block_buf->magic_num = MAGIC_NUM; //write magic num
      super_block_buf->dir_inode_count = 0; 
      super_block_buf->free_block_count = 4062; //2^12-1-1-32
      super_block_buf->free_inode_count = 1024;
      memset(super_block_buf->empty_map,0xffffffff,sizeof(super_block_buf->empty_map)); //fill empty map
      super_block_buf->block_map[0] = 0xffffffff;
      super_block_buf->block_map[1] = 0xC0000000; //34 data blocks are used for system
      write_data_block(0,disk_buf);  //reset super block
      printf("reset super block done\n");
    }
    else
    {
      printf("reset super block fail\n");
      exit(0);
    }
    struct inode inode_arr[32]; //each data block include 32 inode
    for(int i = 0; i < 32;i++){
      inode_arr[i].link = 0;  //no dir_item link to inode at first
      inode_arr[i].file_type = 0;
      inode_arr[i].size = 3; //every inode include 3 data blocks
    }
    uint32_t data_block_num = 34;
    for (int i = 2; i < 34; i++) //start reset inode array
    {
      for(int j = 0;j < 32;j++,data_block_num+=3){
        inode_arr[j].block_point[0] = data_block_num;  //give 3 data blocks to every inode
        inode_arr[j].block_point[1] = data_block_num+1;
        inode_arr[j].block_point[2] = data_block_num+2;
        inode_arr[j].block_point[3] = 0;
        inode_arr[j].block_point[4] = 0;
        inode_arr[j].block_point[5] = 0;
      }
      if(write_data_block(i,(char *)inode_arr)!= 0) //set inodes in data block i
      {
        printf("cannot set node array block %d\n",i);
        exit(0);
      }
    }
    printf("reset inode array done\n");

}

void read_super(){  //read super block
    char disk_buf[DEVICE_BLOCK_SIZE*2]; //data_block buf
    
    if(read_data_block(0,disk_buf)==0){
      
      sp_block *super_block_buf = (sp_block *)disk_buf;
      printf("succesfully read super block\n");
      if (super_block_buf->magic_num != MAGIC_NUM)
      {
        //magic num is broken
        //you should reset the file system
        printf("magic num is broken:%x,resetting file system...\n",super_block_buf->magic_num);
        reset_disk();
        printf("reset finish!\n");
      }
      read_data_block(0,disk_buf);
      super_block_buf = (sp_block *)disk_buf;
      printf("magic num:%x\n",super_block_buf->magic_num);
    }
    else
    {
      printf("cannot read super block\n");
      exit(0);
    }
}

void split_command(char *buf, char * word_list[512]){ //split command line into words
  char *p = buf;
  int i = 0;
  word_list[0] = 0;
  while(*p != '\n' && *p != '\r'){
    if(*p == ' '){
      *p = '\0';
    }
    p++;
  }
  *p = '\0';
  char *q = buf;
  while (q < p)
  {
    word_list[i] = q;
    i++;
    while(*q != '\0' && q < p){
      q++;
    }
    while(*q == '\0' && q < p){
      q++;
    }
  }
  word_list[i] = 0;
}

int split_path(char *path, char *path_list[MAX_PATH_DEPTH]){ //split path into dirs(and file name)
  char *p = path;
  int i = 0;
  while (*p != '\0')
  {
    if(*p == '/'){
      *p = '\0';
      path_list[i] = p+1;
      i++;
      if(i >= 20){
        printf("path overflow!\n");
        return -1;
      }
    }
    p++;
  }
  path_list[i] = 0;
  return 0;
}

// char * find_next_word(char *cur_word){
//   char *next_word = cur_word;
//   while (*next_word != '\0')
//   {
//     next_word++;
//   }
//   while(*next_word == '\0'){
//     next_word++;
//   }
//   return next_word;
// }

int main(int argc, char* argv[]){
    char buf[512]; //读入命令的缓冲区

    
    if(open_disk()==0) {
      printf("disk open\n"); //打开磁盘
    }
    else {
        printf("cannot open file\n");
        return 0;
    }
    //read super block
    read_super();
    //shell begin
    while(getcmd(buf, sizeof(buf)) >= 0){
        char * command_words[512];
        split_command(buf,command_words);
        if(strcmp(buf,"ls")==0){
            //列出目录下的内容
            printf("this is ls\n");

        }
        else if(strcmp(buf,"mkdir")==0){
                //在该目录下创建一个新的子目录
        }
        else if(strcmp(buf,"touch")==0){
                //在该目录下创建一个新的文件
        }
        else if(strcmp(buf,"cp")==0){
            //复制文件
        }
        else if(strcmp(buf,"shutdown")==0){
                //关闭文件系统
            printf("this is shutdown\n");
        }
        else
        {
            printf("unknown command\n");
            

        }
        memset(buf,'\0',sizeof(buf));  //clear command buf
    
    }
    return 0;
    
}
