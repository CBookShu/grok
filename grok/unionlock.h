#pragma once
#include <vector>
#include <unordered_set>
#include <mutex>
#include <thread>

namespace grok
{
template <typename K>
class UnionLockLocal {
public:
    using Key = K;
    using Keys = std::unordered_set<Key>;

    void LockKeys(Keys& keys) {
        std::unordered_set<Key> holds;
        while(true) {
            bool ok = true;
            holds.clear();
            for(auto& k:keys) {
                if(Lock(k)) {
                    holds.insert(k);
                } else {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                return;
            }
            UnLockKeys(holds);
            std::this_thread::yield();
        }
    }
    void UnLockKeys(std::unordered_set<Key>& keys) {
        for(auto&k:keys) {
            UnLock(k);
        }
    }
public:
    class Guard {
        Keys* m_keys = nullptr;
        UnionLockLocal* m_ull = nullptr;
    public:
        Guard(UnionLockLocal* ull, Keys* keys):m_keys(keys),m_ull(ull){}
        ~Guard() {
            if(m_ull && m_keys) {
                m_ull->UnLockKeys(*m_keys);
            }
        }
    };
    Guard LockGuard(Keys& keys) {
        LockKeys(keys);
        return Guard(this, &keys);
    }
protected:
    // __sync_bool_compare_and_swap 的set版本
    bool Lock(const Key& k) {
        std::unique_lock<std::mutex> guard(m_mutex);
        auto r = m_setbits.insert(k);
        return r.second;
    }
    void UnLock(const Key& k) {
        std::unique_lock<std::mutex> guard(m_mutex);
        m_setbits.erase(k);
    }
private:
    std::mutex m_mutex;
    std::unordered_set<Key> m_setbits;
};

} // namespace grok
