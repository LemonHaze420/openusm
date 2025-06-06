// xutility internal header
#pragma once

#ifndef RC_INVOKED
#include <climits>
//#include <crtdbg.h>
#include <utility.hpp>

#include <cstdbool>
#include <cstring>
#include <stdexcept>

#ifdef _MSC_VER
#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)
#endif /* _MSC_VER */

#ifndef _CRT_SECURE_MEMMOVE
#define _CRT_SECURE_MEMMOVE(a1, a2, a3, a4)     \
    do {                                        \
    } while (0)

#endif

#ifndef _STDEXT_BEGIN
#define _STDEXT_BEGIN namespace stdext {
#endif

#ifndef _STDEXT_END
#define _STDEXT_END }
#endif

#ifndef _STD_BEGIN
#define _STD_BEGIN namespace _std {
#endif

#ifndef _STD_END
#define _STD_END }
#endif

#ifndef __CLRCALL_OR_CDECL
#define __CLRCALL_OR_CDECL
#endif

#ifndef __CLR_OR_THIS_CALL
#define __CLR_OR_THIS_CALL
#endif

#ifndef _STD
#define _STD _std::
#endif

#ifndef _STDEXT
#define _STDEXT stdext::
#endif

#ifndef __out_ecount_full
#define __out_ecount_full(a1)
#endif

#ifndef __in_opt
#define __in_opt
#endif

#ifndef __out_ecount_full_opt
#define __out_ecount_full_opt(a1)
#endif

#ifndef _SCL_SECURE_ALWAYS_VALIDATE
#define _SCL_SECURE_ALWAYS_VALIDATE(a1)
#endif

#ifndef _SCL_SECURE_ALWAYS_VALIDATE_RANGE
#define _SCL_SECURE_ALWAYS_VALIDATE_RANGE(a1)
#endif

#ifndef _THROW
#define _THROW(exception, str) throw exception(str)
#endif

#ifndef _SCL_CHECKED_ALGORITHM_WARN
#define _SCL_CHECKED_ALGORITHM_WARN
#endif

_STD_BEGIN

		// RANGE CHECKED ITERATOR TAGS
struct _Unchecked_iterator_tag
	{
	};
struct _Range_checked_iterator_tag
	{
	};

		// CHECKED ITERATOR BASE TAGS
struct _Unchanged_checked_iterator_base_type_tag
	{
	};

struct _Different_checked_iterator_base_type_tag
	{
	};

		// INNER TYPE TAG
struct _Undefined_inner_type_tag
	{
	};

		// MOVE OPERATION TAG
struct _Undefined_move_tag
	{
	};

struct _Swap_move_tag
	{
	};

		// Used to initialize _Mycont when _SECURE_SCL is off
#define _IGNORE_MYCONT ((const _Container_base *)-2)

		// Used to initialize _Myfirstiter and _Mynextiter when _HAS_ITERATOR_DEBUGGING is off
#define _IGNORE_MYITERLIST ((_Iterator_base *)-3)

 #if _HAS_ITERATOR_DEBUGGING
/* 
This internal implementation helper should not have been used externally, but is in our exports
and so cannot be removed right now. However, we deprecate it to avoid new uses 
*/
__declspec(noreturn) __declspec(deprecated) _MRTIMP2_NCEEPURE void __CLRCALL_PURE_OR_CDECL _Debug_message(const char *, const char *);
/* 
Use this one instead
*/

/* stl.net defined its own _Debug_message */
#if !defined(_CLI_MEMORY_)
_MRTIMP2_NCEEPURE void __CLRCALL_PURE_OR_CDECL _Debug_message(const wchar_t *, const wchar_t *, unsigned int line);
#else
void _Debug_message(const wchar_t *, const wchar_t *, unsigned int line);
#endif

		// MACROS
#define _STRIZE(x)	_VAL(x)
#define _VAL(x)	#x
#define _DEBUG_ERROR(mesg)	_DEBUG_ERROR2(mesg, __FILEW__, __LINE__)
#ifndef _DEBUG_ERROR2
#define _DEBUG_ERROR2(mesg, filew, linew) _Debug_message(L ## mesg, filew, linew)
#endif /* _DEBUG_ERROR2 */

		// CLASS _Container_base
class _Iterator_base;

class _CRTIMP2_PURE _Container_base
	{	// store head of iterator chain
public:
	friend class _Iterator_base;

	typedef _Undefined_move_tag _Move_category;

	__CLR_OR_THIS_CALL _Container_base()
		: _Myfirstiter(0)
		{	// construct childless container
		}

	__CLR_OR_THIS_CALL _Container_base(const _Container_base&)
		: _Myfirstiter(0)
		{	// copy a container
		}

	_Container_base& __CLR_OR_THIS_CALL operator=(const _Container_base&)
		{	// assign a container
		return (*this);
		}

	__CLR_OR_THIS_CALL ~_Container_base()
		{	// destroy the container
		_Orphan_all();
		}

	void __CLR_OR_THIS_CALL _Orphan_all() const;	// orphan all iterators
	void __CLR_OR_THIS_CALL _Swap_all(_Container_base&) const;	// swaps all iterators

	_Iterator_base *_Myfirstiter;
	};

		// CLASS _Iterator_base
class _Iterator_base
	{	// store links to container, next iterator
public:
	typedef _Unchecked_iterator_tag _Checked_iterator_category;
	typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
	typedef _Undefined_inner_type_tag _Inner_type;

	friend class _Container_base;

	__CLR_OR_THIS_CALL _Iterator_base()
		: _Mycont(0), _Mynextiter(0)
		{	// construct orphaned iterator
		}

	__CLR_OR_THIS_CALL _Iterator_base(const _Iterator_base& _Right)
		: _Mycont(0), _Mynextiter(0)
		{	// copy an iterator
		*this = _Right;
		}

	_Iterator_base& __CLR_OR_THIS_CALL operator=(const _Iterator_base& _Right)
		{	// assign an iterator
		if (_Mycont != _Right._Mycont)
			{	// change parentage
			_Lockit _Lock(_LOCK_DEBUG);
			_Orphan_me();
			_Adopt(_Right._Mycont);
			}
		return (*this);
		}

	__CLR_OR_THIS_CALL ~_Iterator_base()
		{	// destroy the iterator
		_Lockit _Lock(_LOCK_DEBUG);
		_Orphan_me();
		}

	void __CLR_OR_THIS_CALL _Adopt(const _Container_base *_Parent)
		{	// adopt this iterator by parent
		if (_Mycont != _Parent)
			{	// change parentage
			_Lockit _Lock(_LOCK_DEBUG);
			_Orphan_me();
			if (_Parent != 0 && _Parent->_Myfirstiter != _IGNORE_MYITERLIST)
				{	// switch to new parent
				_Mynextiter = _Parent->_Myfirstiter;
				((_Container_base *)_Parent)->_Myfirstiter = this;
				}
			_Mycont = _Parent;
			}
		}

	void __CLR_OR_THIS_CALL _Orphan_me()
		{	// cut ties with parent
		if (_Mycont != 0 && _Mycont->_Myfirstiter != _IGNORE_MYITERLIST)
			{	// adopted, remove self from list
			_Iterator_base **_Pnext =
				(_Iterator_base **)&_Mycont->_Myfirstiter;
			while (*_Pnext != 0 && *_Pnext != this)
				_Pnext = &(*_Pnext)->_Mynextiter;

			if (*_Pnext == 0)
				_DEBUG_ERROR("ITERATOR LIST CORRUPTED!");
			*_Pnext = _Mynextiter;
			_Mycont = 0;
			}
		}

	const _Container_base *_Mycont;
	_Iterator_base *_Mynextiter;
	};

typedef _Iterator_base _Iterator_base_secure;

inline void __CLR_OR_THIS_CALL _Container_base::_Orphan_all() const
	{	// orphan all iterators
	_Lockit _Lock(_LOCK_DEBUG);
	if (_Myfirstiter != _IGNORE_MYITERLIST)
		{
		for (_Iterator_base **_Pnext = (_Iterator_base **)&_Myfirstiter;
			*_Pnext != 0; *_Pnext = (*_Pnext)->_Mynextiter)
			(*_Pnext)->_Mycont = 0;
		*(_Iterator_base **)&_Myfirstiter = 0;
		}
	}

inline void __CLR_OR_THIS_CALL _Container_base::_Swap_all(_Container_base& _Right) const
	{	// swap all iterators
	_Lockit _Lock(_LOCK_DEBUG);
	_Iterator_base *_Pnext;
	_Iterator_base *_Temp = (_Iterator_base *)_Myfirstiter;
	*(_Iterator_base **)&_Myfirstiter = (_Iterator_base *)_Right._Myfirstiter;
	*(_Iterator_base **)&_Right._Myfirstiter = _Temp;

	if (_Myfirstiter != _IGNORE_MYITERLIST)
		{
		for (_Pnext = (_Iterator_base *)_Myfirstiter;
			_Pnext != 0; _Pnext = _Pnext->_Mynextiter)
			_Pnext->_Mycont = this;
		}
	if (_Right._Myfirstiter != _IGNORE_MYITERLIST)
		{
		for (_Pnext = (_Iterator_base *)_Right._Myfirstiter;
			_Pnext != 0; _Pnext = _Pnext->_Mynextiter)
			_Pnext->_Mycont = &_Right;
		}
	}

		// TEMPLATE FUNCTION _Debug_get_cont
template<class _Iter> inline
	const _Container_base *__CLRCALL_OR_CDECL _Debug_get_cont(const _Iter&)
	{	// return null for general case
	return (0);
	}

inline const _Container_base *__CLRCALL_OR_CDECL _Debug_get_cont(const _Iterator_base& _Where)
	{	// return _Mycont for based iterator
	return (_Where._Mycont);
	}

		// COMPARISON MACROS
 #define _DEBUG_LT(x, y) _Debug_lt(x, y, __FILEW__, __LINE__)

