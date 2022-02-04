/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include "c_sync_data.h"

// 新建用户测试
SyncStatus test_1_sign_up(CSyncData &m_sync_data){
    SyncStatus net_status = Sync_undefined_error;
    ErrorCode error_code = Error_no_error;
    int ret = m_sync_data.sync_sign_up(net_status, error_code);
    printf(">>>> test_1_sign_up ret:%d, status:%d, error_code:%d\n", ret, net_status, error_code);
    return net_status;
}

// 登录测试
SyncStatus test_2_sign_in(CSyncData &m_sync_data){
    SyncStatus net_status = Sync_undefined_error;
    ErrorCode error_code = Error_no_error;
    int ret = m_sync_data.sync_sign_in(net_status, error_code);
    printf(">>>> test_2_sign_in ret:%d, status:%d, error_code:%d\n", ret, net_status, error_code);
    return net_status;
}

int main(){
    // 数据控制
    CDataCtrl data_ctrl;
    // 配置控制
    SettingCtrl setting_ctrl;
    setting_ctrl.load_all_setting();
    // 同步控制
    CSyncData sync_data(&setting_ctrl, &data_ctrl);

    setting_ctrl.set_string(SETTING_USERNAME, "name1");
    setting_ctrl.set_string(SETTING_PASSWORD, "pwd");

    SyncStatus net_status = Sync_undefined_error;
    // 新建用户测试
    net_status = test_1_sign_up(sync_data);
    if(net_status != Sync_success){
        printf("############## [ERROR] create new user failed\n");
    }else{
        printf("############## [INFO] create new user success\n");
    }
    // 新建已存在用户测试
    net_status = test_1_sign_up(sync_data);
    if(net_status != Sync_sign_up_user_exists){
        printf("############## [ERROR] create exists user failed\n");
    }else{
        printf("############## [INFO] create exists user success\n");
    }
    // 已存在用户登录测试
    net_status = test_2_sign_in(sync_data);
    if(net_status != Sync_success){
        printf("############## [ERROR] user sign in failed\n");
    }else{
        printf("############## [INFO] user sign in success\n");
    }
    return 0;
}