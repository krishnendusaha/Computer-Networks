#include <cnet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 1000;

static char *check;
static char* fruit[] = { "apple", "pineapple"};
#define	NFRUITS	(sizeof(fruit) / sizeof(fruit[0]))
extern int random(void);
int SN=0;
int RN=0;
int count=0;
int flag_sen=1;
int flag_rec=0;
int id=0;
static void transmit_frame(){
	if(strcmp(nodeinfo.nodename,"Sender")==0 && flag_sen == 1 && count < 1000 ){
		char *frame=(char *)malloc(64*sizeof(char));
		unsigned int length=64;
		int i=0;


		if(SN==0){ 
		sprintf(frame, "%dmessage %d is %s",SN, count+1, fruit[0]);
		}
		else{ 
		sprintf(frame, "%dmessage %d is %s",SN, count+1, fruit[1]);
		}
		printf("\nsending %u bytes of ' %s ': ", length, frame);
		for(i=1;i<9;i++){
			check[i]=frame[i];
		}
		CHECK(CNET_write_physical(1, frame, &length));
	//	CnetTime to = (64*(CnetTime)3600) + linkinfo[1].propagationdelay;
		id=CNET_start_timer(EV_TIMER1,3600,0);
		flag_sen=0;
	}
	else if(strcmp(nodeinfo.nodename,"Receiver")==0 && flag_rec == 1){
		char *ack=(char *)malloc(48*sizeof(char));
		unsigned int length=48;
		sprintf(ack, "%d",RN);
		CHECK(CNET_write_physical(1, ack, &length));
		flag_rec =0;
		printf("\n Acknowledgement send ");
	}
}

static void frame_arrived(CnetEvent ev, CnetTimerID timer, CnetData data)
{
	if(strcmp(nodeinfo.nodename,"Sender")==0 ){
		char *ack=(char *)malloc(48*sizeof(char));
		int link;
		int seq_check;
		unsigned int length=48;
		//length	= strlen(ack) + 1;
		CNET_stop_timer(id);
		CHECK(CNET_read_physical(&link, ack, &length));
		seq_check = ((int)(ack[0]))-48;
		//printf(" %d ",seq_check);
		if(seq_check == ((SN+1)%2)){
			printf("\n Acknowledgement received ");
			flag_sen=1;
			count++;
			SN= count%2;		
		}
	}
	else if(strcmp(nodeinfo.nodename,"Receiver")==0 ){
		char *frame=(char *)malloc(64*sizeof(char));
		char *tobchecked=(char *)calloc(9,sizeof(char));
		unsigned int length=64;
		int link;
		int i=0;
		printf("\n Receiving.....");
		//length =  strlen(frame) + 1;
		CHECK(CNET_read_physical(&link, frame, &length));
		//printf("\n  %s  \n",frame);
		for(i=1;i<9;i++){
			tobchecked[i]=frame[i];
			//printf(" %c \n",tobchecked[i]);
			//printf(" %c \n",check[i]);
		}
		if((strcmp(check,tobchecked))==0){
			flag_rec = 1;
			//printf("RN==%d , frame[0]=%d ",RN,(int)frame[0]);
			if(RN==((int)frame[0])-48){		
				printf("\nSUCCESSFULLY RECEIVED %u bytes", length);
				RN=(RN+1)%2;
				flag_sen=1;	
			}
			else{
				 printf("\n The SN mismatches !");
			}
			transmit_frame();	//for the acknowledgement
			printf("\n The frame is valid");
		}
		else {
			printf("\n The frame is either lost or error");
		}	
	}
}

static EVENT_HANDLER(resend_frame){
	flag_sen=1;
	printf("Timer expired");
	transmit_frame();
}
static EVENT_HANDLER(application_ready)
{
	transmit_frame();
}
void reboot_node(CnetEvent event){
	check = (char *)malloc(9*sizeof(char));
	CHECK(CNET_set_handler(EV_APPLICATIONREADY,application_ready, 0));
	CHECK(CNET_set_handler(EV_PHYSICALREADY,frame_arrived, 0));
	CHECK(CNET_set_handler(EV_TIMER1,resend_frame, 0));
	CNET_enable_application(ALLNODES);

}
