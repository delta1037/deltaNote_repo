/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#include<thread>
#include "http.h"
#include "base64.h"
#include "log.h"

bool HTTP::s_encode = false;
std::map<std::string, SERVER_CALLBACK> HTTP::m_server_handle_map;

HTTP::HTTP(HTTPType http_type, const std::string &address, int server_port, bool encode) {
    m_http_type = http_type;
    m_address = address;
    m_server_port = server_port;
    s_encode = encode;

    m_client = nullptr;
    m_server = nullptr;

    if(m_address.empty()){
        d_net_error("%s", "http address is empty !!!")
        return;
    }
    if(m_http_type == HTTP_client){
        init_client();
    }else if(m_http_type == HTTP_server){
        init_server();
    }
}

void HTTP::init_client(){
    m_client = new httplib::Client(m_address);
    m_client->set_connection_timeout(0, 300000);
}

void HTTP::init_server(){
    if(m_server_port == 0){
        d_net_error("%s", "server init need port")
        return ;
    }
    m_server = new httplib::Server();
    m_server->set_payload_max_length(1024*1024*5);
}

int HTTP::c_get(const std::string &handle, const std::string &req, std::string &res, ErrorCode &error_code) {
    error_code = Error_no_error;
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        error_code = Error_client_error;
        return RET_FAILED;
    }
    d_net_debug("client get send, size:%d, content:%s", req.size(), req.c_str())

    httplib::Headers headers;
    if(s_encode){
        headers.emplace("get_req", base64_encode(req, true));
    }else{
        headers.emplace("get_req", req);
    }

    d_net_debug("client start to get %s, address:%s", handle.c_str(), m_address.c_str())
    auto http_res = m_client->Get(handle.c_str(), headers);
    if(http_res == nullptr){
        d_net_error("client get %s:%s return NULL", m_address.c_str(), handle.c_str())
        error_code = Error_server_error;
        return RET_FAILED;
    }

    d_net_debug("client get %s:%s return, status :%d", m_address.c_str(), handle.c_str(), http_res->status)
    if(http_res->status == 200){
        if(s_encode){
            res = base64_decode(http_res->body);
        }else{
            res = http_res->body;
        }
        return RET_SUCCESS;
    }

    // 处理遇到的错误
    d_net_debug("handle %s:%s error, reason:%s, body:%s",
                m_address.c_str(),
                handle.c_str(),
                http_res->reason.c_str(),
                http_res->body.c_str())

    error_code = Error_server_error;
    return RET_FAILED;
}


int HTTP::c_post(const std::string &handle, const std::string &req, std::string &res, ErrorCode &error_code) {
    error_code = Error_no_error;
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        error_code = Error_client_error;
        return RET_FAILED;
    }
    d_net_debug("client post send size:%d content:%s", req.size(), req.c_str())

    d_net_debug("client start to post %s, address:%s", handle.c_str(), m_address.c_str())
    auto http_res = m_client->Post(handle.c_str(), base64_encode(req, true), "text/plain");
    if(http_res == nullptr){
        d_net_error("client post %s:%s return NULL", m_address.c_str(), handle.c_str())
        error_code = Error_server_error;
        return RET_FAILED;
    }

    d_net_debug("client post %s:%s return, status :%d", m_address.c_str(), handle.c_str(), http_res->status)
    if(http_res->status == 200){
        if(s_encode){
            res = base64_decode(http_res->body);
        }else{
            res = http_res->body;
        }
        return RET_SUCCESS;
    }

    // 处理遇到的错误
    d_net_debug("handle %s:%s error, reason:%s, body:%s",
                m_address.c_str(),
                handle.c_str(),
                http_res->reason.c_str(),
                http_res->body.c_str())
    error_code = Error_server_error;
    return RET_FAILED;
}

int HTTP::s_register_callback(ComType com_type, const std::string &handle, SERVER_CALLBACK handle_callback, ErrorCode &error_code) {
    error_code = Error_no_error;
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        error_code = Error_server_error;
        return RET_FAILED;
    }

    // 注册到统一接口，之后在统一接口里进行分流
    d_net_debug("server register type:%d,handle:%s,address:%p start", com_type, handle.c_str(), handle_callback)
    if(com_type == ComType_get){
        m_server->Get(handle, s_handle_ctrl);
    }else if(com_type == ComType_post){
        m_server->Post(handle, s_handle_ctrl);
    }else{
        d_net_error("unknown com type %d", com_type)
        error_code = Error_server_error;
        return RET_FAILED;
    }

    m_server_handle_map[handle] = handle_callback;
    d_net_info("server register type:%d,handle:%s,address:%p success", com_type, handle.c_str(), handle_callback)
    return RET_SUCCESS;
}

void HTTP::s_handle_ctrl(const httplib::Request &req, httplib::Response &res) {
    d_net_debug("get request, request headers size:%d", req.headers.size())
    d_net_debug("get request, request params size:%d", req.params.size())
    d_net_debug("get request, request version:%s", req.version.c_str())
    d_net_debug("get request, request target:%s", req.target.c_str())
    d_net_debug("get request, request method:%s", req.method.c_str())
    d_net_debug("get request, request path:%s", req.path.c_str())
    d_net_debug("get request, request body:%s", req.body.c_str())

    // 检测对应的处理函数
    std::string t_handle = req.path;
    if(m_server_handle_map.find(t_handle) == m_server_handle_map.end()){
        d_net_error("handle %s process not register", t_handle.c_str())
        res.status = 404;
        return;
    }
    SERVER_CALLBACK t_handle_func = m_server_handle_map[t_handle];

    // Post还是Get类型（取参数）
    std::string s_req;
    if(req.method == "GET"){
        // GET类型，数据放head了
        auto it = req.headers.find("get_req");
        if(it != req.headers.end()){
            s_req = it->second;
        }else{
            d_logic_error("%s", "method is get but no param")
        }
    }else if(req.method == "POST"){
        s_req = req.body;
    }
    if(s_req.empty()){
        d_net_error("path %s get no param", req.path.c_str())
        return;
    }
    if(s_encode){
        s_req = base64_decode(s_req);
    }
    d_net_debug("server receive data size:%d, content %s", s_req.size(), s_req.c_str())

    std::string s_res;
    ErrorCode error_code = Error_no_error;
    int ret = t_handle_func(s_req, s_res);
    if(ret == RET_FAILED){
        d_net_error("handle %s process error", t_handle.c_str())
    }
    if(s_encode){
        s_res = base64_encode(s_res);
    }
    res.set_content(s_res, "text/plain");
    d_net_debug("server send data size:%d, content %s", s_res.size(), s_res.c_str())
}

int HTTP::s_start_listen() {
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        return RET_FAILED;
    }
    if(!m_server->listen(m_address.c_str(), m_server_port)){
        d_net_error("server listen %s:%d error", m_address.c_str(), m_server_port)
    }
    d_net_debug("server listen %s:%d success", m_address.c_str(), m_server_port)
    return RET_SUCCESS;
}

bool HTTP::check_http_status() {
    if(m_http_type == HTTP_unknown){
        return false;
    }
    if(m_http_type == HTTP_client && m_client == nullptr){
        return false;
    }
    if(m_http_type == HTTP_server && (m_server == nullptr || m_server_port == 0)){
        return false;
    }
    return true;
}
