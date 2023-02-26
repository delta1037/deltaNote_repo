/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "sql_todo_list.h"
#include "log.h"

using namespace std;

SqlTodoList::SqlTodoList(const std::string &db_name, const std::string &table_name) {
    this->db_name = db_name;
    this->table_name = table_name;

    this->sql_base = new SqlBase(this->db_name);
    if(RET_SUCCESS != init_table()){
        d_sql_error("%s", "init table error!!");
    }
}

SqlTodoList::~SqlTodoList() {
    delete sql_base;
}

int SqlTodoList::init_table() {
    static const std::string create_table_sql =
            "CREATE TABLE IF NOT EXISTS %Q ("\
                "create_key VARCHAR(32) NOT NULL, "\
                "edit_key VARCHAR(32) PRIMARY Key, "\
                "op_type VARCHAR(8) NOT NULL, "\
                "group_data VARCHAR(256) NOT NULL"\
            ");";
    SqlRetList sql_ret_list;
    ErrorCode error_code = Error_no_error;
    int ret = sql_base->exec(
            sqlite3_mprintf(create_table_sql.c_str(), table_name.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), create_table_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), create_table_sql.c_str())
    return RET_SUCCESS;
}

void SqlTodoList::turn_to_struct(const SqlRetList &sql_ret_list, TodoList &ret_struct_list) {
    for(auto ret_it : sql_ret_list){
        TodoItem t_item;
        t_item.create_key = (ret_it)[TODO_ITEM_CREATE_KEY];
        t_item.edit_key = (ret_it)[TODO_ITEM_EDIT_KEY];
        t_item.op_type = op_type_enum((ret_it)[TODO_ITEM_OP_TYPE]);
        group_data_parser((ret_it)[TODO_ITEM_GROUP_DATA],
                          t_item.is_check,
                          t_item.tag_type,
                          t_item.reminder,
                          t_item.data);
        ret_struct_list.emplace_back(t_item);
        d_sql_debug("create_key:%s, edit_key:%s, op_type:%s, group_data:%s",
                    (ret_it)[TODO_ITEM_CREATE_KEY].c_str(),
                    (ret_it)[TODO_ITEM_EDIT_KEY].c_str(),
                    (ret_it)[TODO_ITEM_OP_TYPE].c_str(),
                    (ret_it)[TODO_ITEM_GROUP_DATA].c_str())
    }
}

int SqlTodoList::add(
        const std::string &create_key,
        const std::string &edit_key,
        OpType op_type,
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data,
        ErrorCode &error_code) {
    static const std::string add_sql =
            "INSERT INTO %Q (create_key, edit_key, op_type, group_data) "\
                "VALUES (%Q, %Q, %Q, %Q)";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    add_sql.c_str(),
                    table_name.c_str(),
                    create_key.c_str(),
                    edit_key.c_str(),
                    op_type_str(op_type).c_str(),
                    form_group_data(is_check, tag_type, reminder, data).c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), add_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), add_sql.c_str())
    return RET_SUCCESS;
}

int SqlTodoList::del(const std::string &create_key, ErrorCode &error_code) {
    static const std::string del_sql =
            "DELETE from %Q WHERE create_key == %Q";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    del_sql.c_str(),
                    table_name.c_str(),
                    create_key.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), del_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), del_sql.c_str())
    return RET_SUCCESS;
}

int SqlTodoList::del(ErrorCode &error_code) {
    static const std::string del_sql =
            "DELETE from %Q";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    del_sql.c_str(),
                    table_name.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), del_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), del_sql.c_str())
    return RET_SUCCESS;
}

// 获取所有数据时带有规则排序
int SqlTodoList::sel(TodoList &ret_list, ErrorCode &error_code) {
    static const std::string sel_sql =
            "SELECT create_key, edit_key, op_type, group_data from %Q";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    sel_sql.c_str(),
                    table_name.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), sel_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), sel_sql.c_str())
    turn_to_struct(sql_ret_list, ret_list);
    return RET_SUCCESS;
}

int SqlTodoList::sel(const std::string &create_key, TodoList &ret_list, ErrorCode &error_code) {
    static const std::string sel_sql =
            "SELECT create_key, edit_key, op_type, group_data from %Q WHERE create_key == %Q";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    sel_sql.c_str(),
                    table_name.c_str(),
                    create_key.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), sel_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), sel_sql.c_str())
    turn_to_struct(sql_ret_list, ret_list);
    return RET_SUCCESS;
}

int SqlTodoList::sel(const std::string &create_key, OpType op_type, TodoList &ret_list, ErrorCode &error_code) {
    static const std::string sel_sql =
            "SELECT create_key, edit_key, op_type, group_data from %Q WHERE create_key == %Q and op_type == %Q;";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    sel_sql.c_str(),
                    table_name.c_str(),
                    create_key.c_str(),
                    op_type_str(op_type).c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), sel_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), sel_sql.c_str())
    turn_to_struct(sql_ret_list, ret_list);
    return RET_SUCCESS;
}

// 修改数据（根据创建时间）
int SqlTodoList::alt(
        const std::string &create_key,
        const std::string &edit_key,
        OpType op_type,
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data,
        ErrorCode &error_code) {
    static const std::string alt_sql =
            "UPDATE %Q SET edit_key = %Q, op_type = %Q, group_data = %Q "\
                "WHERE create_key == %Q;";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    alt_sql.c_str(),
                    table_name.c_str(),
                    edit_key.c_str(),
                    op_type_str(op_type).c_str(),
                    form_group_data(is_check, tag_type, reminder, data).c_str(),
                    create_key.c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), alt_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), alt_sql.c_str())
    return RET_SUCCESS;
}

// 修改数据（根据创建时间+操作类型）
int SqlTodoList::alt(
        const std::string &create_key,
        OpType op_type,
        const std::string &edit_key,
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data,
        ErrorCode &error_code){
    static const std::string alt_sql =
            "UPDATE %Q SET edit_key = %Q, group_data = %Q "\
                "WHERE create_key == %Q and op_type == %Q;";
    SqlRetList sql_ret_list;
    int ret = sql_base->exec(
            sqlite3_mprintf(
                    alt_sql.c_str(),
                    table_name.c_str(),
                    edit_key.c_str(),
                    form_group_data(is_check, tag_type, reminder, data).c_str(),
                    create_key.c_str(),
                    op_type_str(op_type).c_str()),
            sql_ret_list,
            error_code
    );
    if(ret != SQLITE_OK){
        d_sql_error("SqlTodoList %s exec %s error", db_name.c_str(), alt_sql.c_str())
        return RET_FAILED;
    }
    d_sql_debug("SqlTodoList %s exec %s success", db_name.c_str(), alt_sql.c_str())
    return RET_SUCCESS;
}
