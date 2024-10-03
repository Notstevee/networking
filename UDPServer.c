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

#define PORTNO "33445"
#define BACKLOG 2
#define BUFFERSIZE 1048576

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
    struct addrinfo hints, *res;
    int soc;
    int new_fd[BACKLOG];
    socklen_t addr_size;
    struct sockaddr_storage *inaddr[BACKLOG];
    char ipstr[INET6_ADDRSTRLEN];

    int conns=1;

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC; 
    hints.ai_socktype=SOCK_DGRAM; //TCP. SOCK_DGRAM for UDP.
    hints.ai_flags=AI_PASSIVE;

    int rv;
    if ((rv=getaddrinfo(NULL,PORTNO,&hints,&res))!=0){
        fprintf(stderr,"getaddrinfo: %s, %d\n",strerror(rv),rv);
    }
    
    if((soc=socket(res->ai_family,res->ai_socktype,res->ai_protocol))<0){
        fprintf(stderr,"socket: %s, %d\n",strerror(errno),errno);
    }

     int yesyes=1;
    if (setsockopt(soc,SOL_SOCKET, SO_REUSEADDR, &yesyes,sizeof(int))==-1){
        fprintf(stderr,"setsockopt: %s, %d\n",strerror(errno),errno);
    } 
    if (bind(soc, res->ai_addr, res->ai_addrlen)!=0){
        fprintf(stderr,"bind: %s, %d\n",strerror(errno),errno);
    }
    
    char hostn[128];
    if (gethostname(hostn,sizeof hostn)!=0){
        fprintf(stderr,"gethostname: %s, %d\n",strerror(errno),errno);
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
                    port=ntohs(ipv6->sin6_port);
                }

    printf("Listener %s started with %s at port %d. Listening.\n",hostn,ipstr,port);

    char *msg=(char *)malloc(BUFFERSIZE);
    while (1){
            memset(msg, '\0', BUFFERSIZE); 
            struct sockaddr *from=(sockaddr *)malloc(sizeof(struct sockaddr));
            socklen_t fromlen=sizeof(*from);
            int numbytes;
            if((numbytes=recvfrom(soc,msg,BUFFERSIZE,0,from,&fromlen))==-1){
                fprintf(stderr,"recvfrom: %s, %d\n",strerror(errno),errno);
                exit(1);
            }
            printf("\"");
            printf("Packet %d bytes long received from %s:\"%s\"\n",numbytes,inet_ntop(from->sa_family,get_in_addr(from),ipstr,sizeof ipstr),msg);
            close(soc);

            if((soc=socket(res->ai_family,res->ai_socktype,res->ai_protocol))<0){
                fprintf(stderr,"socket: %s, %d\n",strerror(errno),errno);
            }

            if((numbytes=sendto(soc,msg,strlen(msg),0,from,fromlen))==-1){
                fprintf(stderr,"sendto: %s, %d\n",strerror(errno),errno);
                exit(1);
            }
             hints.ai_family=AF_UNSPEC; 
            hints.ai_socktype=SOCK_DGRAM; //TCP. SOCK_DGRAM for UDP.
            hints.ai_flags=AI_PASSIVE; 
            free(from);


                
            

            //printf("here!");


    }

}