// utility standard header
#pragma once
#ifndef _UTILITY_
#define _UTILITY_
#ifndef RC_INVOKED
#include <iosfwd>

#ifdef _MSC_VER
#pragma pack(push, _CRT_PACKING)
#pragma warning(push, 3)
#endif /* _MSC_VER */

#ifndef _STD_BEGIN
#define _STD_BEGIN namespace _std {
#endif

#ifndef _STD_END
#define _STD_END }
#endif

_STD_BEGIN
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
_STD_END

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _UTILITY_ */

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
