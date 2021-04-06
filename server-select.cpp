#include <signal.h>
#include <stdio.h>
//#include<stdlib.h>
//#include<string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

//const
const int BUFSIZE = 1024;
const int LISTENQ = 5;
//var
int maxfd, nready, clients[FD_SETSIZE];
fd_set rset, allset;
sockaddr_in seraddr, cliaddrs[FD_SETSIZE];
in_addr sa;
char recvbuf[BUFSIZE];
//func
void my_print_addr(const sockaddr_in cliaddr) {
    sa.s_addr = cliaddr.sin_addr.s_addr;
    cout << "receive ok! receive from: " << inet_ntoa(sa) << " port: " << htons(cliaddr.sin_port) << endl;
}
bool my_echo(int connfd, sockaddr_in cliaddr) {
memset(recvbuf, 0, sizeof(recvbuf));
    int recv_ = recv(connfd, recvbuf, BUFSIZE, 0);
    if (recv_==-1) {
        cout << "receive error! " << strerror(errno) << endl;
        return false;
    } else if (recv_==0) {
        cout << "no more message received!" << endl;
        return false;
    } else {
        my_print_addr(cliaddr);
        cout << "message is: " << recvbuf << endl;
        string response = string(recvbuf);
        reverse(response.begin(),response.end());
        int send_ = send(connfd, response.c_str(), response.length() * sizeof(char), 0);
        if (send_==-1) {
            cout << "send error! " << strerror(errno) << endl;
            return false;
        }
    }
    return true;
}
int main(int argc, char **argv) {
    // AF_INET: IPv4 protocols
    // SOCK_STREAM: stream socket
    // 0: default specific protocol
    // return int: non-neg if OK, or -1 if error
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd==-1) {
        cout << "socket error!" << endl;
        return -1;
    } else {
        cout << "listen socket ok! listen socket number: " << listenfd << endl;
    }

    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    // htonl converts the unsigned integer hostlong from host byte order to network byte order
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind the socket to all available interfaces
    // htons converts the unsigned short integer hostshort from host byte order to network byte order
    int portaddr = 12345;
    if (argc>=2) portaddr = stoi(argv[1]);
    seraddr.sin_port = htons(portaddr);
    cout << "address ok! port: " << portaddr << endl;

    // assign a local protocol address to a socket
    int bind_ = ::bind(listenfd, (const struct sockaddr *)&seraddr, sizeof(seraddr));
    if (bind_==-1) {
        cout << "bind error! " << strerror(errno) << endl;
        close(listenfd);
        return -1;
    } else {
        cout << "bind ok!" << endl;
    }

    int listen_ = listen(listenfd, LISTENQ);
    if (listen_==-1) {
        cout << "listen error! " << strerror(errno) << endl;
        close(listenfd);
        return -1;
    } else {
        cout << "listen ok!" << endl;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    memset(clients, -1, sizeof(clients));
    maxfd = listenfd;
    cout << "begin to listen!" << endl;

    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, nullptr, nullptr, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            cout << "new client!" << endl;
            sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listenfd, (sockaddr *)&cliaddr, &clilen);
            if (connfd==-1) {
                cout << "accept error! " << strerror(errno) << endl;
                if (errno == EINTR) continue;
                else return -1;
            } else {
                int i;
                for (i = 0; i < FD_SETSIZE; i++) {
                    if (clients[i] < 0) {
                        clients[i] = connfd;
                        cliaddrs[i] = cliaddr;
                        cout << "client No. " << i << " is accepted!" << endl;
                        break;
                    }
                }
                if (i == FD_SETSIZE) {
                    cout << "too many clients!" << endl;
                    break;
                }
                FD_SET(connfd, &allset);
                maxfd = max(maxfd,connfd);
                if (--nready == 0)
                    continue;
            }
        }
        for (int i = 0; i < FD_SETSIZE; i++) {
            int connfd = clients[i];
            sockaddr_in cliaddr = cliaddrs[i];
            if (connfd == -1)
                continue;
            if (FD_ISSET(connfd, &rset)) {
                if (my_echo(connfd,cliaddr)) ;
                else {
                    FD_CLR(connfd, &allset);
                    clients[i] = -1;
                    close(connfd);
                    cout << "client No. " << i << " is closed!" << endl;
                }
            }
            if (--nready == 0)
                break;
        }
    }
    close(listenfd);
    return 0;
}