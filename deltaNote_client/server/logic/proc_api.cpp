/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "proc_api.h"
SSyncData *ProcAPI::m_sync_data = nullptr;
int ProcAPI::api_sign_up(const std::string &req, std::string &res) {
    return get_sync_ctrl()->sync_sign_up(req, res);
}

int ProcAPI::api_sign_in(const std::string &req, std::string &res) {
    return get_sync_ctrl()->sync_sign_in(req, res);
}

int ProcAPI::api_upload(const std::string &req, std::string &res) {
    return get_sync_ctrl()->sync_upload(req, res);
}

int ProcAPI::api_download(const std::string &req, std::string &res) {
    return get_sync_ctrl()->sync_download(req, res);
}

SSyncData *ProcAPI::get_sync_ctrl() {
    // TODO 非线程安全
    if(m_sync_data == nullptr){
        m_sync_data = new SSyncData();
    }
    return m_sync_data;
}
