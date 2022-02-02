#ifndef SERVER_SYNC_DATA_H
#define SERVER_SYNC_DATA_H

#include <string>
#include "inter_var.h"
#include "s_data_ctrl.h"
#include "s_user_ctrl.h"

// 定义处理接口
class SSyncData{
public:
    // 初始化过程
    static int sync_init();
    static int sync_uninit();
    // 处理注册
    static int sync_sign_up(const std::string& req, std::string& res, ErrorCode &error_code);

    // 处理登录
    static int sync_sign_in(const std::string& req, std::string& res, ErrorCode &error_code);

    // 处理上传
    static int sync_upload(SyncStatus &net_status, ErrorCode &error_code);
    // 处理下载
    static int sync_download(SyncStatus &net_status, ErrorCode &error_code);

private:
    // 数据打包：返回pack
    static std::string pack_packet(const UserItem &data_item, SyncStatus net_status, const std::string &group_data);
    static int unpack_packet(const std::string &pack, UserItem &data_item, std::string &group_data, ErrorCode &error_code);
private:
    static SDataCtrl *m_data_ctrl;
    static SUserCtrl *m_user_ctrl;
};

#endif //SERVER_SYNC_DATA_H