template<class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt(const _Ty1& _Left, const _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Left < _Right and operator< is strict weak ordering
	if (!(_Left < _Right))
		return (false);
	else if (_Right < _Left)
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt(const _Ty1& _Left, _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Left < _Right and operator< is strict weak ordering
	if (!(_Left < _Right))
		return (false);
	else if (_Right < _Left)
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt(_Ty1& _Left, const _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Left < _Right and operator< is strict weak ordering
	if (!(_Left < _Right))
		return (false);
	else if (_Right < _Left)
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt(_Ty1& _Left, _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Left < _Right and operator< is strict weak ordering
	if (!(_Left < _Right))
		return (false);
	else if (_Right < _Left)
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

		// COMPARISON MACRO WITH PRED
 #define _DEBUG_LT_PRED(pred, x, y)	_Debug_lt_pred(pred, x, y, __FILEW__, __LINE__)

template<class _Pr, class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt_pred(_Pr _Pred, const _Ty1& _Left, const _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Pred(_Left, _Right) and _Pred is strict weak ordering
	if (!_Pred(_Left, _Right))
		return (false);
	else if (_Pred(_Right, _Left))
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Pr, class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt_pred(_Pr _Pred, const _Ty1& _Left, _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Pred(_Left, _Right) and _Pred is strict weak ordering
	if (!_Pred(_Left, _Right))
		return (false);
	else if (_Pred(_Right, _Left))
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Pr, class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt_pred(_Pr _Pred, _Ty1& _Left, const _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Pred(_Left, _Right) and _Pred is strict weak ordering
	if (!_Pred(_Left, _Right))
		return (false);
	else if (_Pred(_Right, _Left))
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

template<class _Pr, class _Ty1, class _Ty2> inline
	bool __CLRCALL_OR_CDECL _Debug_lt_pred(_Pr _Pred, _Ty1& _Left, _Ty2& _Right,
		const wchar_t *_Where, unsigned int _Line)
	{	// test if _Pred(_Left, _Right) and _Pred is strict weak ordering
	if (!_Pred(_Left, _Right))
		return (false);
	else if (_Pred(_Right, _Left))
		_DEBUG_ERROR2("invalid operator<", _Where, _Line);
	return (true);
	}

 #else /* _HAS_ITERATOR_DEBUGGING */
		// MACROS
 #define _DEBUG_LT(x, y)	((x) < (y))
 #define _DEBUG_LT_PRED(pred, x, y)	pred(x, y)

  #ifdef _DEBUG
class _Iterator_base;
class _CRTIMP2_PURE _Container_base
	{	// base of all containers
    public:
	typedef _Undefined_move_tag _Move_category;

	__CLR_OR_THIS_CALL _Container_base()
		: _Myfirstiter(0)
		{	// construct childless container
		}

	__CLR_OR_THIS_CALL _Container_base(const _Container_base&)
		: _Myfirstiter(0)
		{	// copy a container
		}
	_Iterator_base *_Myfirstiter;
	};

class _Iterator_base
	{	// base of all iterators
	public:
	typedef _Unchecked_iterator_tag _Checked_iterator_category;
	typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
	typedef _Undefined_inner_type_tag _Inner_type;

	__CLR_OR_THIS_CALL _Iterator_base()
		: _Mycont(0), _Mynextiter(0)
		{	// construct orphaned iterator
		}

	__CLR_OR_THIS_CALL _Iterator_base(const _Iterator_base& _Right)
		: _Mycont(0), _Mynextiter(0)
		{	// copy an iterator
		*this = _Right;
		}

	_Iterator_base& __CLR_OR_THIS_CALL operator=(const _Iterator_base& _Right)
		{	// assign an iterator
		this->_Mycont = _Right._Mycont;
		this->_Mynextiter = _Right._Mynextiter;
		return (*this);
		}

	/* If we end up with an iterator constructed from msvcp80d.dll (which is compiled with _HAS_ITERATOR_DEBUGGING == 1),
	 * we must clean up the parent iterator list when the iterator is destroyed.
	 */
	__CLR_OR_THIS_CALL ~_Iterator_base()
		{	// destroy the iterator
		if (_Mycont != 0 && _Mycont != _IGNORE_MYCONT &&
			_Mycont->_Myfirstiter != 0 && _Mycont->_Myfirstiter != _IGNORE_MYITERLIST)
			_Clean_up_iter_debug();
		}

	private:
	void __CLR_OR_THIS_CALL _Clean_up_iter_debug()
		{	// cut ties with parent, if any
		_Lockit _Lock(_LOCK_DEBUG);
		_Iterator_base **_Pnext =
			(_Iterator_base **)&_Mycont->_Myfirstiter;
		while (*_Pnext != 0 && *_Pnext != this)
			_Pnext = &(*_Pnext)->_Mynextiter;

		if (*_Pnext != 0)
			*_Pnext = _Mynextiter;
		_Mycont = 0;
		}

	public:
	const _Container_base *_Mycont;
	_Iterator_base *_Mynextiter;

	};

typedef _Iterator_base _Iterator_base_secure;

  #else
struct _Container_base
	{	// base of all containers
	};

// In retail, _Iterator_base_secure has always the same size
// regardless of the value of _SECURE_SCL.

// String iterators (see xstring) derives from _Iterator_base_secure
// to ensure that iterators instantiated from the msvcpXX.dll code
// are compatible with the one defined in the include files.

// When _SECURE_SCL is off, the _Mycont field of _Iterator_base_secure
// is always set to _IGNORE_MYCONT.

   #if _SECURE_SCL
#define _ITERATOR_BASE _Iterator_base
   #else
#define _ITERATOR_BASE _Iterator_base_secure
   #endif

struct _ITERATOR_BASE
	{	// base of all iterators
	typedef _Unchecked_iterator_tag _Checked_iterator_category;
	typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
	typedef _Undefined_inner_type_tag _Inner_type;

	// SecureSCL needs the base container
    __CLR_OR_THIS_CALL _ITERATOR_BASE() : _Mycont(nullptr) {}

    const _Container_base *_Mycont;

	};

   #if _SECURE_SCL

typedef _Iterator_base _Iterator_base_secure;

   #else

struct _Iterator_base
	{	// base of all iterators
	typedef _Unchecked_iterator_tag _Checked_iterator_category;
	typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
	typedef _Undefined_inner_type_tag _Inner_type;
	};

   #endif

  #endif
 #endif /* _HAS_ITERATOR_DEBUGGING */

//	ITERATOR STUFF (from <iterator>)

		// ITERATOR TAGS
struct input_iterator_tag
	{	// identifying tag for input iterators
	};

struct output_iterator_tag
	{	// identifying tag for output iterators
	};

struct forward_iterator_tag
	: public input_iterator_tag
	{	// identifying tag for forward iterators
	};

struct bidirectional_iterator_tag
	: public forward_iterator_tag
	{	// identifying tag for bidirectional iterators
	};

struct random_access_iterator_tag
	: public bidirectional_iterator_tag
	{	// identifying tag for random-access iterators
	};

struct _Int_iterator_tag
	{	// identifying tag for integer types, not an iterator
	};

struct _Float_iterator_tag
	{	// identifying tag for floating point types, not an iterator
	};

		// POINTER ITERATOR TAGS
struct _Nonscalar_ptr_iterator_tag
	{	// pointer to unknown type
	};
struct _Scalar_ptr_iterator_tag
	{	// pointer to scalar type
	};

		// TEMPLATE CLASS iterator
template<class _Category,
	class _Ty,
	class _Diff = ptrdiff_t,
	class _Pointer = _Ty *,
	class _Reference = _Ty&>
		struct iterator
			: public _Iterator_base

	{	// base type for all iterator classes
	typedef _Category iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

template<class _Category,
	class _Ty,
	class _Diff = ptrdiff_t,
	class _Pointer = _Ty *,
	class _Reference = _Ty&,
	class _Base_class = _Iterator_base>
		struct _Iterator_with_base
			: public _Base_class

	{	// base type for all iterator classes
	typedef _Category iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

template<class _Ty,
	class _Diff,
	class _Pointer,
	class _Reference>
	struct _Bidit
		: public _Iterator_base

	{	// base type for container bidirectional iterator classes
	typedef bidirectional_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

template<class _Ty,
	class _Diff,
	class _Pointer,
	class _Reference>
	struct _Ranit
		: public _Iterator_base

	{	// base type for container random-access iterator classes
	typedef random_access_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

template<class _Ty,
	class _Diff,
	class _Pointer,
	class _Reference,
	class _Base_class>
	struct _Ranit_base
		: public _Base_class

	{	// base type for container random-access iterator classes
	typedef random_access_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

struct _Outit
	: public iterator<output_iterator_tag, void, void,
		void, void>
	{	// base for output iterators
	};

template <class _Base_class>
struct _Outit_with_base
	: public _Iterator_with_base<output_iterator_tag, void, void,
		void, void, _Base_class>
	{	// base for output iterators
	};

		// TEMPLATE CLASS iterator_traits
template<class _Iter>
	struct iterator_traits
	{	// get traits from iterator _Iter
	typedef typename _Iter::iterator_category iterator_category;
	typedef typename _Iter::value_type value_type;
	typedef typename _Iter::difference_type difference_type;
	typedef difference_type distance_type;	// retained
	typedef typename _Iter::pointer pointer;
	typedef typename _Iter::reference reference;
	};

template<class _Ty>
	struct iterator_traits<_Ty *>
	{	// get traits from pointer
	typedef random_access_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef ptrdiff_t difference_type;
	typedef ptrdiff_t distance_type;	// retained
	typedef _Ty *pointer;
	typedef _Ty& reference;
	};

template<class _Ty>
	struct iterator_traits<const _Ty *>
	{	// get traits from const pointer
	typedef random_access_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef ptrdiff_t difference_type;
	typedef ptrdiff_t distance_type;	// retained
	typedef const _Ty *pointer;
	typedef const _Ty& reference;
	};

template<> struct iterator_traits<_Bool>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef _Bool value_type;
	typedef _Bool difference_type;
	typedef _Bool distance_type;
	typedef _Bool * pointer;
	typedef _Bool& reference;
	};

template<> struct iterator_traits<char>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef char value_type;
	typedef char difference_type;
	typedef char distance_type;
	typedef char * pointer;
	typedef char& reference;
	};

template<> struct iterator_traits<signed char>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef signed char value_type;
	typedef signed char difference_type;
	typedef signed char distance_type;
	typedef signed char * pointer;
	typedef signed char& reference;
	};

template<> struct iterator_traits<unsigned char>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef unsigned char value_type;
	typedef unsigned char difference_type;
	typedef unsigned char distance_type;
	typedef unsigned char * pointer;
	typedef unsigned char& reference;
	};

 #ifdef _NATIVE_WCHAR_T_DEFINED
template<> struct iterator_traits<wchar_t>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef wchar_t value_type;
	typedef wchar_t difference_type;
	typedef wchar_t distance_type;
	typedef wchar_t * pointer;
	typedef wchar_t& reference;
	};
 #endif /* _NATIVE_WCHAR_T_DEFINED */

template<> struct iterator_traits<short>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef short value_type;
	typedef short difference_type;
	typedef short distance_type;
	typedef short * pointer;
	typedef short& reference;
	};

template<> struct iterator_traits<unsigned short>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef unsigned short value_type;
	typedef unsigned short difference_type;
	typedef unsigned short distance_type;
	typedef unsigned short * pointer;
	typedef unsigned short& reference;
	};

template<> struct iterator_traits<int>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef int value_type;
	typedef int difference_type;
	typedef int distance_type;	
	typedef int * pointer;
	typedef int& reference;
	};

template<> struct iterator_traits<unsigned int>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef unsigned int value_type;
	typedef unsigned int difference_type;
	typedef unsigned int distance_type;	
	typedef unsigned int * pointer;
	typedef unsigned int& reference;
	};

template<> struct iterator_traits<long>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef long value_type;
	typedef long difference_type;
	typedef long distance_type;	
	typedef long * pointer;
	typedef long& reference;
	};

template<> struct iterator_traits<unsigned long>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef unsigned long value_type;
	typedef unsigned long difference_type;
	typedef unsigned long distance_type;	
	typedef unsigned long * pointer;
	typedef unsigned long& reference;
	};

template<> struct iterator_traits<double>
	{
	typedef _Float_iterator_tag iterator_category;
	typedef double value_type;
	typedef double difference_type;
	typedef double distance_type;	
	typedef double * pointer;
	typedef double& reference;
	};

template<> struct iterator_traits<float>
	{
	typedef _Float_iterator_tag iterator_category;
	typedef float value_type;
	typedef float difference_type;
	typedef float distance_type;	
	typedef float * pointer;
	typedef float& reference;
	};

 #ifdef _LONGLONG
template<> struct iterator_traits<_LONGLONG>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef _LONGLONG value_type;
	typedef _LONGLONG difference_type;
	typedef _LONGLONG distance_type;	
	typedef _LONGLONG * pointer;
	typedef _LONGLONG& reference;
	};

template<> struct iterator_traits<_ULONGLONG>
	{	// get traits from integer type
	typedef _Int_iterator_tag iterator_category;
	typedef _ULONGLONG value_type;
	typedef _ULONGLONG difference_type;
	typedef _ULONGLONG distance_type;	
	typedef _ULONGLONG * pointer;
	typedef _ULONGLONG& reference;
	};
 #endif /* _LONGLONG */

    // TEMPLATE FUNCTION _Iter_cat
    template<class _Iter>
    inline typename iterator_traits<_Iter>::iterator_category __CLRCALL_OR_CDECL
    _Iter_cat(const _Iter &) { // return category from iterator argument
        typename iterator_traits<_Iter>::iterator_category _Cat;
        return (_Cat);
    }

    // TEMPLATE FUNCTION _Iter_random
    // Checks if both iterators belong to the random_access_iterator category;
    // if not, then it returns the forward_iterator tag.
    template<class _Cat1, class _Cat2>
    class _Iter_random_helper {
    public:
        typedef forward_iterator_tag _Iter_random_cat;
    };

    template<>
    class _Iter_random_helper<random_access_iterator_tag, random_access_iterator_tag> {
    public:
        typedef random_access_iterator_tag _Iter_random_cat;
    };

    template<class _Cat1, class _Cat2, class _Cat3>
    class _Iter_random_helper3 {
    public:
        typedef forward_iterator_tag _Iter_random_cat;
    };

    template<>
    class _Iter_random_helper3<random_access_iterator_tag,
                               random_access_iterator_tag,
                               random_access_iterator_tag> {
    public:
        typedef random_access_iterator_tag _Iter_random_cat;
    };

    template<class _Iter1, class _Iter2>
    inline typename _Iter_random_helper<
        typename iterator_traits<_Iter1>::iterator_category,
        typename iterator_traits<_Iter2>::iterator_category>::_Iter_random_cat __CLRCALL_OR_CDECL
    _Iter_random(const _Iter1 &, const _Iter2 &) { // return category from iterator argument
        typename _Iter_random_helper<
            typename iterator_traits<_Iter1>::iterator_category,
            typename iterator_traits<_Iter2>::iterator_category>::_Iter_random_cat _Cat;
        return (_Cat);
    }

    template<class _Iter1, class _Iter2, class _Iter3>
    inline typename _Iter_random_helper3<
        typename iterator_traits<_Iter1>::iterator_category,
        typename iterator_traits<_Iter2>::iterator_category,
        typename iterator_traits<_Iter3>::iterator_category>::_Iter_random_cat __CLRCALL_OR_CDECL
    _Iter_random(const _Iter1 &,
                 const _Iter2 &,
                 const _Iter3 &) { // return category from iterator argument
        typename _Iter_random_helper3<
            typename iterator_traits<_Iter1>::iterator_category,
            typename iterator_traits<_Iter2>::iterator_category,
            typename iterator_traits<_Iter3>::iterator_category>::_Iter_random_cat _Cat;
        return (_Cat);
    }

    // HELPER CLASS _If
    // Classic metaprogramming _If statement
    template<bool _Cond, class _Ty1, class _Ty2>
    class _If {
    public:
        typedef _Ty2 _Result;
    };

    template<class _Ty1, class _Ty2>
    class _If<true, _Ty1, _Ty2> {
    public:
        typedef _Ty1 _Result;
    };

    // HELPER CLASS _Secure_validation_helper
    template<bool _Secure_validation>
    class _Secure_validation_helper {
    public:
        typedef _Unchecked_iterator_tag _Checked_iterator_category;
    };

    template<>
    class _Secure_validation_helper<true> {
    public:
        typedef _Range_checked_iterator_tag _Checked_iterator_category;
    };

    // TEMPLATE FUNCTION _Checked_cat
    template<class _Iter, bool _Inherits_from_iterator_base>
    class _Checked_iterator_category_helper {
    public:
        typedef _Unchecked_iterator_tag _Checked_cat;
    };

    template<class _Iter>
    class _Checked_iterator_category_helper<_Iter, true> {
    public:
        typedef typename _Iter::_Checked_iterator_category _Checked_cat;
    };

    template<class _Iter>
    class _Checked_iterator_category {
    public:
        typedef
            typename _Checked_iterator_category_helper<_Iter, __is_base_of(_Iterator_base, _Iter)>::
                _Checked_cat _Checked_cat;
    };

    template<class _Iter>
    inline typename _Checked_iterator_category<_Iter>::_Checked_cat _Checked_cat(const _Iter &) {
        typename _Checked_iterator_category<_Iter>::_Checked_cat _Cat;
        return (_Cat);
    }

    // TEMPLATE FUNCTION _Checked_base
    template<class _Iter, bool _Inherits_from_iterator_base>
    class _Checked_iterator_base_helper2 {
    public:
        typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
    };

    template<class _Iter>
    class _Checked_iterator_base_helper2<_Iter, true> {
    public:
        typedef typename _Iter::_Checked_iterator_base_type _Checked_iterator_base_type;
    };

    template<class _Iter, class _Base_type>
    class _Checked_iterator_base_helper1 {
    public:
        typedef _Different_checked_iterator_base_type_tag _Base_type_tag;
        typedef _Base_type _Checked_iterator_base_type;
    };

    template<class _Iter>
    class _Checked_iterator_base_helper1<_Iter, _Unchanged_checked_iterator_base_type_tag> {
    public:
        typedef _Unchanged_checked_iterator_base_type_tag _Base_type_tag;
        typedef _Iter _Checked_iterator_base_type;
    };

    template<class _Iter>
    class _Checked_iterator_base_helper {
    public:
        typedef _Checked_iterator_base_helper2<_Iter, __is_base_of(_Iterator_base, _Iter)>
            _Base_helper2;
        typedef _Checked_iterator_base_helper1<_Iter,
                                               typename _Base_helper2::_Checked_iterator_base_type>
            _Base_helper1;

        typedef typename _Base_helper1::_Base_type_tag _Checked_iterator_base_type_tag;
        typedef typename _Base_helper1::_Checked_iterator_base_type _Checked_iterator_base_type;
    };

    template<class _Iter, class _Base_tag>
    inline
        typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type __CLRCALL_OR_CDECL
        _Checked_base(const _Iter &_It, _Base_tag) {
        return _It._Checked_iterator_base();
    }

    template<class _Iter>
    inline
        typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type __CLRCALL_OR_CDECL
        _Checked_base(const _Iter &_It, _Unchanged_checked_iterator_base_type_tag) {
        return _It;
    }

    template<class _Iter, class _Base_tag>
    inline
        typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type __CLRCALL_OR_CDECL
        _Checked_base(_Iter &_It, _Base_tag) {
        return _It._Checked_iterator_base();
    }

    template<class _Iter>
    inline
        typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type __CLRCALL_OR_CDECL
        _Checked_base(_Iter &_It, _Unchanged_checked_iterator_base_type_tag) {
        return _It;
    }

    template<class _Iter>
	inline
	typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type
		__CLRCALL_OR_CDECL _Checked_base(const _Iter &_It)
	{
		typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type_tag _Base_tag;
		return _Checked_base(_It, _Base_tag);
	}

	template<class _Iter>
	inline
	typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type
		__CLRCALL_OR_CDECL _Checked_base(_Iter &_It)
	{
		typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type_tag _Base_tag;
		return _Checked_base(_It, _Base_tag);
	}

		// TEMPLATE FUNCTION _Checked_assign_from_base

	template<class _DstIter, class _BaseIter>
	inline
	void __CLRCALL_OR_CDECL _Checked_assign_from_base(_DstIter &_Dest, const _BaseIter &_Src)
	{
		_Dest._Checked_iterator_assign_from_base(_Src);
	}

	template<class _Iter>
	inline
	void __CLRCALL_OR_CDECL _Checked_assign_from_base(_Iter &_Dest, const _Iter &_Src)
	{
		_Dest = _Src;
	}

		// TEMPLATE FUNCTION _Move_cat
 #if !defined(_DEFAULT_MOVE_OPERATION_CATEGORY)
 #define _DEFAULT_MOVE_OPERATION_CATEGORY _Undefined_move_tag
 #endif

	// default _Move_operation_category; std containers like vector, list overloads this trait class
	template <class _Value>
	class _Move_operation_category
	{
	public:
		typedef _DEFAULT_MOVE_OPERATION_CATEGORY _Move_cat;
	};

	template<class _Iter>
	inline
		typename _Move_operation_category<typename iterator_traits<_Iter>::value_type>::_Move_cat _Move_cat(const _Iter&)
	{
		typename _Move_operation_category<typename iterator_traits<_Iter>::value_type>::_Move_cat _Cat;
		return (_Cat);
	}

		// TEMPLATE FUNCTION _Ptr_cat
