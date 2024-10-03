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

#define PORTNO "45678"
#define BACKLOG 2
#define BUFFERSIZE 1048576

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
    hints.ai_socktype=SOCK_STREAM; //TCP. SOCK_DGRAM for UDP.
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

    if (listen(soc,BACKLOG)!=0){
        fprintf(stderr,"listen: %s, %d\n",strerror(errno),errno);
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

    printf("Host %s started with %s at port %d. Listening.\n",hostn,ipstr,port);

    
        

            char *msg=(char *)malloc(BUFFERSIZE);
    while (1){
        
            addr_size=sizeof (*inaddr[0]);
        if ((new_fd[0]=accept(soc,(struct sockaddr *)inaddr[0],&addr_size))<0){
            fprintf(stderr,"accept: %s, %d\n",strerror(errno),errno);
        }
        


            socklen_t len;
            struct sockaddr_storage addr;
            len = sizeof addr;
            if (getpeername(new_fd[0], (struct sockaddr*)&addr, &len)!=0){
                fprintf(stderr,"getpeername: %s, %d\n",strerror(errno),errno);
            }

            // deal with both IPv4 and IPv6:
            if (addr.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
            } else { // AF_INET6
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
            }

            printf("Client at %s connected with local port %d.\n", ipstr,port);

            
            memset(msg, '\0', BUFFERSIZE); 
            int recvret=recv(new_fd[0],msg,BUFFERSIZE,0);
            printf("%s",msg);
            if (strncmp(msg,"GET",3)==0){
                char * fl;
                fl=strtok(msg," ");
                fl=strtok(NULL," ");
                //printf("%s\n",fl);
                char *tmp1=(char *)malloc(strlen(fl)-1);
                memcpy(tmp1,fl+1,strlen(fl)-1);

                FILE* fptr;
                fptr=fopen(tmp1,"r");
                if (!fptr){
                    char *rmsg=(char *)malloc(BUFFERSIZE);
                    memset(rmsg, '\0', BUFFERSIZE);
                    strcpy(rmsg,"HTTP/1.1 404 Not Found\n"); 
                    printf("%s\n",rmsg);
                    int msglen=strlen(rmsg);
                    send(new_fd[0],rmsg,msglen,0);
                }
                else {
                    char *rmsg=(char *)malloc(BUFFERSIZE);
                    memset(rmsg, '\0', BUFFERSIZE);
                    strcpy(rmsg,"HTTP/1.1 200 OK\n\n"); 
                    //printf("%s\n",rmsg);
                    int msglen=strlen(rmsg);
                    send(new_fd[0],rmsg,msglen,0);
                    memset(rmsg, '\0', BUFFERSIZE);
                    while (fgets(rmsg,BUFFERSIZE,fptr)){
                        msglen=strlen(rmsg);
                        send(new_fd[0],rmsg,msglen,0);
                        //printf("%s\n",rmsg);
                        memset(rmsg, '\0', BUFFERSIZE);
                    }
                    fclose(fptr);
                }
                close(new_fd[0]);
                
            }

            //printf("here!");
            break;

    }


}

