#pragma once

namespace djinn::util {
    // Collection of wrappers around STL to make the syntax nicer
    // Mostly, this replaces all of the begin/end iterator stuff
    
    template <
        typename tContainer, 
        typename tElement
    >
    bool contains(
        const tContainer& c, 
        const tElement&   value
    );
    
    template <
        typename tContainer, 
        typename tCompareFn
    >
    bool contains_if(
        const tContainer& c, 
        tCompareFn        predicateFn
    );
    
    template <typename tContainer>
    bool contains_all(
        const tContainer& a, 
        const tContainer& b
    ); // returns true if A contains all of B
    
    template <
        typename tContainer,
        typename tElement
    >
    typename tContainer::const_iterator find(
        const tContainer& c, 
        const tElement&   value
    );
    
    template <
        typename tContainer, 
        typename tCompareFn
    >
    typename tContainer::const_iterator find_if(
        const tContainer& c, 
        tCompareFn&&      predicateFn = tCompareFn()
    );
    
    // sorted container variations
    template <
        typename tSortedContainer, 
        typename tElement
    >
    typename tSortedContainer::const_iterator binary_find(
        const tSortedContainer& c,
        const tElement& value
    );

    template <
        typename tSortedContainer, 
        typename tElement, 
        typename tCompareFn
    >
    typename tSortedContainer::const_iterator binary_find(
        const tSortedContainer& c,
        const tElement&         value,
        tCompareFn&&            predicateFn
    );

    template <
        typename tContainer,
        typename tElement
    >
    void erase(
        tContainer&     c, 
        const tElement& value
    );
    
    template <
        typename tContainer, 
        typename tCompareFn
    >
    void erase_if(
        tContainer& c, 
        tCompareFn  predicateFn
    );

    template <typename tContainer>
    void copy(
        const tContainer& source,
              tContainer& destination
    );

    template <
        typename tContainer, 
        typename tPredicateFn
    >
    void copy_if(
        const tContainer&    source,
              tContainer&    destination,
              tPredicateFn&& predicate
    );
    
    template <typename tContainer>
    void sort(tContainer& c);
    
    template <typename tContainer>
    void unique(tContainer& c);
}

#include "algorithm.inl"