template<class _T1, class _T2, class _Checked_Cat1, class _Checked_Cat2>
struct _Ptr_cat_with_checked_cat_helper
	{
	typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};

template<class _T1,	class _T2>
struct _Ptr_cat_helper
	{
	typedef typename _Ptr_cat_with_checked_cat_helper<_T1, _T2, 
		typename _Checked_iterator_category<_T1>::_Checked_cat,
		typename _Checked_iterator_category<_T2>::_Checked_cat>::_Ptr_cat _Ptr_cat;
	};

template<class _T1,	class _T2, class _Checked_Cat1>
struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Checked_Cat1, _Range_checked_iterator_tag>
	{
	typedef typename _Ptr_cat_helper<_T1, typename _T2::_Inner_type>::_Ptr_cat _Ptr_cat;
	};

template<class _T1,	class _T2, class _Checked_Cat2>
struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Range_checked_iterator_tag, _Checked_Cat2>
	{
	typedef typename _Ptr_cat_helper<typename _T1::_Inner_type, _T2>::_Ptr_cat _Ptr_cat;
	};

template<class _T1,	class _T2>
struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Range_checked_iterator_tag, _Range_checked_iterator_tag>
	{
	typedef typename _Ptr_cat_helper<typename _T1::_Inner_type, typename _T2::_Inner_type>::_Ptr_cat _Ptr_cat;
	};

template<class _T1>
struct _Ptr_cat_helper<_T1, _Undefined_inner_type_tag>
	{
	typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};

