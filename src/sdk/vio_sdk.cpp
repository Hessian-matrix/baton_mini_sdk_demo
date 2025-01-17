#include "vio_sdk.h"
#include "syslib.h"
#include "CHttpRequest.h"
#include "linuxdef.h"
#include "vio_msg.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
using namespace std;

#include "cJSON.h"

NET_VIO_API LONG vio_call net_vio_sdk_version()
{
	const LONG sdk_ver = 0x00020008;
	return sdk_ver;
}

NET_VIO_API BOOL vio_call net_vio_sdk_init()
{
#ifdef _WIN32
	curl_global_init(CURL_GLOBAL_ALL);
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		return 0;
#endif
	return 1;
}

NET_VIO_API BOOL vio_call net_vio_sdk_exit()
{
#ifdef _WIN32
	curl_global_cleanup();
	WSACleanup();
#endif
	return 1;
}

#define MAX_RECV_BUFF_SIZE 1024

void* keepalive_thread(void* arg)
{
	DeviceHandle* h = (DeviceHandle*)arg;
	int ret;
	fd_set rfds, wfds;
	struct timeval timeout;
	int recv_len = 0;
	char recv_buf[1024] = { 0 };
	CCommonSocket* sock = new CCommonSocket(3000);
	int fd;
	int heartbeat_timer = 0;

	h->keepalive_sock = sock;
	
	while (h->running)
	{
		h->connected = 0;
		do
		{
			if (!h->running)
				break;
		} while (!sock->Connect(h->ipaddr, h->heartbeat_port));
		fd = sock->_socket;
		if (h->connect_cb)
			h->connect_cb(1, h->userdata);
		h->connected = 1;

		ZeroMemory(recv_buf, MAX_RECV_BUFF_SIZE);
		recv_len = 0;
		heartbeat_timer = 0;
		h->no_heartbeat = 0;

		while (h->running)
		{
			Sleep(10);

			if (heartbeat_timer == 0) {
				vio_keepalive_send_heartbeat(fd);
			}
			heartbeat_timer++;
			if (heartbeat_timer >= 500) {
				heartbeat_timer = 0;
				h->no_heartbeat++;
				if (h->no_heartbeat >= 3) {
					if (h->connect_cb)
						h->connect_cb(0, h->userdata);
					break;
				}
			}

			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_SET(fd, &rfds);
			FD_SET(fd, &wfds);
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
			ret = select(fd + 1, &rfds, &wfds, NULL, &timeout);
			if (ret == 0)
				continue;
			if (ret < 0) {
				if (h->connect_cb)
					h->connect_cb(-1, h->userdata);
				break;
			}

			if (FD_ISSET(fd, &rfds)) {
				ret = recv(fd, recv_buf + recv_len, sizeof(recv_buf) - recv_len - 1, 0);
				if (ret == 0){
					if (h->connect_cb)
						h->connect_cb(0, h->userdata);
					break;// server close
				}
				if (ret == SOCKET_ERROR) {
#ifdef _WIN32
					int err = WSAGetLastError();
					if (err != WSAEINTR && err != WSAEWOULDBLOCK) {
#else
					if (errno != EINTR && errno != EAGAIN) {
#endif
						if (h->connect_cb)
							h->connect_cb(0, h->userdata);
						break;// network disconnected
					}
					ret = 0;
				}
				recv_len += ret;
				recv_buf[recv_len] = 0;
				if (recv_len <= 0)
					continue;

				std::string content_data = "";
				int content_len = 0;
				ret = vio_private_msg_parse_from_array(recv_buf, recv_len, &content_len, content_data);
				if (ret > 0){
					memmove(recv_buf, recv_buf + ret, ret);
					recv_len -= ret;

					vio_private_msg_cmd_type_e cmd_type;
					cmd_type = vio_private_msg_parse_cmd_type(content_data);
					if (cmd_type == vio_private_msg_heartbeat) {
						h->no_heartbeat = 0;
					}

					if (h->event_cb)
						h->event_cb(content_data.c_str(), content_data.length(), h->userdata);
				}
			}
		}
	}

	delete sock;
	sock = NULL;
	if (h->connect_cb)
		h->connect_cb(-2, h->userdata);
	return 0;
}

NET_VIO_API HANDLE vio_call net_vio_login(vio_login_info_s loginInfo)
{
	int ret;
	if (!loginInfo.connect_cb || !loginInfo.event_cb)
		return NULL;
	DeviceHandle* h = (DeviceHandle*)malloc(sizeof(DeviceHandle));
	if (!h)
		return NULL;
	memset(h, 0, sizeof(DeviceHandle));

	h->timeout = 3000;
	h->command_port = loginInfo.port;
	sprintf(h->ipaddr, "%s", loginInfo.ipaddr);

	char buff[10240]={0};
	ret = net_vio_get_cfg((HANDLE)h,"/System/network",buff,10240);
	if(!ret){
		free(h);
		return NULL;
	}

	int heartbeatPort = -1;
	
	cJSON* pJson = cJSON_Parse((const char *) buff);
	if(pJson){
		cJSON* pChild = cJSON_GetObjectItem(pJson,"heartbeatPort");
		if(pChild){
			heartbeatPort = pChild->valueint;
		}
		cJSON_Delete(pJson);
	}
	if(heartbeatPort == -1){
		free(h);
		return NULL;
	}
	
	h->connected = 0;
	h->userdata = loginInfo.userData;
	h->connect_cb = loginInfo.connect_cb;
	h->event_cb = loginInfo.event_cb;
	h->heartbeat_port = heartbeatPort;
	h->running = 1;
	ret = pthread_create(&h->keepalive_thread, NULL, keepalive_thread, h);
	for (size_t i = 0; i < h->timeout/10; i++)
	{
		Sleep(10);
		if (h->connected)
			break;
	}
	return h;
}

NET_VIO_API BOOL vio_call net_vio_logout(HANDLE loginHandle)
{
	if (loginHandle) {
		DeviceHandle* h = (DeviceHandle*)loginHandle;
		h->running = 0;
		pthread_join(h->keepalive_thread, NULL);
		free(loginHandle);
	}
	return 1;
}

#define MAX_FRAME_SIZE (5 * 1024 * 1024)

void* stream_connect_thread(void* arg)
{
	StreamHandle* hStream = (StreamHandle*)arg;
	DeviceHandle* hDevice = hStream->devHandle;
	CCommonSocket* sock = new CCommonSocket(3000);
	char buff[1024] = { 0 };
	unsigned long magic = 0x00;
	FrameInfo* frameInfo = (FrameInfo*)buff;
	char* frame = new char[MAX_FRAME_SIZE];
	std::string reqmsg;
	bool connected = false;

	while (hStream->running)
	{
		if(connected)
			sock->Send("Bye", 3);
		connected = false;
		while (1)
		{
			if (!hStream->running)
				goto _exit;
			Sleep(1000);
			connected = sock->Connect(hDevice->ipaddr, hDevice->command_port);
			if (connected)
				break;
		}

		vio_msg_build_reqpkt_stream(reqmsg, hStream->channel);
		sock->Send(reqmsg.c_str(), reqmsg.length());

		//recv magic
		ZeroMemory(buff, 1024);
		if (!sock->RecvData(buff, 4))
			continue;

		while (hStream->running)
		{
			memcpy(&magic, buff, 4);
			if (magic == 0x33cccc33)
				break;
			else {
				memmove(buff, buff + 1, 3);
			}
			if (!sock->RecvData((char*)buff + 3, 1))
				break;
		}
		if (magic != 0x33cccc33)
			continue;

		//recv frame header,no magic,no data
		if (!sock->RecvData((char*)buff + 4, 28))
			continue;
		//frame length too large
		if (frameInfo->length >= MAX_FRAME_SIZE || frameInfo->length <= 0)
			continue;
		//recv frame
		ZeroMemory(frame, MAX_FRAME_SIZE);
		if (!sock->RecvData(frame, frameInfo->length))
			continue;
		if (hStream->stream_cb) {
			vio_frame_info_s fr;
			fr.type = frameInfo->type;
			fr.timestamp = frameInfo->timestamp;
			fr.seq = frameInfo->seq;
			fr.width = frameInfo->width;
			fr.height = frameInfo->height;
			fr.length = frameInfo->length;

			hStream->stream_cb(hStream->channel, &fr, frame, hDevice->userdata);
		}

		while (hStream->running)
		{
			if (!sock->RecvData(buff, 32))
				break;
			if (frameInfo->magic != 0x33cccc33)
				break;
			if (frameInfo->length > MAX_FRAME_SIZE)
				break;
			ZeroMemory(frame, MAX_FRAME_SIZE);
			if (!sock->RecvData(frame, frameInfo->length))
				break;
			if (hStream->stream_cb) {
				vio_frame_info_s fr;
				fr.type = frameInfo->type;
				fr.timestamp = frameInfo->timestamp;
				fr.seq = frameInfo->seq;
				fr.width = frameInfo->width;
				fr.height = frameInfo->height;
				fr.length = frameInfo->length;

				hStream->stream_cb(hStream->channel, &fr, frame, hDevice->userdata);
			}
		}
	}
_exit:
	delete []frame;
	frame = NULL;
	delete sock;
	sock = NULL;
	return 0;
}

NET_VIO_API HANDLE vio_call net_vio_stream_connect(HANDLE loginHandle, int channel, vio_stream_callback cb)
{
	if (!loginHandle || !cb)
		return 0;
	StreamHandle* h = (StreamHandle*)malloc(sizeof(StreamHandle));
	if (!h)
		return NULL;
	memset(h, 0, sizeof(StreamHandle));

	h->stream_cb = cb;
	h->channel = channel;
	h->devHandle = (DeviceHandle*)loginHandle;
	h->running = 1;
	pthread_create(&h->stream_thread, NULL, stream_connect_thread, h);
	return h;
}

NET_VIO_API BOOL vio_call net_vio_stream_disconnect(HANDLE streamHandle)
{
	if (streamHandle) {
		StreamHandle* h = (StreamHandle*)streamHandle;
		if (h->running) {
			h->running = 0;
			pthread_join(h->stream_thread, NULL);
		}
		free(h);
	}
	return 1;
}

static size_t WriteDataCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	std::string result = std::string((char*)contents, size * nmemb);
	string* pStr = (string*)userp;
	if (pStr)
	{
		(*pStr).append(result);
	}
	return size * nmemb;
}

