#pragma once
#include <memory>
#include <mutex>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_algorithms.hpp>

namespace grok {

// 读写锁
template<typename T>
class grwlock {
public:
	using Mutex = boost::shared_mutex;
	class ReadLock {
	protected:
		boost::shared_lock < Mutex > l;
		const T* p;
	public:
		ReadLock(Mutex& m, const T* _p) :l(m), p(_p) {}
		const T* operator->() { return p; }
		const T* get() { return p; }

		ReadLock(ReadLock&& o) :l(std::move(o.l)), p(o.p){}
	};
	class WriteLock {
	protected:
		boost::unique_lock < Mutex > l;
		T* p;
	public:
		WriteLock(Mutex& m, T* _p) :l(m), p(_p) {}
		T* operator->() { return p; }
		T* get() { return p; }

		WriteLock(WriteLock&& o) :l(std::move(o.l)), p(o.p){}
	};
	ReadLock readGuard(const T* p) {
		return ReadLock(m_mut, p);
	}

	WriteLock writeGuard(T* p) {
		return WriteLock(m_mut, p);
	}

private:
	Mutex m_mut;
};

template <typename T>
class grwtype
{
public:
	using ValueType = T;

	template<typename...Args>
	grwtype(Args...args) :m_d(std::forward<Args>(args)...) {}

	T copy() {
		auto rg = m_rwLck.readGuard();
		return m_d;
	}

	template <typename F>
	void readf(F f) {
		auto rg = m_rwLck.readGuard(&m_d);
		const T& t = m_d;
		f(t);
	}

	template <typename F>
	void writef(F f) {
		auto wg = m_rwLck.writeGuard(&m_d);
		f(m_d);
	}

	typename grwlock<T>::ReadLock readGuard() {
		return m_rwLck.readGuard(&m_d);
	}

	typename grwlock<T>::WriteLock writeGuard() {
		return m_rwLck.writeGuard(&m_d);
	}

private:
	grwlock<T> m_rwLck;
	T m_d;
};

}