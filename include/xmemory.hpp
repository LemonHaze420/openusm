// xmemory internal header (from <memory>)
#pragma once
#ifndef _XMEMORY_
#define _XMEMORY_
#ifndef RC_INVOKED
#include <cstdlib>
#include <exception>
#include <new>
#include <xutility.hpp>

#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)

 #pragma warning(disable: 4100)
#endif  /* _MSC_VER */

#ifndef _FARQ	/* specify standard memory model */
 #define _FARQ
 #define _PDFT	ptrdiff_t
 #define _SIZT	size_t
#endif /* _FARQ */

#define _DESTRUCTOR(ty, ptr) (ptr)->~ty()

#define _THROW0()

#define _CRTIMP2_PURE

_STD_BEGIN
// TEMPLATE FUNCTION _Allocate
template<class _Ty>
inline _Ty _FARQ *_Allocate(_SIZT _Count, _Ty _FARQ *) { // check for integer overflow
    if (_Count <= 0)
        _Count = 0;
    else if (((_SIZT)(-1) / _Count) < sizeof(_Ty))
        throw std::bad_alloc();

    // allocate storage for _Count elements of type _Ty
    return ((_Ty _FARQ *) ::operator new(_Count * sizeof(_Ty)));
}

// TEMPLATE FUNCTION _Construct
template<class _T1, class _T2>
inline void _Construct(_T1 _FARQ *_Ptr,
                       const _T2 &_Val) { // construct object at _Ptr with value _Val
    void _FARQ *_Vptr = _Ptr;
    ::new (_Vptr) _T1(_Val);
}

// TEMPLATE FUNCTION _Destroy
template<class _Ty>
inline void _Destroy(_Ty _FARQ *_Ptr) { // destroy object at _Ptr
    _DESTRUCTOR(_Ty, _Ptr);
}

template<>
inline void _Destroy(char _FARQ *) { // destroy a char (do nothing)
}

template<>
inline void _Destroy(wchar_t _FARQ *) { // destroy a wchar_t (do nothing)
}

		// TEMPLATE FUNCTION _Destroy_range
template<class _Ty,
	class _Alloc> inline
	void _Destroy_range(_Ty *_First, _Ty *_Last, _Alloc& _Al)
	{	// destroy [_First, _Last)
	_Destroy_range(_First, _Last, _Al, _Ptr_cat(_First, _Last));
	}

template<class _Ty,
	class _Alloc> inline
	void _Destroy_range(_Ty *_First, _Ty *_Last, _Alloc& _Al,
		_Nonscalar_ptr_iterator_tag)
	{	// destroy [_First, _Last), arbitrary type
	for (; _First != _Last; ++_First)
		_Al.destroy(_First);
	}

template<class _Ty,
	class _Alloc> inline
	void _Destroy_range(_Ty *, _Ty *, _Alloc& ,
		_Scalar_ptr_iterator_tag)
	{	// destroy [_First, _Last), scalar type (do nothing)
	}
_STD_END

#ifdef  _MSC_VER
  #pragma warning(default: 4100)

#pragma warning(pop)
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _XMEMORY_ */
