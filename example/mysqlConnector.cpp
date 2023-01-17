#include "mysqlConnector.h"
#include <string.h>
#include <mutex>

#ifdef USE_MYSQLCLIENTCPP
namespace grok
{

	class MysqlConnectorImp
	{
	public:
		MysqlConnectorImp()
		{
			std::call_once(onceInit, []()
						   {
			bool ret = true;
			try{
				pDriver = get_driver_instance();
			}
			catch (sql::SQLException &e) {
				ret = false;
			}
			return ret; });
		}
		static std::once_flag onceInit;
		static sql::Driver *pDriver;
	};

	// mysql connector 的源码显示，driver会保存到一个map中,它也无法释放，只能等进程结束
	// 所以，就直接把它单例化，只能有一个实例。并且只提供初始化，不提供释放；并且初始化要线程安全
	sql::Driver *MysqlConnectorImp::pDriver = nullptr;
	std::once_flag MysqlConnectorImp::onceInit;

	std::unique_ptr<MysqlConnectorImp> MysqlConnector::m_imp;
	MysqlConnector::MysqlConnector()
	{
	}
	MysqlConnector::~MysqlConnector()
	{
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

	sql::Connection *MysqlConnector::getConnect(const std::string &sHostName, const std::string &sUserName, const std::string &sPwd)
	{
		auto *con = m_imp->pDriver->connect(sHostName, sUserName, sPwd);
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

	void MysqlSession::mysql_set_connectInfo(boost::string_view szHost,
								   boost::string_view szUser,
								   boost::string_view szPwd,
								   boost::string_view szDbName)
	{
		m_szHostName = szHost.to_string();
		m_szUser = szUser.to_string();
		m_szPwd = szPwd.to_string();
		m_szDbName = szDbName.to_string();
	}

	void MysqlSession::mysql_connect()
	{
		pCon.reset(MysqlConnector::getConnect(
			m_szHostName, m_szUser, m_szPwd));
		pCon->setSchema(m_szDbName);
	}

	void MysqlSession::mysql_close()
	{
		if (pCon)
		{
			pCon->close();
			pCon.reset();
		}
	}

	std::pair<int, std::size_t> MysqlSession::mysql_excute(const std::string &stm, std::function<int(sql::ResultSet *)> op)
	{
		std::pair<int, std::size_t> r{0, 0};
		try
		{
			mysql_check_con();
			std::unique_ptr<sql::PreparedStatement> pStmt(pCon->prepareStatement(stm.c_str()));
			pStmt->execute();
			std::unique_ptr<sql::ResultSet> pRes(pStmt->getResultSet());
			r.second = op(pRes.get());
			clearMoreResult(pStmt.get());
		}
		catch (sql::SQLException &e)
		{
			logSQLException(stm, e);
			m_mqlConBad = true;
			r.first = e.getErrorCode();
			r.first = r.first == 0 ? -1 : r.first;
		}
		return r;
	}

	int MysqlSession::mysql_excute(const std::string &stm)
	{
		int errcode = 0;
		try
		{
			mysql_check_con();
			std::unique_ptr<sql::PreparedStatement> pStmt(pCon->prepareStatement(stm.c_str()));
			pStmt->execute();
			std::unique_ptr<sql::ResultSet> pRes(pStmt->getResultSet());
			clearMoreResult(pStmt.get());
		}
		catch (sql::SQLException &e)
		{
			logSQLException(stm, e);
			m_mqlConBad = true;
			errcode = e.getErrorCode();
			errcode = errcode == 0 ? -1 : errcode;
		}
		return errcode;
	}

	void MysqlSession::mysql_check_con()
	{
		// 刷新检查时间
		auto lastTp = m_tpLastMqlCon;
		m_tpLastMqlCon = std::chrono::system_clock::now();
		bool forceCheckValid = m_mqlConBad;
		m_mqlConBad = false;
		try
		{
			if (!pCon)
			{
				// 当检查到pCon为nil的时候，重新创建
				pCon.reset(MysqlConnector::getConnect(m_szHostName, m_szUser, m_szPwd));
				if (pCon)
				{
					pCon->setSchema(m_szDbName);
					return;
				}
				char buffer[4096];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "MysqlConnect Error:%s,%s,%s",
						m_szHostName.c_str(), m_szUser.c_str(), m_szPwd.c_str());
				throw sql::SQLException(buffer);
			}

			// 前置检查有效性的时候，忽略时间间隔
			if (!forceCheckValid)
			{
				auto dur = std::chrono::duration_cast<std::chrono::hours>(m_tpLastMqlCon - lastTp);
				if (dur.count() < 6)
				{
					// 只有超过6个小时无操作，才会进行mysql con有效性检查
					return;
				}
			}

			if (pCon->isValid())
			{
				return;
			}

			// 重连
			bool r = false;
			r = pCon->reconnect();
			pCon->setSchema(m_szDbName);
			if (r)
			{
				return;
			}

			// 重连失败，就再重新创建一次
			pCon.reset(MysqlConnector::getConnect(m_szHostName, m_szUser, m_szPwd));
			if (pCon)
			{
				pCon->setSchema(m_szDbName);
				return;
			}
			char buffer[4096];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "MysqlConnect Error:%s,%s,%s",
					m_szHostName.c_str(), m_szUser.c_str(), m_szPwd.c_str());
			throw sql::SQLException(buffer);
		}
		catch (...)
		{
			// 操作过程里只要发生了异常，都再抛出去，由外部处理
			std::rethrow_exception(std::current_exception());
		}
	}

	void MysqlSession::logSQLException(const std::string &stm, sql::SQLException &e)
	{
		logformat("SqlException sqltext<%s> errocode:%d, errostate:%s, erromessage:%s", stm.c_str(), e.getErrorCode(), e.getSQLState().c_str(), e.what());
	}

	void MysqlSession::logformat(const char *fmt, ...)
	{
		// 默认不做任何打印操作
	}

	void MysqlSession::clearMoreResult(sql::PreparedStatement *preStm)
	{
		while (preStm != nullptr && true == preStm->getMoreResults())
		{
		}
	}

    bool MysqlConPool::InitDbCon(int count, boost::string_view szHost,
								   boost::string_view szUser,
								   boost::string_view szPwd,
								   boost::string_view szDbName)
    {
		bool exp = false;
        if(!m_init.compare_exchange_strong(exp, true)) {
			return true;
		}

		

		return true;
    }

}
#endif // USE_MYSQLCLIENTCPP