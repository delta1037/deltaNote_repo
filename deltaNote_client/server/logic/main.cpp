#include "log.h"
#include "inter_var.h"
#include "http.h"
#include "s_sync_data.h"


int register_process(HTTP &server){
    ErrorCode error_code;
    if(RET_FAILED == server.s_register_callback(SYNC_SIGN_IN, SSyncData::sync_sign_in, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_SIGN_IN, error_code);
    }
    if(RET_FAILED == server.s_register_callback(SYNC_SIGN_UP, SSyncData::sync_sign_up, error_code)){
        d_logic_error("server %s process register error, code:%d", SYNC_SIGN_UP, error_code);
    }
    return RET_SUCCESS;
}

int main(){
    d_logic_info("%s", "main_start")

    // 初始化同步
    SSyncData::sync_init();

    // 配置服务端
    HTTP server(HTTP_server, "0.0.0.0", 1234);

    // 注册处理
    register_process(server);

    // 启动服务
    server.s_start_listen();

    SSyncData::sync_uninit();
    return 0;
}