#include "syslib.h"
#include "vio_msg.h"

int vio_keepalive_send_heartbeat(int fd)
{
    std::string reqmsg = "";
    vio_private_msg_build_heartbeat(reqmsg);
    return send(fd, reqmsg.c_str(), reqmsg.length(), MSG_NOSIGNAL);
}
