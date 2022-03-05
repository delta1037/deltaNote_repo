/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "s_user_ctrl.h"
#include "json.h"
#include "log.h"

SUserCtrl::SUserCtrl() {
    std::string db_path = get_abs_path() + "/" + DB_NAME;
    d_logic_info("server db path : %s", db_path.c_str())
    m_sql_user = new SqlKeyValue(db_path, DB_USER_TABLE);
}

SUserCtrl::~SUserCtrl() {
    delete m_sql_user;
}

int SUserCtrl::add_user(const UserItem &data_item, ErrorCode &error_code) {
    int ret = m_sql_user->add(data_item.username, group_data(data_item),error_code);
    if(ret == RET_FAILED){
        d_logic_error("add user %s fail", data_item.username.c_str())
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int SUserCtrl::sel_user(const std::string &username, UserItem &user_item, ErrorCode &error_code) {
    d_logic_debug("sql find user |%s| begin", username.c_str())

    user_item.username = username;
    std::list<std::string> t_value_list;
    int ret = m_sql_user->sel(username, t_value_list ,error_code);
    if(ret == RET_FAILED){
        d_logic_error("sel user %s fail", username.c_str())
        error_code = Error_server_error;
        return RET_FAILED;
    }
    if(t_value_list.size() > 1){
        d_logic_warn("username %s is repeat", username.c_str())
        error_code = Error_server_error;
        return RET_FAILED;
    }
    if(t_value_list.empty()){
        d_logic_warn("username %s is not exist", username.c_str())
        error_code = Error_user_not_exist;
        return RET_FAILED;
    }

    ret = group_data(*t_value_list.begin(), user_item, error_code);
    d_logic_debug("sql find user %s success", username.c_str())
    return ret;
}

std::string SUserCtrl::group_data(const UserItem &user_item) {
    Json::Value t_group_data;
    t_group_data[USER_PASSWORD] = user_item.password;
    t_group_data[USER_TOKEN] = user_item.token;
    return t_group_data.toStyledString();
}

int SUserCtrl::group_data(const std::string &data, UserItem &user_item, ErrorCode &error_code){
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(data, t_json_res);
    user_item.password = t_json_res.get(USER_PASSWORD, "").asString();
    user_item.token = t_json_res.get(USER_TOKEN, "").asString();
    return RET_SUCCESS;
}