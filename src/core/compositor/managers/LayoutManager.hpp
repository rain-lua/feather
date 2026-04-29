#pragma once

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