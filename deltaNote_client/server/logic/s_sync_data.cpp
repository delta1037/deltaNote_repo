/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "log.h"
#include "s_inter_var.h"
#include "s_sync_data.h"
#include "s_data_ctrl.h"

int SSyncData::sync_sign_up(const std::string &req, std::string &res) {
    d_logic_debug("%s", "SSyncData::api_sign_up start")
    ErrorCode error_code = Error_no_error;
    SyncStatus res_sync_status = Sync_server_error;

    UserItem req_user_item;
    UserItem db_user_item;
    Json::Value req_group_json;

    int ret = unpack_packet(req, req_user_item, db_user_item, req_group_json, error_code);
    if(error_code != Error_user_not_exist){
        // 不属于用户不存在的情况都是错误的
        if(ret == RET_SUCCESS){
            // 用户已存在
            d_logic_error("user %s is already exist, error_code:%d", req_user_item.username.c_str(), error_code)
            res_sync_status = Sync_sign_up_user_exists;
        }else{
            d_logic_error("user %s get undefined error, error_code:%d", req_user_item.username.c_str(), error_code)
        }
        ret = RET_FAILED;
        goto leave;
    }

    req_user_item.password = req_group_json.get(USER_PASSWORD, "").asString();
    ret = m_user_ctrl.add_user(req_user_item, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("add user %s failed, error_code = %d", req_user_item.username.c_str(), error_code)
        goto leave;
    }
    d_logic_info("add user %s success", req_user_item.username.c_str())

    // TODO group data需要填充token数据
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, "");
    d_logic_debug("sign up: ret:%d, sync_status:%d, return size:%d,content:%s",
                  ret,
                  res_sync_status,
                  res.size(),
                  res.c_str())
    return ret;
}

int SSyncData::check_user(const std::string &req, UserItem &user_item, SyncStatus &sync_status, Json::Value &group_json, ErrorCode &error_code){
    // 解数据包
    UserItem db_user_item;
    int ret = unpack_packet(req, user_item, db_user_item, group_json, error_code);
    if(ret != RET_SUCCESS){
        if(error_code == Error_user_not_exist){
            d_logic_error("user %s is not exist", user_item.username.c_str())
            sync_status = Sync_sign_in_user_not_exits;
        }else{
            d_logic_error("user %s get undefined error", user_item.username.c_str())
            sync_status = Sync_undefined_error;
        }
        return RET_FAILED;
    }

    // 校验密码
    user_item.password = group_json.get(USER_PASSWORD, "").asString();
    if(db_user_item.username != user_item.username || db_user_item.password != user_item.password){
        d_logic_error("user %s check failed", db_user_item.username.c_str())
        d_logic_debug("username:%s~%s, password:%s~%s",
                      db_user_item.username.c_str(),
                      user_item.username.c_str(),
                      db_user_item.password.c_str(),
                      user_item.password.c_str()
        )
        sync_status = Sync_sign_in_passwd_error;
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int SSyncData::sync_sign_in(const std::string &req, std::string &res) {
    d_logic_debug("%s", "SSyncData::sync_sign_in start")
    ErrorCode error_code = Error_no_error;
    SyncStatus res_sync_status = Sync_server_error;

    UserItem req_user_item;
    Json::Value req_group_json;

    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("sync_sign_in user %s check fail", req_user_item.username.c_str())
        goto leave;
    }
    d_logic_info("user %s login success", req_user_item.username.c_str())

    // TODO group data需要填充token数据
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, "");
    d_logic_debug("sign in: ret:%d, sync_status:%d, return size:%d,content:%s",
                  ret,
                  res_sync_status,
                  res.size(),
                  res.c_str())
    return ret;
}

