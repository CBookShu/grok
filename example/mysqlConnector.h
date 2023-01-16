#include <memory>
#include <functional>
#include <chrono>
#include <atomic>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <boost/noncopyable.hpp>
#include <boost/utility/string_view.hpp>
#include "grok/grok.h"

namespace sql
{
	class Connection;
	class PreparedStatement;
	class ResultSet;
}

namespace grok
{
	class MysqlConnectorImp;
	class MysqlConnector
	{
	public:
		MysqlConnector();
		~MysqlConnector();

		// main函数最初调用
		static void init();
		// https://dev.mysql.com/doc/c-api/5.7/en/mysql-thread-init.html
		// 在新thread中mysql操作之前调用
		static void beginThread();
		// 在thread结束前调用
		static void endThread();
		// 线程安全
		static sql::Connection *getConnect(const std::string &sHostName, const std::string &sUserName, const std::string &sPwd);

	private:
		static std::unique_ptr<MysqlConnectorImp> m_imp;
	};

	class MysqlSession
	{
	public:
		MysqlSession();
		~MysqlSession();

		void mysql_set_connectInfo(boost::string_view szHost,
								   boost::string_view szUser,
								   boost::string_view szPwd,
								   boost::string_view szDbName);

		void mysql_connect();
		void mysql_close();

		// 返回值first是err，只要非0就是错误
		// 返回值second是结果集的个数
		// 第二个参数是一个执行解析结果集的函数，它返回结果的个数
		std::pair<int, std::size_t> mysql_excute(const std::string &stm, std::function<int(sql::ResultSet *)> op);
		int mysql_check_result(std::pair<int, std::size_t> res)
		{
			if (res.first)
				return res.first;
			if (res.second <= 0)
				return -1;
			return 0;
		}
		// 执行一句sql语句，无返回结果
		// 返回值:0 正常;其他值 异常
		int mysql_excute(const std::string &stm);
		// 每次在使用sqlcon前，进行一次检查，sql是否有效
		void mysql_check_con();

	protected:
		void clearMoreResult(sql::PreparedStatement *preStm);
		void logSQLException(const std::string &stm, sql::SQLException &e);
		virtual void logformat(const char *fmt, ...);

	protected:
		// 最近的一次mysql操作时间
		std::chrono::system_clock::time_point m_tpLastMqlCon;
		// 如果上一次操作mysql出现异常,则设置为true，这样下一次再操作mysql的时候，会强制检查有效性
		bool m_mqlConBad;

		std::string m_szHostName;
		std::string m_szUser;
		std::string m_szPwd;
		std::string m_szDbName;

		std::unique_ptr<sql::Connection> pCon;
	};

	// TODO: 继续完成pool
	// TODO: 前去mysql官网确认threadini和threadend究竟有什么影响？
	class MysqlConPool : boost::noncopyable {
	public:
		MysqlConPool() = default;
		bool InitDbCon(int count, boost::string_view szHost,
								   boost::string_view szUser,
								   boost::string_view szPwd,
								   boost::string_view szDbName);

	private:
		std::atomic_bool m_init{false};
		LockList<MysqlSession> m_pool;
	};
}