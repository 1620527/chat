#include "CServer.h"
#include "myprint.h"
#include "AsioIOContextPool.h"

CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	print("Server start success, listen on port : ", port);
	StartAccept();
}

CServer::~CServer()
{
	print("Server destruct listen on port : ", _port);
}

void CServer::ClearSession(std::string uuid)
{
	std::lock_guard<mutex> lock(_mutex);
	_sessions.erase(uuid);
}

void CServer::StartAccept() {
	auto& io_context = AsioIOContextPool::GetInstance().GetIOContext();
	std::shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		new_session->Start();
		std::lock_guard<mutex> lock(_mutex);
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.what() << endl;
	}

	StartAccept();
}
