/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief: sqlite 封装接口
 */
#ifndef NOTE_SQL_BASE_H
#define NOTE_SQL_BASE_H
#include <string>
#include <list>
#include <map>
#include "sqlite3.h"
#include "inter_var.h"

#define SqlRetList std::list<std::map<std::string, std::string>>
typedef int (*SqlCallback)(void*,int,char**,char**);

// 数据库接口层
class SqlBase {
public:
    explicit SqlBase(const std::string &db_name);

    int exec(const std::string &sql, SqlRetList &ret_list, ErrorCode &error_code);

protected:
    static int exec_callback(void *data, int argc, char **argv, char **ColName);
    static SqlRetList sql_ret_list;

private:
    // 从回调中获取数据
    void get_exec_data(SqlRetList &ret_list);
private:
    std::string db_name;
    char *db_err_msg;
};
#endif //NOTE_SQL_BASE_H