template<class _T2>
struct _Ptr_cat_helper<_Undefined_inner_type_tag, _T2>
	{
	typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<_Undefined_inner_type_tag, _Undefined_inner_type_tag>
	{
	typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};

		// INTEGER FUNCTION _Ptr_cat
template<>
struct _Ptr_cat_helper<_Bool *, _Bool *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const _Bool *, _Bool *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<char *, char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const char *, char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<signed char *, signed char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const signed char *, signed char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<unsigned char *, unsigned char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const unsigned char *, unsigned char *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

 #ifdef _NATIVE_WCHAR_T_DEFINED

template<>
struct _Ptr_cat_helper<wchar_t *, wchar_t *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const wchar_t *, wchar_t *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

 #endif /* _NATIVE_WCHAR_T_DEFINED */
template<>
struct _Ptr_cat_helper<short *, short *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const short *, short *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<unsigned short *, unsigned short *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const unsigned short *, unsigned short *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<int *, int *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const int *, int *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<unsigned int *, unsigned int *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const unsigned int *, unsigned int *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<long *, long *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const long *, long *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<unsigned long *, unsigned long *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const unsigned long *, unsigned long *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<float *, float *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const float *, float *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<double *, double *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const double *, double *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<long double *, long double *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const long double *, long double *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

 #ifdef _LONGLONG

template<>
struct _Ptr_cat_helper<_LONGLONG *, _LONGLONG *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const _LONGLONG *, _LONGLONG *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<_ULONGLONG *, _ULONGLONG *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<>
struct _Ptr_cat_helper<const _ULONGLONG *, _ULONGLONG *>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

 #endif /* _LONGLONG */

template<class _Ty>
struct _Ptr_cat_helper<_Ty **, _Ty **>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<class _Ty>
struct _Ptr_cat_helper<_Ty **, const _Ty **>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<class _Ty>
struct _Ptr_cat_helper<_Ty *const *, _Ty **>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

template<class _Ty>
struct _Ptr_cat_helper<_Ty *const *, const _Ty **>
	{	// return pointer category from pointer to pointer arguments
	typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};

/* use _Ptr_cat_helper to determine the type of the pointer category */
template<class _T1, class _T2> inline 
typename _Ptr_cat_helper<_T1, _T2>::_Ptr_cat __CLRCALL_OR_CDECL _Ptr_cat(_T1&, _T2&)
	{
	typename _Ptr_cat_helper<_T1, _T2>::_Ptr_cat _Cat;
	return (_Cat);
	}

#ifndef _DEBUG_ORDER_IMPL
 #define _DEBUG_ORDER_IMPL _Debug_order
#endif

#ifndef _DEBUG_ORDER_SINGLE_IMPL
 #define _DEBUG_ORDER_SINGLE_IMPL _Debug_order_single
#endif

 #if _HAS_ITERATOR_DEBUGGING
		// ITERATOR DEBUGGING MACROS
 #define _DEBUG_ORDER(first, last)	\
	_DEBUG_ORDER_IMPL(first, last, __FILEW__, __LINE__)
 #define _DEBUG_ORDER_PRED(first, last, pred)	\
	_DEBUG_ORDER_IMPL(first, last, pred, __FILEW__, __LINE__)
 #define _DEBUG_ORDER_SINGLE(first, last, is_first_iteration)	\
	_DEBUG_ORDER_SINGLE_IMPL(first, last, is_first_iteration, __FILEW__, __LINE__)
 #define _DEBUG_ORDER_SINGLE_PRED(first, last, pred, is_first_iteration)	\
	_DEBUG_ORDER_SINGLE_IMPL(first, last, pred, is_first_iteration, __FILEW__, __LINE__)
 #define _DEBUG_POINTER(first)	\
	_Debug_pointer(first, __FILEW__,__LINE__)
 #define _DEBUG_POINTER2(first, filew, line)	\
	_Debug_pointer(first, filew, line)
 #define _DEBUG_RANGE(first, last)	\
	_Debug_range(first, last, __FILEW__, __LINE__)
 #define _DEBUG_RANGE2(first, last, filew, line)	\
	_Debug_range(first, last, filew, line)

		// TEMPLATE FUNCTION _Debug_pointer
template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_pointer(_InIt&, const wchar_t *, unsigned int)
	{	// test pointer for non-singularity, arbitrary type
	}

 #if 1310 <= _MSC_VER
template<class _Ty> inline
	void __CLRCALL_OR_CDECL _Debug_pointer(const _Ty *_First, const wchar_t *_File, unsigned int _Line)
	{	// test iterator for non-singularity, const pointers
	if (_First == 0)
		_DEBUG_ERROR2("invalid null pointer", _File, _Line);
	}

template<class _Ty> inline
	void __CLRCALL_OR_CDECL _Debug_pointer(_Ty *_First, const wchar_t *_File, unsigned int _Line)
	{	// test iterator for non-singularity, pointers
	if (_First == 0)
		_DEBUG_ERROR2("invalid null pointer", _File, _Line);
	}
 #endif /* 1310 <= _MSC_VER */

		// TEMPLATE FUNCTION _Debug_range
template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_range2(_InIt, _InIt, const wchar_t *, unsigned int ,
		input_iterator_tag)
	{	// test iterator pair for valid range, arbitrary iterators
	}

template<class _RanIt> inline
	void __CLRCALL_OR_CDECL _Debug_range2(_RanIt _First, _RanIt _Last, const wchar_t *_File, unsigned int _Line,
		random_access_iterator_tag)
	{	// test iterator pair for valid range, random-access iterators
	if (_First != _Last)
		{	// check for non-null pointers, valid range
		_DEBUG_POINTER2(_First, _File, _Line);
		_DEBUG_POINTER2(_Last, _File, _Line);
		if (_Last < _First)
			_DEBUG_ERROR2("invalid iterator range", _File, _Line);
		}
	}

template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_range(_InIt _First, _InIt _Last, const wchar_t *_File, unsigned int _Line)
	{	// test iterator pair for valid range
	_Debug_range2(_First, _Last, _File, _Line, _Iter_cat(_First));
	}

		// TEMPLATE FUNCTION _Debug_order
template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_order2(_InIt _First, _InIt _Last,
		const wchar_t *_File, unsigned int _Line, input_iterator_tag)
	{	// test if range is ordered by operator<, input iterators
	}

template<class _FwdIt> inline
	void __CLRCALL_OR_CDECL _Debug_order2(_FwdIt _First, _FwdIt _Last,
		const wchar_t *_File, unsigned int _Line, forward_iterator_tag)
	{	// test if range is ordered by operator<, forward iterators
	if (_First != _Last)
		for (_FwdIt _Next = _First; ++_Next != _Last; ++_First)
			if (_DEBUG_LT(*_Next, *_First))
				_DEBUG_ERROR2("sequence not ordered", _File, _Line);
	}

template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_order(_InIt _First, _InIt _Last,
		const wchar_t *_File, unsigned int _Line)
	{	// test is range is ordered by operator<
	_DEBUG_RANGE2(_First, _Last, _File, _Line);
	_Debug_order2(_First, _Last, _File, _Line, _Iter_cat(_First));
	}

		// TEMPLATE FUNCTION _Debug_order WITH PRED
template<class _InIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order2(_InIt _First, _InIt _Last, _Pr _Pred,
		const wchar_t *_File, unsigned int _Line, input_iterator_tag)
	{	// test if range is ordered by predicate, input iterators
	}

template<class _FwdIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order2(_FwdIt _First, _FwdIt _Last, _Pr _Pred,
		const wchar_t *_File, unsigned int _Line, forward_iterator_tag)
	{	// test if range is ordered by predicate, forward iterators
	if (_First != _Last)
		for (_FwdIt _Next = _First; ++_Next != _Last; ++_First)
			if (_DEBUG_LT_PRED(_Pred, *_Next, *_First))
				_DEBUG_ERROR2("sequence not ordered", _File, _Line);
	}

template<class _InIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order(_InIt _First, _InIt _Last, _Pr _Pred,
		const wchar_t *_File, unsigned int _Line)
	{	// test if range is ordered by predicate
	_DEBUG_RANGE2(_First, _Last, _File, _Line);
	_DEBUG_POINTER2(_Pred, _File, _Line);
	_Debug_order2(_First, _Last, _Pred, _File, _Line, _Iter_cat(_First));
	}

		// TEMPLATE FUNCTION _Debug_order_single
template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_order_single2(_InIt _First, _InIt _Last, bool _IsFirstIteration,
		const wchar_t *_File, unsigned int _Line, input_iterator_tag)
	{	// test if _First and ++_First are ordered by operator<, input iterators
	}

template<class _FwdIt> inline
	void __CLRCALL_OR_CDECL _Debug_order_single2(_FwdIt _First, _FwdIt _Last, bool /* _IsFirstIteration */,
		const wchar_t *_File, unsigned int _Line, forward_iterator_tag)
	{	// test if _First and ++_First are ordered by operator<, forward iterators
	if (_First != _Last)
		{
		_FwdIt _Next = _First;
		if (++_Next != _Last)
			if (_DEBUG_LT(*_Next, *_First))
				_DEBUG_ERROR2("sequence not ordered", _File, _Line);
		}
	}

template<class _InIt> inline
	void __CLRCALL_OR_CDECL _Debug_order_single(_InIt _First, _InIt _Last, bool _IsFirstIteration,
		const wchar_t *_File, unsigned int _Line)
	{	// test if _First and ++_First are ordered by operator<
	_Debug_order_single2(_First, _Last, _IsFirstIteration, _File, _Line, _Iter_cat(_First));
	}

		// TEMPLATE FUNCTION _Debug_order_single WITH PRED
template<class _InIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order_single2(_InIt _First, _InIt _Last, _Pr _Pred, bool _IsFirstIteration,
		const wchar_t *_File, unsigned int _Line, input_iterator_tag)
	{	// test if _First and ++_First ordered by predicate, input iterators
	}

template<class _FwdIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order_single2(_FwdIt _First, _FwdIt _Last, _Pr _Pred, bool _IsFirstIteration,
		const wchar_t *_File, unsigned int _Line, forward_iterator_tag)
	{	// test if _First and ++_First ordered by predicate, forward iterators
	if (_First != _Last)
		{
		_FwdIt _Next = _First;
		if (++_Next != _Last)
			if (_DEBUG_LT_PRED(_Pred, *_Next, *_First))
				_DEBUG_ERROR2("sequence not ordered", _File, _Line);
		}
	}

template<class _InIt,
	class _Pr> inline
	void __CLRCALL_OR_CDECL _Debug_order_single(_InIt _First, _InIt _Last, _Pr _Pred, bool _IsFirstIteration,
		const wchar_t *_File, unsigned int _Line)
	{	// test if _First and ++_First is ordered by predicate
	_Debug_order_single2(_First, _Last, _Pred, _IsFirstIteration, _File, _Line, _Iter_cat(_First));
	}

 #else /* _HAS_ITERATOR_DEBUGGING */
		// ITERATOR DEBUGGING MACROS
 #define _DEBUG_ORDER(first, last)
 #define _DEBUG_ORDER_PRED(first, last, pred)
 #define _DEBUG_ORDER_SINGLE(first, last, is_first_iteration)
 #define _DEBUG_ORDER_SINGLE_PRED(first, last, pred, is_first_iteration)
 #define _DEBUG_POINTER(first)
 #define _DEBUG_POINTER2(first, file, line)
 #define _DEBUG_RANGE(first, last)
 #define _DEBUG_RANGE2(first, last, file, line)
 #endif /* _HAS_ITERATOR_DEBUGGING */

 #if _SECURE_SCL

  // _CHECKED_CAT retrieves whether an iterator is checked.
  // It returns one of the range checked iterator tags:
  //    _Unchecked_iterator_tag: the iterator is unchecked;
  //    _Range_checked_iterator_tag: the iterator is checked.
  // To change the checked iterator category in your own iterators,
  // you will need to overload the _Checked_iterator_category typedef.
  #define _CHECKED_CAT(_Iter) _STD _Checked_cat(_Iter)
  
  // _CHECKED_BASE retrieves the base of a checked iterator.
  // The base type of a checked iterator is supposed to be more
  // performant, but usually do not perform validation. See the standard
  // algorithm implementations for examples on how to use _CHECKED_BASE.
  // To change the checked iterator base type in your own iterators,
  // you will need to overload the _Checked_iterator_base_type typedef.
  #define _CHECKED_BASE(_Iter) _STD _Checked_base(_Iter)
  #define _CHECKED_BASE_TYPE(_Iter_type) \
    typename _STD _Checked_iterator_base_helper<_Iter_type>::_Checked_iterator_base_type

  // _ASSIGN_FROM_BASE assign the value of _Src iterator to _Dest.
  // _Src is assumed to be the base of the checked iterator _Dest.
  // See the standard algorithm implementations for examples on how 
  // to use _ASSIGN_FROM_BASE.
  // To change the checked iterator base type in your own iterators,
  // you will need to overload the _Checked_iterator_assign_from_base()
  // method.
  #define _ASSIGN_FROM_BASE(_Dest, _Src) _STD _Checked_assign_from_base(_Dest, _Src)

 #else

  #define _CHECKED_CAT(_Iter) _STD _Range_checked_iterator_tag()
  #define _CHECKED_BASE(_Iter) _Iter
  #define _CHECKED_BASE_TYPE(_Iter_type) _Iter_type
  #define _ASSIGN_FROM_BASE(_Dest, _Src) _Dest = (_Src)

 #endif

		// TEMPLATE FUNCTION _Val_type

template<class _Iter> inline
	typename iterator_traits<_Iter>::value_type *__CLRCALL_OR_CDECL _Val_type(_Iter)
	{	// return value type from arbitrary argument
	return (0);
	}

		// TEMPLATE FUNCTION advance
template<class _InIt,
	class _Diff> inline
	void __CLRCALL_OR_CDECL advance(_InIt& _Where, _Diff _Off)
	{	// increment iterator by offset, arbitrary iterators
	_Advance(_Where, _Off, _Iter_cat(_Where));
	}

template<class _InIt,
	class _Diff> inline
	void __CLRCALL_OR_CDECL _Advance(_InIt& _Where, _Diff _Off, input_iterator_tag)
	{	// increment iterator by offset, input iterators

 #if _HAS_ITERATOR_DEBUGGING
//	if (_Off < 0)
//		_DEBUG_ERROR("negative offset in advance");
 #endif /* _HAS_ITERATOR_DEBUGGING */

	for (; 0 < _Off; --_Off)
		++_Where;
	}

template<class _FI,
	class _Diff> inline
	void __CLRCALL_OR_CDECL _Advance(_FI& _Where, _Diff _Off, forward_iterator_tag)
	{	// increment iterator by offset, forward iterators

 #if _HAS_ITERATOR_DEBUGGING
//	if (_Off < 0)
//		_DEBUG_ERROR("negative offset in advance");
 #endif /* _HAS_ITERATOR_DEBUGGING */

	for (; 0 < _Off; --_Off)
		++_Where;
	}

template<class _BI,
	class _Diff> inline
	void __CLRCALL_OR_CDECL _Advance(_BI& _Where, _Diff _Off, bidirectional_iterator_tag)
	{	// increment iterator by offset, bidirectional iterators
	for (; 0 < _Off; --_Off)
		++_Where;
	for (; _Off < 0; ++_Off)
		--_Where;
	}

template<class _RI,
	class _Diff> inline
	void __CLRCALL_OR_CDECL _Advance(_RI& _Where, _Diff _Off, random_access_iterator_tag)
	{	// increment iterator by offset, random-access iterators
	_Where += _Off;
	}

		// TEMPLATE FUNCTION _Dist_type

template<class _Iter> inline
	typename iterator_traits<_Iter>::difference_type
		* __CLRCALL_OR_CDECL _Dist_type(_Iter)
	{	// return distance type from arbitrary argument
	return (0);
	}

		// TEMPLATE FUNCTIONS distance and _Distance
template<class _InIt,
	class _Diff> inline
		void __CLRCALL_OR_CDECL _Distance2(_InIt _First, _InIt _Last, _Diff& _Off,
			input_iterator_tag)
	{	// add to _Off distance between input iterators
	for (; _First != _Last; ++_First)
		++_Off;
	}

template<class _FwdIt,
	class _Diff> inline
		void __CLRCALL_OR_CDECL _Distance2(_FwdIt _First, _FwdIt _Last, _Diff& _Off,
			forward_iterator_tag)
	{	// add to _Off distance between forward iterators (redundant)
	for (; _First != _Last; ++_First)
		++_Off;
	}

template<class _BidIt,
	class _Diff> inline
		void __CLRCALL_OR_CDECL _Distance2(_BidIt _First, _BidIt _Last, _Diff& _Off,
			bidirectional_iterator_tag)
	{	// add to _Off distance between bidirectional iterators (redundant)
	for (; _First != _Last; ++_First)
		++_Off;
	}

template<class _RanIt,
	class _Diff> inline
		void __CLRCALL_OR_CDECL _Distance2(_RanIt _First, _RanIt _Last, _Diff& _Off,
			random_access_iterator_tag)
	{	// add to _Off distance between random-access iterators

 #if _HAS_ITERATOR_DEBUGGING
	if (_First != _Last)
		{	// check for null pointers
		_DEBUG_POINTER(_First);
		_DEBUG_POINTER(_Last);
		}
 #endif /* _HAS_ITERATOR_DEBUGGING */

	_Off += _Last - _First;
	}

template<class _InIt> inline
	typename iterator_traits<_InIt>::difference_type
		__CLRCALL_OR_CDECL distance(_InIt _First, _InIt _Last)
	{	// return distance between iterators
	typename iterator_traits<_InIt>::difference_type _Off = 0;
	_Distance2(_First, _Last, _Off, _Iter_cat(_First));
	return (_Off);
}

template<class _InIt, class _Diff>
inline void __CLRCALL_OR_CDECL _Distance(_InIt _First,
                                         _InIt _Last,
                                         _Diff &_Off) { // add to _Off distance between iterators
    auto iterator_cat = _Iter_cat(_First);

    _Distance2(_First, _Last, _Off, iterator_cat);
}

// TEMPLATE CLASS reverse_iterator
template<class _RanIt>
	class reverse_iterator
		: public _Iterator_base_secure
	{	// wrap iterator to run it backwards
public:
	typedef reverse_iterator<_RanIt> _Myt;
	typedef typename iterator_traits<_RanIt>::iterator_category iterator_category;
	typedef typename iterator_traits<_RanIt>::value_type value_type;
	typedef typename iterator_traits<_RanIt>::difference_type difference_type;
	typedef typename iterator_traits<_RanIt>::difference_type distance_type;	// retained
	typedef typename iterator_traits<_RanIt>::pointer pointer;
	typedef typename iterator_traits<_RanIt>::reference reference;
	typedef _RanIt iterator_type;

	__CLR_OR_THIS_CALL reverse_iterator()
		{	// construct with default wrapped iterator
		}

	explicit __CLR_OR_THIS_CALL reverse_iterator(_RanIt _Right)
		: current(_Right)
		{	// construct wrapped iterator from _Right
		}

	template<class _Other>
		__CLR_OR_THIS_CALL reverse_iterator(const reverse_iterator<_Other>& _Right)
		: current(_Right.base())
		{	// initialize with compatible base
		}

	_RanIt __CLR_OR_THIS_CALL base() const
		{	// return wrapped iterator
		return (current);
		}

#if _SECURE_SCL
	typedef typename _STD _Checked_iterator_category<_RanIt>::_Checked_cat _Checked_iterator_category;
	typedef reverse_iterator<typename _Checked_iterator_base_helper<_RanIt>::_Checked_iterator_base_type> _Checked_iterator_base_type;
 
	_Checked_iterator_base_type _Checked_iterator_base() const
	{
		typename _Checked_iterator_base_type _Base(_CHECKED_BASE(current));
		return _Base;
	}

	void _Checked_iterator_assign_from_base(_Checked_iterator_base_type _Base)
	{
		_ASSIGN_FROM_BASE(current, _Base.base());
	}
#endif

	reference __CLR_OR_THIS_CALL operator*() const
		{	// return designated value
		_RanIt _Tmp = current;
		return (*--_Tmp);
		}

	pointer __CLR_OR_THIS_CALL operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_Myt& __CLR_OR_THIS_CALL operator++()
		{	// preincrement
		--current;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		--current;
		return (_Tmp);
		}

	_Myt& __CLR_OR_THIS_CALL operator--()
		{	// predecrement
		++current;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		++current;
		return (_Tmp);
		}

	template<class _Other>
		bool __CLR_OR_THIS_CALL _Equal(const reverse_iterator<_Other>& _Right) const
		{	// test for iterator equality
		return (current == _Right.base());
		}

// N.B. functions valid for random-access iterators only beyond this point

	_Myt& __CLR_OR_THIS_CALL operator+=(difference_type _Off)
		{	// increment by integer
		current -= _Off;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator+(difference_type _Off) const
		{	// return this + integer
		return (_Myt(current - _Off));
		}

	_Myt& __CLR_OR_THIS_CALL operator-=(difference_type _Off)
		{	// decrement by integer
		current += _Off;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator-(difference_type _Off) const
		{	// return this - integer
		return (_Myt(current + _Off));
		}

	reference __CLR_OR_THIS_CALL operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}

	template<class _Other>
		bool __CLR_OR_THIS_CALL _Less(const reverse_iterator<_Other>& _Right) const
		{	// test if this < _Right
		return (_Right.base() < current);
		}

	template<class _Other>
		difference_type __CLR_OR_THIS_CALL _Minus(const reverse_iterator<_Other>& _Right) const
		{	// return difference of iterators
		return (_Right.base() - current);
		}

protected:
	_RanIt current;	// the wrapped iterator
	};

		// reverse_iterator TEMPLATE OPERATORS
template<class _RanIt,
	class _Diff> inline
	reverse_iterator<_RanIt> __CLRCALL_OR_CDECL operator+(_Diff _Off,
		const reverse_iterator<_RanIt>& _Right)
	{	// return reverse_iterator + integer
	return (_Right + _Off);
	}

template<class _RanIt1,
	class _RanIt2> inline
	typename reverse_iterator<_RanIt1>::difference_type
		__CLRCALL_OR_CDECL operator-(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// return difference of reverse_iterators
	return (_Left._Minus(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator==(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator equality
	return (_Left._Equal(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator!=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator inequality
	return (!(_Left == _Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator<(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator < reverse_iterator
	return (_Left._Less(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator>(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator > reverse_iterator
	return (_Right < _Left);
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator<=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator <= reverse_iterator
	return (!(_Right < _Left));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool __CLRCALL_OR_CDECL operator>=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator >= reverse_iterator
	return (!(_Left < _Right));
	}

		// TEMPLATE CLASS reverse_bidirectional_iterator (retained)
template<class _BidIt,
	class _Ty,
	class _Reference = _Ty&,
	class _Pointer = _Ty *,
	class _Diff = ptrdiff_t>
	class reverse_bidirectional_iterator
		: public iterator<bidirectional_iterator_tag, _Ty, _Diff,
			_Pointer, _Reference>
	{	// wrap bidirectional iterator to run it backwards
public:
	typedef reverse_bidirectional_iterator<_BidIt, _Ty, _Reference,
		_Pointer, _Diff> _Myt;
	typedef _BidIt iterator_type;

	__CLR_OR_THIS_CALL reverse_bidirectional_iterator()
		{	// construct with default wrapped iterator
		}

	explicit __CLR_OR_THIS_CALL reverse_bidirectional_iterator(_BidIt _Right)
		: current(_Right)
		{	// construct wrapped iterator from _Right
		}

	_BidIt __CLR_OR_THIS_CALL base() const
		{	// return wrapped iterator
		return (current);
		}

	_Reference __CLR_OR_THIS_CALL operator*() const
		{	// return designated value
		_BidIt _Tmp = current;
		return (*--_Tmp);
		}

	_Pointer __CLR_OR_THIS_CALL operator->() const
		{       // return pointer to class object
		_Reference _Tmp = **this;
		return (&_Tmp);
		}

	_Myt& __CLR_OR_THIS_CALL operator++()
		{	// preincrement
		--current;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		--current;
		return (_Tmp);
		}

	_Myt& __CLR_OR_THIS_CALL operator--()
		{	// predecrement
		++current;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		++current;
		return (_Tmp);
		}

	bool __CLR_OR_THIS_CALL operator==(const _Myt& _Right) const
		{	// test for iterator equality
		return (current == _Right.current);
		}

	bool __CLR_OR_THIS_CALL operator!=(const _Myt& _Right) const
		{	// test for iterator inequality
		return (!(*this == _Right));
		}

protected:
	_BidIt current;	// the wrapped iterator
	};

		// TEMPLATE CLASS _Revbidit
template<class _BidIt,
	class _BidIt2 = _BidIt>
	class _Revbidit
		: public iterator<
			typename iterator_traits<_BidIt>::iterator_category,
			typename iterator_traits<_BidIt>::value_type,
			typename iterator_traits<_BidIt>::difference_type,
			typename iterator_traits<_BidIt>::pointer,
			typename iterator_traits<_BidIt>::reference>
	{	// wrap bidirectional iterator to run it backwards
public:
	typedef _Revbidit<_BidIt, _BidIt2> _Myt;
	typedef typename iterator_traits<_BidIt>::difference_type _Diff;
	typedef typename iterator_traits<_BidIt>::pointer _Pointer;
	typedef typename iterator_traits<_BidIt>::reference _Reference;
	typedef _BidIt iterator_type;

	__CLR_OR_THIS_CALL _Revbidit()
		{	// construct with default wrapped iterator
		}

	explicit __CLR_OR_THIS_CALL _Revbidit(_BidIt _Right)
		: current(_Right)
		{	// construct wrapped iterator from _Right
		}

	__CLR_OR_THIS_CALL _Revbidit(const _Revbidit<_BidIt2>& _Other)
		: current (_Other.base())
		{	// const converter or copy constructor
		}

	_BidIt __CLR_OR_THIS_CALL base() const
		{	// return wrapped iterator
		return (current);
		}

	_Reference __CLR_OR_THIS_CALL operator*() const
		{	// return designated value
		_BidIt _Tmp = current;
		return (*--_Tmp);
		}

	_Pointer __CLR_OR_THIS_CALL operator->() const
		{	// return pointer to class object
		_Reference _Tmp = **this;
		return (&_Tmp);
		}

	_Myt& __CLR_OR_THIS_CALL operator++()
		{	// preincrement
		--current;
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
        --current;
        return (_Tmp);
    }

    _Myt &__CLR_OR_THIS_CALL operator--() { // predecrement
        ++current;
        return (*this);
    }

    _Myt __CLR_OR_THIS_CALL operator--(int) { // postdecrement
        _Myt _Tmp = *this;
        ++current;
        return (_Tmp);
    }

    bool __CLR_OR_THIS_CALL operator==(const _Myt &_Right) const { // test for iterator equality
        return (current == _Right.current);
    }

    bool __CLR_OR_THIS_CALL operator!=(const _Myt &_Right) const { // test for iterator inequality
        return (!(*this == _Right));
    }

protected:
    _BidIt current;
};

#if 0
		// TEMPLATE CLASS istreambuf_iterator
template<class _Elem,
	class _Traits>
	class istreambuf_iterator
		: public _Iterator_with_base<input_iterator_tag,
			_Elem, typename _Traits::off_type, _Elem *, _Elem&, _Iterator_base_secure>
	{	// wrap stream buffer as input iterator
	typedef istreambuf_iterator<_Elem, _Traits> _Myt;
public:
	typedef _Elem char_type;
	typedef _Traits traits_type;
	typedef basic_streambuf<_Elem, _Traits> streambuf_type;
	typedef basic_istream<_Elem, _Traits> istream_type;
	typedef typename traits_type::int_type int_type;

	__CLR_OR_THIS_CALL istreambuf_iterator(streambuf_type *_Sb = 0) _THROW0()
		: _Strbuf(_Sb), _Got(_Sb == 0)
		{	// construct from stream buffer _Sb
		}

	__CLR_OR_THIS_CALL istreambuf_iterator(istream_type& _Istr) _THROW0()
		: _Strbuf(_Istr.rdbuf()), _Got(_Istr.rdbuf() == 0)
		{	// construct from stream buffer in istream _Istr
		}

	_Elem __CLR_OR_THIS_CALL operator*() const
		{	// return designated value
		if (!_Got)
			((_Myt *)this)->_Peek();

#if _HAS_ITERATOR_DEBUGGING
		if (_Strbuf == 0)
			_DEBUG_ERROR("istreambuf_iterator is not dereferencable");
#endif /* _HAS_ITERATOR_DEBUGGING */

		return (_Val);
		}

	_Myt& __CLR_OR_THIS_CALL operator++()
		{	// preincrement

#if _HAS_ITERATOR_DEBUGGING
		if (_Strbuf == 0)
			_DEBUG_ERROR("istreambuf_iterator is not incrementable");
#endif /* _HAS_ITERATOR_DEBUGGING */

		_Inc();
		return (*this);
		}

	_Myt __CLR_OR_THIS_CALL operator++(int)
		{	// postincrement
		if (!_Got)
			_Peek();
		_Myt _Tmp = *this;
		++*this;
		return (_Tmp);
		}

	bool __CLR_OR_THIS_CALL equal(const _Myt& _Right) const
		{	// test for equality
		if (!_Got)
			((_Myt *)this)->_Peek();
		if (!_Right._Got)
			((_Myt *)&_Right)->_Peek();
		return (_Strbuf == 0 && _Right._Strbuf == 0
			|| _Strbuf != 0 && _Right._Strbuf != 0);
		}

private:
	void __CLR_OR_THIS_CALL _Inc()
		{	// skip to next input element
		if (_Strbuf == 0
			|| traits_type::eq_int_type(traits_type::eof(),
				_Strbuf->sbumpc()))
			_Strbuf = 0, _Got = true;
		else
			_Got = false;
		}

	_Elem __CLR_OR_THIS_CALL _Peek()
		{	// peek at next input element
		int_type _Meta;
		if (_Strbuf == 0
			|| traits_type::eq_int_type(traits_type::eof(),
				_Meta = _Strbuf->sgetc()))
			_Strbuf = 0;
		else
			_Val = traits_type::to_char_type(_Meta);
		_Got = true;
		return (_Val);
		}

	streambuf_type *_Strbuf;	// the wrapped stream buffer
	bool _Got;	// true if _Val is valid
	_Elem _Val;	// next element to deliver
	};

		// istreambuf_iterator TEMPLATE OPERATORS
template<class _Elem,
	class _Traits> inline
	bool __CLRCALL_OR_CDECL operator==(
		const istreambuf_iterator<_Elem, _Traits>& _Left,
		const istreambuf_iterator<_Elem, _Traits>& _Right)
	{	// test for istreambuf_iterator equality
	return (_Left.equal(_Right));
	}

template<class _Elem,
	class _Traits> inline
	bool __CLRCALL_OR_CDECL operator!=(
		const istreambuf_iterator<_Elem, _Traits>& _Left,
		const istreambuf_iterator<_Elem, _Traits>& _Right)
	{	// test for istreambuf_iterator inequality
	return (!(_Left == _Right));
	}

		// TEMPLATE CLASS ostreambuf_iterator
template<class _Elem,
	class _Traits>
	class ostreambuf_iterator
		: public _Outit_with_base<_Iterator_base_secure>
	{	// wrap stream buffer as output iterator
	typedef ostreambuf_iterator<_Elem, _Traits> _Myt;
public:
	typedef _Elem char_type;
	typedef _Traits traits_type;
	typedef basic_streambuf<_Elem, _Traits> streambuf_type;
	typedef basic_ostream<_Elem, _Traits> ostream_type;

	typedef _Range_checked_iterator_tag _Checked_iterator_category;

	__CLR_OR_THIS_CALL ostreambuf_iterator(streambuf_type *_Sb) _THROW0()
		: _Failed(false), _Strbuf(_Sb)
		{	// construct from stream buffer _Sb
		}

	__CLR_OR_THIS_CALL ostreambuf_iterator(ostream_type& _Ostr) _THROW0()
		: _Failed(false), _Strbuf(_Ostr.rdbuf())
		{	// construct from stream buffer in _Ostr
		}

	_Myt& __CLR_OR_THIS_CALL operator=(_Elem _Right)
		{	// store element and increment
		if (_Strbuf == 0
			|| traits_type::eq_int_type(_Traits::eof(),
				_Strbuf->sputc(_Right)))
			_Failed = true;
		return (*this);
		}

	_Myt& __CLR_OR_THIS_CALL operator*()
		{	// pretend to get designated element
		return (*this);
		}

	_Myt& __CLR_OR_THIS_CALL operator++()
		{	// pretend to preincrement
		return (*this);
		}

	_Myt& __CLR_OR_THIS_CALL operator++(int)
		{	// pretend to postincrement
		return (*this);
		}

	bool __CLR_OR_THIS_CALL failed() const _THROW0()
		{	// return true if any stores failed
		return (_Failed);
		}

private:
	bool _Failed;	// true if any stores have failed
	streambuf_type *_Strbuf;	// the wrapped stream buffer
	};
#endif

//	ALGORITHM STUFF (from <algorithm>)

		// TEMPLATE FUNCTION copy
template<class _InIt, class _OutIt, class _InOutItCat>
inline
	_OutIt __CLRCALL_OR_CDECL _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		_InOutItCat, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) to [_Dest, ...), arbitrary iterators
	_DEBUG_RANGE(_First, _Last);
	for (; _First != _Last; ++_Dest, ++_First)
		*_Dest = *_First;
	return (_Dest);
	}

#if _SECURE_SCL
template<class _InIt, class _OutIt>
inline
	_OutIt __CLRCALL_OR_CDECL _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		random_access_iterator_tag, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) to [_Dest, ...), random_access iterators
	// if _OutIt is range checked, this will make sure there is enough space for the copy
	_OutIt _Result = _Dest + (_Last - _First);
	_Copy_opt(_First, _Last, _CHECKED_BASE(_Dest), 
		forward_iterator_tag(), _Nonscalar_ptr_iterator_tag(), _Range_checked_iterator_tag());
	return _Result;
	}
#endif

template<class _InIt, class _OutIt, class _InOutItCat>
inline
	_OutIt __CLRCALL_OR_CDECL _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		_InOutItCat, _Scalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) to [_Dest, ...), pointers to scalars

 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First, _Last);
	if (_First != _Last)
		_DEBUG_POINTER(_Dest);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	ptrdiff_t _Off = _Last - _First;	// NB: non-overlapping move
	// if _OutIt is range checked, this will make sure there is enough space for the memmove
	_OutIt _Result = _Dest + _Off;
	if (_Off > 0)
		_CRT_SECURE_MEMMOVE(&*_Dest, _Off * sizeof (*_First), &*_First, _Off * sizeof (*_First));
	return _Result;
	}

template <bool _Cond, class _Result_type>
struct _Enable_if;

template <class _Result_type>
struct _Enable_if<true, _Result_type>
{
	typedef _Result_type _Result;
};

template <class _Checked_iter_cat>
struct _Is_checked_iterator_helper
{
	enum { _Result = false };
};

template <>
struct _Is_checked_iterator_helper<_Range_checked_iterator_tag>
{
	enum { _Result = true };
};

template <class _Iter>
struct _Is_checked_iterator
{
	enum { _Result = 
		_Is_checked_iterator_helper<typename _Checked_iterator_category<_Iter>::_Checked_cat>::_Result };
};

#define _IF_CHK(_Iter_type) \
	typename _STD _Enable_if< _STD _Is_checked_iterator< _Iter_type >::_Result, _Iter_type >::_Result
#define _IF_CHK_(_Iter_type, _Result_type) \
	typename _STD _Enable_if< _STD _Is_checked_iterator< _Iter_type >::_Result, _Result_type >::_Result
#define _IF_CHK_RET_PAIR(_Iter_type, _Result_type1, _Result_type2) \
	typename _STD _Enable_if< _STD _Is_checked_iterator< _Iter_type >::_Result, _STD pair< _Result_type1, _Result_type2 > >::_Result
#define _IF_CHK2_(_Iter1_type, _Iter2_type, _Result_type) \
	typename _STD _Enable_if< _STD _Is_checked_iterator< _Iter1_type >::_Result && _STD _Is_checked_iterator< _Iter2_type >::_Result, _Result_type >::_Result

#define _IF_NOT_CHK(_Iter_type) \
	typename _STD _Enable_if< ! _STD _Is_checked_iterator< _Iter_type >::_Result, _Iter_type >::_Result
#define _IF_NOT_CHK_(_Iter_type, _Result_type) \
	typename _STD _Enable_if< ! _STD _Is_checked_iterator< _Iter_type >::_Result, _Result_type >::_Result
#define _IF_NOT_CHK_RET_PAIR(_Iter_type, _Result_type1, _Result_type2) \
	typename _STD _Enable_if< ! _STD _Is_checked_iterator< _Iter_type >::_Result, _STD pair< _Result_type1, _Result_type2 > >::_Result
#define _IF_NOT_CHK2_(_Iter1_type, _Iter2_type, _Result_type) \
	typename _STD _Enable_if< ! _STD _Is_checked_iterator< _Iter1_type >::_Result || !_STD _Is_checked_iterator< _Iter2_type >::_Result, _Result_type >::_Result

#if _SECURE_SCL

template<class _InIt, class _OutIt>
inline
_IF_CHK(_OutIt) __CLRCALL_OR_CDECL copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
	return (_Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest, 
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _Range_checked_iterator_tag()));
	}

template<class _InIt, class _OutElem, size_t _Size>
inline
_OutElem* __CLRCALL_OR_CDECL copy(_InIt _First, _InIt _Last, _OutElem (&_Dest)[_Size])
	{	// copy [_First, _Last) to [_Dest, ...)
	return copy(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _STDEXT make_checked_array_iterator(_Dest, _Size)).base();
	}

template<class _InIt, class _OutIt>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK(_OutIt) __CLRCALL_OR_CDECL copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
	return (_Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest, 
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _Range_checked_iterator_tag()));
	}

#else

template<class _InIt, class _OutIt>
inline
_OutIt __CLRCALL_OR_CDECL copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
	return (_Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest, 
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _Range_checked_iterator_tag()));
	}

