/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_SERVER_USER_CTRL_H
#define NOTE_SERVER_USER_CTRL_H
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
    // 按照指定用户名获取
    int sel_user(const std::string &username, UserItem &user_item, ErrorCode &error_code);

private:
    std::string group_data(const UserItem &user_item);
    int group_data(const std::string &data, UserItem &user_item, ErrorCode &error_code);
private:
    SqlKeyValue *m_sql_user;
};

#endif //NOTE_SERVER_USER_CTRL_H
