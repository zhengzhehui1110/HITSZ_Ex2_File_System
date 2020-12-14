#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char*
gets(char *buf, int max)
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


int
getcmd(char *buf, int nbuf) //read a command line
{
  printf("EXT2 FILE SYS$");
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
    //printf("read: %x %x %x\n",disk_block_buf[0],disk_block_buf[1],disk_block_buf[2]);//test
    //printf("%x %x %x\n",buf[0x200],buf[0x201],buf[0x202]);//test
    return (flag==0)?0:-1; //if succeed return 0
}

int write_data_block(unsigned int datablock_num, char* buf){ //write a data block
    char disk_block_buf[DEVICE_BLOCK_SIZE];
    int flag = 0;
    memcpy(disk_block_buf,buf,DEVICE_BLOCK_SIZE);  //divide data block into 2 part
    flag += disk_write_block(datablock_num*2,disk_block_buf);//write the first disk block
    memcpy(disk_block_buf,buf+DEVICE_BLOCK_SIZE,DEVICE_BLOCK_SIZE);
    //printf("write: %x %x %x\n",disk_block_buf[0],disk_block_buf[1],disk_block_buf[2]);//test
    flag += disk_write_block(datablock_num*2+1,disk_block_buf); //write the second disk block
    return (flag==0)?0:-1; //if succeed return 0
}


void reset_disk(){  //reset file system
    close_disk();
    open_disk();
    create_disk();//reset all to zero
    char disk_buf[DEVICE_BLOCK_SIZE*2]; //data_block buf
    //memset(disk_buf,0,sizeof(disk_buf));
    //printf("%x %x %x\n",disk_buf[0x200],disk_buf[0x201],disk_buf[0x202]);//test
    if(read_data_block(0,disk_buf)==0){ //start reset super block
      //printf("%x %x %x\n",disk_buf[0x200],disk_buf[0x201],disk_buf[0x202]);//test
      sp_block *super_block_buf = (sp_block *)disk_buf; //super block struct
      super_block_buf->magic_num = MAGIC_NUM; //write magic num
      //TODO

      
      memset(super_block_buf->empty_map,0xffffffff,sizeof(super_block_buf->empty_map)); //fill empty map
      
      write_data_block(0,disk_buf);  //reset super block
      printf("reset finish!!\n");
    }
    else
    {
      printf("reset fail\n");
      exit(0);
    }
  
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
        //create_disk();
        reset_disk();
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
    
    

    //shell
    while(getcmd(buf, sizeof(buf)) >= 0){
        if(buf[0] == 'l'&&buf[1] == 's'){
            //列出目录下的内容
            printf("ls\n");
        }
        else if(buf[0] == 'm'&&buf[1] == 'k'&&
            buf[2] == 'd'&&buf[3] == 'i'&&
            buf[4] == 'r'){
                //在该目录下创建一个新的子目录
        }
        else if(buf[0] == 't'&&buf[1] == 'o'&&
            buf[2] == 'u'&&buf[3] == 'c'&&
            buf[4] == 'h'){
                //在该目录下创建一个新的文件
        }
        else if(buf[0] == 'c'&&buf[1] == 'p'){
            //复制文件
        }
        else if(buf[0] == 's'&&buf[1] == 'h'&&
            buf[2] == 'u'&&buf[3] == 't'&&
            buf[4] == 'd'&&buf[5] == 'o'&&
            buf[6] == 'w'&&buf[7] == 'n'){
                //关闭文件系统
        }
        else
        {
            printf("unknown command\n");
        }
        
    
    }
    return 0;
    
}
