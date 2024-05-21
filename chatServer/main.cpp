
#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOContextPool.h"
#include "CServer.h"
#include "ConfigMgr.h"
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	try {
		auto& cfg = ConfigMgr::GetInstance();
		auto& pool = AsioIOContextPool::GetInstance();
		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, &pool](auto, auto) {
			io_context.stop();
			pool.Stop();
			});
		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}