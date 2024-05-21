#pragma once

#include "myprint.h"
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ConfigMgr.h"
#include <hiredis.h>
#include <win32_hiredis.h>
#include <cassert>
#include "mydefer.h"
#include "Info.h"
#include <unordered_map>



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

const std::string code_prefix = "code_";

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,
	RPCFailed = 1002,
	Deficiency_Email = 1003,
	VarifyExpired = 1004,
	VarifyCodeErr = 1005,
	UserExist = 1006,
	PasswdErr = 1007,
	EmailNotMatch = 1008,
	PasswdUpFailed = 1009,
	PasswdInvalid = 1010,
	RPCGetFailed = 1011,
	TokenInvalid = 1012,
	UidInvalid = 1013,
};

#define MAX_LENGTH  1024*2

#define HEAD_TOTAL_LEN 4

#define HEAD_ID_LEN 2

#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000


enum MSG_IDS {
	MSG_CHAT_LOGIN = 1005, //用户登陆
	MSG_CHAT_LOGIN_RSP = 1006, //用户登陆回包
};
