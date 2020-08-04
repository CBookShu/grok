#include "mysqlConnector.h"
#include <mutex>

class MysqlConnectorImp
{
public:
	MysqlConnectorImp() {
		std::call_once(onceInit, [](){
			bool ret = true;
			try{
				pDriver = get_driver_instance();
			}
			catch (sql::SQLException &e) {
				ret = false;
			}
			return ret;
		});
	}
	static std::once_flag onceInit;
	static sql::Driver* pDriver;
};

// mysql connector 的源码显示，driver会保存到一个map中,它也无法释放，只能等进程结束
// 所以，就直接把它单例化，只能有一个实例。并且只提供初始化，不提供释放；并且初始化要线程安全
sql::Driver* MysqlConnectorImp::pDriver = nullptr;
std::once_flag MysqlConnectorImp::onceInit;

std::unique_ptr<MysqlConnectorImp> MysqlConnector::m_imp;
MysqlConnector::MysqlConnector() {
}
MysqlConnector::~MysqlConnector() {
}
void MysqlConnector::init()
{
	m_imp = std::make_unique<MysqlConnectorImp>();
}
void MysqlConnector::beginThread()
{
	m_imp->pDriver->threadInit();
}
void MysqlConnector::endThread()
{
	m_imp->pDriver->threadEnd();
}

sql::Connection* MysqlConnector::getConnect(const std::string& sHostName, const std::string& sUserName, const std::string& sPwd) {
    auto* con = m_imp->pDriver->connect(sHostName, sUserName, sPwd);
	return con;
}


MysqlSession::MysqlSession()
{
	m_tpLastMqlCon = std::chrono::system_clock::now();
	m_mqlConBad = false;
}

MysqlSession::~MysqlSession()
{
}

void MysqlSession::mysql_set_connectInfo(const std::string& szHost, const std::string& szUser, const std::string& szPwd, const std::string& szDbName)
{
	m_szHostName = szHost;
	m_szUser = szUser;
	m_szPwd = szPwd;
	m_szDbName = szDbName;
}

void MysqlSession::mysql_connect()
{
	pCon.reset(MysqlConnector::getConnect(
		m_szHostName, m_szUser, m_szPwd));
	pCon->setSchema(m_szDbName);
}

