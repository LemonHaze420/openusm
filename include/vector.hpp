// vector standard header
#pragma once

#include <stdexcept>

#include <algorithm>
#include <vector>

#ifndef _THROW
#define _THROW(err, str) throw err(str)
#endif

#ifndef _SCL_SECURE_VALIDATE
#define _SCL_SECURE_VALIDATE(a1)
#endif

#ifndef _SCL_SECURE_VALIDATE_RANGE
#define _SCL_SECURE_VALIDATE_RANGE(a1)
#endif

namespace _std {

template <class _Ty, class _Ax = std::allocator<_Ty>>
struct vector;

// base class for vector to hold allocator _Alval
template <class _Ty, class _Alloc>
struct _Vector_val {
protected:
    _Vector_val(_Alloc _Al = _Alloc()) : _Alval(_Al)
    {  // construct allocator from _Al
    }

    typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<_Ty> _Alty;

    _Alty _Alval;  // allocator object for values
};

// varying size array of values
template <class _Ty, class _Ax>
struct vector : public _Vector_val<_Ty, _Ax> {
    typedef vector<_Ty, _Ax> _Myt;
    typedef _Vector_val<_Ty, _Ax> _Mybase;
    typedef typename _Mybase::_Alty _Alloc;
    typedef _Alloc allocator_type;
    typedef typename _Alloc::size_type size_type;
    typedef typename _Alloc::difference_type _Dift;
    typedef _Dift difference_type;
    typedef typename _Alloc::value_type *_Tptr;
    typedef typename _Alloc::value_type const *_Ctptr;
    typedef _Tptr pointer;
    typedef _Ctptr const_pointer;
    typedef typename _Alloc::value_type &_Reft;
    typedef _Reft reference;
    typedef typename _Alloc::value_type const &const_reference;
    typedef typename _Alloc::value_type value_type;

    typedef typename std::vector<_Ty, _Alloc>::iterator iterator;
    typedef typename std::vector<_Ty, _Alloc>::const_iterator const_iterator;

#if 0

    static _VEC_ITER_BASE(iterator it) {
        return it._Myptr;
    }
#else
#define _VEC_ITER_BASE(it) &(*it)
#endif

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    vector() : _Mybase()
    {  // construct empty vector
        _Buy(0);
    }

    explicit vector(const _Alloc &_Al) : _Mybase(_Al)
    {  // construct empty vector with allocator
        _Buy(0);
    }

    explicit vector(size_type _Count) : _Mybase()
    {  // construct from _Count * _Ty()
        _Construct_n(_Count, _Ty());
    }

    vector(size_type _Count, const _Ty &_Val) : _Mybase()
    {  // construct from _Count * _Val
        _Construct_n(_Count, _Val);
    }

    vector(size_type _Count, const _Ty &_Val, const _Alloc &_Al) : _Mybase(_Al)
    {  // construct from _Count * _Val, with allocator
        _Construct_n(_Count, _Val);
    }

    vector(const _Myt &_Right) : _Mybase(_Right._Alval)
    {  // construct by copying _Right
        if (_Buy(_Right.size())) {
            m_last = _Ucopy(_Right.begin(), _Right.end(), m_first);
        }
    }

    template <class _Iter>
    vector(_Iter _First, _Iter _Last) : _Mybase()
    {  // construct from [_First, _Last)
        _Construct(_First, _Last, _Iter_cat(_First));
    }

    template <class _Iter>
    vector(_Iter _First, _Iter _Last, const _Alloc &_Al) : _Mybase(_Al)
    {  // construct from [_First, _Last), with allocator
        _Construct(_First, _Last, _Iter_cat(_First));
    }

    template <class _Iter>
    void _Construct(_Iter _Count, _Iter _Val)
    {  // initialize with _Count * _Val
        size_type _Size = (size_type)_Count;
        _Construct_n(_Size, (_Ty)_Val);
    }

    template <class _Iter>
    void _Construct(_Iter _First, _Iter _Last, std::input_iterator_tag)
    {  // initialize with [_First, _Last), input iterators
        _Buy(0);
        insert(begin(), _First, _Last);
    }

    void _Construct_n(size_type _Count, const _Ty &_Val)
    {                        // construct from _Count * _Val
        if (_Buy(_Count)) {  // nonzero, fill it
            m_last = _Ufill(m_first, _Count, _Val);
        }
    }

    ~vector()
    {  // destroy the object
        _Tidy();
    }