#endif

    // TEMPLATE FUNCTION _Move
    template<class _InIt, class _OutIt, class _InOutItCat, class _MoveCatTy>
    inline _OutIt __CLRCALL_OR_CDECL _Move_opt(
        _InIt _First,
        _InIt _Last,
        _OutIt _Dest,
        _InOutItCat _First_dest_cat,
        _MoveCatTy,
        _Range_checked_iterator_tag) { // move defaults to copy if there is not a more effecient way
        return _Copy_opt(_First,
                         _Last,
                         _Dest,
                         _First_dest_cat,
                         _Ptr_cat(_First, _Dest),
                         _Range_checked_iterator_tag());
    }

    template<class _InIt, class _OutIt, class _InOutItCat>
    inline _OutIt __CLRCALL_OR_CDECL
    _Move_opt(_InIt _First,
              _InIt _Last,
              _OutIt _Dest,
              _InOutItCat,
              _Swap_move_tag,
              _Range_checked_iterator_tag) { // use swap to instead of the copy constructor
        _DEBUG_RANGE(_First, _Last);
        for (; _First != _Last; ++_Dest, ++_First) {
            std::swap(*_Dest, *_First);
        }
        return (_Dest);
    }

#if _SECURE_SCL
template<class _InIt, class _OutIt, class _InOutItCat>
inline
	_OutIt __CLRCALL_OR_CDECL _Move_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		random_access_iterator_tag, _Swap_move_tag _Move_cat, _Range_checked_iterator_tag)
	{	// use swap to instead of the copy constructor
	// if _OutIt is range checked, this will make sure there is enough space for the copy
	_OutIt _Result = _Dest + (_Last - _First);
	_Move_opt(_First, _Last, _CHECKED_BASE(_Dest),
		forward_iterator_tag(), _Move_cat, _Range_checked_iterator_tag());
	return _Result;
	}
