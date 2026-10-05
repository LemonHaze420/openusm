#pragma once

#include "xtree.hpp"

namespace _std {
template <class T, class Compare, class Alloc>
class _Tset_traits {
public:
    using key_type = T;
    using value_type = T;
    using key_compare = Compare;
    using value_compare = Compare;
    using allocator_type = Alloc;
    using _ITptr = _POINTER_X(value_type, allocator_type);
    using _IReft = _REFERENCE_X(value_type, allocator_type);
    enum { _Multi = false };

    _Tset_traits() : comp() {}
    explicit _Tset_traits(Compare compare) : comp(compare) {}
    static const T &_Kfn(const T &value) { return value; }
    Compare comp;
};

template <class T, class Compare = std::less<T>, class Alloc = std::allocator<T>>
class set : public _Tree<_Tset_traits<T, Compare, Alloc>> {
    using base = _Tree<_Tset_traits<T, Compare, Alloc>>;
public:
    using ret_t = std::pair<typename base::iterator, bool>;
    set() : base(Compare(), Alloc()) {}
};
static_assert(sizeof(set<void *>) == sizeof(void *) * 3);
}  // namespace _std
