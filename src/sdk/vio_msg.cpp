#include "vio_msg.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//static int vio_msg_build_body_network(std::string& body, vio_network_cfg_s* cfg);
//static int vio_msg_parse_body_network(std::string body, vio_network_cfg_s* cfg);

//static int vio_msg_build_body_lens(std::string& body, vio_lens_cfg_s* cfg);
//static int vio_msg_parse_body_lens(std::string& body, vio_lens_cfg_s* cfg);

//static int vio_msg_build_body_smart(std::string& body, vio_smart_cfg_s* cfg);

//build http request msg
static int vio_msg_build_reqpkt(std::string& reqmsg, vio_method_e method, const char* uri, const char* body, const int bodylen)
{
    char line[256];
    if (!uri)
        return -1;
    if (method == vio_method_get)
        reqmsg.append("GET ");
    else
        reqmsg.append("PUT ");
    reqmsg.append(uri).append(" HTTP/1.0\r\n");
    if (body && bodylen > 0) {
        reqmsg.append("Content-Type: application/json\r\n");
        sprintf(line, "Content-Length: %d\r\n", bodylen);
        reqmsg.append(line);
    }
    reqmsg.append("\r\n");
    if (body && bodylen > 0)
        reqmsg.append(body);
    return reqmsg.length();
}

//parse body from http response msg
static int vio_msg_parse_body(std::string respmsg, std::string& body)
{
    const char* frame = respmsg.c_str();
    const char* p = frame;
    p = strstr(frame, "\r\n\r\n");
    if (!p)
        return 0;
    p += 4;
    if (respmsg.length() - (p - frame) <= 0)
        return 0;//no content
    body.append(p, respmsg.length() - (p - frame));
    return body.length();
}

static int WriteValue_cJSON(cJSON* obj, const char* name, const char* value)
{
    if (!obj || !name || !value)
        return 0;
    cJSON_AddStringToObject(obj, name, value);
    return 1;
}

static char* GetStringValue_cJSON(cJSON* obj, const char* name, char* value, int size)
{
    if (!obj || !name)
        return 0;
    cJSON* child = cJSON_GetObjectItem(obj, name);
    if (child && child->valuestring) {
        if (value && size > strlen(child->valuestring)) {
            memcpy(value, child->valuestring, strlen(child->valuestring));
        }
        return child->valuestring;
    }
    return 0;
}

static int GetIntValue_cJSON(cJSON* obj, const char* name, int default_value = -1)
{
    if (!obj || !name)
        return default_value;
    cJSON* child = cJSON_GetObjectItem(obj, name);
    if (child) 
        return child->valueint;
    else
        return default_value;
}

static double GetDoubleValue_cJSON(cJSON* obj, const char* name, double default_value = (-1.0))
{
    if (!obj || !name)
        return default_value;
    cJSON* child = cJSON_GetObjectItem(obj, name);
    if (child)
        return child->valuedouble;
    else
        return default_value;
}

int vio_msg_check_resp(std::string respmsg)
{
    const char* frame = respmsg.c_str();
    const char* p = frame;
    int len = 0, status_code;
    char line[256] = { 0 };
    char ver[256] = { 0 };
    char status_msg[256] = { 0 };
    p = strstr(frame, "\r\n");
    if (!p)
        return -1;
    if (p - frame > 256)
        return -1;
    memcpy(line, frame, p - frame);
    if (strncmp(line, "HTTP", 4) == 0)
        if (sscanf(line, "%s %d %s", ver, &status_code, status_msg) == 3)
            return status_code;
    return -1;
}

