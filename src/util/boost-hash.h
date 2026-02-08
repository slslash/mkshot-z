/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef BOOSTHASH_H
#define BOOSTHASH_H


// Need to use Map and Set, unordered can't handle pair apparently
#include <map>
#include <set>

#include <utility>

/* Wrappers around the boost unordered template classes,
 * exposing an interface similar to Qt's QHash/QSet */

template<typename K, typename V>
class BoostHash
{
private:
    std::map<K, V> p = {};

public:
	typedef typename std::map<K, V>::const_iterator const_iterator;

	inline bool contains(const K &key) const
	{
		const_iterator iter = p.find(key);

		return (iter != p.cend());
	}

	inline void insert(const K &key, const V &value)
	{
        p[key] = value;
        //p.insert(std::pair<K, V>(key, value));
	}

	inline void remove(const K &key)
	{
		p.erase(key);
	}

	inline const V value(const K &key) const
	{
		const_iterator iter = p.find(key);

		if (iter == p.cend())
			return V();

		return iter->second;
	}

	inline const V value(const K &key, const V &defaultValue) const
	{
		const_iterator iter = p.find(key);

		if (iter == p.cend())
			return defaultValue;

		return iter->second;
	}

	inline V &operator[](const K &key)
	{
		return p[key];
	}

	inline const_iterator cbegin() const
	{
		return p.cbegin();
	}

	inline const_iterator cend() const
	{
		return p.cend();
	}
    
    inline void clear()
    {
        p.clear();
    }
};

template<typename K>
class BoostSet
{
private:
	//typedef std::unordered_set<K> BoostType;
	std::set<K> p;

public:
	typedef typename std::set<K>::const_iterator const_iterator;

	inline bool contains(const K &key)
	{
		const_iterator iter = p.find(key);

		return (iter != p.cend());
	}

	inline void insert(const K &key)
	{
		p.insert(key);
	}

	inline void remove(const K &key)
	{
		p.erase(key);
	}

	inline const_iterator cbegin() const
	{
		return p.cbegin();
	}

	inline const_iterator cend() const
	{
		return p.cend();
	}
};

#endif // BOOSTHASH_H
