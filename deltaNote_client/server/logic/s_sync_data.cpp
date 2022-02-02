#include "log.h"
#include "s_sync_data.h"
#include "s_inter_var.h"

SDataCtrl *SSyncData::m_data_ctrl = nullptr;
SUserCtrl *SSyncData::m_user_ctrl = nullptr;
Json::Reader SSyncData::m_json_reader;

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
    std::string res_group_data;
    SyncStatus res_sync_status = Sync_undefined_error;

    UserItem req_user_item;
    std::string req_group_data;
    Json::Value req_group_json;

    int ret = unpack_packet(req, req_user_item, req_group_data, error_code);
    if(error_code != Error_user_not_exist){
        // 不属于用户不存在的情况都是错误的
        if(ret == RET_SUCCESS){
            // 用户已存在
            d_logic_error("user %s is already exist", req_user_item.username.c_str())
            res_sync_status = Sync_sign_up_user_exists;
        }else{
            d_logic_error("user %s get undefined error", req_user_item.username.c_str())
        }
        ret = RET_FAILED;
        goto leave;
    }

    m_json_reader.parse(req_group_data, req_group_json);
    d_logic_debug("group data %s --> %s", req_group_data.c_str(), req_group_json.toStyledString().c_str())
    req_user_item.password = req_group_json.get(USER_PASSWORD, "").asString();
    if(m_user_ctrl == nullptr){
        d_logic_error("%s", "env init fail, m_user_ctrl == nullptr")
        ret = RET_FAILED;
        res_sync_status = Sync_server_error;
        goto leave;
    }
    ret = m_user_ctrl->add_user(req_user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("add user failed, error_code = %d", error_code)
        ret = RET_FAILED;
        res_sync_status = Sync_server_error;
        goto leave;
    }
    d_logic_debug("add user %s success", req_user_item.username.c_str())
    // TODO group data需要填充token数据
    ret = RET_SUCCESS;
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, res_group_data);
    d_logic_debug("sign up: ret:%d,sync_status:%d,return %s",ret, res_sync_status, res.c_str())
    return ret;
}

int SSyncData::check_user(const std::string &req, UserItem &user_item, SyncStatus &sync_status, Json::Value &group_json, ErrorCode &error_code){
    UserItem t_user_item;
    std::string req_group_data;

    int ret = unpack_packet(req, user_item, req_group_data, error_code);
    if(ret != RET_SUCCESS){
        if(error_code == Error_user_not_exist){
            d_logic_error("user %s is not exist", user_item.username.c_str())
            sync_status = Sync_sign_in_user_not_exits;
        }else{
            d_logic_error("user %s get undefined error", user_item.username.c_str())
        }
        return RET_FAILED;
    }

    m_json_reader.parse(req_group_data, group_json);
    d_logic_debug("group data %s --> %s", req_group_data.c_str(), group_json.toStyledString().c_str())
    user_item.password = group_json.get(USER_PASSWORD, "").asString();
    if(m_user_ctrl == nullptr){
        d_logic_error("%s", "env init fail, m_user_ctrl == nullptr")
        sync_status = Sync_server_error;
        return RET_FAILED;
    }

    // 查找用户
    ret = m_user_ctrl->sel_user(user_item.username, t_user_item, error_code);
    d_logic_debug("%s", "find user end")
    if(ret != RET_SUCCESS){
        d_logic_error("sel user failed, error_code = %d", error_code)
        if(error_code == Error_server_error){
            sync_status = Sync_server_error;
        }else if(error_code == Error_user_not_exist){
            sync_status = Sync_sign_in_user_not_exits;
        }
        return RET_FAILED;
    }
    // 校验密码
    if(t_user_item.username != user_item.username || t_user_item.password != user_item.password){
        d_logic_error("user %s check failed", t_user_item.username.c_str())
        d_logic_debug("username:%s~%s, password:%s~%s",
                      t_user_item.username.c_str(),
                      user_item.username.c_str(),
                      t_user_item.password.c_str(),
                      user_item.password.c_str()
        )
        sync_status = Sync_sign_in_passwd_error;
        return RET_FAILED;
    }
    return RET_SUCCESS;
}
int SSyncData::sync_sign_in(const std::string &req, std::string &res, ErrorCode &error_code) {
    d_logic_debug("%s", "SSyncData::sync_sign_in")
    UserItem req_user_item;
    Json::Value req_group_json;

    std::string res_group_data;
    SyncStatus res_sync_status = Sync_undefined_error;
    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret == RET_FAILED){
        d_logic_error("%s", "user check fail");
        goto leave;
    }

    d_logic_info("user %s login success", req_user_item.username.c_str())
    res_sync_status = Sync_success;

    // TODO group data需要填充token数据