#if 0
int vio_msg_build_body_network(std::string& body, vio_network_cfg_s* cfg)
{
    int ret;
    cJSON* obj = cJSON_CreateObject();
    if (!obj)
        return -1;

    do
    {
        ret = 0;
        if (!WriteValue_cJSON(obj, "ipaddr", cfg->ipaddr))
            break;
        if (!WriteValue_cJSON(obj, "submask", cfg->submask))
            break;
        if (!WriteValue_cJSON(obj, "gateway", cfg->gateway))
            break;
        if (!WriteValue_cJSON(obj, "macaddr", cfg->macaddr))
            break;
        cJSON_AddNumberToObject(obj, "heartbeatPort", cfg->heartbeatPort);
        cJSON_AddNumberToObject(obj, "commandPort", cfg->commandPort);
        cJSON_AddNumberToObject(obj, "udpPort", cfg->udpPort);
        ret = 1;
    } while (0);

    if (ret == 0) {
        cJSON_Delete(obj);
        return 0;
    }

    char* out = cJSON_PrintUnformatted(obj);
    body.append(out);
    free(out);
    cJSON_Delete(obj);
    return body.length();
}

int vio_msg_parse_body_network(std::string body, vio_network_cfg_s* cfg)
{
    cJSON* obj = cJSON_Parse(body.c_str());
    if (obj) {
        GetStringValue_cJSON(obj, "ipaddr", cfg->ipaddr, sizeof(cfg->ipaddr));
        GetStringValue_cJSON(obj, "submask", cfg->submask, sizeof(cfg->submask));
        GetStringValue_cJSON(obj, "gateway", cfg->gateway, sizeof(cfg->gateway));
        GetStringValue_cJSON(obj, "macaddr", cfg->macaddr, sizeof(cfg->macaddr));
        cfg->commandPort = GetIntValue_cJSON(obj, "commandPort");
        cfg->heartbeatPort = GetIntValue_cJSON(obj, "heartbeatPort");
        cfg->udpPort = GetIntValue_cJSON(obj, "udpPort");
        cJSON_Delete(obj);
        return 1;
    }
    return 0;
}

int vio_msg_build_reqpkt_network(std::string& reqmsg, vio_method_e method, vio_network_cfg_s* cfg)
{
    int ret;
    std::string body = "";
    if (method == vio_method_put) {
        if (!cfg)
            return -1;
        ret = vio_msg_build_body_network(body, cfg);
        if (ret <= 0)
            return ret;
    }
    return vio_msg_build_reqpkt(reqmsg, method, "/System/network", body.c_str(), body.length());
}

int vio_msg_parse_resppkt_network(std::string respmsg, vio_network_cfg_s* cfg)
{
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0)
        return vio_msg_parse_body_network(body, cfg);
    else
        return 0;
}
#endif

int vio_msg_build_reqpkt_stream(std::string& reqmsg, int channel)
{
    char uri[256];
    sprintf(uri, "/Stream?Channel=%d", channel);
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, uri, NULL, 0);
}

int vio_msg_build_reqpkt_reboot_system(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_put, "/System/reboot", NULL, 0);
}

int vio_msg_build_reqpkt_system_shutdown(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_put, "/System/shutdown", NULL, 0);
}

int vio_msg_build_reqpkt_algorithm_control(std::string& reqmsg, int type, int cmd)
{
    char uri[1024];

    if (cmd == 0)
        sprintf(uri, "/Algorithm/disable/%d", type);
    else if(cmd == 1)
        sprintf(uri, "/Algorithm/enable/%d", type);
    else if (cmd == 2)
        sprintf(uri, "/Algorithm/reboot/%d", type);
    else if (cmd == 3)
        sprintf(uri, "/Algorithm/reset/%d", type);
    else
        return -1;

    return vio_msg_build_reqpkt(reqmsg, vio_method_put, uri, NULL, 0);
}
#if 0
int vio_msg_build_reqpkt_SmartRelocation(std::string& reqmsg, vio_mat3x4_s* mat)
{
    std::string body = "";
    cJSON* arr = cJSON_CreateArray();
    if (arr) {
        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                cJSON_AddItemToArray(arr, cJSON_CreateNumber(mat->value[i][j]));
            }
        }
        char* out = cJSON_PrintUnformatted(arr);
        body.append(out);
        free(out);
        cJSON_Delete(arr);
    }
    if (!body.empty())
        return vio_msg_build_reqpkt(reqmsg, vio_method_put, "/Smart/relocation", body.c_str(), body.length());
    else
        return 0;
}
#endif
int vio_msg_build_reqpkt_SmartAddKeyFrame(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_put, "/Smart/addKeyFrame", NULL, 0);
}

