#pragma once

#include <stdexcept>

#define _POINTER_X(T, A) typename std::allocator_traits<A>::template rebind_alloc<T>::value_type *
#define _REFERENCE_X(T, A) typename std::allocator_traits<A>::template rebind_alloc<T>::value_type &
#define _GENERIC_BASE _Node

#ifndef _THROW
#define _THROW(err, str) throw err(str)
#endif

namespace _std {

/*
 * base class for _List_ptr to hold allocator _Alnod 
 * 
 */
template<class _Ty, class _Alloc>
struct _List_nod
//: public _Container_base {
{
    struct _Node;
    friend struct _Node;
    typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<_GENERIC_BASE>::value_type * _Genptr;

    struct _Node {     // list node
        _Genptr _Next; // successor node, or first element if head
        _Genptr _Prev; // predecessor node, or last element if head
        _Ty _Myval;    // the stored value, unused if head
    };

    _List_nod(_Alloc _Al) : _Alnod(_Al) { // construct allocator from _Al
    }

    typename std::allocator_traits<_Alloc>::template rebind_alloc<_Node> _Alnod; // allocator object for nodes
};

template<class _Ty, class _Alloc>
struct _List_ptr
    : public _List_nod<_Ty, _Alloc> { // base class for _List_val to hold allocator _Alptr

    typedef typename _List_nod<_Ty, _Alloc>::_Node _Node;
    typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<_Node>::value_type * _Nodeptr;

    _List_ptr(_Alloc _Al)
        : _List_nod<_Ty, _Alloc>(_Al), _Alptr(_Al) { // construct base, and allocator from _Al
    }

    typename std::allocator_traits<_Alloc>::template rebind_alloc<_Nodeptr> _Alptr; // allocator object for pointers to nodes
};

template<class _Ty, class _Alloc>
struct _List_val : public _List_ptr<_Ty, _Alloc> { // base class for list to hold allocator _Alval

    typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<_Ty> _Alty;

    _List_val(_Alloc _Al = _Alloc())
        : _List_ptr<_Ty, _Alloc>(_Al), _Alval(_Al) { // construct base, and allocator from _Al
    }

    _Alty _Alval; // allocator object for values stored in nodes
};

template<typename _Ty, typename _Ax = std::allocator<_Ty>>
struct list : public _List_val<_Ty, _Ax> {
    typedef list<_Ty, _Ax> _Myt;
    typedef _List_val<_Ty, _Ax> _Mybase;
    typedef typename _Mybase::_Alty _Alloc;

    typedef typename _List_nod<_Ty, _Ax>::_Genptr _Genptr;
    typedef typename _List_nod<_Ty, _Ax>::_Node _Node;
    typedef _POINTER_X(_Node, _Alloc) _Nodeptr;
    typedef _REFERENCE_X(_Nodeptr, _Alloc) _Nodepref;
    typedef typename _Alloc::value_type & _Vref;

    // return reference to successor pointer in node
    static _Nodepref _Nextnode(_Nodeptr _Pnode) {
        return ((_Nodepref) _Pnode->_Next);
    }

    // return reference to predecessor pointer in node
    static _Nodepref _Prevnode(_Nodeptr _Pnode) {
        return ((_Nodepref) _Pnode->_Prev);
    }

    // return reference to value in node
    static _Vref _Myval(_Nodeptr _Pnode) {
        return ((_Vref) _Pnode->_Myval);
    }

    typedef _Alloc allocator_type;
    typedef typename _Alloc::size_type size_type;
    typedef typename _Alloc::difference_type _Dift;
    typedef _Dift difference_type;
    typedef typename _Alloc::value_type * _Tptr;
    typedef const typename _Alloc::value_type * _Ctptr;
    typedef _Tptr pointer;
    typedef _Ctptr const_pointer;
    typedef typename _Alloc::value_type & _Reft;
    typedef _Reft reference;
    typedef const typename _Alloc::value_type & const_reference;
    typedef typename _Alloc::value_type value_type;

    // iterator for nonmutable list
    class _Const_iterator {
    public:
        typedef _Const_iterator _Myt_iter;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef _Ty value_type;
        typedef _Dift difference_type;
        typedef _Ctptr pointer;
        typedef const_reference reference;

        // construct with null node pointer
        _Const_iterator() : _Ptr(nullptr) {}

        // construct with node pointer _Pnode
        _Const_iterator(_Nodeptr _Pnode) : _Ptr(_Pnode) {}

