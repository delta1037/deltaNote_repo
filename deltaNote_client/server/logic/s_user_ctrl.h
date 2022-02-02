#ifndef SERVER_USER_CTRL_H
#define SERVER_USER_CTRL_H
#include <list>
#include <string>
#include "s_inter_var.h"
#include "sql_key_value.h"

class SUserCtrl{
public:
    SUserCtrl();
    ~SUserCtrl();

    // 新增一条
    int add_user(const UserItem &data_item, ErrorCode &error_code);
    // 修改
    int alt_user(const UserItem &data_item, ErrorCode &error_code);
    // 删除
    int del_user(const std::string &username, ErrorCode &error_code);
    // 获取所有
    int sel_user(UserList &ret_list, ErrorCode &error_code);
    // 按照指定用户名获取
    int sel_user(const std::string &username, UserItem &data_item, ErrorCode &error_code);

private:
    static std::string group_data(const UserItem &user_item);
    static int group_data(const std::string &data, UserItem &user_item, ErrorCode &error_code);
    static int check_user(const UserItem &user_item, ErrorCode &error_code);
private:
    SqlKeyValue *m_sql_user;
};

#endif //SERVER_USER_CTRL_H
