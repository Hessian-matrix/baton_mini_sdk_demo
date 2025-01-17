#include "CHttpRequest.h"
#include "Base64.h"
#include "stdtostring.h"

#include <stdlib.h>
#include <stdio.h>

CHttpRequest::CHttpRequest(const char* ip, int port):
	m_contentLen(0),
	m_numOfHeaders(0),
	m_contentStr(""),
	m_ip(string(ip)),
	m_port(port),
	m_username(""),
	m_password("")
{

}

CHttpRequest::~CHttpRequest()
{

}

int CHttpRequest::ReadLine(char* buffer, int size)
{
	char c;
	int i;
	
	if(buffer == NULL || size <= 0)
		return -1;

	i = 0;
	while(i < size){
		if(Recv(&c,1)){
			buffer[i] = c;
			if(i > 0){
				if (buffer[i-1] == '\r' && buffer[i] == '\n'){
					return i+1;
				}
			}
			i++;
		}else{
			break;
		}
	}
	return 0;
}

int CHttpRequest::ReadHeader()
{
	char buffer[1024];

	m_contentLen = 0;
	m_numOfHeaders = 0;
	memset(m_headers,'\0',sizeof(m_headers));

	while(m_numOfHeaders < HEADER_NUM){
		memset(buffer,0,1024);
		if(ReadLine(buffer,1024) > 0){
			if(buffer[0]=='\r'&&buffer[1]=='\n')
				break;			
			char* p =strstr(buffer,": ");
			char* p2 = strstr(buffer,"\r\n");
			if(p && p2){				
//				sscanf(buffer, "%[^:]", m_headers[m_numOfHeaders].name);
				memcpy(m_headers[m_numOfHeaders].name,buffer,p-buffer);
				memcpy(m_headers[m_numOfHeaders].value,p+2,p2-p-2);
				if (!strncmp(m_headers[m_numOfHeaders].name, "Content-Length", 14)){
					m_contentLen = atoi(m_headers[m_numOfHeaders].value);
				}
			}
			m_numOfHeaders++;
		}else{
			break;
		}
	}
#if 0
	for(int i = 0;i < m_numOfHeaders;i++)
		printf("%s: %s\n",m_headers[i].name, m_headers[i].value);
#endif
	return m_numOfHeaders;
}

int CHttpRequest::ReadContent()
{
	if(m_contentLen <= 0)
		return -1;

	m_contentStr = "";
	char* buffer = new char[m_contentLen+1];
	memset(buffer,'\0',m_contentLen+1);

	bool bRet = Recv(buffer,m_contentLen);
	if(bRet){
		m_contentStr = std::string(buffer);
#if 0
		printf("%s\n",buffer);
#endif
	}
	delete []buffer;
	buffer = NULL;
	return bRet ? m_contentLen : 0;
}

int CHttpRequest::Request(
	const char* pMethod, 
	const char* pUri, 
	const char* pContentBuf,
	int contentLen)
{	
	if(!pMethod || !pUri)
		return -1;

	std::string req = string(pMethod)+" "+string(pUri)+" HTTP/1.0\r\n";
	if(!m_username.empty() && !m_password.empty()){
		CBase64 base64;
		std::string str = m_username+":"+m_password;
		std::string strEncoded = base64.encode(str.c_str(),str.length());
		req.append("Authorization: Basic ").append(strEncoded);
		req.append("\r\n");
	}
	if(contentLen > 0){
		req.append("Content-Type: application/json\r\n");
		req.append("Content-Length: ").append(std::to_string(contentLen));
		req.append("\r\n");
	}
	req.append("\r\n");
	if(pContentBuf){
		req.append(string(pContentBuf));
	}
	
	if(!Connect((char*)m_ip.c_str(), m_port))
		return -2;
	if(!Send((char*)req.c_str(),req.length()))
		return -3;
#if 0
	printf("Sent to %s:%d:\n",m_ip.c_str(), m_port);
	printf("%s",req.c_str());
	printf("Sent end\n");
#endif
	if(Response() <= 0)
		return -4;
	else
		return 1;	
}

int CHttpRequest::Response()
{
	char buffer[1024] = { 0 };
	char version[32] = { 0 };
	char statusMessage[256] = { 0 };
	int statusCode = -1;
	int ret;

	ret = ReadLine(buffer,1024);
	if(ret > 0){
		sscanf(buffer,"%s %d %s",version,&statusCode,statusMessage);
//		printf("%s %d %s\n",
//			version,statusCode,statusMessage);
		if(ReadHeader() > 0){
			ret = ReadContent();
			if(ret != 0)
				return statusCode;// No content or recv content success.
			else
				return 0;// Recv content failed.
		}
	}
	return -1;
}

