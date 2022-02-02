#ifndef INTER_VAR_H
#define INTER_VAR_H
#include <ctime>
#include <sys/time.h>
#include <string>
#include "json.h"
#include <list>

enum SyncStatus {
    // 未定义错误
    Sync_undefined_error = 0,
    // 服务端处理出错
    Sync_server_error,
    Sync_client_error,

    // 登录错误
    Sync_sign_in_passwd_error,      // 密码错误
    Sync_sign_in_user_not_exits,    // 用户不存在

    // 注册错误
    Sync_sign_up_user_exists,       // 用户已存在

    Sync_user_id_null,

    ConnectError = 16,

    PushSuccess = 24,
    PushError = 25,

    CleanSuccess = 26,
    CleanError = 27,

    OperateNotDefine = 36,

    PullSuccess,
    PullError,

    VersionError,
    RecvSizeError,

    GetUpdateLink,
    NoUpdateLink,
    AlreadyLatest,

    Sync_success,
};

enum DeviceType {
    WINDOW_DEVICE = 1,
    LINUX_DEVICE,
    ANDROID_DEVICE,
    CHROME_DEVICE,
    SERVER_DEVICE = 9,
    UNKNOWN_DEVICE = 10
};

#define RET_SUCCESS (0)
#define RET_FAILED (-1)

// 定义错误码
enum ErrorCode {
    Error_no_error = 0,
    Error_server_error,
    Error_client_error,
    // 数据库部分
    DB_CREATE_ERROR = 0x00010000, // 数据库创建失败

    Error_user_not_exist,
    Error_user_empty,

    Error_data_proc_error,
};

enum IsCheck {
    Check_true = '1',
    Check_false = '0',
    Check_null = 0
};
std::string is_check_str(IsCheck is_check);
IsCheck is_check_enum(const std::string &is_check);
enum OpType{
    OpType_add = '1', // 新增
    OpType_del = '2', // 修改
    OpType_alt = '3', // 删除
    OpType_nul = 0,
};
std::string op_type_str(OpType op_type);
OpType op_type_enum(const std::string &op_type);

enum TagType {
    TagType_nul,
    TagType_low,
    TagType_mid,
    TagType_high
};
std::string tag_type_str(TagType tag_type);
TagType tag_type_enum(const std::string &tag_type);

struct TodoItem{
    std::string create_key;
    std::string edit_key;
    OpType op_type;
    IsCheck is_check;
    TagType tag_type;
    std::string reminder; // 题型日期
    std::string data;
    TodoItem();
    TodoItem(
        const std::string &create_key,
        const std::string &edit_key,
        OpType op_type,
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data
    );
};
bool check_item_valid(const TodoItem &item);

#define TodoList std::list<struct TodoItem>
#define TODO_ITEM_CREATE_KEY    "create_key"
#define TODO_ITEM_EDIT_KEY      "edit_key"
#define TODO_ITEM_OP_TYPE       "op_type"
#define TODO_ITEM_IS_CHECK      "is_check"
#define TODO_ITEM_TAG_TYPE      "tag_type"
#define TODO_ITEM_REMINDER      "reminder"
#define TODO_ITEM_DATA          "data"
#define TODO_ITEM_GROUP_DATA    "group_data"

#define SYNC_SIGN_IN    "/sign_in"
#define SYNC_SIGN_UP    "/sign_up"
#define SYNC_UPLOAD     "/upload"
#define SYNC_DOWNLOAD   "/download"

#define SYNC_USERNAME   "username"
#define SYNC_PASSWORD   "password"
#define SYNC_STATUS     "sync_status"
#define SYNC_GROUP_DATA "group_data"
#define SYNC_TODO_LIST  "todo_list"

#define REMINDER_FORMAT "yyyy-MM-dd hh:mm:ss"
int time_int_s(const std::string &s_time);

std::string get_time_key();
uint64_t get_time_of_ms();
uint64_t get_time_of_s();

// 便签内容聚合
std::string form_group_data(
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data);
void group_data_parser(
        const std::string &group_data,
        IsCheck &is_check,
        TagType &tag_type,
        std::string &reminder,
        std::string &data);

// 检查便签内容有效性
bool todo_is_valid(const TodoItem &item);

// 获取当前可执行文件的绝对路径
std::string get_abs_path();

// 字符串加密解密
std::string encrypt_data(const std::string &src_data, const std::string &key);
std::string decrypt_data(const std::string &src_data, const std::string &key);

struct UserItem{
    std::string username;
    std::string password;
    std::string token;
};
#define USER_USERNAME "username"
#define USER_PASSWORD "password"
#define USER_TOKEN    "token"

#define UserList std::list<UserItem>

Json::Value json_list(const TodoList &todo_list);
void json_list(TodoList &todo_list, const Json::Value &json_list);

#endif //INTER_VAR_H
