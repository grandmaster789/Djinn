#pragma once

#include "algorithm.h"
#include <algorithm>

namespace djinn::util {
    template <typename C, typename T>
    bool contains(const C& container, const T& value) {
    	using std::find;
    	using std::begin;
    	using std::end;
    
    	return find(begin(container), end(container), value) != end(container);
    }
    
    template <typename C, typename Fn>
    bool contains_if(const C& container, Fn predicateFn) {
        using std::find_if;
        using std::begin;
        using std::end;
    
        return find_if(begin(container), end(container),  predicateFn) != end(container);
    }
    
    template <typename C>
    bool contains_all(const C& container_a, const C& container_b) {
        for (const auto& value : container_b)
            if (!contains(container_a, value))
                return false;
    
        return true;
    }
    
    template <typename C, typename E>
    typename C::const_iterator find(const C& container, const E& value) {
        using std::find;
        using std::begin;
        using std::end;
    
        return find(begin(container), end(container), value);
    }
    
    template <typename C, typename P>
    typename C::const_iterator find_if(const C& container, P&& predicateFn) {
        using std::find_if;
        using std::begin;
        using std::end;
    
        return find_if(begin(container), end(container), predicateFn);
    }

    template <typename C, typename E>
    typename C::const_iterator binary_find(
        const C& container,
        const E& value
    ) {
        using std::lower_bound;
        using std::begin;
        using std::end;

        return lower_bound(begin(container), end(container), value);
    }

    template <
        typename C, 
        typename E,
        typename P
    >
    typename C::const_iterator binary_find(
        const C& container,
        const E& value,
        P&&      predicateFn
    ) {
        using std::lower_bound;
        using std::begin;
        using std::end;

        return lower_bound(begin(container), end(container), value, predicateFn);
    }
    
    template <typename C, typename E>
    void erase(C& container, const E& value) {
        auto it = find(container, value);
    
        if (it != std::end(container))
            container.erase(it);
    }
    
    template <typename C, typename P>
    void erase_if(C& container, P predicateFn) {
        auto it = find_if(container, predicateFn);
    
        if (it != std::end(container))
            container.erase(it);
    }
    
    template <typename C>
    void sort(C& container) {
        using namespace std;
        
        sort(
            begin(container),
            end(container)
        );
    }
    
    template <typename C>
    void unique(C& container) {
        using namespace std;
    
        sort(container);
        
        auto it = unique(
            begin(container),
            end(container)
        );
    
        container.erase(it, end(container));
    }
}
