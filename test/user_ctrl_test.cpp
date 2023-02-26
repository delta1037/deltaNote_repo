/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "s_user_ctrl.h"
#include "log.h"

int main(){
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
}