#pragma once

#include <optional>
#include <vector>

namespace djinn::util {
    // Collection of wrappers around STL to make the syntax nicer
    // Mostly, this replaces all of the begin/end iterator stuff
    
    // given a container, yield true if it contains the given element
    template <typename tContainer, typename tElement>
    bool contains(
        const tContainer& container, 
        const tElement&   value
    );
    
    // given a container, yield true if the predicate yields true
    // for any element of the container
    template <typename tContainer, typename tCompareFn>
    bool contains_if(
        const tContainer&  container, 
              tCompareFn&& predicateFn
    );
    
    // returns true if A contains all of B
    template <typename tContainer>
    bool contains_all(
        const tContainer& a, 
        const tContainer& b
    ); 
    
    // yield an iterator to the location of the searched-for element
    // or std::end(container) if it wasn't found
    template <typename tContainer, typename tElement>
    typename tContainer::const_iterator find(
        const tContainer& container, 
        const tElement&   value
    );
    
    // yield an iterator to the first location where the given predicate
    // resulted in true when evaluating it with the element at the indicated
    // position. If none of the elements evaluate to true, this yields
    // std::end(container)
    template <typename tContainer, typename tCompareFn>
    typename tContainer::const_iterator find_if(
        const tContainer&  container, 
              tCompareFn&& predicateFn = tCompareFn()
    );
    
    // sorted container variations
    template <typename tSortedContainer, typename tElement>
    typename tSortedContainer::const_iterator binary_find(
        const tSortedContainer& c,
        const tElement& value
    );

    template <typename tSortedContainer, typename tElement, typename tCompareFn>
    typename tSortedContainer::const_iterator binary_find(
        const tSortedContainer& container,
        const tElement&         value,
              tCompareFn&&      predicateFn
    );

    template <typename tContainer, typename tElement>
    void erase(
              tContainer& container, 
        const tElement&   value
    );
    
    template <typename tContainer, typename tCompareFn>
    void erase_if(
        tContainer& c, 
        tCompareFn  predicateFn
    );

    template <typename tContainer>
    void copy(
        const tContainer& source,
              tContainer& destination
    );

    template <typename tContainer, typename tPredicateFn>
    void copy_if(
        const tContainer&    source,
              tContainer&    destination,
              tPredicateFn&& predicate
    );
    
    template <typename tContainer>
    void sort(tContainer& c);
    
    template <typename tContainer>
    void unique(tContainer& c);

    // given a set of options, select one if it is available, 
    // with any given number of fallbacks and nullopt if none 
    // of the fallbacks work
    //
    // [NOTE] my current implementation results in warnings (4702: unreachable code)
    //        and i'm not sure why
    template <typename tContainer, typename...tValues>
    std::optional<typename tContainer::value_type> prefer(
        const tContainer& available_options,
        const tValues&... preferred
    );
}

#pragma warning(push)
#pragma warning(disable: 4702) // unreachable code
    #include "algorithm.inl"
#pragma warning(pop)