#include "s_data_ctrl.h"
#include "s_inter_var.h"
#include "log.h"

using namespace std;

SDataCtrl::SDataCtrl(const std::string &username){
    m_op_list = nullptr;
    if(username.empty()){
        return;
    }
    std::string t_db_path = get_abs_path() + "/" + DB_NAME;
    m_op_list = new SqlTodoList(t_db_path, DB_OP_TABLE + username);
}

SDataCtrl::~SDataCtrl(){
    delete m_op_list;
}

int SDataCtrl::add_todo(const string &create_key, const string &data, ErrorCode &error_code) {
    return RET_SUCCESS;
}

int SDataCtrl::alt_todo(const std::string &create_key, IsCheck is_check, const std::string &data, ErrorCode &error_code) {
    return RET_SUCCESS;
}

int SDataCtrl::del_todo(const std::string &create_key, ErrorCode &error_code) {
    return RET_SUCCESS;
}

int SDataCtrl::sel_todo(TodoList &ret_list, ErrorCode &error_code) {
    if(!check_data_ctrl()){
        return RET_FAILED;
    }
    int ret = m_op_list->sel(ret_list, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("%s", "data ctrl m_op_list sel error");
    }
    return ret;
}

// 默认src_list已经按照编辑时间进行了排序
int SDataCtrl::mrg_todo(const TodoList &src_list, ErrorCode &error_code) {
    if(!check_data_ctrl()){
        return RET_FAILED;
    }

    // 选取服务端列表
    TodoList des_list;
    int ret = m_op_list->sel(des_list, error_code);
    if(ret != RET_SUCCESS){
        d_logic_error("%s", "data ctrl m_op_list sel error");
        return RET_FAILED;
    }

    // 服务端生成map，便于查找
    std::map<std::string, TodoItem> t_des_map;
    for(const auto &it: des_list){
        t_des_map[it.create_key + op_type_str(it.op_type)] = it;
    }

    // 迭代源列表，逐个进行修改
    for(const auto &it: src_list){
        std::string find_key = it.create_key + op_type_str(it.op_type);
        auto it_find = t_des_map.find(find_key);
        int sql_ret = RET_SUCCESS;
        if((it.op_type == OpType_add || it.op_type == OpType_alt) && it_find == t_des_map.end()){
            sql_ret = m_op_list->add(it.create_key, it.edit_key, it.op_type, it.is_check, it.tag_type, it.reminder, it.data, error_code);
        }else if(it.op_type == OpType_alt && it_find != t_des_map.end()){
            // 如果已经存在，如果时间新一些就替换
            if(it.edit_key > it_find->second.edit_key){
                sql_ret = m_op_list->alt(it.create_key, it.edit_key, it.op_type, it.is_check, it.tag_type, it.reminder, it.data, error_code);
            }
        }else if(it.op_type == OpType_del && it_find != t_des_map.end()){
            sql_ret = m_op_list->del(it.create_key, error_code);
        }else{
            d_logic_error("unknown op_type:%s, find_key:%s, is_find:%d",
                          op_type_str(it.op_type).c_str(),
                          find_key.c_str(),
                          it_find == t_des_map.end())
        }
        if(sql_ret != RET_SUCCESS){
            d_logic_error("sql error op_type:%s, find_key:%s, is_find:%d",
                          op_type_str(it.op_type).c_str(),
                          find_key.c_str(),
                          it_find == t_des_map.end())
        }
    }
    return ret;
}

bool SDataCtrl::check_data_ctrl() {
    if(m_op_list == nullptr){
        d_logic_error("%s", "env init fail")
        return false;
    }
    return true;
}
