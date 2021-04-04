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
char sendbuf[BUFSIZE];
char recvbuf[BUFSIZE];
//func

string UrlDecode(const string& szToDecode) {
    string result;
    int hex = 0;
    for (size_t i = 0; i < szToDecode.length(); ++i)
    {
        switch (szToDecode[i])
        {
        case '+':
            result += ' ';
            break;
        case '%':
            if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2])) {
                string hexStr = szToDecode.substr(i + 1, 2);
                hex = strtol(hexStr.c_str(), 0, 16);
                //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
                //可以不经过编码直接用于URL
                if (!((hex >= 48 && hex <= 57) ||  //0-9
                      (hex >= 97 && hex <= 122) || //a-z
                      (hex >= 65 && hex <= 90) ||  //A-Z
                      //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
                      hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29 || hex == 0x2a || hex == 0x2b || hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f || hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f))
                {
                    result += char(hex);
                    i += 2;
                } else {
                    result += '%';
                }
            } else {
                result += '%';
            }
            break;
        default:
            result += szToDecode[i];
            break;
        }
    }
    return result;
}
void doreverse(char *ptr) {
    int num = 0;
    while (*(ptr + num) != '\0')
        num++;
    num--;
    int slow = 0;
    while (slow < num) {
        swap(*(ptr + slow), *(ptr + num));
        slow++;
        num--;
    }
}
void doreverse(string &str) {
    auto iter_begin = str.begin();
    auto iter_end = str.end() - 1;
    while (iter_begin < iter_end) {
        swap(*iter_begin, *(iter_end - 2));
        swap(*(iter_begin + 1), *(iter_end-1));

        swap(*(iter_begin + 2), *iter_end);
        iter_end -= 3;
        iter_begin += 3;
    }
}
string my_parser(string request) {
    string response = "";
    if (request.substr(0, 4) == "POST") {
        string result = request.substr(request.find("fname") + 6);
        cout << "fname: " << result << endl;
        string dresult = UrlDecode(result);
        doreverse(dresult);
        response += "HTTP/1.0 200 OK\r\n";
        response += "\r\n";
        response += "<HTML><B>hello world!</B>";
        response += "<head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head>";
        //response += "<B>You are the " + to_string(connum + 1) + "th clients!</B>";
        response += "<form accept-charset='utf-8' name='myForm' method='post'>字符串: <input type='text' name='fname'><input type='submit' value='submit'></form>";
        response += "<B>Result: " + dresult + "</B>";
        response += "</HTML>";
    } else if (request.substr(0, 3) == "GET") {
        response += "HTTP/1.0 200 OK\r\n";
        response += "\r\n";
        response += "<HTML><B>hello world!</B>";
        response += "<head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head>";
        //response += "<B>You are the " + to_string(connum + 1) + "th clients!</B>";
        response += "<form accept-charset='utf-8' name='myForm' method='post'>字符串: <input type='text' name='fname'><input type='submit' value='submit'></form>";

        response += "</HTML>";
    } else {
        response = "HTTP/1.0 400 BadRequest\r\n";
    }
    return response;
}
void sig_child(int signo) {
    pid_t pid;
    int stat;
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
        int recv_ = recv(connfd, recvbuf, BUFSIZE, 0);
        if (recv_==-1) {
            cout << "receive error!" << endl;
            cout << strerror(errno) << endl;
            break;
        } else if (recv_==0) {
            cout << "no more message received!" << endl;
            break;
        } else {
            my_print_addr(cliaddr);
            cout << "message is: " << recvbuf << endl;
            string response = my_parser(string(recvbuf));
            int send_ = send(connfd, response.c_str(), response.length() * sizeof(char), 0);
            if (send_==-1) {
                cout << "send error!" << endl;
                cout << strerror(errno) << endl;
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
        cout << "bind error!" << endl;
        cout << strerror(errno) << endl;
        close(listenfd);
        return -1;
    } else {
        cout << "bind ok!" << endl;
    }

    int listen_ = listen(listenfd, LISTENQ);
    if (listen_==-1) {
        cout << "listen error!" << endl;
        cout << strerror(errno) << endl;
        close(listenfd);
        return -1;
    } else {
        cout << "listen ok!" << endl;
    }

    signal(SIGCHLD, sig_child); // i deleted this line so that the program worked, but now it still works with this line!!!???
    cout << "begin to listen!" << endl;
    while (1) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (sockaddr *)&cliaddr, &clilen); // return int: non-neg if OK, or -1 if error
        if (connfd==-1) {
            cout << "accept error!" << endl;
            cout << strerror(errno) << endl;
            if (errno == EINTR) continue;
            else {
                close(listenfd);
                return -1;
            }
        } else {
            pid = fork(); // return pid_t (int): 0 if child, pid of child if parent, or -1 if error
            if (pid==-1) {
                cout << "fork error!" << endl;
                cout << strerror(errno) << endl;
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