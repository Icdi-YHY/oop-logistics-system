#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <sstream>
#include <vector>

// 通信协议常量定义
// 请求格式: COMMAND|param1|param2|...
// 响应格式: OK|message\n<data>\nEND  或  ERR|message

namespace Protocol {

// 分隔符
const char DELIMITER = '|';
const std::string END_MARKER = "END";
const std::string OK_PREFIX = "OK";
const std::string ERR_PREFIX = "ERR";

// 登录类型
const std::string LOGIN_TYPE_USER = "user";
const std::string LOGIN_TYPE_ADMIN = "admin";
const std::string LOGIN_TYPE_COURIER = "courier";

// 命令常量
const std::string CMD_LOGIN = "LOGIN";
const std::string CMD_REGISTER = "REGISTER";
const std::string CMD_LOGOUT = "LOGOUT";
const std::string CMD_SEND_PACKAGE = "SEND_PACKAGE";
const std::string CMD_RECEIVE_PACKAGE = "RECEIVE_PACKAGE";
const std::string CMD_QUERY_SENT = "QUERY_SENT";
const std::string CMD_QUERY_RECEIVED = "QUERY_RECEIVED";
const std::string CMD_QUERY_BY_ID = "QUERY_BY_ID";
const std::string CMD_QUERY_BY_TIME_SENT = "QUERY_BY_TIME_SENT";
const std::string CMD_QUERY_BY_TIME_RECEIVED = "QUERY_BY_TIME_RECEIVED";
const std::string CMD_GET_BALANCE = "GET_BALANCE";
const std::string CMD_RECHARGE = "RECHARGE";
const std::string CMD_CHANGE_PWD = "CHANGE_PWD";
const std::string CMD_ADMIN_SHOW_USERS = "ADMIN_SHOW_USERS";
const std::string CMD_ADMIN_SHOW_PACKAGES = "ADMIN_SHOW_PACKAGES";
const std::string CMD_ADMIN_SHOW_BALANCE = "ADMIN_SHOW_BALANCE";
const std::string CMD_ADMIN_ADD_COURIER = "ADMIN_ADD_COURIER";
const std::string CMD_ADMIN_REMOVE_COURIER = "ADMIN_REMOVE_COURIER";
const std::string CMD_ADMIN_SHOW_COURIERS = "ADMIN_SHOW_COURIERS";
const std::string CMD_ADMIN_SHOW_COURIER_DETAIL = "ADMIN_SHOW_COURIER_DETAIL";
const std::string CMD_ADMIN_ASSIGN_COURIER = "ADMIN_ASSIGN_COURIER";
const std::string CMD_ADMIN_QUERY_BY_SENDER = "ADMIN_QUERY_BY_SENDER";
const std::string CMD_ADMIN_QUERY_BY_RECEIVER = "ADMIN_QUERY_BY_RECEIVER";
const std::string CMD_ADMIN_QUERY_BY_ID = "ADMIN_QUERY_BY_ID";
const std::string CMD_ADMIN_QUERY_BY_TIME = "ADMIN_QUERY_BY_TIME";
const std::string CMD_COURIER_MY_TASKS = "COURIER_MY_TASKS";
const std::string CMD_COURIER_COLLECT = "COURIER_COLLECT";
const std::string CMD_COURIER_MY_RECORDS = "COURIER_MY_RECORDS";
const std::string CMD_COURIER_BALANCE = "COURIER_BALANCE";
const std::string CMD_COURIER_CHANGE_PWD = "COURIER_CHANGE_PWD";
const std::string CMD_EXIT = "EXIT";

// 构建请求消息
// @param cmd 命令名称
// @param params 参数列表
// @return 格式化的请求字符串
inline std::string buildRequest(const std::string& cmd, const std::vector<std::string>& params = {}) {
    std::string req = cmd;
    for (const auto& p : params) {
        req += DELIMITER + p;
    }
    req += "\n";
    return req;
}

// 构建成功响应
inline std::string buildOkResponse(const std::string& msg) {
    return OK_PREFIX + "|" + msg + "\n";
}

// 构建错误响应
inline std::string buildErrResponse(const std::string& msg) {
    return ERR_PREFIX + "|" + msg + "\n";
}

// 构建带数据的成功响应
inline std::string buildDataResponse(const std::string& msg, const std::string& data) {
    return OK_PREFIX + "|" + msg + "\n" + data + "\n" + END_MARKER + "\n";
}

} // namespace Protocol

#endif