NET_VIO_API BOOL vio_call net_vio_get_cfg(HANDLE loginHandle, const char* uri, char* dataBuff, int buffSize)
{
	if (loginHandle == NULL || uri == NULL || dataBuff == NULL || buffSize <= 0)
		return false;

	DeviceHandle* hLogin = (DeviceHandle*)loginHandle;

	CHttpRequest cameraHttp(hLogin->ipaddr,hLogin->command_port);
	cameraHttp.m_username = string(hLogin->username);
	cameraHttp.m_password = string(hLogin->password);
	int ret = cameraHttp.Request("GET",uri,NULL,0);
	if(ret < 0 || cameraHttp.m_contentStr.empty())
		return false;

	if (cameraHttp.m_contentStr.length() > buffSize)
	{
		memcpy(dataBuff, cameraHttp.m_contentStr.c_str(), buffSize);
	}
	else
	{
		memcpy(dataBuff, cameraHttp.m_contentStr.c_str(), cameraHttp.m_contentStr.length());
	}
	return true;
}

NET_VIO_API BOOL vio_call net_vio_set_cfg(HANDLE loginHandle, const char* uri, const char* data, int length)
{
	if (loginHandle == NULL || uri == NULL || data == NULL || length <= 0)
		return false;

	DeviceHandle* hLogin = (DeviceHandle*)loginHandle;

	CHttpRequest cameraHttp(hLogin->ipaddr,hLogin->command_port);
	cameraHttp.m_username = string(hLogin->username);
	cameraHttp.m_password = string(hLogin->password);
	int ret = cameraHttp.Request("PUT",uri,data,length);
	if(ret < 0)
		return false;

	ret = 0;
	
	cJSON* obj = cJSON_Parse(cameraHttp.m_contentStr.c_str());
	if (obj) {
		cJSON* child = cJSON_GetObjectItem(obj, "errCode");
		if (child) {
			if (child->valueint == 200)
				ret = true;
			else
				ret = child->valueint;
		}
		cJSON_Delete(obj);
	}
	return ret;

}

