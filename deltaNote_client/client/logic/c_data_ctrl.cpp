#include "c_data_ctrl.h"
#include "log.h"

#include <QCoreApplication>

using namespace std;

CDataCtrl::CDataCtrl(){
    std::string t_db_path = get_abs_path() + "/" + DB_NAME;
    m_op_list = new SqlTodoList(t_db_path, DB_TODO_OP_TABLE_NAME);
    m_ui_list = new SqlTodoList(t_db_path, DB_TODO_UI_TABLE_NAME);
}

CDataCtrl::~CDataCtrl(){
    delete m_op_list;
    delete m_ui_list;
}

int CDataCtrl::add_todo(const TodoItem &data_item, ErrorCode &error_code) {
    // 操作流中新增该记录（创建时间key值保证该便签key不会与之前的key重复）
    string edit_key = get_time_key();
    int ret = m_op_list->add(
            data_item.create_key,
            edit_key,
            OpType_add,
            Check_false,
            data_item.tag_type,
            data_item.reminder,
            data_item.data,
            error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s add m_op_list error", data_item.create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s add m_op_list success", data_item.create_key.c_str())

    // UI表中新增数据
    ret = m_ui_list->add(
            data_item.create_key,
            edit_key,
            OpType_add,
            Check_false,
            data_item.tag_type,
            data_item.reminder,
            data_item.data,
            error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s add m_ui_list error", data_item.create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s add m_ui_list success", data_item.create_key.c_str())
    return RET_SUCCESS;
}

int CDataCtrl::alt_todo(const TodoItem &data_item, ErrorCode &error_code) {
    /*
     * 1、若操作流中没有该key值，说明是第一次操作，将修改记录附加到操作流中；
     * 2、若操作流中已经存在该key值，说明已经不是第一次修改，将之前的修改记录覆盖为现在的修改记录
     * （即修改记录只会存在一条，并且是最新的操作）
     */
    // 操作流表
    string edit_key = get_time_key();
    TodoList ret_list;
    int ret = m_op_list->sel(data_item.create_key, OpType_alt, ret_list, error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s sel m_op_list error", data_item.create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s sel m_op_list success", data_item.create_key.c_str())
    if(ret_list.empty()){
        // 如果还没有则插入一条新的修改记录
        ret = m_op_list->add(
                data_item.create_key,
                edit_key,
                OpType_alt,
                data_item.is_check,
                data_item.tag_type,
                data_item.reminder,
                data_item.data,
                error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s add m_op_list error", data_item.create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s add m_op_list success", data_item.create_key.c_str())
    }else{
        // 如果已存在则修改现有记录(根据创建时间和操作类型修改，别把add类型覆盖了)
        ret = m_op_list->alt(
                data_item.create_key,
                OpType_alt,
                edit_key,
                data_item.is_check,
                data_item.tag_type,
                data_item.reminder,
                data_item.data,
                error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s alt m_op_list error", data_item.create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s alt m_op_list success", data_item.create_key.c_str())
    }

    // UI界面表
    ret_list.clear(); // 注意清理环境
    ret = m_ui_list->sel(data_item.create_key, ret_list, error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s sel m_ui_list error", data_item.create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s sel m_ui_list success", data_item.create_key.c_str())
    if(ret_list.empty()){
        // 如果还没有则插入一条新的记录
        ret = m_ui_list->add(
                data_item.create_key,
                edit_key,
                OpType_alt,
                data_item.is_check,
                data_item.tag_type,
                data_item.reminder,
                data_item.data,
                error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s add m_ui_list error", data_item.create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s add m_ui_list success", data_item.create_key.c_str())
    }else{
        // 如果已存在则修改现有记录(根据创建时间修改)
        ret = m_ui_list->alt(
                data_item.create_key,
                edit_key,
                OpType_alt,
                data_item.is_check,
                data_item.tag_type,
                data_item.reminder,
                data_item.data,
                error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s alt m_ui_list error", data_item.create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s alt m_ui_list success", data_item.create_key.c_str())
    }
    return RET_SUCCESS;
}