    _Myt &operator=(const _Myt &_Right)
    {                           // assign _Right
        if (this != &_Right) {  // worth doing

            if (_Right.size() == 0)
                clear();                         // new sequence empty, erase existing sequence
            else if (_Right.size() <= size()) {  // enough elements, copy new and destroy old
                pointer _Ptr = std::copy(_Right.m_first, _Right.m_last,
                                         m_first);  // copy new
                _Destroy(_Ptr, m_last);             // destroy old
                m_last = m_first + _Right.size();
            } else if (_Right.size() <= capacity()) {  // enough room, copy and construct new
                pointer _Ptr = _Right.m_first + size();
                std::copy(_Right.m_first, _Ptr, m_first);
                m_last = _Ucopy(_Ptr, _Right.m_last, m_last);
            } else {                       // not enough room, allocate new array and construct new
                if (m_first != nullptr) {  // discard old array
                    _Destroy(m_first, m_last);
                    this->_Alval.deallocate(m_first, m_end - m_first);
                }
                if (_Buy(_Right.size()))
                    m_last = _Ucopy(_Right.m_first, _Right.m_last, m_first);
            }
        }

        return (*this);
    }

    void reserve(size_type _Count)
    {  // determine new minimum length of allocated storage
        if (max_size() < _Count) {
            _Xlen();                       // result too long
        } else if (capacity() < _Count) {  // not enough room, reallocate
            pointer _Ptr = this->_Alval.allocate(_Count);

            _Umove(begin(), end(), _Ptr);

            size_type _Size = size();
            if (m_first != nullptr) {  // destroy and deallocate old array
                _Destroy(m_first, m_last);
                this->_Alval.deallocate(m_first, m_end - m_first);
            }

            m_end = _Ptr + _Count;
            m_last = _Ptr + _Size;
            m_first = _Ptr;
        }
    }

    // return current length of allocated storage
    size_type capacity() const
    {
        return (m_first == nullptr ? 0 : m_end - m_first);
    }

    // return iterator for beginning of mutable sequence
    iterator begin()
    {
        return (iterator(m_first));
    }

    // return iterator for beginning of nonmutable sequence
    const_iterator begin() const
    {
        return (const_iterator(m_first));
    }

    iterator end()
    {  // return iterator for end of mutable sequence
        return (iterator(m_last));
    }

    const_iterator end() const
    {  // return iterator for end of nonmutable sequence
        return (const_iterator(m_last));
    }

    reverse_iterator rbegin()
    {  // return iterator for beginning of reversed mutable sequence
        return (reverse_iterator(end()));
    }

    const_reverse_iterator rbegin() const
    {  // return iterator for beginning of reversed nonmutable sequence
        return (const_reverse_iterator(end()));
    }

    reverse_iterator rend()
    {  // return iterator for end of reversed mutable sequence
        return (reverse_iterator(begin()));
    }

    const_reverse_iterator rend() const
    {  // return iterator for end of reversed nonmutable sequence
        return (const_reverse_iterator(begin()));
    }

    void resize(size_type _Newsize)
    {  // determine new length, padding with _Ty() elements as needed
        resize(_Newsize, _Ty());
    }

    void resize(size_type _Newsize, _Ty _Val)
    {  // determine new length, padding with _Val elements as needed
        if (size() < _Newsize)
            _Insert_n(end(), _Newsize - size(), _Val);
        else if (_Newsize < size())
            erase(begin() + _Newsize, end());
    }

    // return length of sequence
    size_type size() const
    {
        return (m_first == nullptr ? 0 : m_last - m_first);
    }

    // return maximum possible length of sequence
    size_type max_size() const
    {
        return std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::max_size(this->_Alval);
    }

    bool empty() const
    {  // test if sequence is empty
        return (size() == 0);
    }

    _Alloc get_allocator() const
    {  // return allocator object for values
        return (this->_Alval);
    }

    const_reference at(size_type _Pos) const
    {  // subscript nonmutable sequence with checking
        if (size() <= _Pos)
            _Xran();
        return (*(begin() + _Pos));
    }

    reference at(size_type _Pos)
    {  // subscript mutable sequence with checking
        if (size() <= _Pos)
            _Xran();
        return (*(begin() + _Pos));
    }

    // subscript nonmutable sequence
    const_reference operator[](size_type _Pos) const
    {
        return (*(m_first + _Pos));
    }

    // subscript mutable sequence
    reference operator[](size_type _Pos)
    {
        return (*(m_first + _Pos));
    }

