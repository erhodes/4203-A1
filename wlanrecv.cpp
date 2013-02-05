// Author: Michel Barbeau, February 2004
// Updated: January 15, 2013
//
// Modified by: John Caskey, Eric Rhodes, Spencer Whyte
// 02/01/2013
//
// File: wlanrecv.cpp
//

// for socket()
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

// for htons()
#include <arpa/inet.h>

// for ifreq
#include <net/if.h>

#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wlan.hpp"
#include "nbr_table.h"

// THREAD INCLUDES

#include <pthread.h>

// THE NEIGHBOUR TABLE
nbr_table table;

// NEIGHBOUR TABLE MUTEX

pthread_mutex_t neighbour_mutex;




// 
// WLANAddr
//

char * WLANAddr::wlan2asc()
{
   static char str[32];
    
   sprintf(str, "%x:%x:%x:%x:%x:%x", 
      data[0],data[1],data[2],data[3],data[4],data[5]);

   return str;
}

static int hexdigit(char a)
{
    if (a >= '0' && a <= '9') return(a-'0');
    if (a >= 'a' && a <= 'f') return(a-'a'+10);
    if (a >= 'A' && a <= 'F') return(a-'A'+10);
    return -1;
}

static int sscanf6(char *str, int *a1, int *a2, int *a3, int *a4, int *a5, int *a6)
{
    int n;

    *a1 = *a2 = *a3 = *a4 = *a5 = *a6 = 0;
    while ((n=hexdigit(*str))>=0)
        (*a1 = 16*(*a1) + n, str++);
    if (*str++ != ':') return 1;
    while ((n=hexdigit(*str))>=0)
        (*a2 = 16*(*a2) + n, str++);
    if (*str++ != ':') return 2;
    while ((n=hexdigit(*str))>=0)
        (*a3 = 16*(*a3) + n, str++);
    if (*str++ != ':') return 3;
    while ((n=hexdigit(*str))>=0)
        (*a4 = 16*(*a4) + n, str++);
    if (*str++ != ':') return 4;
    while ((n=hexdigit(*str))>=0)
        (*a5 = 16*(*a5) + n, str++);
    if (*str++ != ':') return 5;
    while ((n=hexdigit(*str))>=0)
        (*a6 = 16*(*a6) + n, str++);

    return 6;
}

Outcome WLANAddr::str2wlan(char * s)
{
   int a[6], i;
     
   // parse the address
   if (sscanf6(s, a, a+1, a+2, a+3, a+4, a+5) < 6) 
   {
      return NOK;
   }

   // make sure the value of every component does not exceed on byte
   for (i=0; i < 6; i++) 
   {
      if (a[i] > 0xff) return NOK;
   }

   // assign the result to the "data" data member
   for (i=0; i < 6; i++) 
   {
      data[i] = a[i];
   }

   return OK;
}

// Global variables
   
// label of device, e.g. "eth0"
char device[] = "wlan0";
// interface configuration
Ifconfig ifconfig;
 // prototype header
WLANHeader hdr;

