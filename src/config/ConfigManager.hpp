#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <lua.hpp>
#include <string>
#include <unordered_map>
#include <memory>

class Leaf {
public:
    enum Type { INT, FLOAT, BOOL, STRING };

    Leaf(int v) : m_Type(INT), m_I(v) {}
    Leaf(float v) : m_Type(FLOAT), m_F(v) {}
    Leaf(bool v) : m_Type(BOOL), m_B(v) {}
    Leaf(const std::string& v) : m_Type(STRING), m_S(v) {}

    Type m_Type;

    int m_I;
    float m_F;
    bool m_B;
    std::string m_S;

    void SetFromLua(lua_State* L, int idx);
};

class Tree {
public:
    std::unordered_map<std::string, std::unique_ptr<Tree>> m_Trees;
    std::unordered_map<std::string, std::unique_ptr<Leaf>> m_Leaves;

    Tree* GetTree(const std::string& key);
    Leaf* GetLeaf(const std::string& key);

    Tree* AddTree(const std::string& key);
    Leaf* AddLeaf(const std::string& key, Leaf leaf);
};

class ConfigManager {
public:
    ConfigManager();

    void Initialize();
    void Cleanup();
    
    bool Load(const std::string& path);
    void EnsureUserConfigExists();

    Tree* Root();

    int GetInt(const std::string& path);
    float GetFloat(const std::string& path);
    bool GetBool(const std::string& path);
    std::string GetString(const std::string& path);

    std::string m_ConfigPath;

    lua_State* m_State;
    std::unique_ptr<Tree> m_RootTree;

    Leaf* GetLeafFromPath(const std::string& path);

    void ParseTable(int index, Tree* node);
    void ParseValue(int index, Leaf* leaf);
};

#endif