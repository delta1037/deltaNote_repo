#include "c_sync_data.h"
#include "base64.h"
#include "log.h"

CSyncData::CSyncData(SettingCtrl *setting_ctrl, CDataCtrl *data_ctrl){
    this->m_setting_ctrl = setting_ctrl;
    this->m_data_ctrl = data_ctrl;

    this->m_net_handle = new HTTP(HTTP_client, "http://192.168.0.106:1234");
}

int CSyncData::sync_token(SyncStatus &sync_status, ErrorCode &error_code) {
    // 从设置中获取token, 为空重新获取
    //
    return 0;
}

int CSyncData::sync_sign_up(SyncStatus &sync_status, ErrorCode &error_code) {
    d_logic_debug("%s", "api_sign_up start")
    Json::Value req_group_data;
    req_group_data[USER_PASSWORD] = m_setting_ctrl->get_string(SETTING_PASSWORD);
    d_logic_debug("api_sign_up send group data:%s", req_group_data.toStyledString().c_str())

    // 聚合数据需要加密
    std::string t_req = pack_packet(req_group_data.toStyledString());
    std::string t_res;
    int ret = m_net_handle->c_get(SYNC_SIGN_UP, t_req, t_res, error_code);
    if(ret == RET_FAILED){
        d_logic_error("api_sign_up send %s to get data error", t_req.c_str())
        sync_status = Sync_undefined_error;
        return RET_FAILED;
    }
    d_logic_debug("api_sign_up recv data:%s", t_res.c_str())

    std::string res_group_data;
    ret = unpack_packet(t_res, res_group_data, sync_status, error_code);
    if(ret == RET_FAILED){
        d_logic_error("unpack data pack %s failed, status = %d", t_res.c_str(), sync_status)
        return RET_FAILED;
    }
    d_logic_debug("api_sign_up data unpack:%s", res_group_data.c_str())

    // TODO group data 中可以解析出来token
    return RET_SUCCESS;
}

int CSyncData::sync_sign_in(SyncStatus &sync_status, ErrorCode &error_code) {
    d_logic_debug("%s", "api_sign_in start")
    Json::Value req_group_data;
    req_group_data[USER_PASSWORD] = m_setting_ctrl->get_string(SETTING_PASSWORD);
    d_logic_debug("api_sign_up send group data:%s", req_group_data.toStyledString().c_str())

    // 聚合数据需要加密
    std::string t_req = pack_packet(req_group_data.toStyledString());
    std::string t_res;
    int ret = m_net_handle->c_get(SYNC_SIGN_IN, t_req, t_res, error_code);
    if(ret == RET_FAILED){
        d_logic_error("api_sign_up send %s to get data error", t_req.c_str())
        sync_status = Sync_undefined_error;
        return RET_FAILED;
    }
    d_logic_debug("api_sign_up recv data:%s", t_res.c_str())

    std::string res_group_data;
    ret = unpack_packet(t_res, res_group_data, sync_status, error_code);
    if(ret == RET_FAILED){
        d_logic_error("unpack data pack %s failed, status = %d", t_res.c_str(), sync_status)
        return RET_FAILED;
    }
    d_logic_debug("api_sign_up data unpack:%s", res_group_data.c_str())
    return RET_SUCCESS;
}

