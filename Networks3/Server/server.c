#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define LENGTH 512


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

struct state
{
  int zL,pNo,ht,width;
};

int q[3][100];
int curr[3]; 
struct state stt[3];

//the thread function
void *connection_handler(void *);
void *cmd_line_interface(void *); 
int main()
{
  int server_sockfd,client_sockfd;
  int server_len, client_len;
  int *new_sock;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
  int result,id,c;
  fd_set readfds, testfds;
  char ch;
  char str[100];
  int clientfdlist[20];

  if((server_sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
  {
    printf("Socket was not created!\n");
    exit(2);
  }
  
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_address.sin_port = 0;
  bzero(&(server_address.sin_zero), 8);
  server_len = sizeof(server_address);
  
  if(bind(server_sockfd,(struct sockaddr*)&server_address,server_len)<0){
    printf("Bind failed!\n");
    return(0);
  }
  if(getsockname(server_sockfd, (struct sockaddr *) &server_address, &server_len) < 0 )
    {
      printf(" failed to get port number\n");
      exit(4);
    }
  printf("The assigned port is %d\n", server_address.sin_port);
  if(listen(server_sockfd,5)==-1)
    {
      perror("Listen error!\n");
      exit(1);
    }
  
  stt[0].pNo   = stt[1].pNo   = stt[2].pNo   = 1;
  stt[0].zL    = stt[1].zL    = stt[2].zL    = 100;
  stt[0].ht    = stt[1].ht    = stt[2].ht    = 100;
  stt[0].width = stt[1].width = stt[2].width = 100;
  curr[0]      = curr[1]      = curr[2]      = 0;  
  pthread_t interface_thread;
  pthread_create(&interface_thread,NULL,cmd_line_interface, (void*)0); 

  sleep(1);   
  //Accept an incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
     
     
  //Accept an incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  while( (client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t*)&c)) )
    {
      puts("Connection accepted");
         
      pthread_t sniffer_thread;
      new_sock = (int*)malloc(sizeof(int));
      *new_sock = client_sockfd;
         
      if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
          perror("could not create thread");
          return 1;
        }
         
      //Now join the thread , so that we dont terminate before the thread
      //pthread_join( sniffer_thread , NULL);
      puts("Handler assigned");
    }
     
  if (client_sockfd < 0)
    {
      perror("accept failed");
      return 1;
    }
     
    return 0;
}

 

void *connection_handler(void *socket_desc){
  
  //Deals with the clients
  int client_sockfd = *(int*)socket_desc;
  char buf[100];
  char ch = 'A';
  if( (recv(client_sockfd , buf , 100 , 0)) < 0)
  {
      puts("recv failed");
      exit(0);
  }
  int csId = buf[0]-'0';
  q[csId-1][curr[csId-1]++] = client_sockfd;
  printf("Serving client on fd %d and session %d\nFile being transferred! Please wait!\n",client_sockfd,csId);
  char fname[100];
  sprintf(fname,"%d.pdf",csId);
  char sndbuf[LENGTH];
  FILE *fp = fopen(fname,"rb");
  if(fp == NULL)
    {
      printf("ERROR: File %s not found.\n", fname);
      exit(1);
    }
  bzero(sndbuf,LENGTH);
  int f_block_sz;
  while((f_block_sz = fread(sndbuf, sizeof(char), LENGTH, fp))>0)
  {
     if(send(client_sockfd, sndbuf, LENGTH, 0) < 0)
    {
      printf("ERROR: Failed to send file %s.\n", fname);
      break;
    }
    bzero(sndbuf, LENGTH);
  }
  fclose(fp);
  
  //A custom message 'end' is sent
  sndbuf[0]='e';sndbuf[1]='n';sndbuf[2]='d';sndbuf[3]='\0';
  
  if(send(client_sockfd, sndbuf, LENGTH, 0) < 0)
  {
    printf("ERROR: Failed to send file %s.\n", fname);
  }
  // read(client_sockfd,&ch,1);
  printf("\nFile transfer over!\n\n");
  int i;
  for(i=0;i<3;i++)
    printf("Number of Clients on Session %d = %d\n",i+1,curr[i]);
  printf("\n");
  char str[100];
  sprintf(str,"%d %d %d %d",stt[csId-1].pNo,stt[csId-1].zL,stt[csId-1].ht,stt[csId-1].width);
  // printf("State data sent to client %d - %s\n",client_sockfd,str);
  write(client_sockfd , str , strlen(str));
}  

void* cmd_line_interface(void *desc)
{
    
      
    // stt.pNo=1;
    // stt.zL=100;
    // stt.ht=100;
    // stt.width=100;
    char str[100];
    int i,sId,j=100;
    while(j--)
    {
      printf("session:");
      sId=0;
      str[0]=0;
      if(scanf("%d %s",&sId,str)==-1)
        break;
      // printf("\nsessionId=%d str=%s\n",sId,str);
      if(!sId)
        continue;
      if(strcmp(str,"zoom")==0)
      {
        int z;
        scanf("%d",&z);
        stt[sId-1].zL = z;
      }
      else if(strcmp(str,"window")==0)
      {
        int h,w;
        scanf("%d%d",&h,&w);
        stt[sId-1].width = w;
        stt[sId-1].ht = h;
      }
      else if(strcmp(str,"goto")==0)
      {
        int p;
        scanf("%d",&p);
        stt[sId-1].pNo = p;
      }
      sprintf(str,"%d %d %d %d",stt[sId-1].pNo,stt[sId-1].zL,stt[sId-1].ht,stt[sId-1].width);
      // printf("%s\n",str);
      for(i=0;i<curr[sId-1];i++)
      {
          write(q[sId-1][i] , str , strlen(str));
          sleep(2);
          // sprintf(str,"zoom %d",stt.zL);
          // write(q[i] , str , strlen(str));
          // sleep(1);
          // sprintf(str,"window %d %d",stt.ht,stt.width);
          // write(q[i] , str , strlen(str));
          // sleep(1);
      }
    }
}