        const_reference operator*() const { // return designated value
            return (_Myval(_Ptr));
        }

        _Ctptr operator->() const { // return pointer to class object
            return (&**this);
        }

        _Myt_iter &operator++() { // preincrement

            _Ptr = _Nextnode(_Ptr);
            return (*this);
        }

        _Myt_iter operator++(int) { // postincrement
            _Myt_iter _Tmp = *this;
            ++*this;
            return (_Tmp);
        }

        _Myt_iter &operator--() { // predecrement
            _Ptr = _Prevnode(_Ptr);

            return (*this);
        }

        _Myt_iter operator--(int) { // postdecrement
            _Myt_iter _Tmp = *this;
            --*this;
            return (_Tmp);
        }

        bool operator==(const _Myt_iter &_Right) const { // test for iterator equality

            return (_Ptr == _Right._Ptr);
        }

        bool operator!=(const _Myt_iter &_Right) const { // test for iterator inequality
            return (!(*this == _Right));
        }

        _Nodeptr _Mynode() const { // return node pointer
            return (_Ptr);
        }

        _Nodeptr _Ptr; // pointer to node
    };

    typedef _Const_iterator const_iterator;

    // iterator for mutable list
    class _Iterator : public _Const_iterator {
    public:
        friend class list<_Ty, _Ax>;
        typedef _Iterator _Myt_iter;
        typedef _Const_iterator _Mybase_iter;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef _Ty value_type;
        typedef _Dift difference_type;
        typedef _Tptr pointer;
        typedef _Reft reference;

        _Iterator() { // construct with null node
        }

        // construct with node pointer _Pnode
        _Iterator(_Nodeptr _Pnode) : _Mybase_iter(_Pnode) {}

        reference operator*() const { // return designated value
            return (_Myval(this->_Ptr));
        }

        _Tptr operator->() const { // return pointer to class object
            return (&**this);
        }

        _Myt_iter &operator++() { // preincrement
            ++(*(_Mybase_iter *) this);
            return (*this);
        }

        _Myt_iter operator++(int) { // postincrement
            _Myt_iter _Tmp = *this;
            ++*this;
            return (_Tmp);
        }

        _Myt_iter &operator--() { // predecrement
            --(*(_Mybase_iter *) this);
            return (*this);
        }

        _Myt_iter operator--(int) { // postdecrement
            _Myt_iter _Tmp = *this;
            --*this;
            return (_Tmp);
        }
    };

    typedef _Iterator iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // construct empty list
    list() : _Mybase(), m_head(_Buynode()), m_size(0) {}

    // construct list by copying _Right
    list(const _Myt &_Right) : _Mybase(_Right._Alval), m_head(_Buynode()), m_size(0) {
        insert(begin(), _Right.begin(), _Right.end());
    }

    list(const _Alloc &_Right) : _Mybase(_Right), m_head(_Buynode()), m_size(0) {
    }

    // destroy the object
    ~list() {
        _Tidy();
    }

    // assign _Right
    _Myt &operator=(const _Myt &_Right) {
        if (this != &_Right) {
            assign(_Right.begin(), _Right.end());
        }

        return (*this);
    }

    iterator begin() { // return iterator for beginning of mutable sequence
        return (iterator(_Nextnode(m_head)));
    }

    const_iterator begin() const { // return iterator for beginning of nonmutable sequence
        return (const_iterator(_Nextnode(m_head)));
    }

    iterator end() { // return iterator for end of mutable sequence
        return (iterator(m_head));
    }

    const_iterator end() const { // return iterator for end of nonmutable sequence
        return (const_iterator(m_head));
    }

    reverse_iterator rbegin() { // return iterator for beginning of reversed mutable sequence
        return (reverse_iterator(end()));
    }

    const_reverse_iterator rbegin()
        const { // return iterator for beginning of reversed nonmutable sequence
        return (const_reverse_iterator(end()));
    }

    reverse_iterator rend() { // return iterator for end of reversed mutable sequence
        return (reverse_iterator(begin()));
    }

    const_reverse_iterator rend() const { // return iterator for end of reversed nonmutable sequence
        return (const_reverse_iterator(begin()));
    }

    void resize(size_type _Newsize) { // determine new length, padding with _Ty() elements as needed
        resize(_Newsize, _Ty());
    }

    // determine new length, padding with _Val elements as needed
    void resize(size_type _Newsize, _Ty _Val) {
        if (m_size < _Newsize) {
            _Insert_n(end(), _Newsize - m_size, _Val);
        } else {
            while (_Newsize < m_size) {
                pop_back();
            }
        }
    }

