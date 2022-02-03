#ifndef USER_CTRL_TEST_PROCAPI_H
#define USER_CTRL_TEST_PROCAPI_H

#include <string>
#include "s_inter_var.h"
#include "s_sync_data.h"

// 定义处理接口
class ProcAPI{
public:
    // 处理注册
    static int api_sign_up(const std::string& req, std::string& res, ErrorCode &error_code);
    // 处理登录
    static int api_sign_in(const std::string& req, std::string& res, ErrorCode &error_code);
    // 处理上传
    static int api_upload(const std::string& req, std::string& res, ErrorCode &error_code);
    // 处理下载
    static int api_download(const std::string& req, std::string& res, ErrorCode &error_code);

private:
    static SSyncData *get_sync_ctrl();
    // 接口管理对象
    static SSyncData *m_sync_data;
};

#endif //USER_CTRL_TEST_PROCAPI_H