#endif

#if _SECURE_SCL

template<class _InIt, class _OutIt>
inline
_IF_CHK(_OutIt) __CLRCALL_OR_CDECL _Move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
	return _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _InIt, class _OutElem, size_t _Size>
inline
_OutElem* __CLRCALL_OR_CDECL _Move(_InIt _First, _InIt _Last, _OutElem (&_Dest)[_Size])
	{	// move [_First, _Last) to [_Dest, ...)
	return _Move(_CHECKED_BASE(_First), _CHECKED_BASE(_Last),
		_STDEXT make_checked_array_iterator(_Dest, _Size)).base();
	}

template<class _InIt, class _OutIt>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK(_OutIt) __CLRCALL_OR_CDECL _Move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
	return _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

#else

template<class _InIt, class _OutIt>
inline
	_OutIt __CLRCALL_OR_CDECL _Move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
	return _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

#endif

		// TEMPLATE FUNCTION copy_backward
template<class _BidIt1, class _BidIt2, class _InOutItCat>
inline
	_BidIt2 __CLRCALL_OR_CDECL _Copy_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest,
		_InOutItCat, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) backwards to [..., _Dest), arbitrary iterators
	_DEBUG_RANGE(_First, _Last);
	while (_First != _Last)
		*--_Dest = *--_Last;
	return (_Dest);
	}

#if _SECURE_SCL
template<class _InIt, class _OutIt>
inline
	_OutIt __CLRCALL_OR_CDECL _Copy_backward_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		random_access_iterator_tag, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) backwards to [..., _Dest), arbitrary iterators
	// if _OutIt is range checked, this will make sure there is enough space for the copy
	_OutIt _Result = _Dest - (_Last - _First);
	_Copy_backward_opt(_First, _Last, _CHECKED_BASE(_Dest),
		forward_iterator_tag(), _Nonscalar_ptr_iterator_tag(), _Range_checked_iterator_tag());
	return _Result;
	}
#endif

template<class _InIt, class _OutIt, class _InOutItCat>
inline
	_OutIt __CLRCALL_OR_CDECL _Copy_backward_opt(_InIt _First, _InIt _Last, _OutIt _Dest,
		_InOutItCat, _Scalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{	// copy [_First, _Last) backwards to [..., _Dest), pointers to scalars

 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First, _Last);
	if (_First != _Last)
		_DEBUG_POINTER(_Dest);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	ptrdiff_t _Off = _Last - _First;	// NB: non-overlapping move
	/* if _OutIt is range checked, this will make sure there is enough space for 
	 * the memmove
	 */
	_OutIt _Result = _Dest - _Off;
	if (_Off > 0)
		_CRT_SECURE_MEMMOVE(&*_Result, _Off * sizeof (*_First), &*_First, _Off * sizeof (*_First));
	return _Result;
	}

#if _SECURE_SCL

template<class _BidIt1,
	class _BidIt2> inline
_IF_CHK(_BidIt2) __CLRCALL_OR_CDECL copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
	return _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag());
	}

template<class _BidIt1,
	class _BidIt2> inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK(_BidIt2) __CLRCALL_OR_CDECL copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
	return _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag());
	}

#else

template<class _BidIt1,
	class _BidIt2> inline
	_BidIt2 __CLRCALL_OR_CDECL copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
	return _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag());
	}

#endif

    // TEMPLATE FUNCTION _Move_backward
    template<class _BidIt1, class _BidIt2, class _InOutItCat, class _MoveCatTy>
    inline _BidIt2 __CLRCALL_OR_CDECL _Move_backward_opt(
        _BidIt1 _First,
        _BidIt1 _Last,
        _BidIt2 _Dest,
        _InOutItCat _First_dest_cat,
        _MoveCatTy,
        _Range_checked_iterator_tag) { // move defaults to copy if there is not a more effecient way
        return _Copy_backward_opt(_First,
                                  _Last,
                                  _Dest,
                                  _First_dest_cat,
                                  _Ptr_cat(_First, _Dest),
                                  _Range_checked_iterator_tag());
    }

    template<class _BidIt1, class _BidIt2, class _InOutItCat>
    inline _BidIt2 __CLRCALL_OR_CDECL
    _Move_backward_opt(_BidIt1 _First,
                       _BidIt1 _Last,
                       _BidIt2 _Dest,
                       _InOutItCat,
                       _Swap_move_tag,
                       _Range_checked_iterator_tag) { // use swap instead of the copy constructor
        _DEBUG_RANGE(_First, _Last);
        while (_First != _Last) {
            std::swap(*--_Dest, *--_Last);
        }
        return (_Dest);
    }

#if _SECURE_SCL
template<class _BidIt1, class _BidIt2>
inline
	_BidIt2 __CLRCALL_OR_CDECL _Move_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest,
		random_access_iterator_tag, _Swap_move_tag _Move_cat, _Range_checked_iterator_tag)
	{	// use swap to instead of the copy constructor
	// if _OutIt is range checked, this will make sure there is enough space for the copy
	_BidIt2 _Result = _Dest - (_Last - _First);
	_Move_backward_opt(_First, _Last, _CHECKED_BASE(_Dest),
		forward_iterator_tag(), _Move_cat, _Range_checked_iterator_tag());
	return _Result;
	}
#endif

#if _SECURE_SCL

template<class _BidIt1, class _BidIt2>
inline
_IF_CHK(_BidIt2) __CLRCALL_OR_CDECL _Move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
	return _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _BidIt1, class _BidIt2>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK(_BidIt2) __CLRCALL_OR_CDECL _Move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
	return _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

#else

template<class _BidIt1, class _BidIt2>
inline
	_BidIt2 __CLRCALL_OR_CDECL _Move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
	return _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_Iter_random(_First, _Dest), _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

#endif

    // TEMPLATE FUNCTION mismatch
    template<class _InIt1, class _InIt2, class _InItCats>
    inline _STD pair<_InIt1, _InIt2> __CLRCALL_OR_CDECL _Mismatch(
        _InIt1 _First1,
        _InIt1 _Last1,
        _InIt2 _First2,
        _InItCats,
        _Range_checked_iterator_tag) { // return [_First1, _Last1) and [_First2, _Last2) mismatch

#if _HAS_ITERATOR_DEBUGGING
        _DEBUG_RANGE(_First1, _Last1);
        if (_First1 != _Last1)
            _DEBUG_POINTER(_First2);
#endif /* _HAS_ITERATOR_DEBUGGING */

        for (; _First1 != _Last1 && *_First1 == *_First2;)
            ++_First1, ++_First2;
        return (pair<_InIt1, _InIt2>(_First1, _First2));
    }

#if _SECURE_SCL
template<class _InIt1, class _InIt2>
inline
	pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL _Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2,
			random_access_iterator_tag, _Range_checked_iterator_tag)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	// for range checked iterators, this will make sure there is enough space
	_InIt2 _Last2 = _First2 + (_Last1 - _First1); (_Last2);
	pair<_InIt1, _CHECKED_BASE_TYPE(_InIt2)> _Result =
		_Mismatch(_First1, _Last1, _CHECKED_BASE(_First2),
			forward_iterator_tag(), _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First2, _Result.second);
	return (pair<_InIt1, _InIt2>(_Result.first, _First2));
	}
#endif

#if _SECURE_SCL

template<class _InIt1, class _InIt2>
inline
_IF_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
			_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InElem2, size_t _Size>
inline
pair<_InIt1, _InElem2*>
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size])
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	pair<_InIt1, _STDEXT checked_array_iterator<_InElem2*> > _Result =
		mismatch(_First1, _Last1, _STDEXT make_checked_array_iterator(_First2, _Size));
	return (pair<_InIt1, _InElem2*>(_Result.first, _Result.second.base()));
	}

template<class _InIt1, class _InIt2>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
			_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

#else

template<class _InIt1, class _InIt2>
inline
	pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	return _Mismatch(_First1, _Last1, _First2,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

#endif

		// TEMPLATE FUNCTION mismatch WITH PRED
template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
inline
	pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL _Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred,
			_InItCats, _Range_checked_iterator_tag)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred

 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First1, _Last1);
	if (_First1 != _Last1)
		_DEBUG_POINTER(_First2);
	_DEBUG_POINTER(_Pred);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	for (; _First1 != _Last1 && _Pred(*_First1, *_First2); )
		++_First1, ++_First2;
	return (pair<_InIt1, _InIt2>(_First1, _First2));
	}

#if _SECURE_SCL
template<class _InIt1, class _InIt2, class _Pr>
inline
	pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL _Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred,
			random_access_iterator_tag, _Range_checked_iterator_tag)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	// for range checked iterators, this will make sure there is enough space
	_InIt2 _Last2 = _First2 + (_Last1 - _First1); (_Last2);
	pair<_InIt1, _CHECKED_BASE_TYPE(_InIt2)> _Result =
		_Mismatch(_First1, _Last1, _CHECKED_BASE(_First2), _Pred,
			forward_iterator_tag(), _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First2, _Result.second);
	return (pair<_InIt1, _InIt2>(_Result.first, _First2));
	}
#endif

#if _SECURE_SCL

template<class _InIt1, class _InIt2, class _Pr>
inline
_IF_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
			_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InElem2, class _Pr, size_t _Size>
inline
pair<_InIt1, _InElem2*>
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size], _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	pair<_InIt1, _STDEXT checked_array_iterator<_InElem2*> > _Result =
		mismatch(_First1, _Last1, _STDEXT make_checked_array_iterator(_First2, _Size), _Pred);
	return (pair<_InIt1, _InElem2*>(_Result.first, _Result.second.base()));
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
			_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

#else

    template<class _InIt1, class _InIt2, class _Pr>
    inline std::pair<_InIt1, _InIt2> __CLRCALL_OR_CDECL
    mismatch(_InIt1 _First1,
             _InIt1 _Last1,
             _InIt2 _First2,
             _Pr _Pred) { // return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
        return _Mismatch(_First1,
                         _Last1,
                         _First2,
                         _Pred,
                         _Iter_random(_First1, _First2),
                         _STD _Range_checked_iterator_tag());
    }

#endif

		// TEMPLATE FUNCTION equal
template<class _InIt1, class _InIt2, class _InItCats>
inline
	bool __CLRCALL_OR_CDECL _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2,
		_InItCats, _Range_checked_iterator_tag)
	{	// compare [_First1, _Last1) to [First2, ...)
	return (_Mismatch(_First1, _Last1, _First2,
		forward_iterator_tag(), _Range_checked_iterator_tag()).first == _Last1);
	}

inline bool __CLRCALL_OR_CDECL _Equal(const char *_First1,
	const char *_Last1, const char *_First2,
	random_access_iterator_tag, _Range_checked_iterator_tag)
	{	// compare [_First1, _Last1) to [First2, ...), for chars
 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First1, _Last1);
	if (_First1 != _Last1)
		_DEBUG_POINTER(_First2);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	return (::memcmp(_First1, _First2, _Last1 - _First1) == 0);
	}

inline bool __CLRCALL_OR_CDECL _Equal(const signed char *_First1,
	const signed char *_Last1, const signed char *_First2,
	random_access_iterator_tag, _Range_checked_iterator_tag)
	{	// compare [_First1, _Last1) to [First2, ...), for signed chars
 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First1, _Last1);
	if (_First1 != _Last1)
		_DEBUG_POINTER(_First2);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	return (::memcmp(_First1, _First2, _Last1 - _First1) == 0);
	}

