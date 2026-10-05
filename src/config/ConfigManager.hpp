#pragma once

#include <fcntl.h>
#include <lua.hpp>
#include <memory>
#include <optional>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

class Leaf {
  public:
    enum Type { INT, FLOAT, BOOL, STRING };

    Leaf(int v) : m_type(INT), m_I(v) {}
    Leaf(float v) : m_type(FLOAT), m_F(v) {}
    Leaf(bool v) : m_type(BOOL), m_B(v) {}
    Leaf(const std::string& v) : m_type(STRING), m_S(v) {}

    Type        m_type;

    int         m_I;
    float       m_F;
    bool        m_B;
    std::string m_S;

    void        SetFromLua(lua_State* L, int idx);
};

class Tree {
  public:
    std::unordered_map<std::string, std::unique_ptr<Tree>> m_trees;
    std::unordered_map<std::string, std::unique_ptr<Leaf>> m_leaves;

    Tree*                                                  GetTree(const std::string& key);
    Leaf*                                                  GetLeaf(const std::string& key);

    Tree*                                                  AddTree(const std::string& key);
    Leaf*                                                  AddLeaf(const std::string& key, Leaf leaf);
};

struct MonitorConfig {
    std::string name;

    int         width   = 0;
    int         height  = 0;

    double      refresh = 0.00;
};

class ConfigManager {
  public:
    ConfigManager();
    ~ConfigManager();

    bool                       Load(const std::string& path);
    void                       EnsureUserConfigExists();

    static int                 Monitor(lua_State* L);
    static int                 Exec(lua_State* L);

    static int                 TreeIndex(lua_State* L);
    static int                 TreeNewIndex(lua_State* L);
    static int                 ConfigureTree(lua_State* L);

    static void                LuaInstructionLimit(lua_State* L, lua_Debug* ar); // so while true do end doesn't halt

    void                       RegisterFeatherAPI();

    void                       HandleExec(const std::string& command);
    void                       RunStartupExecs();

    std::optional<pid_t>       ExecProc(const char* name);

    Tree*                      Root();

    int                        GetInt(const std::string& path);
    float                      GetFloat(const std::string& path);
    bool                       GetBool(const std::string& path);
    std::string                GetString(const std::string& path);

    std::vector<MonitorConfig> m_monitorConfigs;

    const MonitorConfig*       GetMonitorConfig(const std::string& name) const;

    std::string                m_configPath;

    lua_State*                 m_state;
    std::unique_ptr<Tree>      m_rootTree;

    std::vector<std::string>   m_startupExecs;

  private:
    bool  m_firstLoad = true;

    Leaf* GetLeafFromPath(const std::string& path);

    void  ParseTable(int index, Tree* node);
    void  ParseValue(int index, Leaf* leaf);

    void  RegisterTree(lua_State* L, Tree* tree);
};