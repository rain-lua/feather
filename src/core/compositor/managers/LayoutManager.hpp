#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include "../../../include/Defines.hpp"

class Compositor;

class LayoutManager {
public:
    LayoutManager();

    void Initialize();
    void Cleanup();
    
    std::string m_Layout;
    float m_MasterFact;

    void Tile();
};

#endif