    reference front()
    {  // return first element of mutable sequence
        return (*begin());
    }

    const_reference front() const
    {  // return first element of nonmutable sequence
        return (*begin());
    }

    reference back()
    {  // return last element of mutable sequence
        return (*(end() - 1));
    }

    const_reference back() const
    {  // return last element of nonmutable sequence
        return (*(end() - 1));
    }

    // insert element at end
    void push_back(const _Ty &_Val)
    {
        if (size() < capacity()) {
            m_last = _Ufill(m_last, 1, _Val);
        } else {
            insert(end(), _Val);
        }
    }

    void pop_back()
    {                    // erase element at end
        if (!empty()) {  // erase last element
            _Destroy(m_last - 1, m_last);
            --m_last;
        }
    }

    template <class _Iter>
    void assign(_Iter _First, _Iter _Last)
    {  // assign [_First, _Last)
        _Assign(_First, _Last, _Iter_cat(_First));
    }

    template <class _Iter>
    void _Assign(_Iter _Count, _Iter _Val)
    {  // assign _Count * _Val
        _Assign_n((size_type)_Count, (_Ty)_Val);
    }

    template <class _Iter>
    void _Assign(_Iter _First, _Iter _Last, std::input_iterator_tag)
    {  // assign [_First, _Last), input iterators
        erase(begin(), end());
        insert(begin(), _First, _Last);
    }

    void assign(size_type _Count, const _Ty &_Val)
    {  // assign _Count * _Val
        _Assign_n(_Count, _Val);
    }

    iterator insert(iterator _Where, const _Ty &_Val)
    {  // insert _Val at _Where
        size_type _Off = size() == 0 ? 0 : _Where - begin();
        _Insert_n(_Where, (size_type)1, _Val);
        return (begin() + _Off);
    }

    void insert(iterator _Where, size_type _Count, const _Ty &_Val)
    {  // insert _Count * _Val at _Where
        _Insert_n(_Where, _Count, _Val);
    }

    template <class _Iter>
    void insert(iterator _Where, _Iter _First, _Iter _Last)
    {  // insert [_First, _Last) at _Where
        _Insert(_Where, _First, _Last, _Iter_cat(_First));
    }

    template <class _Iter>
    void _Insert(iterator _Where, _Iter _First, _Iter _Last)
    {  // insert _Count * _Val at _Where
        _Insert_n(_Where, (size_type)_First, (_Ty)_Last);
    }

    template <class _Iter>
    void _Insert(iterator _Where, _Iter _First, _Iter _Last, std::input_iterator_tag)
    {  // insert [_First, _Last) at _Where, input iterators
        for (; _First != _Last; ++_First, ++_Where)
            _Where = insert(_Where, *_First);
    }

    template <class _Iter>
    void _Insert(iterator _Where, _Iter _First, _Iter _Last, std::forward_iterator_tag)
    {  // insert [_First, _Last) at _Where, forward iterators

        size_type _Count = 0;
        _Distance(_First, _Last, _Count);
        size_type _Capacity = capacity();

        if (_Count == 0)
            ;
        else if (max_size() - size() < _Count)
            _Xlen();                             // result too long
        else if (_Capacity < size() + _Count) {  // not enough room, reallocate
            _Capacity = max_size() - _Capacity / 2 < _Capacity ? 0 : _Capacity + _Capacity / 2;  // try to grow by 50%
            if (_Capacity < size() + _Count) {
                _Capacity = size() + _Count;
            }
            pointer _Newvec = this->_Alval.allocate(_Capacity);
            pointer _Ptr = _Newvec;

            _Ptr = _Umove(m_first, _VEC_ITER_BASE(_Where),
                          _Newvec);                        // copy prefix
            _Ptr = _Ucopy(_First, _Last, _Ptr);            // add new stuff
            _Umove(_VEC_ITER_BASE(_Where), m_last, _Ptr);  // copy suffix

            _Count += size();
            if (m_first != 0) {  // destroy and deallocate old array
                _Destroy(m_first, m_last);
                this->_Alval.deallocate(m_first, m_end - m_first);
            }

            m_end = _Newvec + _Capacity;
            m_last = _Newvec + _Count;
            m_first = _Newvec;
        } else if ((size_type)(end() - _Where) < _Count) {  // new stuff spills off end
            _Umove(_VEC_ITER_BASE(_Where), m_last,
                   _VEC_ITER_BASE(_Where) + _Count);  // copy suffix
            _Iter _Mid = _First;
            advance(_Mid, end() - _Where);

            _Ucopy(_Mid, _Last, m_last);  // insert new stuff off end

            m_last += _Count;

            std::copy(_First, _Mid,
                      _VEC_ITER_BASE(_Where));  // insert to old end
        } else {                                // new stuff can all be assigned
            pointer _Oldend = m_last;
            m_last = _Umove(_Oldend - _Count, _Oldend,
                            m_last);  // copy suffix
            std::move_backward(_VEC_ITER_BASE(_Where), _Oldend - _Count,
                               _Oldend);  // copy hole

            std::copy(_First, _Last,
                      _VEC_ITER_BASE(_Where));  // insert into hole
        }
    }

