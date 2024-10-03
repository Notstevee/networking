#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#include <unistd.h>

#define PORTNO "33445"
#define BUFFERSIZE 512

int main(){
    struct addrinfo hints, *res;
    int soc;
    char ipstr[INET6_ADDRSTRLEN];

    struct timeval tv;
    tv.tv_sec=0;
    tv.tv_usec=100000;        

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC; 
    hints.ai_socktype=SOCK_STREAM; //TCP. SOCK_DGRAM for UDP.
    hints.ai_flags=AI_PASSIVE;

    int rv;
    if ((rv=getaddrinfo(NULL,PORTNO,&hints,&res))!=0){
        fprintf(stderr,"getaddrinfo: %s, %d\n",strerror(rv),rv);
    }
    
    if((soc=socket(res->ai_family,res->ai_socktype,res->ai_protocol))<0){
        fprintf(stderr,"socket: %s, %d\n",strerror(errno),errno);
    }
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0,&readfds);//stdin
    FD_SET(soc,&readfds);

    if (connect(soc,res->ai_addr, res->ai_addrlen)!=0){
        fprintf(stderr,"connenct: %s, %d\n",strerror(errno),errno);
        exit(1);
    }

    
    

    if (!inet_ntop(res->ai_family,res->ai_addr,ipstr,sizeof ipstr)){
        fprintf(stderr,"inet_ntop: %s, %d\n",strerror(errno),errno);
    }
     
    int port=0;
    if (res->ai_family==AF_INET){
        struct sockaddr_in *ipv4=(struct sockaddr_in *) res->ai_addr;
        port=ntohs(ipv4->sin_port);

    }
    else {
        struct sockaddr_in6 *ipv6=(struct sockaddr_in6 *) res->ai_addr;
        port=ntohs(ipv6->sin6_port);}
        
printf("Connected to host %s at port %d.\n",ipstr,port);
char *msg=(char *)malloc(BUFFERSIZE);
/*     int recvret=recv(soc,msg,1024,0);
            printf("%s",msg); */
            memset(msg, '\0', BUFFERSIZE);
    
    while (1){
        FD_ZERO(&readfds);                      //man page: Upon return, each of the file descriptor sets is
        FD_SET(0,&readfds);                     //modified in place to indicate which file descriptors are
        FD_SET(soc,&readfds);                   //currently "ready".  Thus, if using select() within a loop, the
        select(soc+1,&readfds,NULL,NULL,NULL);  //sets must be reinitialized before each call.


        if (FD_ISSET(0,&readfds)){
            fgets(msg,BUFFERSIZE,stdin);
            int len,bytes_sent;
            
            len=strlen(msg);
            if ((bytes_sent=send(soc,msg,len,0))<0){
                fprintf(stderr,"inet_ntop: %s, %d\n",strerror(errno),errno);
                exit(1);
            }
            printf("%d bytes sent.\n",bytes_sent);
            if (strcmp(msg,"exit\n")==0) {
                close(soc);
                break;
            }        
            memset(msg, '\0', BUFFERSIZE); 
        }
        else if (FD_ISSET(soc,&readfds)){
            int recvret=recv(soc,msg,1024,0);
            printf("%s",msg);
            if (strcmp(msg,"Connection pool to server full!! Connection unsuccessful. Disconnecting...\n")==0){
                close(soc);
                exit(1);
            }
            memset(msg, '\0', BUFFERSIZE); 
        }

        

        
    }
    
    printf("Disconnected! Closing...\n");

    return 0;
}