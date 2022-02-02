#include "log.h"
#include "json.h"
#include "s_sync_data.h"
#include "s_inter_var.h"

SDataCtrl *SSyncData::m_data_ctrl = nullptr;
SUserCtrl *SSyncData::m_user_ctrl = nullptr;

int SSyncData::sync_init() {
    // 用户控制初始化
    m_user_ctrl = new SUserCtrl();
    return RET_SUCCESS;
}

int SSyncData::sync_uninit() {
    // 用户控制初始化
    delete m_user_ctrl;
    return RET_SUCCESS;
}

int SSyncData::sync_sign_up(const std::string &req, std::string &res, ErrorCode &error_code) {
    d_logic_debug("%s", "SSyncData::sync_sign_up")
    UserItem user_item;
    std::string group_data;
    std::string ret_group_data = "";
    SyncStatus net_status;
    int ret = unpack_packet(req, user_item, group_data, error_code);
    if(error_code != Error_user_not_exist){
        // 不属于用户不存在的情况
        if(ret == RET_SUCCESS){
            // 用户已存在
            res = pack_packet(user_item, Sync_sign_up_user_exists, "");
        }else{
            res = pack_packet(user_item, Sync_undefined_error, "");
        }
        return RET_FAILED;
    }

    d_logic_debug("group data is %s", group_data.c_str())
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(group_data, t_json_res);
    user_item.password = t_json_res.get(USER_PASSWORD, "").asString();
    d_logic_debug("user_item.password:%s", user_item.password.c_str())
    if(m_user_ctrl == nullptr){
        res = pack_packet(user_item, Sync_undefined_error, "");
        d_logic_error("%s", "env init fail, m_user_ctrl == nullptr")
        return RET_FAILED;
    }
    ret = m_user_ctrl->add_user(user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("add user failed, error_code = %d", error_code)
        return RET_FAILED;
    }
    d_logic_debug("add user %s success", user_item.username.c_str())

    // TODO group data需要填充token数据
    res = pack_packet(user_item, Sync_success, "");
    return RET_SUCCESS;
}

int SSyncData::sync_sign_in(const std::string &req, std::string &res, ErrorCode &error_code) {
    d_logic_debug("%s", "SSyncData::sync_sign_in")
    // 登录，校验用户名和密码
    return RET_SUCCESS;
}

int SSyncData::sync_upload(SyncStatus &net_status, ErrorCode &error_code) {
    return RET_SUCCESS;
}

int SSyncData::sync_download(SyncStatus &net_status, ErrorCode &error_code) {
    return RET_SUCCESS;
}

// 数据包最外层封装
std::string SSyncData::pack_packet(const UserItem &data_item, SyncStatus net_status, const std::string &group_data) {
    // 数据包封装
    Json::Value t_pack;
    t_pack["username"] = data_item.username;
    t_pack["password"] = "local";
    t_pack["net_status"] = net_status;
    t_pack["group_data"] = encrypt_data(group_data, REMINDER_FORMAT);
    return t_pack.asString();
}

// 数据包最外层封装
int SSyncData::unpack_packet(const std::string &pack, UserItem &data_item, std::string &group_data, ErrorCode &error_code) {
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(pack, t_json_res);
    data_item.username = t_json_res.get("username", "").asString();

    int ret = m_user_ctrl->sel_user(data_item.username, data_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("user %s not exist", data_item.username.c_str())
        error_code = Error_user_not_exist;
    }

    std::string t_group_data = t_json_res.get("group_data", "").asString();
    group_data = decrypt_data(t_group_data, REMINDER_FORMAT);
    d_logic_debug("user %s, group data from %s --> %s", data_item.username.c_str(), t_group_data.c_str(), group_data.c_str())
    return ret;
}
