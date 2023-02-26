#pragma once

#include <list>
#include <unordered_map>
#include <vector>

namespace grok
{
	/*
	list<v>          : v 业务数据
	unorder_map <k,v>: k业务key，v = pair(list::value_type, list::iter)
	*/

	template <typename K, typename V>
	class gLru {
	public:
		using Link = std::list<K>;
		using Pair = std::pair<V, typename Link::iterator>;
		using Map = std::unordered_map<K, Pair>;

		gLru(std::size_t cap) :m_cap(cap){}
		auto set(K& k, V& v) ->void;
		auto set(K&& k, V&& v) -> void;
		auto get(K& k)->V*;
		auto get(const K& k) const -> const V*;
		auto keys()->std::vector<K>;
		auto Keys() const->std::vector<K>;

		auto erase_earliest() -> void;
	protected:
		mutable Map m_key2vnode;
		mutable Link m_ordervals;
		std::size_t m_cap;
	};



	template <typename K, typename V>
	inline auto gLru<K, V>::set(K &k, V &v) -> void
	{
		auto it = m_key2vnode.find(k);
		if (it == m_key2vnode.end()) {
			if (m_key2vnode.size() == m_cap) {
				erase_earliest();
			}
			m_ordervals.emplace_back(k);
			auto e = m_ordervals.end();
			e--;
			m_key2vnode[k] = std::make_pair(v, e);
		}
		else {
			m_ordervals.erase(it->second.second);
			m_ordervals.emplace_back(k);
			auto e = m_ordervals.end();
			e--;
			 m_key2vnode[k] = std::make_pair(v, e);
		}
	}
	template <typename K, typename V>
	inline auto gLru<K, V>::set(K &&k, V &&v) -> void
	{
		auto it = m_key2vnode.find(k);
		if (it == m_key2vnode.end()) {
			if (m_key2vnode.size() == m_cap) {
				erase_earliest();
			}
			m_ordervals.emplace_back(k);
			auto e = m_ordervals.end();
			e--;
			m_key2vnode[k] = std::make_pair(std::move(v), e);
		}
		else {
			m_ordervals.erase(it->second.second);
			m_ordervals.emplace_back(k);
			auto e = m_ordervals.end();
			e--;
			m_key2vnode[k] = std::make_pair(std::move(v), e);
		}
	}
	template <typename K, typename V>
	inline auto gLru<K, V>::get(K &k) -> V*
	{
		auto it = m_key2vnode.find(k);
		if (it == m_key2vnode.end()) {
			return nullptr;
		}

		auto e = --m_ordervals.end();
		if (it->second.second == e) {
			return &it->second.first;
		}
		m_ordervals.erase(it->second.second);
		m_ordervals.push_back(k);
		m_key2vnode[k] = std::make_pair(it->second.first, --m_ordervals.end());
		it = m_key2vnode.find(k);
		return &it->second.first;
	}
	template <typename K, typename V>
	inline auto gLru<K, V>::get(const K &k) const -> const V*
	{
		auto it = m_key2vnode.find(k);
		if (it == m_key2vnode.end()) {
			return nullptr;
		}

		auto e = --m_ordervals.end();
		if (it->second.second == e) {
			return &it->second.first;
		}
		m_ordervals.erase(it->second.second);
		m_ordervals.push_back(k);
		m_key2vnode[k] = std::make_pair(it->second.first, --m_ordervals.end());
		it = m_key2vnode.find(k);
		return &it->second.first;
	}
		template <typename K, typename V>
	inline auto gLru<K, V>::keys() -> std::vector<K>
	{
		std::vector<K> res;
		res.reserve((m_ordervals.size()));
		for (auto it = m_ordervals.begin(); it != m_ordervals.end(); ++it) {
			res.push_back((*it).first);
		}
		return res;
	}
	template <typename K, typename V>
	inline auto gLru<K, V>::Keys() const -> std::vector<K>
	{
		std::vector<K> res;
		res.reserve((m_ordervals.size()));
		for (auto it = m_ordervals.begin(); it != m_ordervals.end(); ++it) {
			res.push_back((*it).first);
		}
		return res;
	}
	template <typename K, typename V>
	inline auto gLru<K, V>::erase_earliest() -> void
	{
		auto k = m_ordervals.front();
		m_key2vnode.erase(k);
		m_ordervals.pop_front();
	}
} // namespace grok
