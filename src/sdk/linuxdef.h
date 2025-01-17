#ifndef _LINUXDEF_H_
#define _LINUXDEF_H_

#include <string.h>
#include <unistd.h>

#ifdef __linux__
#define closesocket(fd) close(fd)
#define Sleep(delay) usleep(delay*1000)

#define EqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define MoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#define SET_SOCKBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)&~O_NONBLOCK)
#define SET_SOCKNONBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK)
#endif

#endif