int CSyncData::sync_data(SyncStatus &sync_status, ErrorCode &error_code) {
    // 同步操作先上传再下载
    if(RET_FAILED == sync_upload(sync_status, error_code)){
        return RET_FAILED;
    }
    //m_data_ctrl->del_todo()
    if(RET_FAILED == sync_download(sync_status, error_code)){
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int CSyncData::sync_upload(SyncStatus &sync_status, ErrorCode &error_code) {
    d_logic_debug("%s", "api_upload start")
    Json::Value req_group_data;
    req_group_data[SYNC_PASSWORD] = m_setting_ctrl->get_string(SETTING_PASSWORD);
    TodoList t_todo_list;
    int ret = m_data_ctrl->sel_todo(ListType_OP, t_todo_list, error_code);
    if(ret == RET_FAILED){
        d_logic_error("api_upload db sel data error, coee:%d", error_code)
        sync_status = Sync_client_error;
        return RET_FAILED;
    }
    req_group_data[SYNC_TODO_LIST] = json_list(t_todo_list);
    d_logic_debug("api_upload send group data:%s", req_group_data.toStyledString().c_str())

    // 聚合数据需要加密
    std::string t_req = pack_packet(req_group_data.toStyledString());
    std::string t_res;
    ret = m_net_handle->c_post(SYNC_UPLOAD, t_req, t_res, error_code);
    if(ret == RET_FAILED){
        d_logic_error("api_upload send %s to get data error", t_req.c_str())
        sync_status = Sync_undefined_error;
        return RET_FAILED;
    }
    d_logic_debug("api_upload recv data:%s", t_res.c_str())

    std::string res_group_data;
    ret = unpack_packet(t_res, res_group_data, sync_status, error_code);
    if(ret == RET_FAILED){
        d_logic_error("unpack data pack %s failed, status = %d", t_res.c_str(), sync_status)
        return RET_FAILED;
    }
    d_logic_debug("api_upload data unpack:%s", res_group_data.c_str())
    return RET_SUCCESS;
}

int CSyncData::sync_download(SyncStatus &sync_status, ErrorCode &error_code) {
    d_logic_debug("%s", "api_download start")
    Json::Value req_group_data;
    req_group_data[SYNC_PASSWORD] = m_setting_ctrl->get_string(SETTING_PASSWORD);
    d_logic_debug("api_download send group data:%s", req_group_data.toStyledString().c_str())

    // 聚合数据需要加密
    std::string t_req = pack_packet(req_group_data.toStyledString());
    std::string t_res;
    int ret = m_net_handle->c_get(SYNC_DOWNLOAD, t_req, t_res, error_code);
    if(ret == RET_FAILED){
        d_logic_error("api_download send %s to get data error", t_req.c_str())
        sync_status = Sync_undefined_error;
        return RET_FAILED;
    }
    d_logic_debug("api_download recv data:%s", t_res.c_str())

    std::string res_group_data;
    ret = unpack_packet(t_res, res_group_data, sync_status, error_code);
    if(ret == RET_FAILED){
        d_logic_error("unpack data pack %s failed, status = %d", t_res.c_str(), sync_status)
        return RET_FAILED;
    }
    d_logic_debug("api_download data unpack:%s", res_group_data.c_str())

    // 获取list数据，合并到本地
    Json::Value group_json;
    m_json_reader.parse(res_group_data, group_json);
    TodoList t_todo_list;
    json_list(t_todo_list, group_json[SYNC_TODO_LIST]);
    t_todo_list.sort([](const TodoItem &item1, const TodoItem &item2) {
        return item1.edit_key < item2.edit_key;
    });

    ret = m_data_ctrl->mrg_todo(t_todo_list, error_code);
    if(ret == RET_FAILED){
        d_logic_error("mrg_todo fail, error_code:%d", error_code);
    }
    return ret;
}

std::string CSyncData::pack_packet(const std::string &group_data){
    // 数据包封装
    Json::Value t_pack;
    t_pack[SYNC_USERNAME] = m_setting_ctrl->get_string(SETTING_USERNAME);
    t_pack[SYNC_PASSWORD] = "local";
    t_pack[SYNC_GROUP_DATA] = encrypt_data(group_data, REMINDER_FORMAT);
    return t_pack.toStyledString();
}

int CSyncData::unpack_packet(const std::string &pack, std::string &group_data, SyncStatus &sync_status, ErrorCode &error_code){
    std::string username = m_setting_ctrl->get_string(SETTING_USERNAME);
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(pack, t_json_res);
    std::string t_username = t_json_res.get(SYNC_USERNAME, "").asString();
    // 校验用户名
    if(t_username != username){
        d_logic_error("%s","username not match %s != %s", t_username.c_str(), username.c_str())
        error_code = Error_data_proc_error;
        return RET_FAILED;
    }
    // 校验数据包状态
    sync_status = (SyncStatus)t_json_res.get(SYNC_STATUS, Json::Value(int(Sync_undefined_error))).asInt();
    if(sync_status != Sync_success){
        d_logic_error("sync error, status:%d", sync_status)
        return RET_FAILED;
    }

    std::string t_group_data = t_json_res.get(SYNC_GROUP_DATA, "").asString();
    group_data = decrypt_data(t_group_data, REMINDER_FORMAT);
    return RET_SUCCESS;
}
