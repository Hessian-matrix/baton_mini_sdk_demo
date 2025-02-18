#include "CommonSocket.h"
#include "linuxdef.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <iostream>
#include <fstream>
using namespace std;

#if 0
#ifdef __linux__
#define closesocket(fd) close(fd)
#define Sleep(delay) usleep(delay*1000)

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#define SET_SOCKBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)&~O_NONBLOCK)
#define SET_SOCKNONBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK)
#endif
#endif

CCommonSocket::CCommonSocket(int timeout):
	_socket(INVALID_SOCKET),
	_timeout(timeout)
{
	memset(&_sockaddr, 0, sizeof(_sockaddr));
}


CCommonSocket::~CCommonSocket()
{
	CloseSocket();
}

bool	CCommonSocket::CreateSocket()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	// 设置接收、发送超时
#ifdef WIN32
	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&_timeout, sizeof(int));
	setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&_timeout, sizeof(int));
#else
	struct timeval tv = { _timeout/1000, 0 };
	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
	setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(struct timeval));
#endif
	// SO_RCVBUF  为接收确定缓冲区大小   
	int ret = MAX_RECV_BUFFER_COMMSOCKET;
	setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&ret, sizeof(ret));
	// SO_LINGER选项，如关闭时有未发送数据，则逗留。	
	linger  lg;	lg.l_onoff = 1;
	lg.l_linger = 4;
	setsockopt(_socket, SOL_SOCKET, SO_LINGER, (char*)&lg, sizeof(lg));

	return _socket == INVALID_SOCKET ? false : true;
}

bool	CCommonSocket::CloseSocket()
{
	try{
		if (_socket != INVALID_SOCKET){
			closesocket(_socket);
			_socket = INVALID_SOCKET;
		}
	}
	catch(...){
		return false;
	}
	return true;
}

bool	CCommonSocket::SelectRead()
{
	fd_set fdread;
	struct timeval tv;

	FD_ZERO(&fdread);
	FD_SET(_socket, &fdread);

	tv.tv_sec = _timeout / 1000;
	tv.tv_usec = _timeout % 1000 * 1000;

	int ret = select(_socket + 1, &fdread, NULL, NULL, &tv);
	return ret > 0 ? true : false;
}

bool	CCommonSocket::SelectWrite()
{
	fd_set fdwrite;
	struct timeval tv;

	FD_ZERO(&fdwrite);
	FD_SET(_socket, &fdwrite);

	tv.tv_sec = _timeout / 1000;
	tv.tv_usec = _timeout % 1000 * 1000;

	int ret = select(_socket + 1, NULL, &fdwrite, NULL, &tv);
	return ret > 0 ? true : false;
}

bool	CCommonSocket::Connect(const char* ip, const int port)
{	
	CloseSocket();

	if (!CreateSocket()){
		return false;
	}
#ifdef _WIN32
	// non-block
	u_long mode = 1;
	ioctlsocket(_socket, FIONBIO, &mode);
#else
	SET_SOCKNONBLOCK(_socket);
#endif
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons(port);
	_sockaddr.sin_addr.s_addr = inet_addr(ip);
#ifdef __linux__
	errno = 0;
#endif
	if (connect(_socket, (struct sockaddr*)&_sockaddr, sizeof(_sockaddr)) == SOCKET_ERROR){
		int count = _timeout / 10;// 将超时按10ms切分
		while (count-- > 0)
		{
			Sleep(10);
			connect(_socket, (struct sockaddr*)&_sockaddr, sizeof(_sockaddr));
#ifdef _WIN32
			// 连接成功
			if (WSAGetLastError() == WSAEISCONN){
				break;
			}
#else
			if (EISCONN == errno)
				break;
#endif
		}
		// 超时
		if (count <= 0){
			CloseSocket();
			return false;
		}
	}
#ifdef _WIN32
	// block
	mode = 0;
	ioctlsocket(_socket, FIONBIO, &mode);
#else
	SET_SOCKBLOCK(_socket);
#endif
	return true;
}

bool	CCommonSocket::Send(const char* data, const int dataLen)
{
	int ret;
	int nLeft = dataLen;
	int nSent = 0;

	while (nLeft > 0)
	{
		if (!SelectWrite())
			break;

		ret = send(_socket, data + nSent, nLeft, 0);
		if (ret <= 0)break;

		nLeft -= ret;
		nSent += ret;
	}
		
	return nLeft == 0 ? true : false;
}

bool	CCommonSocket::Recv(char *buff, const int buffLen)
{
	int ret;

	if (!SelectRead())
		return false;

	ret = recv(_socket, buff, buffLen, 0);
	return ret > 0 ? true : false;
}

bool	CCommonSocket::RecvData(char *buff, const int dataLen)
{
	int ret = 0;
	int nLeft = dataLen;
	char *pDst = buff;

	while (nLeft > 0)
	{
		if (!SelectRead())
			break;

		ret = recv(_socket, pDst, nLeft, 0);
		if (ret <= 0){
			break;
		}
		pDst += ret;
		nLeft -= ret;
	}

	return nLeft == 0 ? true : false;
}

bool CCommonSocket::SendFile(const char* filename)
{
	ifstream file(filename, ios::binary | ios::in);
	if (!file.is_open()) {
		return false;
	}
	
	int fileLength = 0;
	file.seekg(0, ios::end);
	fileLength = file.tellg();
	file.seekg(0, ios::beg);
	if (fileLength <= 0) {
		file.close();
		return false;
	}

	int totalLen = 0;
	char buffer[1024];
	while (!file.eof())
	{
		file.read(buffer, 1024);
		if (!Send(buffer, file.gcount())) {
			break;
		}
		totalLen += file.gcount();
	}
	file.close();
	
	return (totalLen == fileLength ? true : false);
}

