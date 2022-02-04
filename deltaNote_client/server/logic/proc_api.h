/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_SERVER_PROC_API_H
#define NOTE_SERVER_PROC_API_H

#include <string>
#include "s_inter_var.h"
#include "s_sync_data.h"

// 定义处理接口
class ProcAPI{
public:
    // 处理注册
    static int api_sign_up(const std::string& req, std::string& res);
    // 处理登录
    static int api_sign_in(const std::string& req, std::string& res);
    // 处理上传
    static int api_upload(const std::string& req, std::string& res);
    // 处理下载
    static int api_download(const std::string& req, std::string& res);

private:
    static SSyncData *get_sync_ctrl();
    // 接口管理对象
    static SSyncData *m_sync_data;
};

#endif //NOTE_SERVER_PROC_API_H
