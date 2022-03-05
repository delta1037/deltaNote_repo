/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_CLIENT_SYNC_DATA_H
#define NOTE_CLIENT_SYNC_DATA_H
#include <mutex>
#include "http.h"
#include "setting_ctrl.h"
#include "c_data_ctrl.h"
#include "json.h"

// 与服务器进行同步
// 同步逻辑可复用
class CSyncData {
public:
    CSyncData(SettingCtrl *setting_ctrl, CDataCtrl *data_ctrl);

    int sync_reset_server();
    // 获取token
    int sync_token(SyncStatus &net_status, ErrorCode &error_code);
    // 注册
    int sync_sign_up(SyncStatus &net_status, ErrorCode &error_code);
    // 登录
    int sync_sign_in(SyncStatus &net_status, ErrorCode &error_code);
    // 同步数据
    int sync_data(SyncStatus &net_status, ErrorCode &error_code);
private:
    // 上传
    int sync_upload(SyncStatus &net_status, ErrorCode &error_code);
    // 下载
    int sync_download(SyncStatus &net_status, ErrorCode &error_code);

    // 数据打包：返回pack
    std::string pack_packet(const std::string &group_data);
    int unpack_packet(const std::string &pack, std::string &group_data, SyncStatus &sync_status, ErrorCode &error_code);

private:
    HTTP *m_net_handle;
    std::mutex m_net_handle_lock;

    SettingCtrl *m_setting_ctrl;
    CDataCtrl *m_data_ctrl;

    Json::Reader m_json_reader;
};
#endif //NOTE_CLIENT_SYNC_DATA_H
