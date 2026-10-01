#include "Listener.hpp"

Listener::Listener() : m_Owner(nullptr), m_Callback(nullptr) {
    m_Listener.notify = &Listener::Notify;
    wl_list_init(&m_Listener.link);
}

Listener::~Listener() {
    Remove();
}

void Listener::Init(wl_signal* signal, void* owner, Callback callback) {
    if (IsConnected()) {
        Remove();
    }

    m_Owner = owner;
    m_Callback = callback;

    wl_signal_add(signal, &m_Listener);
}

void Listener::Remove() {
    if (!IsConnected()) {
        return;
    }

    wl_list_remove(&m_Listener.link);
    wl_list_init(&m_Listener.link);
}

bool Listener::IsConnected() const {
    return !wl_list_empty(&m_Listener.link);
}

void Listener::Notify(wl_listener* listener, void* data) {
    Listener* wrapper = wl_container_of(listener, wrapper, m_Listener);
    wrapper->m_Callback(wrapper->m_Owner, data);
}