#include <signal.h>
#include <stdio.h>
//#include<stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

//const
const int BUFSIZE = 1024;
const int LISTENQ = 5;
//var
sockaddr_in seraddr, cliaddr;
in_addr sa;
pid_t pid;
int connum = 0;
char recvbuf[BUFSIZE];
//func
void sig_child(int signo) {
    int stat;
    pid_t pid;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        cout << "child process " << pid << " ends now!" << endl;
        cout << "there are " << --connum << " clients still connected!" << endl;
    }
    return;
}
void my_print_addr(const sockaddr_in cliaddr) {
    sa.s_addr = cliaddr.sin_addr.s_addr;
    cout << "receive ok! receive from: " << inet_ntoa(sa) << " port: " << htons(cliaddr.sin_port) << endl;
}
void my_echo(int connfd) {
    while (1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        int recv_ = recv(connfd, recvbuf, BUFSIZE, 0);
        if (recv_==-1) {
            cout << "receive error! " << strerror(errno) << endl;
            break;
        } else if (recv_==0) {
            cout << "no more message received!" << endl;
            break;
        } else {
            my_print_addr(cliaddr);
            cout << "message is: " << recvbuf << endl;
            string response = string(recvbuf);
            reverse(response.begin(),response.end());
            int send_ = send(connfd, response.c_str(), response.length() * sizeof(char), 0);
            if (send_==-1) {
                cout << "send error! " << strerror(errno) << endl;
                break;
            }
        }
    }
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

    signal(SIGCHLD, sig_child);
    cout << "begin to listen!" << endl;

    while (1) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (sockaddr *)&cliaddr, &clilen); // return int: non-neg if OK, or -1 if error
        if (connfd==-1) {
            cout << "accept error! " << strerror(errno) << endl;
            if (errno == EINTR) continue;
            else {
                close(listenfd);
                return -1;
            }
        } else {
            pid = fork(); // return pid_t (int): 0 if child, pid of child if parent, or -1 if error
            if (pid==-1) {
                cout << "fork error! " << strerror(errno) << endl;
                close(listenfd);
                close(connfd);
                return -1;
            } else if (pid == 0) { // child
                close(listenfd);
                cout << "child precess " << getpid() << " starts now!" << endl;
                my_echo(connfd);
                exit(0);
            } else { // parent
                close(connfd);
                cout << "one more client! there are " << ++connum << " clients being connected!" << endl;
            }
        }
    }
    close(listenfd);
    return 0;
}