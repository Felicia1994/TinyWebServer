### this is a tiny toy project

#### it is forked from [this project](https://github.com/chaishilin/Tiny-webserver)

#### i use it to learn the basics of c++, sockets, etc.

### notes

#### client.cpp is for client end

#### server-fork.cpp and server-select.cpp are for server end

#### use ''nc -zv 127.0.0.1 12345'' from the client side to check if the port 12345 is able to connect

#### use ''ps -fA | grep g++'' to check c++ processes that are running

#### <sys/socket.h> has bind function, while C++11 introduced its own std::bind function into namespace std

#### either remove using namespace std, or use ::bind to denote it's in the global namespace

#### the original version does work when you don't compile with c++11 !!!