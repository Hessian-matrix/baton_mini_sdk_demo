#pragma once

#include <pthread.h>
#include "CommonSocket.h"
#include "vio_sdk.h"

typedef struct
{
	int running;
	int timeout;
	int no_heartbeat;// >=3 is disconnected
	int connected;
	CCommonSocket* keepalive_sock;
	pthread_t keepalive_thread;
	int command_port;
	int heartbeat_port;
	char ipaddr[32];
	char username[32];
	char password[32];
	void* userdata;
	vio_connect_callback connect_cb;
	vio_event_callback event_cb;
}DeviceHandle;

typedef struct
{
	int running;
	int channel;
	pthread_t stream_thread;
	DeviceHandle* devHandle;
	vio_stream_callback stream_cb;
}StreamHandle;

//#pragma pack(1)
typedef struct
{
	unsigned int	magic;//0x33cccc33 fix value
	unsigned int 	type;
	double			timestamp;
	unsigned int	seq;
	unsigned int 	width;
	unsigned int 	height;
	unsigned int 	length;
	char* data;
}FrameInfo;
//#pragma pack(pop)

int vio_keepalive_send_heartbeat(int fd);

