#pragma once

#include "deque.hpp"

namespace _std {

// TEMPLATE CLASS stack
template<class _Ty,
	class _Container = deque<_Ty> >
	class stack
	{	// LIFO queue implemented with a container
public:
	typedef _Container container_type;
	typedef typename _Container::value_type value_type;
	typedef typename _Container::size_type size_type;

	stack()
		: c()
		{	// construct with empty container
		}

	explicit stack(const _Container& _Cont)
		: c(_Cont)
		{	// construct by copying specified container
		}

	bool empty() const
		{	// test if stack is empty
		return (c.empty());
		}

	size_type size() const
		{	// test length of stack
		return (c.size());
		}

	value_type& top()
		{	// return last element of mutable stack
		return (c.back());
		}

	const value_type& top() const
		{	// return last element of nonmutable stack
		return (c.back());
		}

	void push(const value_type& _Val)
		{	// insert element at end
		c.push_back(_Val);
		}

	void pop()
		{	// erase last element
		c.pop_back();
		}

//protected:
	_Container c;	// the underlying container
	};

		// stack TEMPLATE FUNCTIONS
template<class _Ty,
	class _Container> inline
	bool operator==(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test for stack equality
	return (_Left.c == _Right.c);
	}

template<class _Ty,
	class _Container> inline
	bool operator!=(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test for stack inequality
	return (!(_Left == _Right));
	}

template<class _Ty,
	class _Container> inline
	bool operator<(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test if _Left < _Right for stacks
	return (_Left.c < _Right.c);
	}

template<class _Ty,
	class _Container> inline
	bool operator>(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test if _Left > _Right for stacks
	return (_Right < _Left);
	}

template<class _Ty,
	class _Container> inline
	bool operator<=(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test if _Left <= _Right for stacks
	return (!(_Right < _Left));
	}

template<class _Ty,
	class _Container> inline
	bool operator>=(const stack<_Ty, _Container>& _Left,
		const stack<_Ty, _Container>& _Right)
	{	// test if _Left >= _Right for stacks
	return (!(_Left < _Right));
}
} // namespace _std

