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

#define PORTNO "50002"
#define BACKLOG 2
#define BUFFERSIZE 512

void add_peer(struct pollfd *sockets[],int &conns,struct addrinfo *res){
    struct sockaddr *inaddr;
    socklen_t addr_size=sizeof(inaddr);
    int prt;
    int port=0;
    if ((prt=accept(sockets[0]->fd,inaddr,&addr_size))<0){
        fprintf(stderr,"accept: %s, %d\n",strerror(errno),errno);
    }
            socklen_t len;
            struct sockaddr_storage addr;
            len = sizeof addr;
            if (getpeername(prt, (struct sockaddr*)&addr, &len)!=0){
                fprintf(stderr,"getpeername: %s, %d\n",strerror(errno),errno);
            }

            // deal with both IPv4 and IPv6:
            char ipstr[INET6_ADDRSTRLEN];
            if (addr.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
            } else { // AF_INET6
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
            }

            
            if (conns>BACKLOG) {
                printf("Connection pool full!! Connection for %s with local port %d unsuccessful. Disconnecting...\n", ipstr,port);
                char *msg=(char *)malloc(BUFFERSIZE);
                msg="Connection pool to server full!! Connection unsuccessful. Disconnecting...\n";
                int msglen=strlen(msg);
                send(prt,msg,msglen,0);
                close(prt);
                }
            else {
                
                printf("Peer at %s connected with local port %d.\n", ipstr,port);
                char *msg=(char *)malloc(BUFFERSIZE);
                
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

                /* char tt[6];
                sprintf(tt,"%d",port);
                strcat(msg,"Connected to host ");
                msg="Connected to host ";
                int msglen=strlen(msg);
                send(prt,msg,msglen,0); */
                (*sockets)[conns].fd=prt;
                (*sockets)[conns].events=POLLIN;
                conns++;

            }
}
void broadcast(char *msg,struct pollfd sockets[],int &conns,int i){
    for (int j=1;j<conns;j++){
                            if (j!=i) {
                                
                                int msglen=strlen(msg);
                                send(sockets[j].fd,msg,msglen,0);
                            }
                        }
}
void del_peer(struct pollfd sockets[],int &conns,int i){
    if (conns-1>i) {
        sockets[i].fd=sockets[conns-1].fd;
    }
    else {
        sockets[i].fd=-1;
    }
    
    conns--;
}

int main(){
    struct addrinfo hints, *res;
    int soc;
    int new_fd[BACKLOG];
    socklen_t addr_size;
    struct sockaddr_storage *inaddr[BACKLOG];
    char ipstr[INET6_ADDRSTRLEN];

    struct pollfd *sockets=(pollfd*)malloc(sizeof *sockets * (BACKLOG+1));
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

    sockets[0].fd=soc;
    sockets[0].events = POLLIN;
    
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
        int poll_cnt=poll(sockets,conns,-1);
        if (poll_cnt==-1){
            fprintf(stderr,"poll: %s, %d\n",strerror(errno),errno);
            exit(1);
        }

        for (int i=0;i<conns;i++){
            if (sockets[i].revents & POLLIN){
                if (i==0) add_peer(&sockets,conns,res);

                else {
                    memset(msg, '\0', BUFFERSIZE); 
                    int recvret=recv(sockets[i].fd,msg,1024,0);

                    if (recvret>0){
                        if (strcmp(msg,"exit\n")==0) {
                            printf("Peer %d exited!\n",i);
                            char *msg1=(char *)malloc(BUFFERSIZE);
                            char tt[2];
                            sprintf(tt,"%d",i);
                            strcpy(msg1,"Peer ");
                            
                            strcat(msg1,tt);
                            strcat(msg1," exited!\n");
                            broadcast(msg1,sockets,conns,i);
                            close(sockets[i].fd);
                            del_peer(sockets,conns,i);
                        }
                        else {
                            printf("Peer %d: %s (%d bytes)\n",i,msg,recvret);
                            for (int j=1;j<conns;j++){
                                if (j!=i) {
                                    char tt[2];
                                    sprintf(tt,"%d",i);
                                    char *msg1=(char *)malloc(BUFFERSIZE);
                                    strcpy(msg1,"Peer ");
                                    strcat(msg1,tt);
                                    strcat(msg1,": ");
                                    strcat(msg1,msg);
                                    int msglen=strlen(msg1);
                                    send(sockets[j].fd,msg1,msglen,0);
                                }
                            } 
                        }
                        

                    }
                    else if (recvret==0) {
                        printf("Connection closed.");
                        del_peer(sockets,conns,i);
                    }
                    else fprintf(stderr,"recv: %s (%d)\n",strerror(errno),errno);
                    
                }
            }


        }


        
    }
    printf("Client exited.\n");

    return 0;
}