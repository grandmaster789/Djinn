#pragma once

#include <functional>
#include <iosfwd>
#include <utility>
#include <vector>

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

        V*              operator[](const K& k);        // will return nullptr if not found (!)
        const V*        operator[](const K& k) const;  // will return nullptr if not found
        std::pair<K, V> at(int index) const;           // will throw if index is out of bounds

        void               assign(const K& key, const V& value);
        void               assign(const K& key, V&& value);
        [[nodiscard]] bool insert(
            const K& key,
            const V& value);  // returns false if this would overwrite an entry
        [[nodiscard]] bool insert(
            const K& key,
            V&&      value);  // returns false if this would overwrite an entry

        bool contains(const K& key) const;
        void erase(const K& key);
        void clear() noexcept;

        size_t size() const noexcept;

        const std::vector<K>& getKeys() const noexcept;
        const std::vector<V>& getValues() const noexcept;

        void foreach (const KeyValueCallback& callback) const;

    private:
        std::vector<Key>   m_Keys;
        std::vector<Value> m_Values;
    };

    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& os, const FlatMap<K, V>& fm);
}  // namespace djinn::util

#include "flat_map.inl"
