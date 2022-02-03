#ifndef SERVER_SYNC_DATA_H
#define SERVER_SYNC_DATA_H

#include <string>
#include "inter_var.h"
#include "s_data_ctrl.h"
#include "s_user_ctrl.h"
#include "json.h"

class SSyncData{
public:
    // 初始化过程
    SSyncData();
    ~SSyncData();
    // 注册
    int sync_sign_up(const std::string& req, std::string& res, ErrorCode &error_code);
    // 登录
    int sync_sign_in(const std::string& req, std::string& res, ErrorCode &error_code);
    // 上传
    int sync_upload(const std::string& req, std::string& res, ErrorCode &error_code);
    // 下载
    int sync_download(const std::string& req, std::string& res, ErrorCode &error_code);
private:
    // 校验用户
    int check_user(const std::string &req, UserItem &user_item, SyncStatus &sync_status, Json::Value &group_json, ErrorCode &error_code);
    // 数据打包：返回pack
    std::string pack_packet(const UserItem &data_item, SyncStatus sync_status, const std::string &group_data);
    int unpack_packet(const std::string &pack, UserItem &req_user_item, UserItem &db_user_item, std::string &group_data, ErrorCode &error_code);
private:
    SUserCtrl *m_user_ctrl;
    Json::Reader m_json_reader;
};


#endif //SERVER_SYNC_DATA_H