    iterator erase(iterator _Where)
    {  // erase element at where
        std::copy(_VEC_ITER_BASE(_Where) + 1, m_last, _VEC_ITER_BASE(_Where));
        _Destroy(m_last - 1, m_last);
        --m_last;
        return (_Where);
    }

    iterator erase(iterator _First, iterator _Last)
    {                           // erase [_First, _Last)
        if (_First != _Last) {  // worth doing, copy down over hole

            pointer _Ptr = std::copy(_VEC_ITER_BASE(_Last), m_last, _VEC_ITER_BASE(_First));

            _Destroy(_Ptr, m_last);
            m_last = _Ptr;
        }

        return (_First);
    }

    void clear()
    {  // erase all
        erase(begin(), end());
    }

    void swap(_Myt &_Right)
    {                                         // exchange contents with _Right
        if (this->_Alval == _Right._Alval) {  // same allocator, swap control information

            std::swap(m_first, _Right.m_first);
            std::swap(m_last, _Right.m_last);
            std::swap(m_end, _Right.m_end);
        } else {  // different allocator, do multiple assigns
            _Myt _Ts = *this;
            *this = _Right, _Right = _Ts;
        }
    }

    void _Assign_n(size_type _Count, const _Ty &_Val)
    {                     // assign _Count * _Val
        _Ty _Tmp = _Val;  // in case _Val is in sequence
        erase(begin(), end());
        insert(begin(), _Count, _Tmp);
    }

    // allocate array with _Capacity elements
    bool _Buy(size_type _Capacity)
    {
        m_first = nullptr, m_last = nullptr, m_end = nullptr;
        if (_Capacity == 0) {
            return false;
        } else if (max_size() < _Capacity) {
            _Xlen();  // result too long
        } else {      // nonempty array, allocate storage
            m_first = this->_Alval.allocate(_Capacity);
            m_last = m_first;
            m_end = m_first + _Capacity;
        }

        return true;
    }

    void _Destroy(pointer _First, pointer _Last)
    {  // destroy [_First, _Last) using allocator
        for (; _First != _Last; ++_First) {
            std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::destroy(this->_Alval, _First);
        }
    }

    void _Tidy()
    {                              // free all storage
        if (m_first != nullptr) {  // something to free, destroy and deallocate it

            _Destroy(m_first, m_last);
            this->_Alval.deallocate(m_first, m_end - m_first);
        }
        m_first = nullptr, m_last = nullptr, m_end = nullptr;
    }

    template <class _Iter>
    pointer _Ucopy(_Iter _First, _Iter _Last, pointer _Dest)
    {  // copy initializing [_First, _Last), using allocator

        pointer _Next = _Dest;

        try {
            for (; _First != _Last; ++_Dest, ++_First) {
                std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::construct(this->_Alval, _Dest, *_First);
            }
        } catch (...) {
            for (; _Next != _Dest; ++_Next) {
                std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::destroy(this->_Alval, _Next);
            }

            throw;
        }

        return (_Dest);
    }

    template <class _Iter>
    pointer _Umove(_Iter _First, _Iter _Last, pointer _Ptr)
    {  // move initializing [_First, _Last), using allocator

        return _Ucopy(std::make_move_iterator(_First), std::make_move_iterator(_Last), _Ptr);
    }

