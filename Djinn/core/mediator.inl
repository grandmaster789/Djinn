#pragma once

#include "mediator.h"

namespace djinn {
    template <typename T, typename H>
    void add_handler(H* handler) {
        core::detail::MediatorQueue<T>::instance().add(handler);
    }

    template <typename T, typename H>
    void remove_handler(H* handler) {
        core::detail::MediatorQueue<T>::instance().remove(handler);
    }

    template <typename T>
    void remove_all_handlers() {
        core::detail::MediatorQueue<T>::instance().removeAll();
    }

    template <typename T>
    void broadcast(const T& message) {
        core::detail::MediatorQueue<T>::instance().broadcast(message);
    }

    template <typename T>
    MessageHandler<T>::MessageHandler() {
        add_handler<T>(this);
    }

    template <typename T>
    MessageHandler<T>::~MessageHandler() {
        remove_handler<T>(this);
    }
}