inline bool __CLRCALL_OR_CDECL _Equal(const unsigned char *_First1,
	const unsigned char *_Last1, const unsigned char *_First2,
	random_access_iterator_tag, _Range_checked_iterator_tag)
	{	// compare [_First1, _Last1) to [First2, ...), for unsigned chars
 #if _HAS_ITERATOR_DEBUGGING
	_DEBUG_RANGE(_First1, _Last1);
	if (_First1 != _Last1)
		_DEBUG_POINTER(_First2);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	return (::memcmp(_First1, _First2, _Last1 - _First1) == 0);
	}

#if _SECURE_SCL
template<class _InIt1, class _InIt2>
inline
	bool __CLRCALL_OR_CDECL _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2,
		random_access_iterator_tag, _Range_checked_iterator_tag)
	{
	// for range checked iterators, this will make sure there is enough space
	_InIt2 _Last2 = _First2 + (_Last1 - _First1); (_Last2);
	return _Equal(_First1, _Last1, _CHECKED_BASE(_First2),
		forward_iterator_tag(), _Range_checked_iterator_tag());
	}
#endif

#if _SECURE_SCL

template<class _InIt1, class _InIt2>
inline
_IF_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InElem2, size_t _Size>
inline
bool __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size])
	{	// compare [_First1, _Last1) to [First2, ...)
	return equal(_First1, _Last1,
		_STDEXT make_checked_array_iterator(_First2, _Size));
	}

template<class _InIt1, class _InIt2>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

#else

template<class _InIt1, class _InIt2>
inline
	bool __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

#endif

		// TEMPLATE FUNCTION equal WITH PRED
template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
inline
	bool __CLRCALL_OR_CDECL _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred,
		_InItCats, _Range_checked_iterator_tag)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return (_Mismatch(_First1, _Last1, _First2, _Pred,
		forward_iterator_tag(), _Range_checked_iterator_tag()).first == _Last1);
	}

#if _SECURE_SCL
template<class _InIt1, class _InIt2, class _Pr>
inline
	bool __CLRCALL_OR_CDECL _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred,
		random_access_iterator_tag, _Range_checked_iterator_tag)
	{
	// for range checked iterators, this will make sure there is enough space
	_InIt2 _Last2 = _First2 + (_Last1 - _First1); (_Last2);
	return _Equal(_First1, _Last1, _CHECKED_BASE(_First2), _Pred,
		forward_iterator_tag(), _Range_checked_iterator_tag());
	}
#endif

#if _SECURE_SCL

template<class _InIt1, class _InIt2, class _Pr>
inline
_IF_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InElem2, class _Pr, size_t _Size>
inline
bool __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size], _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return equal(_First1, _Last1,
		_STDEXT make_checked_array_iterator(_First2, _Size), _Pred);
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

#else

template<class _InIt1, class _InIt2, class _Pr>
inline
	bool __CLRCALL_OR_CDECL equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

#endif

		// TEMPLATE FUNCTION fill
template<class _FwdIt, class _Ty> inline
	void __CLRCALL_OR_CDECL _Fill(_FwdIt _First, _FwdIt _Last, const _Ty& _Val)
	{	// copy _Val through [_First, _Last)
	_DEBUG_RANGE(_First, _Last);
	for (; _First != _Last; ++_First)
		*_First = _Val;
	}

inline void __CLRCALL_OR_CDECL _Fill(__out_ecount_full(_Last-_First) char *_First, 
	__in_opt char *_Last, int _Val)
	{	// copy char _Val through [_First, _Last)
	_DEBUG_RANGE(_First, _Last);
	::memset(_First, _Val, _Last - _First);
	}

inline void __CLRCALL_OR_CDECL _Fill(__out_ecount_full(_Last-_First) signed char *_First, 
	__in_opt signed char *_Last, int _Val)
	{	// copy signed char _Val through [_First, _Last)
	_DEBUG_RANGE(_First, _Last);
	::memset(_First, _Val, _Last - _First);
	}

inline void __CLRCALL_OR_CDECL _Fill(
	__out_ecount_full(_Last-_First) unsigned char *_First,
	__in_opt unsigned char *_Last, int _Val)
	{	// copy unsigned char _Val through [_First, _Last)
	_DEBUG_RANGE(_First, _Last);
	::memset(_First, _Val, _Last - _First);
	}

template<class _FwdIt, class _Ty> inline
	void __CLRCALL_OR_CDECL fill(_FwdIt _First, _FwdIt _Last, const _Ty& _Val)
	{	// copy _Val through [_First, _Last)
	_Fill(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Val);
	}

		// TEMPLATE FUNCTION fill_n
template<class _OutIt,
	class _Diff,
	class _Ty> inline
	void __CLRCALL_OR_CDECL _Fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val,
		_Range_checked_iterator_tag)
	{	// copy _Val _Count times through [_First, ...)
	for (; 0 < _Count; --_Count, ++_First)
		*_First = _Val;
	}

inline void __CLRCALL_OR_CDECL _Fill_n(__out_ecount_full_opt(_Count) char *_First, 
		size_t _Count, int _Val, _Range_checked_iterator_tag)
	{	// copy char _Val _Count times through [_First, ...)

 #if _HAS_ITERATOR_DEBUGGING
	if (0 < _Count)
		_DEBUG_POINTER(_First);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	::memset(_First, _Val, _Count);
	}

inline void __CLRCALL_OR_CDECL _Fill_n(__out_ecount_full_opt(_Count) signed char *_First, 
		size_t _Count, int _Val, _Range_checked_iterator_tag)
	{	// copy signed char _Val _Count times through [_First, ...)

 #if _HAS_ITERATOR_DEBUGGING
	if (0 < _Count)
		_DEBUG_POINTER(_First);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	::memset(_First, _Val, _Count);
	}

inline void __CLRCALL_OR_CDECL _Fill_n(__out_ecount_full_opt(_Count) unsigned char *_First, 
		size_t _Count, int _Val, _Range_checked_iterator_tag)
	{	// copy unsigned char _Val _Count times through [_First, ...)

 #if _HAS_ITERATOR_DEBUGGING
	if (0 < _Count)
		_DEBUG_POINTER(_First);
 #endif /* _HAS_ITERATOR_DEBUGGING */

	::memset(_First, _Val, _Count);
	}

template<class _OutIt, class _Diff, class _Ty, class _OutCat>
inline
	void __CLRCALL_OR_CDECL _Fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val,
		_OutCat, _Range_checked_iterator_tag)
	{
		_Fill_n(_First, _Count, _Val, _Range_checked_iterator_tag());
	}

#if _SECURE_SCL
template<class _OutIt, class _Diff, class _Ty>
inline
	void __CLRCALL_OR_CDECL _Fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val,
		random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		// if _OutIt is checked, this will ensure we have enough space
		_OutIt _Last = _First + _Count; (_Last);
		_Fill_n(_CHECKED_BASE(_First), _Count, _Val,
			_Range_checked_iterator_tag());
	}
#endif

#if _SECURE_SCL

template<class _OutIt,
	class _Diff,
	class _Ty>