leave:
    res = pack_packet(req_user_item, res_sync_status, res_group_data);
    d_logic_debug("sign in: ret:%d,sync_status:%d,return %s",ret, res_sync_status, res.c_str())
    return ret;
}

int SSyncData::sync_upload(const std::string& req, std::string& res, ErrorCode &error_code) {
    d_logic_debug("%s", "SSyncData::sync_upload")
    UserItem req_user_item;
    Json::Value req_group_json;
    Json::Value json_todo_list;
    TodoList todo_list;

    std::string res_group_data;
    SyncStatus res_sync_status = Sync_undefined_error;
    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret == RET_FAILED){
        d_logic_error("%s", "user check fail");
        res = pack_packet(req_user_item, res_sync_status, res_group_data);
        return RET_FAILED;
    }
    d_logic_info("user %s login success", req_user_item.username.c_str())

    // 校验成功，获取数据处理对象
    json_todo_list = req_group_json[SYNC_TODO_LIST];
    json_list(todo_list, json_todo_list);
    todo_list.sort([](const TodoItem &item1, const TodoItem &item2) {
        return item1.edit_key < item2.edit_key;
    });

    SDataCtrl t_data_ctrl(req_user_item.username);
    ret = t_data_ctrl.mrg_todo(todo_list, error_code);
    if(ret == RET_FAILED){
        d_logic_error("mrg_todo fail, error_code:%d", error_code);
        goto leave;
    }
    res_sync_status = Sync_success;
leave:
    d_logic_debug("sign in: ret:%d,sync_status:%d,return %s",ret, res_sync_status, res.c_str())
    return ret;
}

int SSyncData::sync_download(const std::string& req, std::string& res, ErrorCode &error_code) {
    d_logic_debug("%s", "SSyncData::sync_download")
    UserItem req_user_item;
    Json::Value req_group_json;
    Json::Value json_todo_list;
    TodoList todo_list;

    std::string res_group_data;
    SyncStatus res_sync_status = Sync_undefined_error;
    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret == RET_FAILED){
        d_logic_error("%s", "user check fail");
        res = pack_packet(req_user_item, res_sync_status, res_group_data);
        return RET_FAILED;
    }
    d_logic_info("user %s login success", req_user_item.username.c_str())

    // 校验成功，获取数据处理对象
    SDataCtrl t_data_ctrl(req_user_item.username);
    ret = t_data_ctrl.sel_todo(todo_list, error_code);
    if(ret == RET_FAILED){
        d_logic_error("sel_todo fail, error_code:%d", error_code);
        goto leave;
    }
    req_group_json[SYNC_TODO_LIST] = json_list(todo_list);
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, res_group_data);
    d_logic_debug("sign in: ret:%d,sync_status:%d,return %s",ret, res_sync_status, res.c_str())
    return ret;
}

// 数据包最外层封装
std::string SSyncData::pack_packet(const UserItem &data_item, SyncStatus sync_status, const std::string &group_data) {
    // 数据包封装
    Json::Value t_pack;
    t_pack[SYNC_USERNAME] = data_item.username;
    t_pack[SYNC_PASSWORD] = "local";
    t_pack[SYNC_STATUS] = sync_status;
    t_pack[SYNC_GROUP_DATA] = encrypt_data(group_data, REMINDER_FORMAT);
    return t_pack.toStyledString();
}

// 数据包最外层封装
int SSyncData::unpack_packet(const std::string &pack, UserItem &data_item, std::string &group_data, ErrorCode &error_code) {
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(pack, t_json_res);
    d_logic_debug("un packet data is %s", t_json_res.toStyledString().c_str())
    data_item.username = t_json_res.get(SYNC_USERNAME, "").asString();
    if(data_item.username.empty()){
        d_logic_error("%s", "user is empty")
        error_code = Error_user_empty;
        return RET_FAILED;
    }
    int ret = m_user_ctrl->sel_user(data_item.username, data_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("user %s not exist", data_item.username.c_str())
        error_code = Error_user_not_exist;
    }

    std::string t_group_data = t_json_res.get(SYNC_GROUP_DATA, "").asString();
    group_data = decrypt_data(t_group_data, REMINDER_FORMAT);
    d_logic_debug("user %s, group data from %s --> %s", data_item.username.c_str(), t_group_data.c_str(), group_data.c_str())
    return ret;
}
