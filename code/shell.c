#include "disk.h"
#include <stdio.h>


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
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}


int
getcmd(char *buf, int nbuf)
{
  printf("ext2 file sys $ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}




int main(int argc, char* argv[]){
    static char buf[512]; //读入命令的缓冲区

    if(open_disk()) printf("file open\n"); //打开磁盘
    else {
        printf("cannot open file\n");
        return 0;
    }
    while(getcmd(buf, sizeof(buf)) >= 0){
        if(buf[0] == 'l'&&buf[1] == 's'){
            //列出目录下的内容
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
    


}