    size_type size() const { // return length of sequence
        return m_size;
    }

    size_type max_size() const { // return maximum possible length of sequence
        return std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::max_size(this->_Alval);
    }

    bool empty() const { // test if sequence is empty
        return (m_size == 0);
    }

    allocator_type get_allocator() const { // return allocator object for values
        return (this->_Alval);
    }

    reference front() { // return first element of mutable sequence
        return (*begin());
    }

    const_reference front() const { // return first element of nonmutable sequence
        return (*begin());
    }

    reference back() { // return last element of mutable sequence
        return (*(--end()));
    }

    const_reference back() const { // return last element of nonmutable sequence
        return (*(--end()));
    }

    void push_front(const _Ty &_Val) { // insert element at beginning
        _Insert(begin(), _Val);
    }

    // erase element at beginning
    void pop_front() {
        this->erase(this->begin());
    }

    void push_back(const _Ty &_Val) { // insert element at end
        _Insert(end(), _Val);
    }

    void pop_back() { // erase element at end
        erase(--end());
    }

    template<class _Iter>
    void assign(_Iter _First, _Iter _Last) { // assign [_First, _Last)
        typename std::iterator_traits<_Iter>::iterator_category cat;

        _Assign(_First, _Last, cat);
    }

    template<class _Iter>
    void _Assign(_Iter _Count, _Iter _Val) { // assign _Count * _Val
        _Assign_n((size_type) _Count, (_Ty) _Val);
    }

    template<class _Iter>
    void _Assign(_Iter _First,
                 _Iter _Last,
                 std::input_iterator_tag) { // assign [_First, _Last), input iterators
        clear();
        insert(begin(), _First, _Last);
    }

    void assign(size_type _Count, const _Ty &_Val) { // assign _Count * _Val
        _Assign_n(_Count, _Val);
    }

    // insert _Val at _Where
    iterator insert(iterator _Where, const _Ty &_Val) {
        _Insert(_Where, _Val);
        return (--_Where);
    }

    // insert _Val at _Where
    void _Insert(iterator _Where, const _Ty &_Val) {
        _Nodeptr _Pnode = _Where._Mynode();
        _Nodeptr _Newnode = _Buynode(_Pnode, _Prevnode(_Pnode), _Val);
        _Incsize(1);
        _Prevnode(_Pnode) = _Newnode;
        _Nextnode(_Prevnode(_Newnode)) = _Newnode;
    }

    void insert(iterator _Where,
                size_type _Count,
                const _Ty &_Val) { // insert _Count * _Val at _Where
        _Insert_n(_Where, _Count, _Val);
    }

    template<class _Iter>
    void insert(iterator _Where, _Iter _First, _Iter _Last) { // insert [_First, _Last) at _Where

        typename std::iterator_traits<_Iter>::iterator_category cat;

        _Insert(_Where, _First, _Last, cat);
    }

    template<class _Iter>
    void _Insert(iterator _Where,
                 _Iter _Count,
                 _Iter _Val) { // insert _Count * _Val at _Where
        _Insert_n(_Where, (size_type) _Count, (_Ty) _Val);
    }

    template<class _Iter>
    void _Insert(iterator _Where,
                 _Iter _First,
                 _Iter _Last,
                 std::input_iterator_tag) { // insert [_First, _Last) at _Where, input iterators
        size_type _Num = 0;

        for (; _First != _Last; ++_First, ++_Num) {
            _Insert(_Where, *_First);
        }
    }

    template<class _Iter>
    void _Insert(iterator _Where,
                 _Iter _First,
                 _Iter _Last,
                 std::forward_iterator_tag) { // insert [_First, _Last) at _Where, forward iterators

        for (; _First != _Last; ++_First) {
            _Insert(_Where, *_First);
        }
    }

    // erase element at _Where
    iterator erase(iterator _Where)
    {
        _Nodeptr _Pnode = (_Where++)._Mynode();

        if (_Pnode != m_head) { // not list head, safe to erase
            _Nextnode(_Prevnode(_Pnode)) = _Nextnode(_Pnode);
            _Prevnode(_Nextnode(_Pnode)) = _Prevnode(_Pnode);

            using alloc_type = typename std::allocator_traits<std::decay_t<decltype(this->_Alnod)>>;
            alloc_type::destroy(this->_Alnod, _Pnode);
            alloc_type::deallocate(this->_Alnod, _Pnode, 1);
            --this->m_size;
        }

        return _Where;
    }

