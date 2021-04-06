#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h> 
using namespace std;

// const
const int BUFSIZE = 1024;
// var
sockaddr_in seraddr;
char sendbuf[BUFSIZE];
char recvbuf[BUFSIZE];
// func
void my_send(int connfd)
{
    while (1)
    {
        cout << "waiting for message! input \"exit\" to exit!" << endl;
        cin.getline(sendbuf, BUFSIZE);
        cout << "message ok! message: " << sendbuf << endl;

        if (string(sendbuf)=="exit") {
            cout << "send ends!" << endl;
            break;
        }

        int send_ = send(connfd, sendbuf, BUFSIZE, 0);
        if (send_==-1) {
            cout << "send error!" << endl;
            cout << strerror(errno) << endl;
            break;
        } else {
            cout << "send ok! message: " << sendbuf << endl;
        }

        memset(recvbuf, 0, sizeof(recvbuf));
        int recv_ = recv(connfd, recvbuf, BUFSIZE, 0);
        if (recv_==-1) {
            cout << "receive error!" << endl;
            cout << strerror(errno) << endl;
            break;
        } else {
            cout << "receive ok! receive: " << recvbuf << endl;
        }
    }
}
int main(int argc, char **argv)
{
    // AF_INET: IPv4 protocols
    // SOCK_STREAM: stream socket
    // 0: default specific protocol
    // return int: non-neg if OK, or -1 if error
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd==-1) {
        cout << "socket error!" << endl;
        return -1;
    } else {
        cout << "socket ok! socket number: " << sockfd << endl;
    }

    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    // convert IPv4 and IPv6 addresses from text to binary form
    const char* ipaddr = new char[32];
    if (argc>=2) ipaddr = argv[1];
    else ipaddr = "127.0.0.1";
    inet_pton(AF_INET,ipaddr,&seraddr.sin_addr);
    // htons converts the unsigned short integer hostshort from host byte order to network byte order
    int portaddr = 12345;
    if (argc>=3) portaddr = stoi(argv[2]);
    seraddr.sin_port = htons(portaddr);
    cout << "address ok! address: " << ipaddr << " port: " << portaddr << endl;

    int conn_ = connect(sockfd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
    if (conn_==-1) {
        cout << "connect error!" << endl;
        cout << strerror(errno) << endl;
        close(sockfd);
        return -1;
    } else {
        cout << "connect ok!" << endl;
    }
    
    my_send(sockfd);
    cout << "all send ok!" << endl;
    close(sockfd);
    cout << "connect close!" << endl;
    return 0;
}
