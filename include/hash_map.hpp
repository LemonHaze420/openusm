// hash_map standard header
#pragma once
#ifndef _HASH_MAP_
#define _HASH_MAP_
#ifndef RC_INVOKED
#include <xhash.hpp>

#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)
#endif  /* _MSC_VER */

_STDEXT_BEGIN

 #if _HAS_ITERATOR_DEBUGGING
using std::_Debug_message;
 #endif

		// TEMPLATE CLASS _Hmap_traits
template<class _Kty,	// key type
	class _Ty,	// mapped type
	class _Tr,	// comparator predicate type
	class _Alloc,	// actual allocator type (should be value allocator)
	bool _Mfl>	// true if multiple equivalent keys are permitted
	class _Hmap_traits
		: public _STD _Container_base
	{	// traits required to make _Hash behave like a map
public:
	typedef _Kty key_type;
	typedef _STD pair<const _Kty, _Ty> value_type;
	typedef _Tr key_compare;
	typedef typename _Alloc::template rebind<value_type>::other
		allocator_type;

 #if _HAS_IMMUTABLE_SETS
	typedef _POINTER_X(value_type, allocator_type) _ITptr;
	typedef _REFERENCE_X(value_type, allocator_type) _IReft;
 #endif /* _HAS_IMMUTABLE_SETS */

	enum
		{	// make multi parameter visible as an enum constant
		_Multi = _Mfl};

	_Hmap_traits()
		: comp()
		{	// construct with default comparator
		}

	_Hmap_traits(const _Tr& _Traits)
		: comp(_Traits)
		{	// construct with specified comparator
		}

        class value_compare
            : public std::binary_function<value_type,
                                          value_type,
                                          bool> { // functor for comparing two element values
            friend class _Hmap_traits<_Kty, _Ty, _Tr, _Alloc, _Mfl>;

        public:
            bool operator()(const value_type &_Left,
                            const value_type &_Right)
                const { // test if _Left precedes _Right by comparing just keys
                return (comp(_Left.first, _Right.first));
			}

		value_compare(const key_compare& _Traits)
			: comp(_Traits)
			{	// construct with specified predicate
			}

	protected:
		key_compare comp;	// the comparator predicate for keys
        };

    static const _Kty& _Kfn(const value_type& _Val)
		{	// extract key from element value
		return (_Val.first);
		}

	_Tr comp;	// the comparator predicate for keys
};

// TEMPLATE CLASS hash_map
template<class _Kty,
         class _Ty,
         class _Tr = hash_compare<_Kty, std::less<_Kty>>,
         class _Alloc = _STD allocator<_STD pair<const _Kty, _Ty>>>
