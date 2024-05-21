#pragma once

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;


class RPC_Pool {
public:
	RPC_Pool(size_t poolsize, std::string host, std::string port)
		:_b_stop(false), _poolSize(poolsize), _host(host), _port(port) {
		for (size_t i = 0; i < _poolSize; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(_host+":"+_port, grpc::InsecureChannelCredentials());
			_stubs.push(VarifyService::NewStub(channel));
		}
	}

	~RPC_Pool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_stubs.empty()) {
			_stubs.pop();
		}
	}

	void Close() {
		_b_stop = true;
		_c_v.notify_all();
	}

	std::unique_ptr<VarifyService::Stub> GetStub() {
		std::unique_lock<std::mutex> lock(_mutex);
		_c_v.wait(lock, [this]() {
			if (_b_stop)
				return true;
			return !_stubs.empty();
			});

		if (_b_stop)
			return nullptr;

		std::unique_ptr < VarifyService::Stub> stub = std::move(_stubs.front());
		_stubs.pop();
		return stub;
	}

	void ReturnStub(std::unique_ptr<VarifyService::Stub> stub) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_b_stop)
			return;
		_stubs.push(std::move(stub));
		_c_v.notify_one();
	}

private:
	std::atomic<bool> _b_stop;
	size_t _poolSize;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<VarifyService::Stub>> _stubs;
	std::mutex _mutex;
	std::condition_variable _c_v;
};


class VerifyGrpcClient :public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyReq request;
		GetVarifyRsp response;
		request.set_email(email);

		auto stub = _pool->GetStub();
		Status status = stub->GetVarifyCode(&context, request, &response);
		if (status.ok()) {
			_pool->ReturnStub(std::move(stub));
			response.set_error(ErrorCodes::Success);
			return response;
		}
		else {
			_pool->ReturnStub(std::move(stub));
			response.set_error(ErrorCodes::RPCFailed);
			return response;
		}
	}

private:
	VerifyGrpcClient() {
		auto& config =ConfigMgr::GetInstance();
		std::string host = config["VarifyServer"]["Host"];
		std::string port = config["VarifyServer"]["Port"];
		_pool = std::make_unique<RPC_Pool>(5, host, port);
	}

	std::unique_ptr< RPC_Pool> _pool;
};