int CDataCtrl::del_todo(const std::string &create_key, ErrorCode &error_code) {
    /**
     * 1、如果操作流中有该key值的新增操作，则直接将所有相同的key值便签删除（新增记录和修改记录）；
     * 2、如果操作流中不存在该key值的新增操作，说明该key值的新增操作已经上传到服务端了，则先将该操作保存到操作流中，等待同步
     * （如果服务端永久失联则会称为一条僵尸记录）
     */
    // 操作流表
    string edit_key = get_time_key();
    TodoList ret_list;
    int ret = m_op_list->sel(create_key, OpType_add, ret_list, error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s sel m_op_list error", create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s sel m_op_list success", create_key.c_str())
    if(ret_list.empty()){
        // 不存在新增删除操作，内容无所谓了，都删了
        ret = m_op_list->add(
                create_key,
                edit_key,
                OpType_del,
                Check_true,
                TagType_nul,
                "",
                "",
                error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s add m_op_list error", create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s add m_op_list success", create_key.c_str())
    }else{
        // 如果存在新增操作，删除所有内容
        ret = m_op_list->del(create_key, error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("CDataCtrl key %s alt m_op_list error", create_key.c_str())
            return RET_FAILED;
        }
        d_logic_debug("CDataCtrl key %s alt m_op_list success", create_key.c_str())
    }

    // UI表操作，删除相关内容即可
    ret = m_ui_list->del(create_key, error_code);
    if(ret == SQLITE_ERROR){
        d_logic_error("CDataCtrl key %s alt m_ui_list error", create_key.c_str())
        return RET_FAILED;
    }
    d_logic_debug("CDataCtrl key %s alt m_ui_list success", create_key.c_str())
    return RET_SUCCESS;
}


int CDataCtrl::del_todo(ListType list_type, ErrorCode &error_code) {
    if(list_type == ListType_OP){
        int ret = m_op_list->del(error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl del m_op_list error")
            return RET_FAILED;
        }
        d_logic_debug("%s", "CDataCtrl sel m_op_list success")
    }else if(list_type == ListType_UI){
        int ret = m_ui_list->del(error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl del m_ui_list error")
            return RET_FAILED;
        }
        d_logic_debug("%s", "CDataCtrl del m_ui_list success")
    }else{
        d_logic_error("CDataCtrl del type %d error", list_type)
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int CDataCtrl::sel_todo(ListType list_type, TodoList &ret_list, ErrorCode &error_code) {
    if(list_type == ListType_OP){
        int ret = m_op_list->sel(ret_list, error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl sel m_op_list error")
            return RET_FAILED;
        }
        d_logic_debug("%s", "CDataCtrl sel m_op_list success")
    }else if(list_type == ListType_UI){
        int ret = m_ui_list->sel(ret_list, error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl sel m_ui_list error")
            return RET_FAILED;
        }
        // UI界面表排好序返回
        ret_list.sort([](const TodoItem &item1, const TodoItem &item2) {
            if(tag_type_str(item1.tag_type) != (tag_type_str(item2.tag_type))){
                return tag_type_str(item1.tag_type) < (tag_type_str(item2.tag_type));
            }else{
                return item1.create_key < item2.create_key;
            }
        });
        d_logic_debug("%s", "CDataCtrl sel m_ui_list success")
    }else{
        d_logic_error("CDataCtrl sel type %d error", list_type)
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int CDataCtrl::sel_todo(ListType list_type, const string &create_key, list<struct TodoItem> &ret_list,
                        ErrorCode &error_code) {
    if(list_type == ListType_OP){
        int ret = m_op_list->sel(create_key, ret_list, error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl sel m_op_list error")
            return RET_FAILED;
        }
        d_logic_debug("%s", "CDataCtrl sel m_op_list success")
    }else if(list_type == ListType_UI){
        int ret = m_ui_list->sel(create_key, ret_list, error_code);
        if(ret == SQLITE_ERROR){
            d_logic_error("%s", "CDataCtrl sel m_ui_list error")
            return RET_FAILED;
        }
        d_logic_debug("%s", "CDataCtrl sel m_ui_list success")
    }else{
        d_logic_error("CDataCtrl sel type %d error", list_type)
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int CDataCtrl::mrg_todo(const TodoList &src_list, ErrorCode &error_code){
    // TODO 将远程获取到的操作流生成UI表
    return RET_SUCCESS;
}
