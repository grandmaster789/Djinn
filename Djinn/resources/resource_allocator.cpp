#include "resource_allocator.h"

namespace djinn::resources {
    // --------------------------------------
    //  ResourceID
    // --------------------------------------
    ResourceID::ResourceID(uint32_t value) :
        m_Value(value)
    {
    }

    uint32_t ResourceID::operator()() const {
        return m_Value;
    }

    // --------------------------------------
    //  ResourceAllocatorBase
    // --------------------------------------
    bool ResourceAllocatorBase::isAllocated(ResourceID id) const {
        return m_Table.test(id());
    }

    size_t ResourceAllocatorBase::getNumAllocated() const {
        return m_Table.count();
    }

    size_t ResourceAllocatorBase::getCapacity() const {
        return m_Table.size();
    }

    ResourceAllocatorBase::ResourceAllocatorBase(size_t capacity):
        m_Table(capacity)
    {
    }
}