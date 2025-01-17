#pragma once


// fow windows
#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#define usleep(t) Sleep(t/1000)
#define MSG_NOSIGNAL 0
typedef int socklen_t;
#ifdef _WIN64
// for windows x64
#else
// for windows x86
#endif
#endif

// fow linux
#ifdef __linux__
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define closesocket(fd) close(fd)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define Sleep(t) usleep(t*1000)
typedef void* HANDLE;
typedef int SOCKET;

#define SET_SOCKBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)&~O_NONBLOCK)
#define SET_SOCKNONBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK)

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
#endif
