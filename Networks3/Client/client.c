#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#define LENGTH 512

void zoom(int wid, int z)
{
  // printf("wid = %d zl = %d\n",wid,z);
  char str[100],num[100],splitnum[100];
  int i;
  sprintf(str,"xdotool windowactivate --sync %d",wid);
  system(str);
  usleep(10000);
  sprintf(num,"%d",z);
  for(i=0;num[i];i++)
  {
    splitnum[2*i]=num[i];
    splitnum[2*i+1]=' ';
  }
  splitnum[2*i]=0;
  sprintf(str,"xdotool key --delay 100 ctrl+f Tab Tab Tab Tab Tab %s Return Escape", splitnum);
  system(str);
}
void window_resize(int wid, int ht, int width)
{
  char str[100],h[100],splitht[100],w[100],splitwidth[100];
  int i;
  sprintf(str,"xdotool windowactivate --sync %d",wid);
  system(str);
  usleep(10000);
  system("xdotool key ctrl+super+Down");
  usleep(10000);
  sprintf(str,"xdotool getactivewindow windowsize %d%% %d%%",width,ht);
  // printf("%s\n", str);
  system(str);
}
void goto_page(int wid, int p)
{
  char str[100],num[100],splitnum[100];
  int i;
  sprintf(str,"xdotool windowactivate --sync %d",wid);
  system(str);
  usleep(10000);
  sprintf(num,"%d",p);
  for(i=0;num[i];i++)
  {
    splitnum[2*i]=num[i];
    splitnum[2*i+1]=' ';
  }
  splitnum[2*i]=0;
  sprintf(str,"xdotool key --delay 100 ctrl+g %s Return",splitnum);
  system(str);  
}

int main(int argc, char *argv[])
{
  int sockfd;
  int len;
  struct sockaddr_in address;
  struct hostent* server;
  char recbuf[LENGTH];
  //printf("%s\n",argv[1]);
  if(argc < 4)
    return 0;
  int result,portno = atoi(argv[2]);
  char ch = 'A';
  srand((unsigned int)time(NULL));
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd<0)
  {
    perror("ERROR opening socket");
    exit(1);
  }
  //bzero((char *) &server_address, sizeof(serv_address));
  address.sin_family = AF_INET;
  //address.sin_addr.s_addr = inet_addr("10.109.67.168");
  address.sin_addr.s_addr = inet_addr(argv[1]);
  address.sin_port = portno;
  bzero(&(address.sin_zero), 8);

  len = sizeof(address);
  
  result = connect(sockfd,(struct sockaddr*)&address,len);
  if(result==-1){
    perror("oops: client 1");
    exit(1);
  }
  //write(sockfd,&ch,1);
  printf("Client connected on %d and set to receive file\n",sockfd);
  // int sessId;
  // printf("Enter the session you want to connect (1-3):");
  // scanf("%d",&sessId);
  // char temp_str[10];
  // sprintf(temp_str,"%d",sessId);
  write(sockfd,argv[3],strlen(argv[3]));
  char fname[20];
  int rn = rand()%25;
  fname[0]=(char)(65+rn);
  fname[1]='\0';
  strcat(fname,".pdf");
  sleep(1);
  printf("Client will open %s\n",fname);
  FILE *fp = fopen(fname,"wb");
  if(fp==NULL){
    printf("Cannot write to file!");
    exit(0);
  }
  bzero(recbuf,LENGTH);
  int f_block_sz=0,success=0;
  while(success==0){
    printf("Writing!\n");
    //write(sockfd,&ch,1); 
    /* read(sockfd,&ch,1); */
    /* printf("From serv - %c\n",ch); */
    usleep(1000);
    while((f_block_sz = recv(sockfd,recbuf,LENGTH,0))>0){
      usleep(1000);
      // printf("Client recvs of size %d\n",f_block_sz);
      if(strcmp(recbuf,"end")==0){
        printf("Got out via end\n");
        break;
      }
      if(f_block_sz<0){
        printf("receive file error\n");
        break;
      }
      else if(f_block_sz==0){
        printf("Connection ended!\n");
        break;
      }
      int write_sz=fwrite(recbuf,sizeof(char),f_block_sz,fp);
      if(write_sz<f_block_sz){
        printf("File write failed\n");
        break;
      }
      bzero(recbuf,LENGTH);
      
    }
    success=1;
    printf("Correctly written\n");
    fclose(fp);
  }
  if(!fork())
  {
    execlp("okular","okular",fname,NULL);
  }
  else
  {
    sleep(1);
    system("xdotool getactivewindow > out");
    sleep(1);
    
    FILE *fp = fopen("out","r");
    int id;
    fscanf(fp,"%d",&id);
    fclose(fp);
    int c=20;
    while(c--)
      {
                  
          int read_size=0; 
          char buf[100],str[100];
          //Receive a reply from the server
          if( (read_size=recv(sockfd , buf , 2000 , 0)) < 0)
          {
              puts("recv failed");
              exit(0);
          }
           
          puts("Server reply :");
          if(read_size==0)
            continue;
          buf[read_size]=0;
          puts(buf);
          int pNo,zL,h,w;
          sscanf(buf,"%d%d%d%d",&pNo,&zL,&h,&w);
          // printf("%s\n",str );
          goto_page(id,pNo);
          zoom(id,zL);
          window_resize(id,h,w);
          // if(strcmp(str,"zoom")==0)
          // {
          //   int s;
          //   sscanf(buf,"%s%d",str,&s);
          // }
          // else if(strcmp(str,"window")==0)
          // {
          //   int h,w;
          //   sscanf(buf,"%s%d%d",str,&h,&w);
          // }
          // else if(strcmp(str,"goto")==0)
          // {
          //   printf("hi\n");
          //   int p;
          //   sscanf(buf,"%s%d",str,&p);
          // }

      }  
  }
  return(0);
  
}
  
