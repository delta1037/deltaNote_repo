/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief: sqlite 封装接口
 */
#include <unistd.h>
#include "sql_base.h"
#include "log.h"

using namespace std;

// 全局数据库操作对象管理
static std::map<std::string, sqlite3 *> g_db_handle;
// 获取数据库对象
sqlite3 *get_db_handle(const std::string &db_name){
    auto it_find = g_db_handle.find(db_name);
    if(it_find != g_db_handle.end()){
        return it_find->second;
    }

    sqlite3 *t_db_handle;
    char *db_err_msg = nullptr;
    int ret = 0;
    ret = sqlite3_open(db_name.c_str(), &t_db_handle);
    if(ret != SQLITE_OK){
        d_sql_error("open db %s error: %s", db_name.c_str(), db_err_msg)
        return nullptr;
    }
    g_db_handle[db_name] = t_db_handle;
    return t_db_handle;
}

SqlRetList SqlBase::sql_ret_list;
SqlBase::SqlBase(const std::string &db_name){
    this->db_name = db_name;
    this->db_err_msg = nullptr;
}

int SqlBase::exec_callback(void *param, int col_count, char **col_val, char **col_name) {
    // 向静态变量中填充数据
    d_sql_debug("run %s get data", (const char*)param)
    map<string, string> one_line;
    for(int i = 0; i < col_count; i++){
        d_sql_debug("col_name:%s,col_val:%s", col_name[i], col_val[i])
        one_line[string(col_name[i])] = string(col_val[i]);
    }
    sql_ret_list.push_back(one_line);
    d_sql_debug("data collect rows %d", sql_ret_list.size())
    return 0;
}

int SqlBase::exec(const std::string &sql, SqlRetList &ret_list, ErrorCode &error_code) {
    error_code = Error_no_error;
    sqlite3 *t_db_handle = get_db_handle(db_name);
    if(t_db_handle == nullptr){
        d_sql_error("db %s not init", db_name.c_str())
        error_code = Error_database_init_error;
        return RET_FAILED;
    }
    int ret = sqlite3_exec(t_db_handle, sql.c_str(), (SqlCallback)&(SqlBase::exec_callback), (void *)sql.c_str(), &db_err_msg);
    if(ret != SQLITE_OK){
        d_sql_error("db %s exec %s error: %s", db_name.c_str(), sql.c_str(), db_err_msg)
        error_code = Error_database_exec_error;
        return RET_FAILED;
    }
    d_sql_debug("db %s exec %s success", db_name.c_str(), sql.c_str())

    // 获取数据
    get_exec_data(ret_list);
    return RET_SUCCESS;
}

void SqlBase::get_exec_data(SqlRetList &ret_list) {
    d_sql_debug("get data from %s, sql_ret_list size:%d", this->db_name.c_str(), sql_ret_list.size());
    for(const auto& list_it : sql_ret_list){
        ret_list.push_back(list_it);
    }
    sql_ret_list.clear();
}
