/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_SERVER_SYNC_DATA_H
#define NOTE_SERVER_SYNC_DATA_H
#include <string>
#include "json.h"
#include "inter_var.h"
#include "s_user_ctrl.h"

class SSyncData{
public:
    // 注册
    int sync_sign_up(const std::string& req, std::string& res);
    // 登录
    int sync_sign_in(const std::string& req, std::string& res);
    // 上传
    int sync_upload(const std::string& req, std::string& res);
    // 下载
    int sync_download(const std::string& req, std::string& res);
private:
    // 校验用户
    int check_user(const std::string &req, UserItem &user_item, SyncStatus &sync_status, Json::Value &group_json, ErrorCode &error_code);
    // 数据打包：返回pack string
    std::string pack_packet(const UserItem &data_item, SyncStatus sync_status, const std::string &group_data);
    // 数据包解包
    int unpack_packet(const std::string &pack, UserItem &req_user_item, UserItem &db_user_item, Json::Value &req_group_json, ErrorCode &error_code);
private:
    SUserCtrl m_user_ctrl;
    Json::Reader m_json_reader;
};
#endif //NOTE_SERVER_SYNC_DATA_H
