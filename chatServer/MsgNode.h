#pragma once
#pragma once
#include <string>
#include "const.h"
#include <iostream>
#include <boost/asio.hpp>
using namespace std;
using boost::asio::ip::tcp;
class LogicSystem;
class MsgNode
{
public:
	MsgNode(short data_len) :_total_len(data_len), _cur_len(0) {
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}

	~MsgNode() {
		std::cout << "destruct MsgNode" << endl;
		delete[] _data;
	}

	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	short _cur_len;
	short _total_len;
	char* _data;
};

class RecvNode :public MsgNode {
	friend class LogicSystem;
public:
	RecvNode(short data_len, short msg_id);
private:
	short _msg_id;
};

class SendNode :public MsgNode {
	friend class LogicSystem;
public:
	SendNode(const char* msg, short data_len, short msg_id);
private:
	short _msg_id;
};
