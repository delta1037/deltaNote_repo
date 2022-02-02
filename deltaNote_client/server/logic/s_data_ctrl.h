#ifndef SERVER_DATA_CTRL_H
#define SERVER_DATA_CTRL_H
#include <string>
#include "sql_todo_list.h"

class SDataCtrl {
public:
    SDataCtrl(const std::string &username);
    ~SDataCtrl();

    // 新增一条
    int add_todo(const std::string &create_key, const std::string &data, ErrorCode &error_code);
    // 修改
    int alt_todo(const std::string &create_key, IsCheck is_check, const std::string &data, ErrorCode &error_code);
    // 删除
    int del_todo(const std::string &create_key, ErrorCode &error_code);
    // 获取所有
    int sel_todo(TodoList &ret_list, ErrorCode &error_code);
    // 合并
    int mrg_todo(const TodoList &op_list, ErrorCode &error_code);

private:
    bool check_data_ctrl();

private:
    SqlTodoList *m_op_list;
};
#endif //SERVER_DATA_CTRL_H