class hash_map
    : public _Hash<
          _Hmap_traits<_Kty, _Ty, _Tr, _Alloc, false>> { // hash table of {key, mapped} values, unique keys
public:
    typedef hash_map<_Kty, _Ty, _Tr, _Alloc> _Myt;
    typedef _Hash<_Hmap_traits<_Kty, _Ty, _Tr, _Alloc, false>> _Mybase;
    typedef _Kty key_type;
    typedef _Ty mapped_type;
    typedef _Ty referent_type;
    typedef _Tr key_compare;
    typedef typename _Mybase::value_compare value_compare;
    typedef typename _Mybase::allocator_type allocator_type;
    typedef typename _Mybase::size_type size_type;
    typedef typename _Mybase::difference_type difference_type;
    typedef typename _Mybase::pointer pointer;
    typedef typename _Mybase::const_pointer const_pointer;
    typedef typename _Mybase::reference reference;
    typedef typename _Mybase::const_reference const_reference;
    typedef typename _Mybase::iterator iterator;
    typedef typename _Mybase::const_iterator const_iterator;
    typedef typename _Mybase::reverse_iterator reverse_iterator;
    typedef typename _Mybase::const_reverse_iterator const_reverse_iterator;
    typedef typename _Mybase::value_type value_type;

    hash_map() : _Mybase(key_compare(), allocator_type()) { // construct empty map from defaults
    }

    explicit hash_map(const key_compare &_Traits)
        : _Mybase(_Traits, allocator_type()) { // construct empty map from comparator
    }

    hash_map(const key_compare& _Traits, const allocator_type& _Al)
		: _Mybase(_Traits, _Al)
		{	// construct empty map from comparator and allocator
		}

	template<class _Iter>
		hash_map(_Iter _First, _Iter _Last)
		: _Mybase(key_compare(), allocator_type())
		{	// construct map from sequence, defaults
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

	template<class _Iter>
		hash_map(_Iter _First, _Iter _Last,
			const key_compare& _Traits)
		: _Mybase(_Traits, allocator_type())
		{	// construct map from sequence, comparator
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

	template<class _Iter>
		hash_map(_Iter _First, _Iter _Last,
			const key_compare& _Traits,
			const allocator_type& _Al)
		: _Mybase(_Traits, _Al)
		{	// construct map from sequence, comparator, and allocator
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

        mapped_type &operator[](
            const key_type &_Keyval) { // find element matching _Keyval or insert with default mapped
            iterator _Where = this->lower_bound(_Keyval);
            if (_Where == this->end()) {
                _Where = this->insert(value_type(_Keyval, mapped_type())).first;
            }
            return ((*_Where).second);
        }
};

template<class _Kty,
	class _Ty,
	class _Tr,
	class _Alloc> inline
	void swap( _STDEXT hash_map<_Kty, _Ty, _Tr, _Alloc>& _Left,
		_STDEXT hash_map<_Kty, _Ty, _Tr, _Alloc>& _Right)
	{	// swap _Left and _Right hash_maps
	_Left.swap(_Right);
	}

    // TEMPLATE CLASS hash_multimap
    template<class _Kty,
             class _Ty,
             class _Tr = hash_compare<_Kty, std::less<_Kty>>,
             class _Alloc = _STD allocator<_STD pair<const _Kty, _Ty>>>
    class hash_multimap
        : public _Hash<
              _Hmap_traits<_Kty, _Ty, _Tr, _Alloc, true>> { // hash table of {key, mapped} values, non-unique keys
    public:
        typedef hash_multimap<_Kty, _Ty, _Tr, _Alloc> _Myt;
        typedef _Hash<_Hmap_traits<_Kty, _Ty, _Tr, _Alloc, true>> _Mybase;
        typedef _Kty key_type;
        typedef _Ty mapped_type;
        typedef _Ty referent_type; // old name, magically gone
        typedef _Tr key_compare;
        typedef typename _Mybase::value_compare value_compare;
        typedef typename _Mybase::allocator_type allocator_type;
        typedef typename _Mybase::size_type size_type;
        typedef typename _Mybase::difference_type difference_type;
        typedef typename _Mybase::pointer pointer;
        typedef typename _Mybase::const_pointer const_pointer;
        typedef typename _Mybase::reference reference;
        typedef typename _Mybase::const_reference const_reference;
        typedef typename _Mybase::iterator iterator;
        typedef typename _Mybase::const_iterator const_iterator;
        typedef typename _Mybase::reverse_iterator reverse_iterator;
        typedef typename _Mybase::const_reverse_iterator const_reverse_iterator;
        typedef typename _Mybase::value_type value_type;

        hash_multimap()
            : _Mybase(key_compare(), allocator_type()) { // construct empty map from defaults
        }

        explicit hash_multimap(const key_compare &_Traits)
            : _Mybase(_Traits, allocator_type()) { // construct empty map from comparator
		}

	hash_multimap(const key_compare& _Traits,
		const allocator_type& _Al)
		: _Mybase(_Traits, _Al)
		{	// construct empty map from comparator and allocator
		}

	template<class _Iter>
		hash_multimap(_Iter _First, _Iter _Last)
		: _Mybase(key_compare(), allocator_type())
		{	// construct map from sequence, defaults
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

	template<class _Iter>
		hash_multimap(_Iter _First, _Iter _Last,
			const key_compare& _Traits)
		: _Mybase(_Traits, allocator_type())
		{	// construct map from sequence, comparator
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

	template<class _Iter>
		hash_multimap(_Iter _First, _Iter _Last,
			const key_compare& _Traits,
			const allocator_type& _Al)
		: _Mybase(_Traits, _Al)
		{	// construct map from sequence, comparator, and allocator
		_DEBUG_RANGE(_First, _Last);
		for (; _First != _Last; ++_First)
			this->insert(*_First);
		}

	iterator insert(const value_type& _Val)
		{	// insert a {key, mapped} value
		return (_Mybase::insert(_Val).first);
		}

	iterator insert(iterator _Where, const value_type& _Val)
		{	// insert a {key, mapped} value, with hint
		return (_Mybase::insert(_Where, _Val));
		}

	template<class _Iter>
		void insert(_Iter _First, _Iter _Last)
		{	// insert [_First, _Last), arbitrary iterators
 #if _HAS_ITERATOR_DEBUGGING
		_DEBUG_RANGE(_First, _Last);
		if (_Debug_get_cont(_First) == this)
			_DEBUG_ERROR("hash_multimap insertion overlaps range");
 #endif /* _HAS_ITERATOR_DEBUGGING */
		for (; _First != _Last; ++_First)
			insert(*_First);
    }
    };

template<class _Kty,
	class _Ty,
	class _Tr,
	class _Alloc> inline
	void swap( _STDEXT hash_multimap<_Kty, _Ty, _Tr, _Alloc>& _Left,
		_STDEXT hash_multimap<_Kty, _Ty, _Tr, _Alloc>& _Right)
	{	// swap _Left and _Right hash_multimaps
	_Left.swap(_Right);
	}

_STDEXT_END

_STD_BEGIN

	// _STDEXT hash_map implements a performant swap
template<class _Kty, class _Ty, class _Tr, class _Alloc>
	class _Move_operation_category<_STDEXT hash_map<_Kty, _Ty, _Tr, _Alloc> >
	{
	public:
		typedef _Swap_move_tag _Move_cat;
	};

	// _STDEXT hash_multimap implements a performant swap
template<class _Kty, class _Ty, class _Tr, class _Alloc>
	class _Move_operation_category<_STDEXT hash_multimap<_Kty, _Ty, _Tr, _Alloc> >
	{
	public:
		typedef _Swap_move_tag _Move_cat;
	};

_STD_END

#ifdef  _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _HASH_MAP_ */

/*
 * Copyright (c) 1992-2005 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 V4.05:0009 */
