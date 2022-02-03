#include "proc_api.h"
SSyncData *ProcAPI::m_sync_data = nullptr;
int ProcAPI::api_sign_up(const std::string &req, std::string &res, ErrorCode &error_code) {
    return get_sync_ctrl()->sync_sign_up(req, res, error_code);
}

int ProcAPI::api_sign_in(const std::string &req, std::string &res, ErrorCode &error_code) {
    return get_sync_ctrl()->sync_sign_in(req, res, error_code);
}

int ProcAPI::api_upload(const std::string &req, std::string &res, ErrorCode &error_code) {
    return get_sync_ctrl()->sync_upload(req, res, error_code);
}

int ProcAPI::api_download(const std::string &req, std::string &res, ErrorCode &error_code) {
    return get_sync_ctrl()->sync_download(req, res, error_code);
}

SSyncData *ProcAPI::get_sync_ctrl() {
    // TODO 非线程安全
    if(m_sync_data == nullptr){
        m_sync_data = new SSyncData();
    }
    return m_sync_data;
}
