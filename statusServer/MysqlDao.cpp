#include "MysqlDao.h"

MySqlPool::MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolsize)
	:_url(url), _user(user), _pass(pass), _schema(schema), _poolSize(poolsize)
{
	try {
		for (int i = 0; i < _poolSize; ++i) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* con = driver->connect(_url, _user, _pass);
			con->setSchema(_schema);
			//��ȡ��ǰʱ���
			auto current_time = std::chrono::system_clock::now().time_since_epoch();
			//��ʱ���ת��Ϊ��
			long long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(current_time).count();
			_pool.push(std::make_unique<SqlConnection>(std::move(con), timeStamp));
		}
		
		_check_thread = std::thread([this] {
			while (!_b_stop) {
				this->CheckConnection();
				std::this_thread::sleep_for(std::chrono::seconds(600));
			}
			});
		_check_thread.detach();
	}
	catch (sql::SQLException& e) {
		print("MySqlPool",e.what());
	}
}

std::unique_ptr<SqlConnection> MySqlPool::GetConnection()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_c_v.wait(lock, [this]() {
		if (_b_stop)
			return true;
		return !_pool.empty();
		});
	if (_b_stop) {
		return nullptr;
	}
	std::unique_ptr<SqlConnection> connection = std::move(_pool.front());
	_pool.pop();
	return connection;
}

void MySqlPool::ReturnConnection(std::unique_ptr<SqlConnection> connection)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_pool.push(std::move(connection));
	_c_v.notify_one();
}

void MySqlPool::CheckConnection()
{
	std::lock_guard<std::mutex> lock(_mutex);
	int poolsize = _pool.size();
	auto currentime = std::chrono::system_clock::now().time_since_epoch();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentime).count();
	for (int i = 0; i < poolsize; ++i) {
		std::unique_ptr<SqlConnection> connection = std::move(_pool.front());
		_pool.pop();
		Defer defer([this, &connection]() {
			this->_pool.push(std::move(connection));
			});
		if (timestamp - connection->_last_oper_time < 5) {
			continue;
		}
		try {
			std::unique_ptr<sql::Statement> statement(connection->_con->createStatement());
			statement->executeQuery("SELECT 1");
			//statement->execute("SELECT 1");
			connection->_last_oper_time = timestamp;
			print("executer timer alive query, curtime is:", timestamp);
		}
		catch (sql::SQLException& e) {
			print(e.what());
			//���´����µ����������滻
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
			auto* newcon = driver->connect(_url, _user, _pass);
			newcon->setSchema(_schema);
			connection->_con.reset(newcon);
			connection->_last_oper_time = timestamp;
		}
	}
}

void MySqlPool::Close()
{
	_b_stop = true;
	_c_v.notify_all();
}

MySqlPool::~MySqlPool()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_pool.empty())
		_pool.pop();
}

MysqlDao::MysqlDao()
{
	auto& cfg =ConfigMgr::GetInstance();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	_pool.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao()
{
	_pool->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto con = _pool->GetConnection();
	Defer defer([this, &con]() {
		this->_pool->ReturnConnection(std::move(con));
		});

	try {
		if (con == nullptr && con->_con == nullptr) {
			return -1;
		}
		//׼�����ô洢����
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// �����������
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		// ����PreparedStatement��ֱ��֧��ע�����������������Ҫʹ�ûỰ������������������ȡ���������ֵ

		// ִ�д洢����
		stmt->execute();

		// ����洢���������˻Ự��������������ʽ��ȡ���������ֵ�������������ִ��SELECT��ѯ����ȡ����
		// ���磬����洢����������һ���Ự����@result���洢������������������ȡ��
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			print("(res->next)result is:", result);
			return result;
		}
		return -1;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email)
{
	auto con = _pool->GetConnection();

	Defer defer([this, &con]() {
		this->_pool->ReturnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user_table WHERE name = ?"));

		// �󶨲���
		pstmt->setString(1, name);

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// ���������
		while (res->next()) {
			std::cout << "Check Email: " << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				return false;
			}
			print("is true");
			return true;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd)
{
	auto con = _pool->GetConnection();

	Defer defer([this, &con]() {
		this->_pool->ReturnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user_table SET pwd = ? WHERE name = ?"));

		// �󶨲���
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		// ִ�и���
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo)
{
	auto con = _pool->GetConnection();
	Defer defer([this, &con]() {
		_pool->ReturnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		// ׼��SQL���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user_table WHERE name = ?"));
		pstmt->setString(1, name); // ��username�滻Ϊ��Ҫ��ѯ���û���

		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// ���������
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// �����ѯ��������
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		userInfo.name = name;
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		userInfo.passwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

