#include "Listener.hpp"

Listener::Listener() : m_owner(nullptr), m_callback(nullptr) {
    m_listener.notify = &Listener::Notify;
    wl_list_init(&m_listener.link);
}

Listener::~Listener() {
    Remove();
}

void Listener::Init(wl_signal* signal, void* owner, Callback callback) {
    if (IsConnected()) {
        Remove();
    }

    m_owner    = owner;
    m_callback = callback;

    wl_signal_add(signal, &m_listener);
}

void Listener::Remove() {
    if (!IsConnected()) {
        return;
    }

    wl_list_remove(&m_listener.link);
    wl_list_init(&m_listener.link);
}

bool Listener::IsConnected() const {
    return !wl_list_empty(&m_listener.link);
}

void Listener::Notify(wl_listener* listener, void* data) {
    Listener* wrapper = wl_container_of(listener, wrapper, m_listener);
    wrapper->m_callback(wrapper->m_owner, data);
}