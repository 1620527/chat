#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOContextPool.h"


CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	:_ioc(ioc), _acceptor(ioc, tcp::endpoint(net::ip::make_address("127.0.0.1"), port))
{
	print("has server");
}

void CServer::Start()
{
	auto self = shared_from_this();
	auto& ioc = AsioIOContextPool::GetInstance().GetIOContext();
	std::shared_ptr<HttpConnection> newConnection = std::make_shared<HttpConnection>(ioc);
	_acceptor.async_accept(newConnection->GetSocket(), [self, newConnection](beast::error_code ec) {
		try {
			//�������������ӣ�����������������
			if (ec) {
				self->Start();
				return;
			}

			print("Do async accept thread id:  ",std::this_thread::get_id());//
			//���������ӣ�������HttpConnection������������
			newConnection->Start();

			//��������
			self->Start();
		}
		catch(std::exception& excep){
			print(excep.what());
		}
		});
}
