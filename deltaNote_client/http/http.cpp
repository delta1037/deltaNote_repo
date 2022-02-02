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
}

int HTTP::c_get(const std::string &handle, const std::string &req, std::string &res, ErrorCode &error_code) {
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        error_code = Error_client_error;
        return RET_FAILED;
    }

    d_net_debug("client get send %s", req.c_str())

    httplib::Headers headers;
    if(s_encode){
        headers.emplace("get_req", base64_encode(req, true));
    }else{
        headers.emplace("get_req", req);
    }
    d_net_debug("client start to get %s, addr:%s", handle.c_str(), m_address.c_str())
    auto http_res = m_client->Get(handle.c_str(), headers); // , headers
    if(http_res == nullptr){
        d_net_error("client get %s return NULL", handle.c_str())
        error_code = Error_server_error;
        return RET_FAILED;
    }

    d_net_debug("client get return %s, status :%d", handle.c_str(), http_res->status)
    if(http_res->status == 200){
        d_net_debug("handle %s get result success", handle.c_str())
        if(s_encode){
            res = base64_decode(http_res->body);
        }else{
            res = http_res->body;
        }
        return RET_SUCCESS;
    }
    error_code = Error_server_error;
    // 处理遇到的错误
    d_net_error("handle %s get result fail, status is %d", handle.c_str(), http_res->status)
    d_net_debug("handle %s recv body %s", handle.c_str(), http_res->body.c_str())
    d_net_debug("handle %s recv reason %s", handle.c_str(), http_res->reason.c_str())
    return RET_FAILED;
}


int HTTP::c_post(const std::string &handle, const std::string &req, std::string &res, ErrorCode &error_code) {
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        return RET_FAILED;
    }

    d_net_debug("client get send %s", req.c_str())
    httplib::Params params;
    if(s_encode){
        params.emplace("post_req", base64_encode(req, true));
    }else{
        params.emplace("post_req", req);
    }

    auto http_res = m_client->Post(handle.c_str(), params);
    if(http_res->status == 200){
        d_net_debug("handle %s get result success", handle.c_str())
        if(s_encode){
            res = base64_decode(http_res->body);
        }else{
            res = http_res->body;
        }
        return RET_SUCCESS;
    }
    // 处理遇到的错误
    d_net_error("handle %s get result fail, status is %d", handle.c_str(), http_res->status)
    return RET_FAILED;
}

int HTTP::s_register_callback(const std::string &handle, SERVER_CALLBACK handle_callback, ErrorCode &error_code) {
    if(!check_http_status()){
        d_net_error("%s", "http check fail")
        return RET_FAILED;
    }
    // 注册到统一接口，之后在统一接口里进行多线程分流
    m_server->Get(handle, s_handle_ctrl);
    m_server_handle_map[handle] = handle_callback;
    d_net_debug("server register handle %s", handle.c_str())
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

    // 对处理程序进行分流
    std::string t_handle = req.path;
    //t_handle = t_handle.erase(0);
    // 检测对应的处理有没有注册
    if(m_server_handle_map.find(t_handle) == m_server_handle_map.end()){
        d_net_error("handle %s process not register", t_handle.c_str())
        res.status = 404;
        return;
    }

    SERVER_CALLBACK t_handle_func = m_server_handle_map[t_handle];
    ErrorCode error_code;
    // 分清楚是Post还是Get类型
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
        // POST类型放param了
        auto it = req.params.find("body");
        if(it != req.params.end()){
            s_req = it->second;
        }else{
            d_logic_error("%s", "method is post but no param")
        }
    }
    if(s_req.empty()){
        d_net_error("path %s get no param", req.path.c_str())
        return;
    }
    if(s_encode){
        s_req = base64_decode(s_req);
    }
    d_net_debug("server receive data is %s", s_req.c_str())

    std::string s_res;
    d_net_debug("handle func %s start call", t_handle.c_str())
    int ret = t_handle_func(s_req, s_res, error_code);
    d_net_debug("handle func %s end call", t_handle.c_str())
    d_net_debug("server send data is %s", s_res.c_str())
    if(ret == RET_FAILED){
        d_net_error("handle %s process error", t_handle.c_str())
    }
    if(s_encode){
        s_res = base64_encode(s_res);
    }
    d_net_debug("s_res: %s", s_res.c_str())
    res.set_content(s_res, "text/plain");
    d_net_debug("handle %s process success", t_handle.c_str())
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
