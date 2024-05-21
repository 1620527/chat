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
};
