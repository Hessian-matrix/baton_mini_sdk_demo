#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_RECV_BUFFER_COMMSOCKET (512 * 1024)

class CCommonSocket
{
public:
	CCommonSocket(int timeout = 3000);
	virtual ~CCommonSocket();

public:
	bool	Connect(const char* ip, const int port);
	bool	Send(const char* data, const int dataLen);
	bool	Recv(char *buff, const int buffLen);
	bool	RecvData(char *buff, const int dataLen);
	bool	SendFile(const char* filename);

	int		_timeout;
	int		_socket;
private:
	struct sockaddr_in	_sockaddr;

	bool	CreateSocket();
	bool	CloseSocket();
	bool	SelectRead();
	bool	SelectWrite();
};

class CBcastSocket
{
public:
	CBcastSocket();
	~CBcastSocket();
public:
	struct sockaddr_in _from;
	socklen_t		_fromLen;
	bool	CreateSocket(int timeout = 3000);
	bool	Bind(const char* ip,const int port);
	bool	SendMsg(const char* msg,const int msgLen,const char* bcastAddr,const int bcastPort);
	int		RecvMsg(char* buf,const int bufLen);
protected:
private:
	int		_timeout;
	int		_socket;
	bool	CloseSocket();
};
