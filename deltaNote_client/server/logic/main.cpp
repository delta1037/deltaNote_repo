/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
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

int main(int argc, char* argv[]){
    int server_port = 1234;
    if(argc > 1){
        server_port = atoi(argv[1]);
    }
    d_logic_info("delta note server start at %s:%d", "0.0.0.0", server_port)
    // 配置服务端
    HTTP server(HTTP_server, "0.0.0.0", server_port);

    // 注册处理
    register_process(server);

    // 启动服务
    server.s_start_listen();
    return 0;
}