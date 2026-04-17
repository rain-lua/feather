#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include "../../../include/Defines.hpp"

class Compositor;

class LayoutManager {
public:
    LayoutManager();

    float m_MasterFact;

    void Tile(Compositor *server);
};

#endif