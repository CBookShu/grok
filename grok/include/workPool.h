#pragma once
#include "eventPool.h"
#include <vector>
#include <functional>
#include <future>
#include <unordered_set>
#include <mutex>
#include <deque>
#include <memory>
#include <set>

namespace grok {

	class WorkPool : public EventPools
	{
	public:
		using Keys = std::vector<std::string>;

		template <typename R, typename Entry>
		std::future<R> insertWorkJob(const Keys& keys, std::function<R(Entry*)> invoke);

		template <typename Entry>
		std::future<void> insertWorkJob(const Keys& keys, std::function<void(Entry*)> invoke);
	protected:
		struct Job
		{
			using Ptr = std::shared_ptr < Job >;
			Keys keys;
			std::function<void()> jobcb;
		};
		struct AutoReuseChannel
		{
			AutoReuseChannel(WorkPool* p, const Keys& k) :parrent(p), keys(k) {};
			~AutoReuseChannel() {
				parrent->completeJob(keys);
			}

			WorkPool* parrent = nullptr;
			Keys keys;
		};
		void insertJob(const Keys& keys, std::function<void()> f);
		void completeJob(const Keys& keys);

	private:
		std::mutex m_lock;
		std::unordered_set<std::string> m_keyRunnings;
		std::deque<Job::Ptr> m_jobs;
	};

	// 当支持c++17的时候，最方便
	/*template <typename R, typename Entry>
	std::future<R> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<R(Entry*)> invoke) {
		auto p = std::make_shared<std::promise < R >>();
		auto f = [this, invoke, p, keys]() {
			auto entry = getThreadEntryByType<ThreadEntryBase>();
			AutoReuseChannel erase_key(this, keys);
			try {
				static_assert(std::is_base_of<ThreadEntryBase, Entry>::value, "Entry Must Driver from PoolEntry");
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
		insertJob(keys, f);
		return p->get_future();
	}*/

	template <typename R, typename Entry>
	std::future<R> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<R(Entry*)> invoke) {
		static_assert(!std::is_same<R, void>::value, "Must not void");

		auto p = std::make_shared<std::promise < R >>();
		auto f = [this, invoke, p, keys]() {
			auto entry = getThreadEntryByType<ThreadEntryBase>();
			AutoReuseChannel erase_key(this, keys);
			try {
				static_assert(std::is_base_of<ThreadEntryBase, Entry>::value, "Entry Must Driver from PoolEntry");
				p->set_value(invoke(static_cast<Entry*>(entry)));
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		insertJob(keys, f);
		return p->get_future();
	}

	template <typename Entry>
	std::future<void> WorkPool::insertWorkJob(const WorkPool::Keys& keys, std::function<void(Entry*)> invoke) {
		auto p = std::make_shared<std::promise < void >>();
		auto f = [this, invoke, p, keys]() {
			auto entry = getThreadEntryByType<ThreadEntryBase>();
			AutoReuseChannel erase_key(this, keys);
			try {
				static_assert(std::is_base_of<ThreadEntryBase, Entry>::value, "Entry Must Driver from PoolEntry");
				invoke(static_cast<Entry*>(entry));
				p->set_value();
			}
			catch (...) {
				p->set_exception(std::current_exception());
			}
		};
		insertJob(keys, f);
		return p->get_future();
	}
};
