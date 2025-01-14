#ifndef _SYSYF_INTERNAL_TYPES_H_
#define _SYSYF_INTERNAL_TYPES_H_

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include <set>

namespace SysYF
{
template <typename T>
using Ptr = std::shared_ptr<T>;

// Vector of reference of type
template <typename T>
using PtrVec = std::vector<Ptr<T>>;

// List of reference of type
template <typename T>
using PtrList = std::list<Ptr<T>>;

template <typename K, typename V, typename Comp = std::less<K>>
using Map = std::map<K, V, Comp>;

// Set of reference of type
template <typename T>
using PtrSet = std::set<Ptr<T>>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using WeakPtrCmp = std::owner_less<std::weak_ptr<T>>;

template <typename T>
using WeakPtrSet = std::set<std::weak_ptr<T>, WeakPtrCmp<T>>;

template <typename T>
bool WeakPtrSetEq(WeakPtrSet<T> &a, WeakPtrSet<T> &b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (auto &ptr: a) {
        if (b.find(ptr) == b.end()) {
            return false;
        }
    }
    return true;
}

template <typename T>
using WeakPtrList = std::list<std::weak_ptr<T>>;

/**
 * @brief a ∪ b
 */
template <typename T>
WeakPtrSet<T> setUnion(WeakPtrSet<T> &a, WeakPtrSet<T> &b) {
    WeakPtrSet<T> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::inserter(res, res.begin()), WeakPtrCmp<T>());
    return res;
}

/**
 * @brief a - b
 */
template <typename T>
WeakPtrSet<T> setDiff(WeakPtrSet<T> &a, WeakPtrSet<T> &b) {
    WeakPtrSet<T> res;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(res, res.begin()), WeakPtrCmp<T>());
    return res;
}

/**
 * @brief a ∩ b
 */
template <typename T>
WeakPtrSet<T> setIntersect(WeakPtrSet<T> &a, WeakPtrSet<T> &b) {
    WeakPtrSet<T> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(res, res.begin()), WeakPtrCmp<T>());
    return res;
}

template <typename K, typename V>
using WeakPtrMap = std::map<std::weak_ptr<K>, V, std::owner_less<std::weak_ptr<K>>>;

template <typename T>
using WeakPtrVec = std::vector<std::weak_ptr<T>>;

using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;

}


#endif // !_SYSYF_INTERNAL_TYPES_H_
