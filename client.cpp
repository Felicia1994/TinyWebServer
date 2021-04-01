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

//const
const int BUFSIZE = 1024;
const int LISTENQ = 5;
const int PORT = 12345;
//var
sockaddr_in seraddr;
pid_t childpid;
char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];
//func
void my_send(int connfd, char *msg)
{
    while (1)
    {
        //cout << msg << endl;
        if (*msg == '\0')
            cin >> sendbuf;
        else
            strcpy(sendbuf, msg);
        
        cout << "message created!" << endl;

        //cin >> sendbuf;
        bool ans = (*sendbuf);
        //sleep(1);

        int tmp = send(connfd, sendbuf, BUFSIZE, 0);
        cout << "tmp: " << tmp << endl;
        if (tmp <= 0 && ans != 0) {
            cout << "client recv error" << endl;
            break;
        } else if (ans == 0) {
            cout << "end the send" << endl;
            break;
        } else {
            cout << "none of above!" << endl;
        }
        cout << "here!" << endl;
        sendbuf[0] = '\0'; //clear the buf
        if (recv(connfd, recvbuf, BUFSIZE, 0) <= 0) {
            cout << "client recv error" << endl;
            break;
        }
        cout << "here!" << endl;

        cout << recvbuf << endl;
    }
}
int main(int argc, char **argv)
{
    // AF_INET: IPv4 protocols
    // SOCK_STREAM: stream socket
    // 0: default specific protocol
    // return int: non-neg if OK, or -1 if error
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    cout << sockfd << endl;    
    cout << "socket ok!" << endl;

    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    // convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET,"127.0.0.1",&seraddr.sin_addr);
    // htons converts the unsigned short integer hostshort from host byte order to network byte order
    seraddr.sin_port = htons(PORT);
    cout << "address ok!" << endl;

    int conn = connect(sockfd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
    cout << conn << endl;
    cout << strerror(errno) << endl;
    cout << "connect ok!" << endl;

    
    if (argc == 2)
        my_send(sockfd, argv[1]);
    else {
        char t = '\0';
        my_send(sockfd,&t);
    }
    cout << "send ok!" << endl;

    close(sockfd);
    cout << "close connect" << endl;
    exit(0);
    return 0;
}