inline
_IF_CHK_(_OutIt, void) __CLRCALL_OR_CDECL fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_Fill_n(_First, _Count, _Val, _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

template<class _OutElem,
	class _Diff,
	class _Ty, size_t _Size>
inline
void __CLRCALL_OR_CDECL fill_n(_OutElem (&_First)[_Size], _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		fill_n(_STDEXT make_checked_array_iterator(_First, _Size), _Count, _Val);
	}

template<class _OutIt,
	class _Diff,
	class _Ty>
inline
_SCL_INSECURE_DEPRECATE
_IF_NOT_CHK_(_OutIt, void) __CLRCALL_OR_CDECL fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_Fill_n(_First, _Count, _Val, _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

#else

template<class _OutIt,
	class _Diff,
	class _Ty> inline
	void __CLRCALL_OR_CDECL fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_Fill_n(_First, _Count, _Val, _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

#endif

		// TEMPLATE FUNCTION lexicographical_compare
template<class _InIt1, class _InIt2> inline
	bool __CLRCALL_OR_CDECL _Lexicographical_compare(_InIt1 _First1, _InIt1 _Last1,
		_InIt2 _First2, _InIt2 _Last2)
	{	// order [_First1, _Last1) vs. [First2, Last2)
	_DEBUG_RANGE(_First1, _Last1);
	_DEBUG_RANGE(_First2, _Last2);
	for (; _First1 != _Last1 && _First2 != _Last2; ++_First1, ++_First2)
		if (_DEBUG_LT(*_First1, *_First2))
			return (true);
		else if (*_First2 < *_First1)
			return (false);
	return (_First1 == _Last1 && _First2 != _Last2);
	}

template<class _InIt1, class _InIt2> inline
	bool __CLRCALL_OR_CDECL lexicographical_compare(_InIt1 _First1, _InIt1 _Last1,
		_InIt2 _First2, _InIt2 _Last2)
	{	// order [_First1, _Last1) vs. [First2, Last2)
	return _Lexicographical_compare(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1),
		_CHECKED_BASE(_First2), _CHECKED_BASE(_Last2));
	}

inline bool __CLRCALL_OR_CDECL lexicographical_compare(
	const unsigned char *_First1, const unsigned char *_Last1,
	const unsigned char *_First2, const unsigned char *_Last2)
	{	// order [_First1, _Last1) vs. [First2, Last2), for unsigned char
	_DEBUG_RANGE(_First1, _Last1);
	_DEBUG_RANGE(_First2, _Last2);
	ptrdiff_t _Num1 = _Last1 - _First1;
	ptrdiff_t _Num2 = _Last2 - _First2;
    int _Ans = ::memcmp(_First1, _First2, _Num1 < _Num2 ? _Num1 : _Num2);
    return (_Ans < 0 || (_Ans == 0 && _Num1 < _Num2));
    }

 #if CHAR_MAX == UCHAR_MAX
inline bool __CLRCALL_OR_CDECL lexicographical_compare(
	const char *_First1, const char *_Last1,
	const char *_First2, const char *_Last2)
	{	// order [_First1, _Last1) vs. [First2, Last2), for nonnegative char
	_DEBUG_RANGE(_First1, _Last1);
	_DEBUG_RANGE(_First2, _Last2);
	ptrdiff_t _Num1 = _Last1 - _First1;
	ptrdiff_t _Num2 = _Last2 - _First2;
	int _Ans = ::memcmp(_First1, _First2, _Num1 < _Num2 ? _Num1 : _Num2);
	return (_Ans < 0 || _Ans == 0 && _Num1 < _Num2);
	}
 #endif /* CHAR_MAX == UCHAR_MAX */

		// TEMPLATE FUNCTION lexicographical_compare WITH PRED
template<class _InIt1,
	class _InIt2,
	class _Pr> inline
	bool __CLRCALL_OR_CDECL _Lexicographical_compare(_InIt1 _First1, _InIt1 _Last1,
		_InIt2 _First2, _InIt2 _Last2, _Pr _Pred)
	{	// order [_First1, _Last1) vs. [First2, Last2) using _Pred
	_DEBUG_RANGE(_First1, _Last1);
	_DEBUG_RANGE(_First2, _Last2);
	_DEBUG_POINTER(_Pred);
	for (; _First1 != _Last1 && _First2 != _Last2; ++_First1, ++_First2)
		if (_DEBUG_LT_PRED(_Pred, *_First1, *_First2))
			return (true);
		else if (_Pred(*_First2, *_First1))
			return (false);
	return (_First1 == _Last1 && _First2 != _Last2);
	}

template<class _InIt1, class _InIt2, class _Pr> inline
	bool __CLRCALL_OR_CDECL lexicographical_compare(_InIt1 _First1, _InIt1 _Last1,
		_InIt2 _First2, _InIt2 _Last2, _Pr _Pred)
	{	// order [_First1, _Last1) vs. [First2, Last2) using _Pred
	return _Lexicographical_compare(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1),
		_CHECKED_BASE(_First2), _CHECKED_BASE(_Last2), _Pred);
	}

 #ifndef _cpp_max
  #define _cpp_max  max /* retained */
  #define _cpp_min  min /* retained */
 #endif

 #ifndef _MAX	/* avoid collision with common (nonconforming) macros */
  #define _MAX	(max)
  #define _MIN	(min)
  #define _IMPL_MAX max
  #define _IMPL_MIN min
 #else
  #define _IMPL_MAX _MAX
  #define _IMPL_MIN _MIN
 #endif

		// TEMPLATE FUNCTION max
template<class _Ty> inline
	const _Ty& (__CLRCALL_OR_CDECL _IMPL_MAX)(const _Ty& _Left, const _Ty& _Right)
	{	// return larger of _Left and _Right
	return (_DEBUG_LT(_Left, _Right) ? _Right : _Left);
	}

		// TEMPLATE FUNCTION max WITH PRED
template<class _Ty,
	class _Pr> inline
	const _Ty& (__CLRCALL_OR_CDECL _IMPL_MAX)(const _Ty& _Left, const _Ty& _Right, _Pr _Pred)
	{	// return larger of _Left and _Right using _Pred
	return (_DEBUG_LT_PRED(_Pred, _Left, _Right) ? _Right : _Left);
	}

		// TEMPLATE FUNCTION min
template<class _Ty> inline
	const _Ty& (__CLRCALL_OR_CDECL _IMPL_MIN)(const _Ty& _Left, const _Ty& _Right)
	{	// return smaller of _Left and _Right
	return (_DEBUG_LT(_Right, _Left) ? _Right : _Left);
	}

		// TEMPLATE FUNCTION min WITH PRED
template<class _Ty,
	class _Pr> inline
	const _Ty& (__CLRCALL_OR_CDECL _IMPL_MIN)(const _Ty& _Left, const _Ty& _Right, _Pr _Pred)
	{	// return smaller of _Left and _Right using _Pred
	return (_DEBUG_LT_PRED(_Pred, _Right, _Left) ? _Right : _Left);
	}

  #undef _IMPL_MAX
  #undef _IMPL_MIN

_STD_END

_STDEXT_BEGIN

// checked_array_iterator
template <class _Iterator>
	class checked_array_iterator
		: public _STD iterator<
			typename _STD iterator_traits<_Iterator>::iterator_category, 
			typename _STD iterator_traits<_Iterator>::value_type, 
			typename _STD iterator_traits<_Iterator>::difference_type, 
			typename _STD iterator_traits<_Iterator>::pointer, 
			typename _STD iterator_traits<_Iterator>::reference>
	{
public:
	typedef checked_array_iterator<_Iterator> _Myt;
	typedef typename _STD iterator_traits<_Iterator>::difference_type difference_type;
	typedef typename _STD iterator_traits<_Iterator>::pointer pointer;
	typedef typename _STD iterator_traits<_Iterator>::reference reference;

	typedef _STD _Range_checked_iterator_tag _Checked_iterator_category;
	typedef _Iterator _Inner_type;

	typedef _Iterator _Checked_iterator_base_type;
 
	_Checked_iterator_base_type _Checked_iterator_base() const
	{
		return _Mycont + _Current;
	}

	void _Checked_iterator_assign_from_base(_Checked_iterator_base_type _Base)
	{
		_Current = _Base - _Mycont;
	}

	// use default copy constructor and copy assignement

	checked_array_iterator():
		_Size(0), _Current(0)
	{
	}

	checked_array_iterator(_Iterator _Cont, size_t _S, size_t _Index = 0)
	{
		_SCL_SECURE_ALWAYS_VALIDATE(_Index <= _S);
		_Mycont = _Cont;
		_Size = _S;
		_Current =_Index;
	}

	_Iterator base() const
	{
		return _Mycont + _Current;
	}

	size_t __Size() const
	{
		return _Size;
	}

	bool operator==(const _Myt& _Right) const
	{
		_SCL_SECURE_ALWAYS_VALIDATE(_Mycont == _Right._Mycont);
		return _Current == _Right._Current;
	}

	bool operator!=(const _Myt& _Right) const
	{
		return !(*this == _Right);
	}

	bool operator<(const _Myt& _Right) const
	{
		_SCL_SECURE_ALWAYS_VALIDATE(_Mycont == _Right._Mycont);
		return _Current < _Right._Current;
	}

	bool operator>(const _Myt& _Right) const
	{
		return _Right < *this;
	}

	bool operator<=(const _Myt& _Right) const
	{
		return !(_Right < *this);
	}

	bool operator>=(const _Myt& _Right) const
	{
		return !(*this < _Right);
	}

	reference operator*() const
	{
		_SCL_SECURE_ALWAYS_VALIDATE_RANGE(_Current < _Size);
		return *(_Mycont + _Current);
	}

	pointer operator->() const
	{
		return (&**this);
	}

	checked_array_iterator& operator++()
	{
		_SCL_SECURE_ALWAYS_VALIDATE_RANGE(_Current < _Size);
		++_Current;
		return *this;
	}

	_Myt operator++(int)
	{
		checked_array_iterator _Tmp = *this;
		++*this;
		return _Tmp;
	}

	_Myt& operator--()
	{
		_SCL_SECURE_ALWAYS_VALIDATE_RANGE(_Current > 0);
		--_Current;
		return *this;
	}

	_Myt operator--(int)
	{
		checked_array_iterator _Tmp = *this;
		--*this;
		return _Tmp;
	}

	// random access iterators methods

	_Myt& operator+=(difference_type _Off)
	{
		_SCL_SECURE_ALWAYS_VALIDATE_RANGE(_Current + _Off <= _Size && _Current + _Off >= 0);
		_Current += _Off;
		return *this;
	}

	_Myt operator+(difference_type _Off) const
	{
		checked_array_iterator _Tmp = *this;
		return (_Tmp += _Off);
	}

	_Myt& operator-=(difference_type _Off)
	{
		return (*this += -_Off);
	}

	_Myt operator-(difference_type _Off) const
	{
		checked_array_iterator _Tmp = *this;
		return (_Tmp -= _Off);
	}

	difference_type operator-(const checked_array_iterator& _Right) const
	{
		_SCL_SECURE_ALWAYS_VALIDATE(_Mycont == _Right._Mycont);
		return _Current - _Right._Current;
	}

	reference operator[](difference_type _Off) const
	{
		_SCL_SECURE_ALWAYS_VALIDATE_RANGE(_Current + _Off < _Size && _Current + _Off >= 0);
		return *(_Mycont + _Current + _Off);
	}

protected:
    void _Xran() const { // report an out_of_range error
        _THROW(std::out_of_range, "invalid checked_array_iterator<T> subscript");
    }

    void _Xinvarg() const { // report an invalid_argument error
        _THROW(std::invalid_argument, "invalid checked_array_iterator<T> argument");
    }

    _Iterator _Mycont; // points to the start of the array
    size_t _Current;   // the current index
    size_t _Size; // the size of array
	};

template <class _Iter>
checked_array_iterator<_Iter> make_checked_array_iterator(_Iter _Ptr, size_t _Size)
{
	return checked_array_iterator<_Iter>(_Ptr, _Size);
}

template<class _InIt,
	class _OutIt> inline
	_OutIt __CLRCALL_OR_CDECL unchecked_copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
		return (_STD _Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
			_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _InIt, class _OutIt>
inline
_IF_CHK(_OutIt) __CLRCALL_OR_CDECL checked_copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
	return (_STD _Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest, 
		_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _InIt, class _OutElem, size_t _Size>
inline
_OutElem* __CLRCALL_OR_CDECL checked_copy(_InIt _First, _InIt _Last, _OutElem (&_Dest)[_Size])
	{	// copy [_First, _Last) to [_Dest, ...)
	return checked_copy(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _STDEXT make_checked_array_iterator(_Dest, _Size)).base();
	}

template<class _InIt, class _OutIt>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK(_OutIt) __CLRCALL_OR_CDECL checked_copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// copy [_First, _Last) to [_Dest, ...)
	return (_STD _Copy_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest, 
		_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _BidIt1,
	class _BidIt2> inline
	_BidIt2 __CLRCALL_OR_CDECL unchecked_copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
		return (_STD _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
			_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _BidIt1,
	class _BidIt2> inline
_IF_CHK(_BidIt2) __CLRCALL_OR_CDECL checked_copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
	return _STD _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag());
	}

template<class _BidIt1,
	class _BidIt2> inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK(_BidIt2) __CLRCALL_OR_CDECL checked_copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// copy [_First, _Last) backwards to [..., _Dest)
	return _STD _Copy_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Ptr_cat(_First, _Dest), _STD _Range_checked_iterator_tag());
	}

template<class _InIt,
	class _OutIt> inline
	_OutIt __CLRCALL_OR_CDECL _Unchecked_move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
		return (_STD _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
			_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _InIt, class _OutIt>
inline
_IF_CHK(_OutIt) __CLRCALL_OR_CDECL _Checked_move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
	return _STD _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _InIt, class _OutElem, size_t _Size>
inline
_OutElem* __CLRCALL_OR_CDECL _Checked_move(_InIt _First, _InIt _Last, _OutElem (&_Dest)[_Size])
	{	// move [_First, _Last) to [_Dest, ...)
	return _Checked_move(_CHECKED_BASE(_First), _CHECKED_BASE(_Last),
		_STDEXT make_checked_array_iterator(_Dest, _Size)).base();
	}

template<class _InIt, class _OutIt>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK(_OutIt) __CLRCALL_OR_CDECL _Checked_move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{	// move [_First, _Last) to [_Dest, ...)
	return _STD _Move_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _BidIt1,
	class _BidIt2> inline
	_BidIt2 __CLRCALL_OR_CDECL _Unchecked_move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
		return (_STD _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
			_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag()));
	}

template<class _BidIt1, class _BidIt2>
inline
_IF_CHK(_BidIt2) __CLRCALL_OR_CDECL _Checked_move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
	return _STD _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _BidIt1, class _BidIt2>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK(_BidIt2) __CLRCALL_OR_CDECL _Checked_move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{	// move [_First, _Last) backwards to [..., _Dest)
	return _STD _Move_backward_opt(_CHECKED_BASE(_First), _CHECKED_BASE(_Last), _Dest,
		_STD _Iter_random(_First, _Dest), _STD _Move_cat(_Dest), _STD _Range_checked_iterator_tag());
	}

template<class _OutIt,
	class _Diff,
	class _Ty> inline
	void __CLRCALL_OR_CDECL unchecked_fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_STD _Fill_n(_First, _Count, _Val, _STD _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

template<class _OutIt,
	class _Diff,
	class _Ty>
inline
_IF_CHK_(_OutIt, void) __CLRCALL_OR_CDECL checked_fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_Fill_n(_First, _Count, _Val, _STD _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

template<class _OutElem,
	class _Diff,
	class _Ty, size_t _Size>
inline
void __CLRCALL_OR_CDECL checked_fill_n(_OutElem (&_First)[_Size], _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		checked_fill_n(_STDEXT make_checked_array_iterator( _First, _Size), _Count, _Val);
	}

template<class _OutIt,
	class _Diff,
	class _Ty>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK_(_OutIt, void) __CLRCALL_OR_CDECL checked_fill_n(_OutIt _First, _Diff _Count, const _Ty& _Val)
	{	// copy _Val _Count times through [_First, ...)
		_Fill_n(_First, _Count, _Val, _STD _Iter_cat(_First), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InIt2>
inline
	_STD pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL unchecked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
#if _SECURE_SCL
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
#else
	return _STD _Mismatch(_First1, _Last1, _First2,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
#endif
	}

template<class _InIt1, class _InIt2>
inline
_IF_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InElem2, size_t _Size>
inline
_STD pair<_InIt1, _InElem2*>
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size])
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	_STD pair<_InIt1, _STDEXT checked_array_iterator<_InElem2*> > _Result =
		checked_mismatch(_First1, _Last1, _STDEXT make_checked_array_iterator(_First2, _Size));
	return (_STD pair<_InIt1, _InElem2*>(_Result.first, _Result.second.base()));
	}

template<class _InIt1, class _InIt2>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
	_STD pair<_InIt1, _InIt2>
		__CLRCALL_OR_CDECL unchecked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch
#if _SECURE_SCL
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
#else
	return _STD _Mismatch(_First1, _Last1, _First2, _Pred,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
#endif
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_IF_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InElem2, class _Pr, size_t _Size>
inline
_STD pair<_InIt1, _InElem2*>
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size], _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	_STD pair<_InIt1, _STDEXT checked_array_iterator<_InElem2*> > _Result =
		checked_mismatch(_First1, _Last1, _STDEXT make_checked_array_iterator(_First2, _Size), _Pred);
	return (_STD pair<_InIt1, _InElem2*>(_Result.first, _Result.second.base()));
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK_RET_PAIR(_InIt2, _InIt1, _InIt2)
		__CLRCALL_OR_CDECL checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// return [_First1, _Last1) and [_First2, _Last2) mismatch using _Pred
	_STD pair<_CHECKED_BASE_TYPE(_InIt1), _InIt2> _Result =
		_STD _Mismatch(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
			_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	_ASSIGN_FROM_BASE(_First1, _Result.first);
	return (_STD pair<_InIt1, _InIt2>(_First1, _Result.second));
	}

template<class _InIt1, class _InIt2>
inline
	bool __CLRCALL_OR_CDECL unchecked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InIt2>
inline
_IF_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InElem2, size_t _Size>
inline
bool __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size])
	{	// compare [_First1, _Last1) to [First2, ...)
	return checked_equal(_First1, _Last1,
		_STDEXT make_checked_array_iterator(_First2, _Size));
	}

template<class _InIt1, class _InIt2>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
	bool __CLRCALL_OR_CDECL unchecked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...)
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_IF_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

template<class _InIt1, class _InElem2, class _Pr, size_t _Size>
inline
bool __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InElem2 (&_First2)[_Size], _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return checked_equal(_First1, _Last1,
		_STDEXT make_checked_array_iterator(_First2, _Size), _Pred);
	}

template<class _InIt1, class _InIt2, class _Pr>
inline
_SCL_CHECKED_ALGORITHM_WARN
_IF_NOT_CHK_(_InIt2, bool) __CLRCALL_OR_CDECL checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{	// compare [_First1, _Last1) to [First2, ...) using _Pred
	return _STD _Equal(_CHECKED_BASE(_First1), _CHECKED_BASE(_Last1), _First2, _Pred,
		_STD _Iter_random(_First1, _First2), _STD _Range_checked_iterator_tag());
	}

_STDEXT_END

/* _SECURE_CRT definitions */

 #define __STR2WSTR(str)    L##str
 #define _STR2WSTR(str)     __STR2WSTR(str)

 #define __FILEW__          _STR2WSTR(__FILE__)
 #define __FUNCTIONW__      _STR2WSTR(__FUNCTION__)

 #if !defined(_W64)
 #if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
 #define _W64 __w64
 #else
 #define _W64
 #endif
 #endif

 #ifndef _UINTPTR_T_DEFINED
 #ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
 #else
typedef _W64 unsigned int   uintptr_t;
 #endif
 #define _UINTPTR_T_DEFINED
 #endif

 #ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
 #define _WCHAR_T_DEFINED
 #endif

 #ifndef _CRTIMP

  #ifdef	_DLL
   #define _CRTIMP __declspec(dllimport)

  #else	/* ndef _DLL */
   #define _CRTIMP
  #endif	/* _DLL */

 #endif	/* _CRTIMP */

#ifdef _DEBUG 

#if !defined(_NATIVE_WCHAR_T_DEFINED) && defined(_M_CEE_PURE)
extern "C++"
#else
extern "C"
#endif
_CRTIMP void __cdecl _invalid_parameter(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t);

#else /* _DEBUG */

extern "C"
_CRTIMP void __cdecl _invalid_parameter_noinfo(void);

#endif /* def _DEBUG */


#ifdef  _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */

/*
 * Copyright (c) 1992-2005 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 */

/*
 * This file is derived from software bearing the following
 * restrictions:
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Hewlett-Packard Company makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 V4.05:0009 */
