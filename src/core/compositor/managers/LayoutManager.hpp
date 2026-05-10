#pragma once

#include "../../../include/Defines.hpp"

class LayoutManager {
public:
    LayoutManager();
    ~LayoutManager();
    
    std::string m_Layout;
    float m_MasterFact;

    void Tile();
};