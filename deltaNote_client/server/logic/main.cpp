#include "log.h"
#include "inter_var.h"
#include "http.h"
#include "proc_api.h"

int register_process(HTTP &server){
    ErrorCode error_code;
    if(RET_FAILED == server.s_register_callback(ComType_get, SYNC_SIGN_IN, ProcAPI::api_sign_in, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_SIGN_IN, error_code);
    }
    if(RET_FAILED == server.s_register_callback(ComType_get, SYNC_SIGN_UP, ProcAPI::api_sign_up, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_SIGN_UP, error_code);
    }
    if(RET_FAILED == server.s_register_callback(ComType_post, SYNC_UPLOAD, ProcAPI::api_upload, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_UPLOAD, error_code);
    }
    if(RET_FAILED == server.s_register_callback(ComType_get, SYNC_DOWNLOAD, ProcAPI::api_download, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_DOWNLOAD, error_code);
    }
    return RET_SUCCESS;
}

int test(){
    static auto *user_ctrl = new SUserCtrl();
    ErrorCode error_code;
    UserItem user_item;
    user_item.username = "123";
    int ret = user_ctrl->sel_user(user_item.username, user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("1,user %s not exist", user_item.username.c_str())
    }

    ret = user_ctrl->sel_user(user_item.username, user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("2,user %s not exist", user_item.username.c_str())
    }

    ret = user_ctrl->sel_user(user_item.username, user_item, error_code);
    if(ret == RET_FAILED){
        d_logic_error("3,user %s not exist", user_item.username.c_str())
    }

    delete user_ctrl;
    return 0;
}

int main(){
    d_logic_info("%s", "main_start")

    // 配置服务端
    HTTP server(HTTP_server, "0.0.0.0", 1234);

    // 注册处理
    register_process(server);

    // 启动服务
    server.s_start_listen();
    return 0;
}