int vio_msg_build_reqpkt_SmartSaveKeyFrame(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_put, "/Smart/saveKeyFrame", NULL, 0);
}
#if 0
int vio_msg_build_reqpkt_imu_inter(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, "/Config/imuInter", NULL, 0);
}

int vio_msg_parse_resppkt_imu_inter(std::string respmsg, vio_imu_inter_cfg_s* cfg)
{
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0) {
        cJSON* obj = cJSON_Parse(body.c_str());
        if (obj) {
            cfg->acc_n = GetDoubleValue_cJSON(obj, "acc_n");
            cfg->acc_w = GetDoubleValue_cJSON(obj, "acc_w");
            cfg->gyr_n = GetDoubleValue_cJSON(obj, "gyr_n");
            cfg->gyr_w = GetDoubleValue_cJSON(obj, "gyr_w");
            cJSON_Delete(obj);
            obj = NULL;
            return 1;
        }
    }
    return 0;
}

static int vio_msg_build_body_lens(std::string& body, vio_lens_cfg_s* cfg)
{
    cJSON* obj = cJSON_CreateObject();
    if (obj) {
        cJSON_AddNumberToObject(obj, "focal_length_x", cfg->focal_length_x);
        cJSON_AddNumberToObject(obj, "focal_length_y", cfg->focal_length_x);
        cJSON_AddNumberToObject(obj, "optical_center_point_x", cfg->optical_center_point_x);
        cJSON_AddNumberToObject(obj, "optical_center_point_y", cfg->optical_center_point_y);
        cJSON_AddNumberToObject(obj, "radia_distortion_coef_k1", cfg->radia_distortion_coef_k1);
        cJSON_AddNumberToObject(obj, "radia_distortion_coef_k2", cfg->radia_distortion_coef_k2);
        cJSON_AddNumberToObject(obj, "radia_distortion_coef_k1", cfg->tangential_distortion_p1);
        cJSON_AddNumberToObject(obj, "radia_distortion_coef_k2", cfg->tangential_distortion_p2);

        char* out = cJSON_PrintUnformatted(obj);
        body.append(out);
        free(out);
        cJSON_Delete(obj);
        return body.length();
    }
    return -1;
}

static int vio_msg_parse_body_lens(std::string& body, vio_lens_cfg_s* cfg)
{
    cJSON* obj = cJSON_Parse(body.c_str());
    if (obj) {
        cfg->focal_length_x = GetDoubleValue_cJSON(obj, "focal_length_x");
        cfg->focal_length_y = GetDoubleValue_cJSON(obj, "focal_length_y");
        cfg->optical_center_point_x = GetDoubleValue_cJSON(obj, "optical_center_point_x");
        cfg->optical_center_point_y = GetDoubleValue_cJSON(obj, "optical_center_point_y");
        cfg->radia_distortion_coef_k1 = GetDoubleValue_cJSON(obj, "radia_distortion_coef_k1");
        cfg->radia_distortion_coef_k2 = GetDoubleValue_cJSON(obj, "radia_distortion_coef_k2");
        cfg->tangential_distortion_p1 = GetDoubleValue_cJSON(obj, "tangential_distortion_p1");
        cfg->tangential_distortion_p2 = GetDoubleValue_cJSON(obj, "tangential_distortion_p2");
        cJSON_Delete(obj);
        return 1;
    }
    return -1;
}

int vio_msg_build_reqpkt_lens(std::string& reqmsg, vio_method_e method, int camera, vio_lens_cfg_s* cfg)
{
    int ret;
    std::string body = "";
    if (method == vio_method_put) {
        if (!cfg)
            return -1;
        ret = vio_msg_build_body_lens(body, cfg);
        if (ret <= 0)
            return ret;
    }
    char uri[256] = { 0 };
    sprintf(uri, "/Config/lens?Camera=%d", camera);
    return vio_msg_build_reqpkt(reqmsg, method, uri, body.c_str(), body.length());
}

int vio_msg_parse_resppkt_lens(std::string respmsg, vio_lens_cfg_s* cfg)
{
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0)
        return vio_msg_parse_body_lens(body, cfg);
    else
        return 0;
}

