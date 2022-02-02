#ifdef WINDOW_BUILD
#include <Windows.h>
#include <sphelper.h>

#endif
#ifdef LINUX_BUILD
#include <dir.h>
#endif
#include "inter_var.h"
#include "json.h"
#include "log.h"
#include "aes_encryption.hpp"

std::string is_check_str(IsCheck is_check){
    if(is_check == Check_true){
        return "true";
    } else if(is_check == Check_false){
        return "false";
    }
    return "null";
}

IsCheck is_check_enum(const std::string &is_check){
    if(is_check == "true"){
        return Check_true;
    }else if(is_check == "false"){
        return Check_false;
    }
    return Check_null;
}

std::string op_type_str(OpType op_type){
    if(op_type == OpType_add){
        return "add";
    }else if(op_type == OpType_del){
        return "del";
    }else if(op_type == OpType_alt){
        return "alt";
    }
    return "nul";
}

OpType op_type_enum(const std::string &op_type){
    if(op_type == "add"){
        return OpType_add;
    }else if(op_type == "del"){
        return OpType_del;
    }else if(op_type == "alt"){
        return OpType_alt;
    }
    return OpType_nul;
}

std::string tag_type_str(TagType tag_type){
    if(tag_type == TagType_low){
        return "3-low";
    }else if(tag_type == TagType_mid){
        return "2-mid";
    }else if(tag_type == TagType_high){
        return "1-high";
    }
    return "4-nul";
}

TagType tag_type_enum(const std::string &tag_type){
    if(tag_type == "3-low"){
        return TagType_low;
    }else if(tag_type == "2-mid"){
        return TagType_mid;
    }else if(tag_type == "1-high"){
        return TagType_high;
    }
    return TagType_nul;
}

TodoItem::TodoItem(){
    this->create_key = "";
    this->edit_key = "";
    this->op_type = OpType_nul;
    this->is_check = Check_false;
    this->tag_type = TagType_low;
    this->reminder = "";
    this->data = "";
}

TodoItem::TodoItem(
        const std::string &create_key,
        const std::string &edit_key,
        OpType op_type,
        IsCheck is_check,
        TagType tag_type,
        const std::string &reminder,
        const std::string &data
        ){
    this->create_key = create_key;
    this->edit_key = edit_key;
    this->op_type = op_type;
    this->is_check = is_check;
    this->tag_type = tag_type;
    this->reminder = reminder;
    this->data = data;
}

bool check_item_valid(const TodoItem &item){
    if(item.create_key.empty()){
        return false;
    }
    if(item.edit_key.empty()){
        return false;
    }
    if(item.is_check == Check_null){
        return false;
    }
    if(item.op_type == OpType_nul){
        return false;
    }
    return true;
}

int time_int_s(const std::string &s_time){
    tm tm_{};
    int year, month, day, hour, minute,second;
    sscanf(s_time.c_str(),"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_.tm_year  = year-1900;
    tm_.tm_mon   = month-1;
    tm_.tm_mday  = day;
    tm_.tm_hour  = hour;
    tm_.tm_min   = minute;
    tm_.tm_sec   = second;
    tm_.tm_isdst = 0;

    time_t t_ = mktime(&tm_); //已经减了8个时区
    return t_; //秒时间
}

/* 获取MS时间 -3 */
uint64_t get_time_of_ms(){
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * (uint64_t)1000 + tv.tv_usec / 1000;
}

uint64_t get_time_of_s(){
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}

std::string get_time_key(){
    return std::to_string(get_time_of_ms());
}

std::string form_group_data(IsCheck is_check, TagType tag_type, const std::string &reminder, const std::string &data) {
    Json::Value t_group_data;
    t_group_data[TODO_ITEM_IS_CHECK] = is_check_str(is_check);
    t_group_data[TODO_ITEM_TAG_TYPE] = tag_type_str(tag_type);
    t_group_data[TODO_ITEM_REMINDER] = reminder;
    t_group_data[TODO_ITEM_DATA] = data;
    return t_group_data.toStyledString();
}

void group_data_parser(const std::string &group_data, IsCheck &is_check, TagType &tag_type, std::string &reminder, std::string &data) {
    Json::Value t_json_res;
    Json::Reader t_reader;
    t_reader.parse(group_data, t_json_res);
    is_check = is_check_enum(t_json_res.get(TODO_ITEM_IS_CHECK, "").asString());
    tag_type = tag_type_enum(t_json_res.get(TODO_ITEM_TAG_TYPE, "").asString());
    reminder = t_json_res.get(TODO_ITEM_REMINDER, "").asString();
    data = t_json_res.get(TODO_ITEM_DATA, "").asString();
}

bool todo_is_valid(const TodoItem &item){
    if(item.is_check == Check_null){
        return false;
    }
    if(item.op_type == OpType_nul){
        return false;
    }
    if(item.data.empty()){
        return false;
    }
    return true;
}

std::string get_abs_path(){
    // window 获取存放路径
    char path_buffer[1024];
#ifdef WINDOW_BUILD
    ::GetModuleFileNameA(NULL, path_buffer, 1024);
    (strrchr(path_buffer, '\\'))[1] = 0;
#endif
#ifdef LINUX_BUILD
    getcwd(path_buffer, 1024);
#endif
    return std::string(path_buffer);
}

std::string encrypt_data(const std::string &src_data, const std::string &key) {
    AesEncryption aes("cbc", 256);
    CryptoPP::SecByteBlock enc = aes.encrypt(src_data, key);
    return std::string(enc.begin(), enc.end());
}

std::string decrypt_data(const std::string &src_data, const std::string &key) {
    AesEncryption aes("cbc", 256);
    CryptoPP::SecByteBlock enc = aes.decrypt(src_data, key);
    return std::string(enc.begin(), enc.end());
}