    void _Insert_n(iterator _Where, size_type _Count, const _Ty &_Val)
    {  // insert _Count * _Val at _Where

        _Ty _Tmp = _Val;  // in case _Val is in sequence
        size_type _Capacity = capacity();

        if (_Count == 0) {
            ;
        } else if (max_size() - size() < _Count) {
            _Xlen();                               // result too long
        } else if (_Capacity < size() + _Count) {  // not enough room, reallocate
            _Capacity = max_size() - _Capacity / 2 < _Capacity ? 0 : _Capacity + _Capacity / 2;  // try to grow by 50%
            if (_Capacity < size() + _Count) {
                _Capacity = size() + _Count;
            }
            pointer _Newvec = this->_Alval.allocate(_Capacity);
            pointer _Ptr = _Newvec;

            _Ptr = _Umove(m_first, _VEC_ITER_BASE(_Where),
                          _Newvec);                        // copy prefix
            _Ptr = _Ufill(_Ptr, _Count, _Tmp);             // add new stuff
            _Umove(_VEC_ITER_BASE(_Where), m_last, _Ptr);  // copy suffix

            _Count += size();
            if (m_first != nullptr) {  // destroy and deallocate old array
                _Destroy(m_first, m_last);
                this->_Alval.deallocate(m_first, m_end - m_first);
            }

            m_end = _Newvec + _Capacity;
            m_last = _Newvec + _Count;
            m_first = _Newvec;
        } else if ((size_type)(m_last - _VEC_ITER_BASE(_Where)) < _Count) {  // new stuff spills off end
            _Umove(_VEC_ITER_BASE(_Where), m_last,
                   _VEC_ITER_BASE(_Where) + _Count);  // copy suffix

            _Ufill(m_last, _Count - (m_last - _VEC_ITER_BASE(_Where)),
                   _Tmp);  // insert new stuff off end

            m_last += _Count;

            std::fill(_VEC_ITER_BASE(_Where), m_last - _Count,
                      _Tmp);  // insert up to old end
        } else {              // new stuff can all be assigned
            pointer _Oldend = m_last;
            m_last = _Umove(_Oldend - _Count, _Oldend,
                            m_last);  // copy suffix

            std::move_backward(_VEC_ITER_BASE(_Where), _Oldend - _Count,
                               _Oldend);  // copy hole
            std::fill(_VEC_ITER_BASE(_Where), _VEC_ITER_BASE(_Where) + _Count,
                      _Tmp);  // insert into hole
        }
    }

    pointer _Ufill(pointer _First, size_type _Count, const _Ty &_Val)
    {  // copy initializing _Count * _Val, using allocator
        pointer _Next = _First;

        using alloc_type = std::allocator_traits<std::decay_t<decltype(this->_Alval)>>;

        try {
            for (; 0 < _Count; --_Count, ++_First) {
                alloc_type::construct(this->_Alval, _First, _Val);
            }
        } catch (...) {
            for (; _Next != _First; ++_Next) {
                alloc_type::destroy(this->_Alval, _Next);
            }
            throw;
        }

        return (_First + _Count);
    }

    static void _Xlen()
    {  // report a length_error
        _THROW(std::length_error, "vector<T> too long");
    }

    static void _Xran()
    {  // report an out_of_range error
        _THROW(std::out_of_range, "invalid vector<T> subscript");
    }

    static void _Xinvarg()
    {  // report an invalid_argument error
        _THROW(std::invalid_argument, "invalid vector<T> argument");
    }

    pointer m_first;  // pointer to beginning of array
    pointer m_last;   // pointer to current end of sequence
    pointer m_end;    // pointer to end of array
};

// vector TEMPLATE FUNCTIONS
template <class _Ty, class _Alloc>
inline bool operator==(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test for vector equality
    return (_Left.size() == _Right.size() && equal(_Left.begin(), _Left.end(), _Right.begin()));
}

template <class _Ty, class _Alloc>
inline bool operator!=(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test for vector inequality
    return (!(_Left == _Right));
}

template <class _Ty, class _Alloc>
inline bool operator<(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test if _Left < _Right for vectors
    return (lexicographical_compare(_Left.begin(), _Left.end(), _Right.begin(), _Right.end()));
}

template <class _Ty, class _Alloc>
inline bool operator>(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test if _Left > _Right for vectors
    return (_Right < _Left);
}

template <class _Ty, class _Alloc>
inline bool operator<=(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test if _Left <= _Right for vectors
    return (!(_Right < _Left));
}

template <class _Ty, class _Alloc>
inline bool operator>=(const vector<_Ty, _Alloc> &_Left, const vector<_Ty, _Alloc> &_Right)
{  // test if _Left >= _Right for vectors
    return (!(_Left < _Right));
}

template <class _Ty, class _Alloc>
inline void swap(vector<_Ty, _Alloc> &_Left, vector<_Ty, _Alloc> &_Right)
{  // swap _Left and _Right vectors
    _Left.swap(_Right);
}

}  // namespace _std
