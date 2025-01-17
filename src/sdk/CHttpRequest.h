#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "CommonSocket.h"

#include <string>
using namespace std;

#define HEADER_NUM 32
#define HEADER_NAME_LEN 64
#define HEADER_VALUE_LEN 256

typedef struct
{
	char name[HEADER_NAME_LEN];
	char value[HEADER_VALUE_LEN];
}HttpHeader;

class CHttpRequest : public CCommonSocket{
public:
	CHttpRequest(const char* ip, int port);
	~CHttpRequest();

	int m_numOfHeaders;
	HttpHeader m_headers[HEADER_NUM];
	int m_contentLen;
	std::string m_contentStr;
	std::string m_ip;
	int m_port;
	std::string m_username;
	std::string m_password;

	int ReadLine(char* buffer, int size);
	int ReadHeader();
	int ReadContent();
	int Request(
		const char* pMethod,
		const char* pUri,
		const char* pContentBuf,
		int contentLen);
	int Response();
};

#endif
