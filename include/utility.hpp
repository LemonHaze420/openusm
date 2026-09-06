// utility standard header
#pragma once
#ifndef _STDEXT_UTILITY_HPP_
#define _STDEXT_UTILITY_HPP_
#ifndef RC_INVOKED
#include <iosfwd>

#ifdef _MSC_VER
#pragma pack(push, _CRT_PACKING)
#pragma warning(push, 3)
#endif /* _MSC_VER */

#ifndef _STDEXT_STD_BEGIN
#define _STDEXT_STD_BEGIN namespace _std {
#endif

#ifndef _STDEXT_STD_END
#define _STDEXT_STD_END }
#endif

_STDEXT_STD_BEGIN
// TEMPLATE FUNCTION swap (from <algorithm>)
template <class _Ty>
inline void swap(_Ty &_Left, _Ty &_Right)
{  // exchange values stored at _Left and _Right
    _Ty _Tmp = _Left;
    _Left = _Right, _Right = _Tmp;
}

// TEMPLATE OPERATORS
namespace rel_ops {  // nested namespace to hide relational operators from std
template <class _Ty>
inline bool operator!=(const _Ty &_Left, const _Ty &_Right)
{  // test for inequality, in terms of equality
    return (!(_Left == _Right));
}

template <class _Ty>
inline bool operator>(const _Ty &_Left, const _Ty &_Right)
{  // test if _Left > _Right, in terms of operator<
    return (_Right < _Left);
}

template <class _Ty>
inline bool operator<=(const _Ty &_Left, const _Ty &_Right)
{  // test if _Left <= _Right, in terms of operator<
    return (!(_Right < _Left));
}

template <class _Ty>
inline bool operator>=(const _Ty &_Left, const _Ty &_Right)
{  // test if _Left >= _Right, in terms of operator<
    return (!(_Left < _Right));
}
}  // namespace rel_ops
_STDEXT_STD_END

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _STDEXT_UTILITY_HPP_ */
