#include "RedisMgr.h"

RedisMgr::RedisMgr()
{
	auto& config_mgr =ConfigMgr::GetInstance();
	auto host = config_mgr["Redis"]["Host"];
	auto port = config_mgr["Redis"]["Port"];
	auto pwd = config_mgr["Redis"]["Passwd"];
	int i_port = atoi(port.c_str());
	_connectPool.reset(new RedisConnectPool(5, host.c_str(), i_port, pwd.c_str()));

}


RedisMgr::~RedisMgr()
{
	Close();
}

bool RedisMgr::Get(const std::string& key, std::string& value)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	if (this->_reply == NULL) {
		_connectPool->returnConnection(connect);
		std::cout << "[ GET  " << key << " ] failed" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_STRING) {
		_connectPool->returnConnection(connect);
		std::cout << "[ GET  " << key << " ] failed" << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	_connectPool->returnConnection(connect);
	value = this->_reply->str;
	freeReplyObject(this->_reply);

	std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
	return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) {
	//执行redis命令行
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
	_connectPool->returnConnection(connect);

	//如果返回NULL则说明执行失败
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	//如果执行失败则释放连接
	if (!(this->_reply->type == REDIS_REPLY_STATUS && (strcmp(this->_reply->str, "OK") == 0 || strcmp(this->_reply->str, "ok") == 0)))
	{
		std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	//执行成功 释放redisCommand执行后返回的redisReply所占用的内存

	freeReplyObject(this->_reply);
	std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
	return true;
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
	_connectPool->returnConnection(connect);
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) {
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
	_connectPool->returnConnection(connect);
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
	_connectPool->returnConnection(connect);
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
	_connectPool->returnConnection(connect);
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) {
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	_connectPool->returnConnection(connect);
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	const char* argv[4];
	size_t argvlen[4];
	argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = hvaluelen;
	this->_reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
	_connectPool->returnConnection(connect);
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return "";
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	this->_reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
	_connectPool->returnConnection(connect);
	if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(this->_reply);
		std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
		return "";
	}

	std::string value = this->_reply->str;
	freeReplyObject(this->_reply);
	std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
	return value;
}

bool RedisMgr::Del(const std::string& key)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
	_connectPool->returnConnection(connect);
	if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

bool RedisMgr::ExistsKey(const std::string& key)
{
	auto connect = _connectPool->getConnection();
	if (connect == nullptr)
		return false;
	this->_reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
	_connectPool->returnConnection(connect);
	if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0) {
		std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}

void RedisMgr::Close()
{
	_connectPool->Close();
}

RedisConnectPool::RedisConnectPool(size_t poolSize, const char* host, int port, const char* pwd)
	:_poolSize(poolSize), _host(host), _port(port), _b_stop(false)
{
	for (size_t i = 0; i < _poolSize; ++i) {
		auto* context = redisConnect(host, port);
		if (context == nullptr || context->err != 0) {
			if (context != nullptr)
				redisFree(context);
			continue;
		}
		auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
		if (reply->type == REDIS_REPLY_ERROR) {
			print("认证失败");
			freeReplyObject(reply);
			continue;
		}
		print("认证成功");
		freeReplyObject(reply);
		_connections.push(context);
	}
}

RedisConnectPool::~RedisConnectPool()
{
	if (_b_stop == false)
		Close();
	std::lock_guard<std::mutex> lock(_mutex);
	while (!_connections.empty()) {
		auto* c = _connections.front();
		redisFree(c);
		_connections.pop();
	}
}

redisContext* RedisConnectPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_c_v.wait(lock, [this] {
		if (_b_stop)
			return true;
		return !_connections.empty();
		});
	if (_b_stop)
		return nullptr;
	auto* context = _connections.front();
	_connections.pop();
	return context;
}

void RedisConnectPool::returnConnection(redisContext* context)
{
	_mutex.lock();
	if (_b_stop) {
		_mutex.unlock();
		redisFree(context);
		return;
	}
	_connections.push(context);
	_mutex.unlock();
	_c_v.notify_one();
}

void RedisConnectPool::Close()
{
	_b_stop = true;
	_c_v.notify_all();
}
