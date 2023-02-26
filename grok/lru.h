#pragma once
#include <list>
#include <unordered_map>
#include <vector>

namespace grok
{   
    /*
        list<v>          : v 业务数据
        unorder_map <k,v>: k业务key，v = list<v>::iter
    */

    template <typename K, typename V>
    class gLru {
    public:
        using Pair = std::pair<K,V>;
        using Link = std::list<Pair>;
        using Map = std::unordered_map<K,typename Link::iterator>;

        gLru(std::size_t cap):m_cap(cap){}
        auto set(K& k, V& v) ->void;
        auto set(K&& k, V&& v) -> void;
        auto get(K& k) ->V*;
        auto get(const K& k) const -> const V*;
        auto keys() -> std::vector<K>;
        auto Keys() const ->std::vector<K>;

        auto erase_earliest() -> Pair;
    protected:
        Map m_key2vnode;
        Link m_ordervals;
        std::size_t m_cap;
    };



    template <typename K, typename V>
    inline auto gLru<K, V>::set(K &k, V &v) -> void
    {
        auto p = std::make_pair(k, v);
        auto it = m_key2vnode.find(p.first);
        if (it == m_key2vnode.end()) {
            if(m_key2vnode.size() == m_cap) {
                erase_earliest();
            }
            m_ordervals.push_back(p);
            auto e = m_ordervals.end();
            e--;
            m_key2vnode[p.first] = e;
        } else {
            m_ordervals.erase(it->second);
            m_ordervals.push_back(p);
            auto e = m_ordervals.end();
            e--;
            m_key2vnode[p.first] = e;
        }
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::set(K &&k, V &&v) -> void
    {
        auto p = std::make_pair(std::forward<K>(k), std::forward<V>(v));

        auto it = m_key2vnode.find(p.first);
        if (it == m_key2vnode.end()) {
            if(m_key2vnode.size() == m_cap) {
                erase_earliest();
            }
            m_ordervals.emplace_back(p);
            auto e = m_ordervals.end();
            e--;
            m_key2vnode[p.first] = e;
        } else {
            m_ordervals.erase(it->second);
            m_ordervals.emplace_back(p);
            auto e = m_ordervals.end();
            e--;
            m_key2vnode[p.first] = e;
        }
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::get(K &k) -> V*
    {
        auto it = m_key2vnode.find(k);
        if (it == m_key2vnode.end()) {
            return nullptr;
        }
        return &(*(it->second)).second;
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::get(const K &k) const -> const V*
    {
        auto it = m_key2vnode.find(k);
        if (it == m_key2vnode.end()) {
            return nullptr;
        }
        return &(*(it->second)).second;
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::keys() -> std::vector<K>
    {
        std::vector<K> res;
        res.reserve((m_ordervals.size()));
        for(auto it = m_ordervals.begin(); it != m_ordervals.end();++it) {
            res.push_back((*it).first);
        }
        return res;
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::Keys() const -> std::vector<K>
    {
        std::vector<K> res;
        res.reserve((m_ordervals.size()));
        for(auto it = m_ordervals.begin(); it != m_ordervals.end();++it) {
            res.push_back((*it).first);
        }
        return res;
    }
    template <typename K, typename V>
    inline auto gLru<K, V>::erase_earliest() -> Pair
    {
        auto p = m_ordervals.front();
        m_key2vnode.erase(p.first);
        m_ordervals.pop_front();
        return p;
    }
} // namespace grok
