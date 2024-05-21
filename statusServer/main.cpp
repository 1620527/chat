#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOContextPool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"

#include <grpcpp/grpcpp.h>

void RunServer() {
	auto& cfg = ConfigMgr::GetInstance();

	std::string server_address(cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"]);
	StatusServiceImpl service;

	grpc::ServerBuilder builder;
	// �����˿ں���ӷ���
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	
	// ����������gRPC������
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// ����Boost.Asio��io_context
	boost::asio::io_context io_context;
	// ����signal_set���ڲ���SIGINT
	boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

	// �����첽�ȴ�SIGINT�ź�
	signals.async_wait([&server](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			std::cout << "Shutting down server..." << std::endl;
			server->Shutdown(); // ���ŵعرշ�����
		}
		});

	// �ڵ������߳�������io_context
	std::thread([&io_context]() { io_context.run(); }).detach();

	// �ȴ��������ر�
	server->Wait();
	io_context.stop(); // ֹͣio_context
}

int main(int argc, char** argv) {
	try {
		RunServer();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}