Outcome init()
{
   // create device level socket
   // - AF_PACKET : packet interface on device level
   // - SOCK_RAW : raw packets including link level header
   // - ETH_P_ALL : all frames will be received
   if ((ifconfig.sockid = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
   { 
      printf("cannot open socket: %s\n", strerror(errno));
      return NOK;
   }

   // fetch the interface index
   struct ifreq ifr;
   strcpy(ifr.ifr_name, device);
   if (ioctl(ifconfig.sockid, SIOGIFINDEX, &ifr) < 0)
   {
      printf("failed to fetch ifindex: %s\n", strerror(errno));
      return NOK;
   }
   ifconfig.ifindex = ifr.ifr_ifindex;
   printf("ifindex: %d\n", ifconfig.ifindex);

   // fetch the hardware address
   if (ioctl(ifconfig.sockid, SIOCGIFHWADDR, &ifr) == -1)
   {
      printf("failed to fetch hardware address: %s\n", strerror(errno));
      return NOK;
   }
   memcpy(&ifconfig.hwaddr.data, &ifr.ifr_hwaddr.sa_data, WLAN_ADDR_LEN);
   printf("hwaddr: %s\n", ifconfig.hwaddr.wlan2asc());

   // fetch the MTU
   if (ioctl(ifconfig.sockid, SIOCGIFMTU, &ifr) == -1)
   {
      printf("WLANProtocol, failed to the MTU: %s\n", strerror(errno));
      return NOK;
   }
   ifconfig.mtu = ifr.ifr_mtu;
   printf("MTU: %d\n", ifconfig.mtu);

   // add the promiscuous mode
   struct packet_mreq mr;
   memset(&mr,0,sizeof(mr));
   mr.mr_ifindex = ifconfig.ifindex;
   mr.mr_type =  PACKET_MR_PROMISC;
   if (setsockopt(ifconfig.sockid, SOL_PACKET, PACKET_ADD_MEMBERSHIP, 
      (char *)&mr, sizeof(mr)) < 0) 
   {
      printf("WLANProtocol, failed to add the promiscuous mode: %s\n", 
         strerror(errno)); 
      return NOK;
   }

   // bind the socket to the interface
   // only traffic from that interface will be recieved
   struct sockaddr_ll sll;
   memset(&sll, 0, sizeof(sll));
   sll.sll_family = AF_PACKET;
   sll.sll_ifindex = ifconfig.ifindex;
   sll.sll_protocol = htons(ETH_P_ALL);
   if (bind(ifconfig.sockid, (struct sockaddr*)&sll, sizeof(sll)) < 0) 
   {
      printf("WLANProtocol, failed to bind the socket: %s\n", strerror(errno)); 
      return NOK;
   }
   return OK;
}

// receive data over a socket
void Receive()
{
   // pointer to received data
   unsigned char * buff = new unsigned char[ifconfig.mtu];
   unsigned int i; // frame length
   struct sockaddr_ll from; // source address of the  message
   socklen_t fromlen = sizeof(struct sockaddr_ll); 
   int error;

   // infinite loop
   while (true)
   {
      // loop until a non-empty frame has been received on "device"
      while (true)
      {
         // wait and receive a frame
         fromlen = sizeof(from);
         i = recvfrom(ifconfig.sockid, buff, ifconfig.mtu, 0, 
            (struct sockaddr *) &from, &fromlen);
         if (i == -1)
         {
            printf("cannot receive data: %s\n", strerror(errno));
            // sleep for 10 milliseconds before re-trying
            usleep(10000);
         }
	 else
	 {
            break; // exit the loop
	 }
      }
      printf("frame received\n");

      char asciiSrcAddr[32], asciiDestAddr[32];
  
      // casting to the WLAN header format
      WLANHeader * wlanHdr = (WLANHeader *) buff;

      char * dest = wlanHdr->destAddr.wlan2asc();
      strcpy(asciiDestAddr, dest);
 
      char * src = wlanHdr->srcAddr.wlan2asc();
      strcpy(asciiSrcAddr, src);

      printf("wlan:%s<<%s\n", asciiDestAddr, asciiSrcAddr);

	//is it actually a hello beacon!?
	char * message = (char *)(buff + sizeof(WLANHeader));
	const char* beacon = "hello";
	if(strcmp(message,beacon)==0){
		pthread_mutex_lock (&neighbour_mutex);
		table.beaconRecieved(src);
    		table.printTable();
		pthread_mutex_unlock (&neighbour_mutex);
	}
	printf("message: %s\n",message);
   }
}

void shutdown()
{
   printf("shutdown\n");
   // close the socket
   if (ifconfig.sockid != -1) close(ifconfig.sockid);
}


Outcome MySend() 
{ 
   // send buffer size
   # define BUFFSIZE 256
   // send buffer
   unsigned char buff[BUFFSIZE]; 
   // destination address (ASCII)
   char rp[] ="ff:ff:ff:ff:ff:ff"; // broadcast
   //char rp[] ="1c:bd:b9:7e:b6:5a"; // unicast
   // destination address (binary)
   WLANAddr daddr;
   // data
   char dp[] = "hello"; 

   // convert destination address from ASCII to binary
   daddr.str2wlan(rp);

   // store the destination address
   memmove(&hdr.destAddr, daddr.data, WLAN_ADDR_LEN);

   // store the source address
   memmove(&hdr.srcAddr, ifconfig.hwaddr.data, WLAN_ADDR_LEN);

   // set the type field
   hdr.type = htons(IP_TYPE);

   // store the header into the message 
   memmove(buff, &hdr, WLAN_HEADER_LEN);

   // store the payload
   memmove(buff+WLAN_HEADER_LEN, dp, strlen(dp));

   // set the "to address"
   struct sockaddr_ll to = {0};
   int tolen = sizeof(to);
   to.sll_family = AF_PACKET;
   to.sll_ifindex = ifconfig.ifindex;
   memmove(&(to.sll_addr), daddr.data, WLAN_ADDR_LEN);
   to.sll_halen = 6;

   // send a frame
   int sentlen = sendto(
      ifconfig.sockid, buff, WLAN_HEADER_LEN+strlen(dp) + 1, 0, 
      (sockaddr *) &to, tolen);

   if (sentlen == -1 ) 
   {
      printf("sendto() failed %s\n", strerror(errno));
      return NOK;
   }
   return OK;
}





void *receivingThread(void * threadid){
	long tid;
	tid = (long)threadid;
	

	printf("Receive Thread Started Successfully");

	Receive();

	pthread_exit(NULL);
}


void *sendHelloBeaconThread(void * threadid){
	long tid;
	tid = (long)threadid;

	printf("Send Thread Started Successfully");
	
	int MINIMUM_WAIT_TIME = 2;
	int MAXIMUM_WAIT_TIME = 10;

	long randomWaitTime = MINIMUM_WAIT_TIME + rand() % (MAXIMUM_WAIT_TIME - MINIMUM_WAIT_TIME);

	while(1){
		
		printf("SENDING\n");
		MySend();
		sleep(randomWaitTime);
	}


	pthread_exit(NULL);
}



Outcome createSendThread(){
	pthread_t sendThread;
	long tSend;
	int sendThreadCreationStatus = pthread_create(&sendThread, NULL, sendHelloBeaconThread, (void*)tSend);
        if(sendThreadCreationStatus){
                printf("ERROR: Failed to create the send hello beacon thread with status code %d\n", sendThreadCreationStatus);
                exit(-1);
        }else{
		return OK;
	}
	return NOK;
}

Outcome createReceiveThread(){
	pthread_t receiveThread;
	long tReceive;
	int receiveThreadCreationStatus = pthread_create(&receiveThread, NULL, receivingThread, (void*)tReceive);
        if(receiveThreadCreationStatus){
                printf("ERROR: Failed to create the reception thread %d\n", receiveThreadCreationStatus);
                exit(-1);
        }else{
		return OK;
	}
	return NOK;
}



main()
{
   pthread_mutex_init(&neighbour_mutex, NULL);
   int update_time = rand() % 10 + 3;
   if (init()==OK) 
   {
	if(createSendThread() == OK){
		if(createReceiveThread() == OK){
			while(1){
				sleep(update_time);
				pthread_mutex_lock (&neighbour_mutex);
				table.update();
				table.printTable();
				pthread_mutex_unlock (&neighbour_mutex);
			}
      			shutdown();
		}
	}
   }
	pthread_mutex_destroy(&neighbour_mutex);
}