static int vio_msg_build_body_smart(std::string& body, vio_smart_cfg_s* cfg)
{
    cJSON* obj = cJSON_CreateObject();
    if (obj) {
        cJSON_AddNumberToObject(obj, "gray_image_enable", cfg->gray_image_enable);
        cJSON_AddNumberToObject(obj, "imu_enable", cfg->imu_enable);
        cJSON_AddNumberToObject(obj, "tof_enable", cfg->tof_enable);
        cJSON_AddNumberToObject(obj, "tof_deep_image_enable", cfg->tof_deep_image_enable);
        cJSON_AddNumberToObject(obj, "tof_amp_image_enable", cfg->tof_amp_image_enable);

        char* out = cJSON_PrintUnformatted(obj);
        body.append(out);
        free(out);
        cJSON_Delete(obj);
        return body.length();
    }
    return -1;
}

int vio_msg_build_reqpkt_smart(std::string& reqmsg, vio_method_e method, vio_smart_cfg_s* cfg)
{
    int ret;
    std::string body = "";
    if (method == vio_method_put) {
        if (!cfg)
            return -1;
        ret = vio_msg_build_body_smart(body, cfg);
        if (ret <= 0)
            return ret;
    }
    return vio_msg_build_reqpkt(reqmsg, method, "/Config/smart", body.c_str(), body.length());
}

int vio_msg_parse_resppkt_smart(std::string respmsg, vio_smart_cfg_s* cfg)
{
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0){
        cJSON* obj = cJSON_Parse(body.c_str());
        if (obj) {
            cfg->gray_image_enable = GetIntValue_cJSON(obj, "gray_image_enable");
            cfg->imu_enable = GetIntValue_cJSON(obj, "imu_enable");
            cfg->tof_enable = GetIntValue_cJSON(obj, "tof_enable");
            cfg->tof_deep_image_enable = GetIntValue_cJSON(obj, "tof_deep_image_enable");
            cfg->tof_amp_image_enable = GetIntValue_cJSON(obj, "tof_amp_image_enable");
            cJSON_Delete(obj);
            return 1;
        }
    }
    return 0;
}

static int vio_msg_parse_resppkt_mat3x4(std::string respmsg, vio_mat3x4_s* cfg)
{
    int ret;
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0) {
        cJSON* obj = cJSON_Parse(body.c_str());
        if (obj) {
            ret = 0;
            int arrsize = cJSON_GetArraySize(obj);
            if (arrsize == sizeof(vio_mat3x4_s) / sizeof(cfg->value[0][0])) {
                int index = 0;
                for (size_t i = 0; i < 3; i++)
                {
                    for (size_t j = 0; j < 4; j++)
                    {
                        cfg->value[i][j] = cJSON_GetArrayItem(obj, index)->valuedouble;
                        index++;
                    }
                }
                ret = 1;
            }
            cJSON_Delete(obj);
            return ret;
        }
    }
    return 0;
}

int vio_msg_build_reqpkt_cam2imu(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, "/Config/cam2imu", NULL, 0);
}

int vio_msg_parse_resppkt_cam2imu(std::string respmsg, vio_mat3x4_s* cfg)
{
    return vio_msg_parse_resppkt_mat3x4(respmsg, cfg);
}

int vio_msg_build_reqpkt_imu2cam(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, "/Config/imu2cam", NULL, 0);
}

int vio_msg_parse_resppkt_imu2cam(std::string respmsg, vio_mat3x4_s* cfg)
{
    return vio_msg_parse_resppkt_mat3x4(respmsg, cfg);
}

int vio_msg_build_reqpkt_tof2imu(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, "/Config/tof2imu", NULL, 0);
}

int vio_msg_parse_resppkt_tof2imu(std::string respmsg, vio_mat3x4_s* cfg)
{
    return vio_msg_parse_resppkt_mat3x4(respmsg, cfg);
}

int vio_msg_build_reqpkt_imu2tof(std::string& reqmsg)
{
    return vio_msg_build_reqpkt(reqmsg, vio_method_get, "/Config/imu2tof", NULL, 0);
}

