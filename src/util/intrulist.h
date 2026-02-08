/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef INTRULIST_H
#define INTRULIST_H

template <typename T>
struct IntruListLink
{
	IntruListLink<T> *prev;
	IntruListLink<T> *next;
	T *data;

	IntruListLink(T *data)
	    : prev(0),
	      next(0),
	      data(data)
	{}

	~IntruListLink()
	{
		if (prev && next)
		{
			next->prev = prev;
			prev->next = next;
		}
	}
};

template <typename T>
class IntruList
{
	IntruListLink<T> root;
	int size;

public:
	IntruList()
	    : root(0),
	      size(0)
	{
		root.prev = &root;
		root.next = &root;
	}

	void prepend(IntruListLink<T> &node)
	{
		root.next->prev = &node;
		node.prev = &root;
		node.next = root.next;
		root.next = &node;

		size++;
	}

	void append(IntruListLink<T> &node)
	{
		root.prev->next = &node;
		node.next = &root;
		node.prev = root.prev;
		root.prev = &node;

		size++;
	}

	void insertBefore(IntruListLink<T> &node,
	                  IntruListLink<T> &prev)
	{
		node.next = &prev;
		node.prev = prev.prev;
		prev.prev->next = &node;
		prev.prev = &node;

		size++;
	}

	void remove(IntruListLink<T> &node)
	{
		if (!node.next)
			return;

		node.prev->next = node.next;
		node.next->prev = node.prev;

		node.prev = 0;
		node.next = 0;

		size--;
	}

	void clear()
	{
		remove(root);
		root.prev = &root;
		root.next = &root;

		size = 0;
	}

	T *tail() const
	{
		IntruListLink<T> *node = root.prev;
		if (node == &root)
			return 0;

		return node->data;
	}

	IntruListLink<T> *begin()
	{
		return root.next;
	}

	IntruListLink<T> *end()
	{
		return &root;
	}

	bool isEmpty() const
	{
		return root.next == &root;
	}

	int getSize() const
	{
		return size;
	}
};

#endif // INTRULIST_H