int SSyncData::sync_upload(const std::string& req, std::string& res) {
    d_logic_debug("%s", "SSyncData::sync_upload start")
    ErrorCode error_code = Error_no_error;
    SyncStatus res_sync_status = Sync_server_error;

    UserItem req_user_item;
    Json::Value req_group_json;

    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret != RET_SUCCESS){
        d_logic_info("sync_upload user %s check fail", req_user_item.username.c_str())
        res = pack_packet(req_user_item, res_sync_status, "");
        return RET_FAILED;
    }
    d_logic_info("sync_upload user %s login success", req_user_item.username.c_str())

    // 校验成功，获取数据处理对象 // 按照编辑时间排序
    TodoList todo_list;
    json_list(todo_list, req_group_json[SYNC_TODO_LIST]);
    todo_list.sort([](const TodoItem &item1, const TodoItem &item2) {
        return item1.edit_key < item2.edit_key;
    });

    // 与服务端的操作流进行合并
    SDataCtrl t_data_ctrl(req_user_item.username);
    ret = t_data_ctrl.mrg_todo(todo_list, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("sync_upload mrg_todo fail, error_code:%d", error_code)
        goto leave;
    }
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, "");
    d_logic_debug("upload: ret:%d, sync_status:%d, return size:%d,content:%s",
                  ret,
                  res_sync_status,
                  res.size(),
                  res.c_str())
    return ret;
}

int SSyncData::sync_download(const std::string& req, std::string& res) {
    d_logic_debug("%s", "SSyncData::api_download start")
    ErrorCode error_code = Error_no_error;
    SyncStatus res_sync_status = Sync_server_error;

    UserItem req_user_item;
    Json::Value req_group_json;

    int ret = check_user(req, req_user_item, res_sync_status, req_group_json, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("%s", "user check fail")
        res = pack_packet(req_user_item, res_sync_status, "");
        return RET_FAILED;
    }
    d_logic_info("sync_download user %s login success", req_user_item.username.c_str())

    // 校验成功，获取数据处理对象
    Json::Value res_group_json;
    TodoList todo_list;
    SDataCtrl t_data_ctrl(req_user_item.username);
    ret = t_data_ctrl.sel_todo(todo_list, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("sel_todo fail, error_code:%d", error_code)
        goto leave;
    }
    res_group_json[SYNC_TODO_LIST] = json_list(todo_list);
    res_sync_status = Sync_success;

leave:
    res = pack_packet(req_user_item, res_sync_status, res_group_json.toStyledString());
    d_logic_debug("download: ret:%d, sync_status:%d, return size:%d,content:%s",
                  ret,
                  res_sync_status,
                  res.size(),
                  res.c_str())
    return ret;
}

// 返回的数据包封装
std::string SSyncData::pack_packet(const UserItem &data_item, SyncStatus sync_status, const std::string &group_data) {
    Json::Value t_pack;
    t_pack[SYNC_USERNAME] = data_item.username;
    t_pack[SYNC_PASSWORD] = "local";
    t_pack[SYNC_STATUS] = sync_status;
    t_pack[SYNC_GROUP_DATA] = encrypt_data(group_data, REMINDER_FORMAT);
    return t_pack.toStyledString();
}

// 收到的数据包解包
int SSyncData::unpack_packet(const std::string &pack, UserItem &req_user_item, UserItem &db_user_item, Json::Value &req_group_json, ErrorCode &error_code) {
    d_logic_debug("unpack data size:%d, content",pack.size(), pack.c_str())
    Json::Value t_json_value;
    m_json_reader.parse(pack, t_json_value);
    req_user_item.username = t_json_value.get(SYNC_USERNAME, "").asString();
    if(req_user_item.username.empty()){
        d_logic_error("%s", "user is empty")
        error_code = Error_user_not_exist;
        return RET_FAILED;
    }

    int ret = m_user_ctrl.sel_user(req_user_item.username, db_user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("user %s not exist", req_user_item.username.c_str())
        error_code = Error_user_not_exist;
    }

    std::string t_group_data = t_json_value.get(SYNC_GROUP_DATA, "").asString();
    m_json_reader.parse(decrypt_data(t_group_data, REMINDER_FORMAT), req_group_json);
    d_logic_debug("user %s, group data from %s --> %s", req_user_item.username.c_str(), t_group_data.c_str(), req_group_json.toStyledString().c_str())
    return ret;
}
