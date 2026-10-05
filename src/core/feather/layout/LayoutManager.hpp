#pragma once

#include "../../../include/Defines.hpp"

class LayoutManager {
  public:
    LayoutManager();
    ~LayoutManager() = default;

    std::string m_layout;
    float       m_masterFact;

    void        Tile();
};