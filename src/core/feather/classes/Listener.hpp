#pragma once

#include "../../../include/Defines.hpp"

class Listener {
  public:
    using Callback = void (*)(void*, void*);

    Listener();
    ~Listener();

    Listener(const Listener&)            = delete;
    Listener(Listener&&)                 = delete;
    Listener& operator=(const Listener&) = delete;
    Listener& operator=(Listener&&)      = delete;

    void Init(wl_signal* signal, void* owner, Callback callback);

    void Remove();

    [[nodiscard]] bool IsConnected() const;

  private:
    static void Notify(wl_listener* listener, void* data);

    wl_listener m_Listener;
    void* m_Owner;
    Callback m_Callback;
};