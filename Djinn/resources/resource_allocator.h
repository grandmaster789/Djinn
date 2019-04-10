#pragma once

#include <cstdint>
#include <vector>
#include "util/dynamic_bitset.h"

namespace djinn::resources {
    /*
        Note -- this is not threadsafe. Should be pretty fast though
    */

    class ResourceID {
    public:
        ResourceID() = default;
        
        ResourceID             (const ResourceID&) = default;
        ResourceID& operator = (const ResourceID&) = default;
        ResourceID             (ResourceID&&)      = default;
        ResourceID& operator = (ResourceID&&)      = default;

        explicit ResourceID(uint32_t value);

        uint32_t operator()() const;

    private:
        uint32_t m_Value = 0;
    };

	class ResourceAllocatorBase	{
	public:
		virtual ~ResourceAllocatorBase() = default;

		bool isAllocated(ResourceID id) const;

		virtual ResourceID allocate()                = 0;
		virtual void       allocateID(ResourceID id) = 0;
		virtual void       deallocate()              = 0;

		virtual void resize(size_t count) = 0;

		size_t getNumAllocated() const;
		size_t getCapacity() const;

	protected:
        ResourceAllocatorBase(size_t capacity);

        util::DynamicBitset m_Table;
	};

    /*
        Resources are stored in std::vector and thus should be:
        - default constructible
        - movable
    */
    template <typename T>
    class ResourceAllocator:
        public ResourceAllocatorBase
    {
    public:
        ResourceAllocator(size_t capacity);
        virtual ~ResourceAllocator() = default;

        ResourceID allocate()                override;
        void       allocateID(ResourceID id) override;
        void       deallocate(ResourceID id) override;

              T* get(ResourceID id);
        const T* get(ResourceID id) const;

    private:
        std::vector<T> m_Resources;
    };

    // TODO make this a smart-ptr like type?
    // TODO less, hash, ostream?
    template <typename T>
    class Handle {
    public:
        Handle() = default;

        Handle             (const Handle&) = default;
        Handle& operator = (const Handle&) = default;
        Handle             (Handle&&)      = default;
        Handle& operator = (Handle&&)      = default;

        Handle(ResourceID id, ResourceAllocator<T>* allocator);

        bool operator == (const Handle& h) const;
        bool operator != (const Handle& h) const;

        T* operator -> () const;

        void deallocate();
        bool isValid() const;

    private:
        ResourceID            m_ID        = ResourceID(0);
        ResourceAllocator<T>* m_Allocator = nullptr;
    };
}

#include "resource_allocator.inl"