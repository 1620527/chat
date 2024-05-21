#pragma once
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <queue>
#include <condition_variable>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::StatusService;


class StatusConPool {
public:
	StatusConPool(size_t poolSize, std::string host, std::string port)
		: _poolSize(poolSize), _host(host), _port(port), _b_stop(false) {
		for (size_t i = 0; i < _poolSize; ++i) {

			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());

			_stubs.push(StatusService::NewStub(channel));
		}
	}

	~StatusConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_stubs.empty()) {
			_stubs.pop();
		}
	}

	std::unique_ptr<StatusService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_c_v.wait(lock, [this] {
			if (_b_stop) {
				return true;
			}
			return !_stubs.empty();
			});
		//如果停止则直接返回空指针
		if (_b_stop) {
			return  nullptr;
		}
		auto context = std::move(_stubs.front());
		_stubs.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<StatusService::Stub> context) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop) {
			return;
		}
		_stubs.push(std::move(context));
		_c_v.notify_one();
	}

	void Close() {
		_b_stop = true;
		_c_v.notify_all();
	}

private:
	std::atomic<bool> _b_stop;
	size_t _poolSize;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<StatusService::Stub>> _stubs;
	std::mutex _mutex;
	std::condition_variable _c_v;
};

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;

public:
	~StatusGrpcClient() {}
	GetChatServerRsp GetChatServer(int uid);
	LoginRsp Login(int uid, std::string token);
private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;
};

