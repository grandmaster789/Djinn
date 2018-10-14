#pragma once

#include "flat_map.h"
#include "algorithm.h"
#include <iterator> // for std::distance
#include <ostream>

namespace djinn::util {
    template <typename K, typename V>
    const V* FlatMap<K, V>::operator[](const K& k) const {
        // [NOTE] could make an util::algorithm version of lower_bound
        using namespace std;
        
        auto it = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            k
        );

        if (it == end(m_Keys))
            return nullptr;
        else {
            auto index = distance(begin(m_Keys), it);
            return &m_Values[index];
        }
    }

    template <typename K, typename V>
    std::pair<K, V> FlatMap<K, V>::at(int index) const {
        return std::make_pair(
            m_Keys[index], 
            m_Values[index]
        );
    }

    template <typename K, typename V>
    void FlatMap<K, V>::assign(const K& key, const V& value) {
        using namespace std;

        // [NOTE] could make an util::algorithm version of lower_bound
        auto lower = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            key
        );

        if (lower != end(m_Keys)) {
            // lower_bound found some location that is either
            // 1) where the exact key was found
            //   or
            // 2) the insertion point where it should be
            //
            auto index = distance(begin(m_Keys), lower);

            if (*lower == key)
                m_Values[index] = value;
            else {
                m_Keys.insert(lower, key);
                m_Values.insert(begin(m_Values) + index, value);
            }

            return;
        }

        // lower_bound reached the end
        m_Keys.push_back(key);
        m_Values.push_back(value);
    }

    template <typename K, typename V>
    void FlatMap<K, V>::assign(const K& key, V&& value) {
        using namespace std;

        // [NOTE] could make an util::algorithm version of lower_bound
        auto lower = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            key
        );

        if (lower != end(m_Keys)) {
            // lower_bound found some location that is either
            // 1) where the exact key was found
            //   or
            // 2) the insertion point where it should be
            //
            auto index = distance(begin(m_Keys), lower);

            if (*lower == key)
                m_Values[index] = std::forward<V>(value);
            else {
                m_Keys.insert(lower, key);
                m_Values.insert(begin(m_Values) + index, std::forward<V>(value));
            }

            return;
        }

        // lower_bound reached the end
        m_Keys.push_back(key);
        m_Values.push_back(std::forward<V>(value));
    }

    template <typename K, typename V>
    bool FlatMap<K, V>::insert(const K& key, const V& value) {
        using namespace std;

        // [NOTE] could make an util::algorithm version of lower_bound
        auto lower = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            key
        );

        if (lower != end(m_Keys)) {
            // lower_bound found some location that is either
            // 1) where the exact key was found
            //   or
            // 2) the insertion point where it should be
            //
            auto index = distance(begin(m_Keys), lower);

            if (*lower == key)
                return false;
            else {
                m_Keys.insert(lower, key);
                m_Values.insert(begin(m_Values) + index, value);
                
                return true;
            }
        }

        // lower_bound reached the end
        m_Keys.push_back(key);
        m_Values.push_back(value);

        return true;
    }

    template <typename K, typename V>
    bool FlatMap<K, V>::insert(const K& key, V&& value) {
        using namespace std;

        // [NOTE] could make an util::algorithm version of lower_bound
        auto lower = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            key
        );

        if (lower != end(m_Keys)) {
            // lower_bound found some location that is either
            // 1) where the exact key was found
            //   or
            // 2) the insertion point where it should be
            //
            auto index = distance(begin(m_Keys), lower);

            if (*lower == key)
                return false;
            else {
                m_Keys.insert(lower, key);
                m_Values.insert(begin(m_Values) + index, std::forward<V>(value));
                return true;
            }
        }

        // lower_bound reached the end
        m_Keys.push_back(key);
        m_Values.push_back(std::forward<V>(value));

        return true;
    }

    template <typename K, typename V>
    bool FlatMap<K, V>::contains(const K& key) const {
        return util::contains(m_Keys, key);
    }

    template <typename K, typename V>
    void FlatMap<K, V>::erase(const K& key) {
        using namespace std;

        // [NOTE] could make an util::algorithm version of lower_bound
        auto it = lower_bound(
            begin(m_Keys),
            end(m_Keys),
            key
        );

        if (it == end(m_Keys))
            return;

        auto index = distance(begin(m_Keys), it);

        m_Keys.erase(it);
        m_Values.erase(begin(m_Values) + index);
    }

    template <typename K, typename V>
    void FlatMap<K, V>::clear() {
        m_Keys.clear();
        m_Values.clear();
    }

    template <typename K, typename V>
    size_t FlatMap<K, V>::size() const {
        return m_Keys.size();
    }

    template <typename K, typename V>
    const std::vector<K>& FlatMap<K, V>::getKeys() const {
        return m_Keys;
    }

    template <typename K, typename V>
    const std::vector<V>& FlatMap<K, V>::getValues() const {
        return m_Values;
    }

    template <typename K, typename V>
    void FlatMap<K, V>::foreach(KeyValueCallback&& callback) {
        for (size_t i = 0; i < m_Keys.size(); ++i)
            callback(m_Keys[i], m_Values[i]);
    }

    template <typename K, typename V>
    std::ostream& operator << (std::ostream& os, const FlatMap<K, V>& fm) {
        os << "[FlatMap]:\n";

        fm.foreach([&](const K& key, const V& value) {
            os << "\t" << key << " = " << value << "\n";
        });

        return os;
    }
}
    