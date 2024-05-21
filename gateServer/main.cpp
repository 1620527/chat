#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "const.h"
#include "MysqlMgr.h"



int main()
{
	ConfigMgr& gCfgMgr =ConfigMgr::GetInstance();
	RedisMgr::GetInstance();
	MysqlMgr::GetInstance();
	std::string gate_port_str = gCfgMgr["[GateServer]"]["Port"];
	unsigned short gate_port = atoi(gate_port_str.c_str());

	try {
		unsigned short port = 8080;
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](boost::system::error_code ec, int signal_number) {
			if (ec) {
				return;
			}
			ioc.stop();
			});
		std::make_shared<CServer>(ioc, port)->Start();
		print("Gata server listen on port: ", port);
		ioc.run();
	}
	catch (std::exception& e) {
		print(e.what());
		return EXIT_FAILURE;
	}

}