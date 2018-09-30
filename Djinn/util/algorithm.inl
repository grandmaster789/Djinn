#pragma once

#include "algorithm.h"
#include <algorithm>

namespace djinn::util {
    template <typename C, typename T>
    bool contains(const C& c, const T& value) {
    	using std::find;
    	using std::begin;
    	using std::end;
    
    	return (find(begin(c), end(c), value) != end(c));
    }
    
    template <typename C, typename Fn>
    bool contains_if(const C& c, Fn predicateFn) {
        using std::find_if;
        using std::begin;
        using std::end;
    
        return (find_if(begin(c), end(c),  predicateFn) != end(c));
    }
    
    template <typename C>
    bool contains_all(const C& a, const C& b) {
        for (const auto& value : b)
            if (!contains(a, value))
                return false;
    
        return true;
    }
    
    template <typename tContainer, typename tElement>
    typename tContainer::const_iterator find(const tContainer& c, const tElement& value) {
        using std::find;
        using std::begin;
        using std::end;
    
        return find(begin(c), end(c), value);
    }
    
    template <typename tContainer, typename tCompareFn>
    typename tContainer::const_iterator find_if(const tContainer& c, tCompareFn&& predicateFn) {
        using std::find_if;
        using std::begin;
        using std::end;
    
        return find_if(begin(c), end(c), predicateFn);
    }

    template <typename tSortedContainer, typename tElement>
    typename tSortedContainer::const_iterator binary_find(
        const tSortedContainer& c,
        const tElement& value
    ) {
        using std::lower_bound;
        using std::begin;
        using std::end;

        return lower_bound(begin(c), end(c), value);
    }
    
    template <typename tContainer, typename tElement>
    void erase(tContainer& c, const tElement& value) {
        auto it = find(c, value);
    
        if (it != std::end(c))
            c.erase(it);
    }
    
    template <typename tContainer, typename tCompareFn>
    void erase_if(tContainer& c, tCompareFn predicateFn) {
        auto it = find_if(c, predicateFn);
    
        if (it != std::end(c))
            c.erase(it);
    }
    
    template <typename tContainer>
    void sort(tContainer& c) {
        using namespace std;
        
        sort(
            begin(c), 
            end(c)
        );
    }
    
    template <typename tContainer>
    void unique(tContainer& c) {
        using namespace std;
    
        sort(c);
        
        auto it = unique(
            begin(c), 
            end(c)
        );
    
        c.erase(it, end(c));
    }
}
