#pragma once

#include <cassert>

namespace djinn::resources {
    // --------------------------------------
    //  ResourceAllocator
    // --------------------------------------
    template <typename T>
    ResourceAllocator<T>::ResourceAllocator(size_t capacity) :
        ResourceAllocatorBase(capacity),
        m_Resources(capacity)
    {
    }

    template <typename T>
    ResourceID ResourceAllocator<T>::allocate() {
        size_t id = m_Table.findNext(0, m_Table.size(), false);
        assert(id != m_Table.size()); // if this triggers, we ran out of capacity        
        m_Table.set(id);
        return ResourceID(id);
    }

    template <typename T>
    void ResourceAllocator<T>::allocateID(ResourceID id) {
        assert(!isAllocated(id));

    }

    template <typename T>
    void ResourceAllocator<T>::deallocate(ResourceID id) {
        assert(isAllocated(id));
        m_Table.set(id(), false);
    }

    template <typename T>
    T* ResourceAllocator<T>::get(ResourceID id) {
        assert(isAllocated(id));
        return &m_Resources[id()];
    }

    template <typename T>
    const T* ResourceAllocator<T>::get(ResourceID id) const {
        assert(isAllocated(id));
        return &m_Resources[id()];
    }

    // --------------------------------------
    //  Handle
    // --------------------------------------
    template <typename T>
    Handle<T>::Handle(ResourceID id, ResourceAllocator<T>* allocator) :
        m_ID(id),
        m_Allocator(allocator)
    {
    }

    template <typename T>
    bool Handle<T>::operator == (const Handle& h) const {
        return
            (m_ID        == h.m_ID) &&
            (m_Allocator == h.m_Allocator);
    }
    
    template <typename T>
    bool Handle<T>::operator != (const Handle& h) const {
        return
            (m_ID        != h.m_ID) ||
            (m_Allocator != h.m_Allocator);
    }

    template <typename T>
    T* Handle<T>::operator -> () const {
        return m_Allocator->get(m_ID);
    }

    template <typename T>
    void Handle<T>::deallocate() {
        assert(m_Allocator && m_Allocator->isAllocated(m_ID));

        m_Allocator->deallocate(m_ID);

        m_ID = 0;
        m_Allocator = nullptr;
    }

    template <typename T>
    bool Handle<T>::isValid() const {
        return
            (m_Allocator) &&
            (m_ID() < m_Allocator->getCapacity()) &&
            (m_Allocator->isAllocated(m_ID));
    }
}