    iterator erase(iterator _First, iterator _Last) { // erase [_First, _Last)
        if (_First == begin() && _Last == end()) {    // erase all and return fresh iterator
            clear();
            return (end());
        } else { // erase subrange
            while (_First != _Last) {
                _First = erase(_First);
            }

            return (_Last);
        }
    }

    void _Splice(iterator _Where,
                 _Myt &_Right,
                 iterator _First,
                 iterator _Last,
                 size_type _Count,
                 [[maybe_unused]] bool _Keep = false) { // splice _Right [_First, _Last) before _Where

        if (this->_Alval == _Right._Alval) { // same allocator, just relink

            if (this != &_Right) { // splicing from another list, adjust counts
                _Incsize(_Count);
                _Right.m_size -= _Count;
            }
            _Nextnode(_Prevnode(_First._Mynode())) = _Last._Mynode();
            _Nextnode(_Prevnode(_Last._Mynode())) = _Where._Mynode();
            _Nextnode(_Prevnode(_Where._Mynode())) = _First._Mynode();
            _Nodeptr _Pnode = _Prevnode(_Where._Mynode());
            _Prevnode(_Where._Mynode()) = _Prevnode(_Last._Mynode());
            _Prevnode(_Last._Mynode()) = _Prevnode(_First._Mynode());
            _Prevnode(_First._Mynode()) = _Pnode;
        } else { // different allocator, copy nodes then erase source
            insert(_Where, _First, _Last);
            _Right.erase(_First, _Last);
        }
    }

    // free all storage
    void _Tidy()
    {
        clear();
        std::allocator_traits<std::decay_t<decltype(this->_Alptr)>>::destroy(this->_Alptr, &_Nextnode(m_head));
        std::allocator_traits<std::decay_t<decltype(this->_Alptr)>>::destroy(this->_Alptr, &_Prevnode(m_head));
        std::allocator_traits<std::decay_t<decltype(this->_Alnod)>>::deallocate(this->_Alnod, m_head, 1);
        m_head = nullptr;
    }

    // alter element count, with checking
    void _Incsize(size_type _Count) {
        if (max_size() - m_size < _Count) {
            _THROW(std::length_error, "list<T> too long");
        }

        m_size += _Count;
    }

    // erase all
    void clear() {
        _Nodeptr _Pnext;
        _Nodeptr _Pnode = _Nextnode(m_head);
        _Nextnode(m_head) = m_head;
        _Prevnode(m_head) = m_head;
        m_size = 0;

        for (; _Pnode != m_head; _Pnode = _Pnext) { // delete an element
            _Pnext = _Nextnode(_Pnode);
            std::allocator_traits<std::decay_t<decltype(this->_Alnod)>>::destroy(this->_Alnod, _Pnode);
            std::allocator_traits<std::decay_t<decltype(this->_Alnod)>>::deallocate(this->_Alnod, _Pnode, 1);
        }
    }

    // allocate a head node and set links
    _Nodeptr _Buynode() {
        _Nodeptr _Pnode = this->_Alnod.allocate(1);

        using alloc_type = typename std::allocator_traits<std::decay_t<decltype(this->_Alptr)>>;
        alloc_type::construct(this->_Alptr, &_Nextnode(_Pnode), _Pnode);

        alloc_type::construct(this->_Alptr, &_Prevnode(_Pnode), _Pnode);

        return (_Pnode);
    }

    // allocate a node and set links and value
    _Nodeptr _Buynode(_Nodeptr _Next, _Nodeptr _Prev, const _Ty &_Val) {
        _Nodeptr _Pnode = this->_Alnod.allocate(1);

        std::allocator_traits<std::decay_t<decltype(this->_Alptr)>>::construct(this->_Alptr, &_Nextnode(_Pnode), _Next);

        std::allocator_traits<std::decay_t<decltype(this->_Alptr)>>::construct(this->_Alptr, &_Prevnode(_Pnode), _Prev);

        std::allocator_traits<std::decay_t<decltype(this->_Alval)>>::construct(this->_Alval, &_Myval(_Pnode), _Val);
        return (_Pnode);
    }

    _Nodeptr m_head;  // pointer to head node
    size_type m_size; // number of elements
};
} // namespace _std

#undef _GENERIC_BASE
#undef _REFERENCE_X
#undef _POINTER_X
