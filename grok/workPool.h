#pragma once
#include "eventPool.h"
#include "boost/any.hpp"
#include <vector>
#include <functional>
#include <future>
#include <unordered_set>
#include <mutex>
#include <deque>
#include <memory>
#include <set>
#include <map>

namespace grok {

	struct WorkEntryWithCache : public grok::ThreadEntryBase
	{
	public:
		using Keys = std::vector<std::string>;
		struct Job
		{
			using Ptr = std::shared_ptr < Job >;
			Keys keys;
			std::function<void()> jobcb;
			std::map<std::string, std::shared_ptr<boost::any>> jobCaches;
		};

		virtual void enterThread() override {};
		virtual void leaveThread() override {};

		boost::any& getCache(const std::string& key) {
			return *job->jobCaches[key];
		}

		template <typename T>
		T* castType(boost::any& c) {
			return boost::any_cast<T>(&c);
		}
		template <typename T, typename...Args>
		T* setType(boost::any& c, Args&&...args) {
			c = T(std::forward<Args>(args)...);
			return castType<T>(c);
		}
		template<typename T>
		// 返回已有的缓存，如果没有强制初始化一份【不提供构建的参数，否则会比较混乱】
		T* getTypeOrInit(boost::any& c) {
			if (c.empty()) {
				setType<T>(c);
			}
			return castType<T>(c);
		}

		template<typename T>
		T* getTypeOrInit(const std::string& key) {
			auto& a = getCache(key);
			return getTypeOrInit<T>(a);
		}

		// 缓存
		Job::Ptr job;
	};

	class WorkPool : public EventPools
	{
	public:
		using Keys = WorkEntryWithCache::Keys;
		using Job = WorkEntryWithCache::Job;

		virtual void start(int num = 8) override;
		virtual void stop() override;

		template <typename R, typename Entry>
		std::future<R> insertWorkJob(const Keys& keys, std::function<R(Entry*)> invoke);

		template <typename Entry>
		std::future<void> insertWorkJob(const Keys& keys, std::function<void(Entry*)> invoke);
	protected:
		struct AutoReuseChannel
		{
			AutoReuseChannel(WorkPool* p, Job::Ptr j, WorkEntryWithCache* e) :parrent(p), job(j), entry(e) {};
			~AutoReuseChannel() {
				job->jobcb = nullptr;
				entry->job = nullptr;
				parrent->completeJob(job);
			}

			WorkPool* parrent = nullptr;
			Job::Ptr job;
			WorkEntryWithCache* entry = nullptr;
		};
		void insertJob(Job::Ptr job);
		void completeJob(Job::Ptr job);


		// 关于缓存最后请求时间戳的函数
		void setJobLastTime(const std::string& key);
		void onTimerClearCache();
		// 单位秒 其他业务重写它来改变Key缓存的存活时间
		virtual int getKeepLiveTime();
	private:
		std::mutex m_lock;
		std::unordered_set<std::string> m_keyRunnings;
		std::deque<Job::Ptr> m_jobs;

		// 执行缓存
		std::map<std::string, std::shared_ptr<boost::any>> m_jobCaches;
		std::map<std::string, std::chrono::system_clock::time_point> m_jobLastTime;
		std::multimap<std::chrono::system_clock::time_point, std::string> m_jobTimeRank;
		grok::stdtimerPtr m_timerCacheClear;
	};

	// 当支持c++17的时候，最方便
	/*template <typename R, typename Entry>
	std::future<R> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<R(Entry*)> invoke) {
		auto job = std::make_shared<Job>();
		auto p = std::make_shared<std::promise < R >>();
		auto f = [this, invoke, p, job]() {
			auto entry = getThreadEntryByType<WorkEntryWithCache>();
			AutoReuseChannel erase_key(this, job);
			try {
				static_assert(std::is_base_of<WorkEntryWithCache, Entry>::value, "Entry Must Driver from PoolEntry");
				entry->job = job;
				if constexpr (std::is_void<R>::value) {
					invoke(static_cast<Entry*>(entry));
					p->set_value();
				}
				else {
					p->set_value(invoke(static_cast<Entry*>(entry)));
				}
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		job->keys = keys;
		job->jobcb = f;
		insertJob(job);
		return p->get_future();
	}*/

	template <typename R, typename Entry>
	std::future<R> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<R(Entry*)> invoke) {
		static_assert(!std::is_same<R, void>::value, "Must not void");

		auto job = std::make_shared<Job>();
		auto p = std::make_shared<std::promise < R >>();
		auto f = [this, invoke, p, job]() {
			auto entry = getThreadEntryByType<WorkEntryWithCache>();
			AutoReuseChannel erase_key(this, job, entry);
			try {
				static_assert(std::is_base_of<WorkEntryWithCache, Entry>::value, "Entry Must Driver from PoolEntry");
				entry->job = job;
				p->set_value(invoke(static_cast<Entry*>(entry)));
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		job->keys = keys;
		job->jobcb = f;
		insertJob(job);
		return p->get_future();
	}

	template <typename Entry>
	std::future<void> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<void(Entry*)> invoke) {
		auto job = std::make_shared<Job>();
		auto p = std::make_shared<std::promise < void >>();
		auto f = [this, invoke, p, job]() {
			auto entry = getThreadEntryByType<WorkEntryWithCache>();
			AutoReuseChannel erase_key(this, job, entry);
			try {
				static_assert(std::is_base_of<WorkEntryWithCache, Entry>::value, "Entry Must Driver from PoolEntry");
				entry->job = job;
				invoke(static_cast<Entry*>(entry));
				p->set_value();
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		job->keys = keys;
		job->jobcb = f;
		insertJob(job);
		return p->get_future();
	}
};
