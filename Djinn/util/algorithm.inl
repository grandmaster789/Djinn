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
    bool contains_if(const C& container, Fn&& predicateFn) {
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
    void copy(const C& source, C& destination) {
        using std::copy;
        using std::begin;
        using std::end;
        using std::back_inserter;

        copy(
            begin(source),
            end(source),
            back_inserter(destination)
        );
    }

    template <typename C, typename P>
    void copy_if(const C& source, C& destination, P&& predicateFn) {
        using std::copy_if;
        using std::begin;
        using std::end;
        using std::back_inserter;

        copy_if(
            begin(source),
            end(source),
            back_inserter(destination),
            predicateFn
        );
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

    namespace detail {
        template <typename C, typename T, typename...Vs>
        std::optional<typename C::value_type> prefer_impl(
            const C& options,
            const T& candidate,
            const Vs&... vs
        ) {
            static_assert(std::is_same_v<typename C::value_type, T>);

            if (contains(options, candidate))
                return candidate;
                
            if constexpr (sizeof...(Vs) > 0) {
                return prefer_impl(options, vs...);
            }

            return std::nullopt;
        };
    }

    template <typename tContainer, typename...tValues>
    std::optional<typename tContainer::value_type> prefer(
        const tContainer& available_options,
        const tValues&... preferred
    ) {
        if constexpr (sizeof...(tValues) == 0) {
            return std::nullopt;
        }

        return detail::prefer_impl(available_options, preferred...);
    }

    template <typename T, size_t N>
    constexpr uint32_t CountOf(T(&)[N]) {
        assert(N < std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(N);
    }

	template <
		typename T,
		typename It,
		typename
	>
	constexpr auto enumerate(T&& iterable) {
		struct Iterator {
			bool operator != (const Iterator& it) const {
				return m_Iterator != it.m_Iterator;
			}

			void operator ++ () {
				++m_Index;
				++m_Iterator;
			}

			auto operator* () const {
				return std::tie(m_Index, *m_Iterator);
			}

			size_t m_Index;
			It     m_Iterator;
		};

		struct IteratorWrapper {
			T m_Iterable;

			auto begin() {
				return Iterator{ 0, std::begin(m_Iterable) };
			}

			auto end() {
				return Iterator{ 0, std::end(m_Iterable) };
			}
		};

		return IteratorWrapper{
			std::forward<T>(iterable)
		};
	}
}
