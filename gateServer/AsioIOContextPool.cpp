#include "AsioIOContextPool.h"
#include "myprint.h"

AsioIOContextPool::AsioIOContextPool(std::size_t size)
	:_ioContexts(size), _works(size), _nextIOContext(0)
{
	for (std::size_t i = 0; i < size; ++i) {
		_works[i] = std::unique_ptr<Work>(new Work(_ioContexts[i]));
	}

	for (std::size_t i = 0; i < size; ++i) {
		_threads.emplace_back([this, i]() {
			_ioContexts[i].run();
		});
	}
}

AsioIOContextPool::~AsioIOContextPool() {
	Stop();
	print("AsioIOContextPool destruct");
}

boost::asio::io_context& AsioIOContextPool::GetIOContext() {
	auto& ioc = _ioContexts[_nextIOContext++];
	if (_nextIOContext == _ioContexts.size())
		_nextIOContext = 0;
	return ioc;
}

void AsioIOContextPool::Stop() {
	for (auto& work : _works) {
		work->get_io_context().stop();
		work.reset(nullptr);
	}
	for (auto& t : _threads) {
		t.join();
	}
}
