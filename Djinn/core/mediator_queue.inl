#pragma once

#include "mediator_queue.h"
#include "util/algorithm.h"
#include "logger.h"

#include <algorithm>
#include <iterator> // for std::distance

namespace djinn::core::detail {
    template <typename T>
    MediatorQueue<T>& MediatorQueue<T>::instance() {
        static MediatorQueue<T> specific;
        return specific;
    }

    template <typename T>
    template <typename H>
    void MediatorQueue<T>::add(H* handler) {
        LockGuard guard(m_Mutex);
        
        m_Handlers.push_back(
            [handler](const T& message) { 
                (*handler)(message); 
            }
        );
        m_Originals.push_back(handler);
    }

    template <typename T>
    template <typename H>
    void MediatorQueue<T>::remove(H* handler) {
        using namespace std;

        LockGuard guard(m_Mutex);

        auto it = find(
            begin(m_Originals),
            end(m_Originals),
            handler
        );

        if (it != end(m_Originals)) {
            size_t index = distance(begin(m_Originals), it);

            m_Originals.erase(it);
            m_Handlers.erase(begin(m_Handlers) + index);
        }
        else
            gLogError << "Tried to remove an unregistered message handler";
    }

    template <typename T>
    void MediatorQueue<T>::removeAll() {
        LockGuard guard(m_Mutex);

        m_Handlers.clear();
        m_Originals.clear();
    }

    template <typename T>
    void MediatorQueue<T>::broadcast(const T& message) {
        std::vector<Handler> localCopy;

        {
            LockGuard guard(m_Mutex);
            localCopy = m_Handlers;
        }

        for (const auto& handler : localCopy)
            handler(message);
    }
}