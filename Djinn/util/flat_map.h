#pragma once

#include <vector>
#include <utility>
#include <iosfwd>
#include <functional>

namespace djinn::util {
    // Thin wrapper around std::lower_bound and std::vector, should be pretty fast for
    // most 'normal' key-value lookups. Keeps the keys in sorted order.
    // 
    // [NOTE] Key must implement < (less) and == (equals). Value should be at least movable
    // [NOTE] not threadsafe, but should be fine  
    // [NOTE] could possibly customize the sorting order, but that would cause a bunch 
    //        of template noise
    //
    // [TODO] implement FlatMapIterator, compatibility with ranged-for loops
    //
    template <typename K, typename V>
    class FlatMap {
    public:
        using Key              = K;
        using Value            = V;
        using KeyValueCallback = std::function<void(const K&, const V&)>;
        
        FlatMap() = default;

        const V* operator[](const K& k) const; // will return nullptr if not found
        std::pair<K, V> at(int index) const; // will throw if index is out of bounds
        
        void assign_or_insert(const K& key, const V&  value);
        void assign_or_insert(const K& key,       V&& value);
        bool contains(const K& key) const;
        void erase(const K& key);
        void clear();

        size_t size() const;

        const std::vector<K>& getKeys() const;
        const std::vector<V>& getValues() const;

        void foreach(KeyValueCallback&& callback);

    private:
        std::vector<Key> m_Keys;
        std::vector<Value> m_Values;
    };

    template <typename K, typename V>
    std::ostream& operator << (std::ostream& os, const FlatMap<K, V>& fm);
}

#include "flat_map.inl"