int vio_msg_parse_resppkt_imu2tof(std::string respmsg, vio_mat3x4_s* cfg)
{
    return vio_msg_parse_resppkt_mat3x4(respmsg, cfg);
}

int vio_msg_build_reqpkt_osd(std::string& reqmsg, vio_method_e method, vio_osd_cfg_s* cfg)
{
    int ret = 0;
    std::string body = "";
    if (method == vio_method_put) {
        if (!cfg)
            return -1;
        cJSON* obj = cJSON_CreateObject();
        if (obj) {
            cJSON_AddNumberToObject(obj, "dispaly_feature_pots", cfg->dispaly_feature_pots);
            cJSON* DateTime = cJSON_CreateObject();
            if (DateTime) {
                cJSON_AddNumberToObject(DateTime, "display", cfg->osd_datetime.display);
                cJSON_AddNumberToObject(DateTime, "x", cfg->osd_datetime.x);
                cJSON_AddNumberToObject(DateTime, "y", cfg->osd_datetime.y);
            }
            cJSON_AddItemToObject(obj, "DateTime", DateTime);
            char* out = cJSON_PrintUnformatted(obj);
            body.append(out);
            free(out);
            cJSON_Delete(obj);
        }
        if (body.empty())
            return -1;
    }
    return vio_msg_build_reqpkt(reqmsg, method, "/Config/osd", body.c_str(), body.length());
}

int vio_msg_parse_resppkt_osd(std::string respmsg, vio_osd_cfg_s* cfg)
{
    std::string body = "";
    if (!cfg)
        return -1;
    if (vio_msg_parse_body(respmsg, body) > 0) {
        cJSON* obj = cJSON_Parse(body.c_str());
        if (obj) {
            cfg->dispaly_feature_pots = GetIntValue_cJSON(obj, "dispaly_feature_pots");
            cJSON* DateTime = cJSON_GetObjectItem(obj, "DateTime");
            if (DateTime) {
                cfg->osd_datetime.display = GetIntValue_cJSON(DateTime, "display");
                cfg->osd_datetime.x = GetIntValue_cJSON(DateTime, "x");
                cfg->osd_datetime.y = GetIntValue_cJSON(DateTime, "y");
            }
            cJSON_Delete(obj);
            return 1;
        }
    }
    return 0;
}
#endif

int vio_private_msg_parse_from_array(const char* data, int size, int* content_len, std::string& content_data)
{
    int ret;
    char strlen[8] = { 0 };
    const char* frame = data;
    const char* p = frame;

    //check pkt header : "ZCKJ"
    ret = 0;
    for (size_t i = 0; i < size; i++)
    {
        p = strstr(frame + i, "ZCKJ");
        if (p) {
            ret = 1;
            break;
        }
    }
    if (!ret)
        return 0;// no found header

    //check msg length
    p += 4;
    if (p - frame + 4 > size)
        return 0;// no found length
    strncpy(strlen, p, 4);
    strlen[4] = '\0';
    *content_len = atoi(strlen);
    if (*content_len <= 0 || *content_len > 1024)
        return -1;//content length too large

    //check content
    p += 4;
    if (p - frame + *content_len > size)
        return -2;// content not enough

    content_data.append(p, *content_len);
    ret = p - frame + *content_len;
    return ret;// return real msg size
}

vio_private_msg_cmd_type_e vio_private_msg_parse_cmd_type(std::string content)
{
    vio_private_msg_cmd_type_e cmd_type = vio_private_msg_null;
    if (!content.empty()) {
        cJSON* json = cJSON_Parse(content.c_str());
        if (json) {
            cJSON* child;
            child = cJSON_GetObjectItem(json, "Command");
            if (child && child->valuestring)
                if (strcmp(child->valuestring, "Heartbeat") == 0)
                    cmd_type = vio_private_msg_heartbeat;
            cJSON_Delete(json);
        }
    }
    return cmd_type;
}

int vio_private_msg_build_heartbeat(std::string& msg)
{
    msg.append("ZCKJ0023{\"Command\":\"Heartbeat\"}");
    return msg.length();
}
