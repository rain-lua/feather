#include "ConfigManager.hpp"

#include "../debug/Logger.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "Default.hpp"

void Leaf::SetFromLua(lua_State* L, int idx) {
    switch (m_type) {
    case INT:
        m_I = (int)lua_tointeger(L, idx);
        break;
    case FLOAT:
        m_F = (float)lua_tonumber(L, idx);
        break;
    case BOOL:
        m_B = lua_toboolean(L, idx);
        break;
    case STRING:
        m_S = lua_tostring(L, idx) ? lua_tostring(L, idx) : "";
        break;
    }
}

Tree* Tree::GetTree(const std::string& key) {
    std::unordered_map<std::string, std::unique_ptr<Tree>>::iterator it = m_trees.find(key);

    if (it != m_trees.end()) {
        return it->second.get();
    }

    return nullptr;
}

Leaf* Tree::GetLeaf(const std::string& key) {
    std::unordered_map<std::string, std::unique_ptr<Leaf>>::iterator it = m_leaves.find(key);

    if (it != m_leaves.end()) {
        return it->second.get();
    }

    return nullptr;
}

Tree* Tree::AddTree(const std::string& key) {
    std::unique_ptr<Tree>& ref = m_trees[key];

    if (!ref) {
        ref = std::make_unique<Tree>();
    }

    return ref.get();
}

Leaf* Tree::AddLeaf(const std::string& key, Leaf leaf) {
    std::unique_ptr<Leaf>& ref = m_leaves[key];

    if (!ref) {
        ref = std::make_unique<Leaf>(leaf);
    }

    return ref.get();
}

ConfigManager::ConfigManager() {
    m_rootTree = std::make_unique<Tree>();

    m_state    = luaL_newstate();
    luaL_openlibs(m_state);

    const char* home = std::getenv("HOME");

    if (!home) {
        Logger::Log(LogLevel::ERROR, "HOME not found");
        return;
    }

    m_configPath = std::string(home) + "/.config/feather/feather.lua";

    EnsureUserConfigExists();

    Tree* input    = m_rootTree->AddTree("input");
    Tree* layout   = m_rootTree->AddTree("layout");

    Tree* master   = layout->AddTree("master");
    Tree* keyboard = input->AddTree("keyboard");

    keyboard->AddLeaf("layout", Leaf(std::string("us")));
    keyboard->AddLeaf("repeat_rate", Leaf(25));
    keyboard->AddLeaf("repeat_delay", Leaf(600));

    layout->AddLeaf("mode", Leaf(std::string("master")));
    master->AddLeaf("factor", Leaf(0.5f));

    RegisterFeatherAPI();

    Load(m_configPath);
}

Tree* ConfigManager::Root() {
    return m_rootTree.get();
}

ConfigManager::~ConfigManager() {
    if (m_state) {
        lua_close(m_state);

        m_state = nullptr;
    }
}

bool ConfigManager::Load(const std::string& path) {
    Logger::Log(LogLevel::INFO, "Loading config: %s", path.c_str());

    m_startupExecs.clear();

    if (luaL_loadfile(m_state, path.c_str()) != LUA_OK) {
        const char* err = lua_tostring(m_state, -1);
        Logger::Log(LogLevel::ERROR, "[LUA] Loadfile failed: %s", err ? err : "unknown");
        lua_pop(m_state, 1);
        m_startupExecs.clear();
        return false;
    }

    lua_sethook(m_state, LuaInstructionLimit, LUA_MASKCOUNT, 1000000);

    const int result = lua_pcall(m_state, 0, 0, 0);

    lua_sethook(m_state, nullptr, 0, 0);

    if (result != LUA_OK) {
        const char* err = lua_tostring(m_state, -1);
        Logger::Log(LogLevel::ERROR, "[LUA] Runtime failed: %s", err ? err : "unknown");
        lua_pop(m_state, 1);
        m_startupExecs.clear();
        return false;
    }

    if (m_firstLoad) {
        m_firstLoad = false;
    }

    Logger::Log(LogLevel::INFO, "Config loaded successfully");
    return true;
}

Leaf* ConfigManager::GetLeafFromPath(const std::string& path) {
    Tree*  node  = m_rootTree.get();

    size_t start = 0;
    size_t end   = 0;

    while ((end = path.find('.', start)) != std::string::npos) {
        std::string key = path.substr(start, end - start);

        node            = node->GetTree(key);

        if (!node) {
            Logger::Log(LogLevel::ERROR, "Invalid config path: %s", path.c_str());
            return nullptr;
        }

        start = end + 1;
    }

    std::string leafKey = path.substr(start);
    return node->GetLeaf(leafKey);
}

int ConfigManager::GetInt(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_type != Leaf::INT) {
        return 0;
    }

    return leaf->m_I;
}

float ConfigManager::GetFloat(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_type != Leaf::FLOAT) {
        return 0.f;
    }

    return leaf->m_F;
}

bool ConfigManager::GetBool(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_type != Leaf::BOOL) {
        return false;
    }

    return leaf->m_B;
}

std::string ConfigManager::GetString(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_type != Leaf::STRING) {
        return "";
    }

    return leaf->m_S;
}

void ConfigManager::ParseTable(int index, Tree* node) {
    index = lua_absindex(m_state, index);

    lua_pushnil(m_state);

    while (lua_next(m_state, index)) {
        if (!lua_isstring(m_state, -2)) {
            lua_pop(m_state, 1);
            continue;
        }

        std::string key  = lua_tostring(m_state, -2);
        Leaf*       leaf = node->GetLeaf(key);

        if (leaf) {
            ParseValue(-1, leaf);
            lua_pop(m_state, 1);
            continue;
        }

        Tree* tree = node->GetTree(key);

        if (tree) {
            if (lua_istable(m_state, -1)) {
                ParseTable(-1, tree);
            }

            lua_pop(m_state, 1);
            continue;
        }

        Logger::Log(LogLevel::DEBUG, "Unknown config key ignored: %s", key.c_str());
        lua_pop(m_state, 1);
    }
}

void ConfigManager::ParseValue(int index, Leaf* leaf) {
    leaf->SetFromLua(m_state, index);
}

void ConfigManager::EnsureUserConfigExists() {
    namespace fs         = std::filesystem;

    fs::path        dir  = fs::path(std::getenv("HOME")) / ".config" / "feather";
    fs::path        file = dir / "feather.lua";

    std::error_code ec;
    fs::create_directories(dir, ec);

    if (ec) {
        Logger::Log(LogLevel::ERROR, "Failed creating config dir");
        return;
    }

    if (fs::exists(file)) {
        return;
    }

    Logger::Log(LogLevel::INFO, "Creating feather config file");

    std::ofstream ofs(file, std::ios::binary);

    if (!ofs.is_open()) {
        Logger::Log(LogLevel::ERROR, "Failed opening config file");
        return;
    }

    ofs.write(reinterpret_cast<const char*>(FEATHER_DEFAULT_CONFIG_BYTES), sizeof(FEATHER_DEFAULT_CONFIG_BYTES));
}

int ConfigManager::TreeIndex(lua_State* L) {
    Tree* tree = static_cast<Tree*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (!tree) {
        return luaL_error(L, "Config tree missing!");
    }

    const char* key  = luaL_checkstring(L, 2);
    Leaf*       leaf = tree->GetLeaf(key);

    if (!leaf) {
        return 0;
    }

    switch (leaf->m_type) {
    case Leaf::INT:
        lua_pushinteger(L, leaf->m_I);
        return 1;
    case Leaf::FLOAT:
        lua_pushnumber(L, leaf->m_F);
        return 1;
    case Leaf::BOOL:
        lua_pushboolean(L, leaf->m_B);
        return 1;
    case Leaf::STRING:
        lua_pushstring(L, leaf->m_S.c_str());
        return 1;
    }

    return 0;
}

int ConfigManager::TreeNewIndex(lua_State* L) {
    Tree* tree = static_cast<Tree*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (!tree) {
        return luaL_error(L, "Config tree missing!");
    }

    const char* key  = luaL_checkstring(L, 2);
    Leaf*       leaf = tree->GetLeaf(key);

    if (!leaf) {
        return luaL_error(L, "unknown config key '%s'", key);
    }

    leaf->SetFromLua(L, 3);
    return 0;
}

int ConfigManager::ConfigureTree(lua_State* L) {
    ConfigManager* self = static_cast<ConfigManager*>(lua_touserdata(L, lua_upvalueindex(1)));
    Tree*          tree = static_cast<Tree*>(lua_touserdata(L, lua_upvalueindex(2)));

    if (!self || !tree) {
        return luaL_error(L, "Config tree missing!");
    }

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "configuration expects a table");
    }

    self->ParseTable(lua_absindex(L, 2), tree);
    return 0;
}

void ConfigManager::RegisterTree(lua_State* L, Tree* tree) {
    lua_newtable(L);

    int tableIndex = lua_absindex(L, -1);

    for (const auto& [name, child] : tree->m_trees) {
        RegisterTree(L, child.get());
        lua_setfield(L, tableIndex, name.c_str());
    }

    lua_newtable(L);

    lua_pushlightuserdata(L, this);
    lua_pushlightuserdata(L, tree);
    lua_pushcclosure(L, ConfigureTree, 2);
    lua_setfield(L, -2, "__call");

    lua_pushlightuserdata(L, tree);
    lua_pushcclosure(L, TreeIndex, 1);
    lua_setfield(L, -2, "__index");

    lua_pushlightuserdata(L, tree);
    lua_pushcclosure(L, TreeNewIndex, 1);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, tableIndex);
}

void ConfigManager::LuaInstructionLimit(lua_State* L, lua_Debug*) {
    luaL_error(L, "Lua config exceeded instruction limit");
}

int ConfigManager::Exec(lua_State* L) {
    ConfigManager* self = static_cast<ConfigManager*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (!self) {
        return luaL_error(L, "ConfigManager missing!");
    }

    self->HandleExec(luaL_checkstring(L, 1));
    return 0;
}

void ConfigManager::RunStartupExecs() {
    for (const std::string& command : m_startupExecs) {
        ExecProc(command.c_str());
    }

    m_startupExecs.clear();
}

void ConfigManager::HandleExec(const std::string& command) {
    if (m_firstLoad) {
        m_startupExecs.emplace_back(command);
        return;
    }

    ExecProc(command.c_str());
}

std::optional<pid_t> ConfigManager::ExecProc(const char* cmd) { // no setsid() or setsid()?
    if (!cmd) {
        Logger::Log(LogLevel::ERROR, "ExecProc called with null command");
        return std::nullopt;
    }

    pid_t pid = fork();

    if (pid < 0) {
        Logger::Log(LogLevel::ERROR, "Failed to fork process for command: %s", cmd);
        return std::nullopt;
    }

    if (pid == 0) {
        sigset_t set;

        sigemptyset(&set);
        sigprocmask(SIG_SETMASK, &set, nullptr);

        int fd = open("/dev/null", O_RDWR);

        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);

            if (fd > 2) {
                close(fd);
            }
        }

        execl("/bin/sh", "sh", "-c", cmd, nullptr);

        _exit(1);
    }

    return pid;
}

int ConfigManager::Monitor(lua_State* L) {
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "feather.monitor expects a table");
    }

    ConfigManager* self = static_cast<ConfigManager*>(lua_touserdata(L, lua_upvalueindex(1)));

    if (!self) {
        return luaL_error(L, "ConfigManager missing!");
    }

    MonitorConfig config;

    lua_getfield(L, 1, "name");

    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "feather.monitor: 'name' must be a string");
    }

    config.name = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "mode");

    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);

        return luaL_error(L, "feather.monitor: 'mode' must be a string like \"1920x1080@120\"");
    }

    const char* modeString = lua_tostring(L, -1);

    int         width      = 0;
    int         height     = 0;

    double      refresh    = 0.00;

    int         parsed     = std::sscanf(modeString, "%dx%d@%lf", &width, &height, &refresh);

    lua_pop(L, 1);

    if (parsed != 3) {
        return luaL_error(L, "feather.monitor: invalid mode '%s', expected WIDTHxHEIGHT@REFRESH", modeString);
    }

    if (width <= 0) {
        return luaL_error(L, "feather.monitor: mode width must be greater than 0");
    }

    if (height <= 0) {
        return luaL_error(L, "feather.monitor: mode height must be greater than 0");
    }

    if (refresh <= 0) {
        return luaL_error(L, "feather.monitor: mode refresh must be greater than 0");
    }

    config.width   = width;
    config.height  = height;
    config.refresh = refresh;

    Logger::Log(LogLevel::INFO, "Registered monitor config: %s %dx%d@%.3lf", config.name.c_str(), config.width, config.height, config.refresh);

    self->m_monitorConfigs.push_back(std::move(config));

    return 0;
}

const MonitorConfig* ConfigManager::GetMonitorConfig(const std::string& name) const {
    for (const MonitorConfig& config : m_monitorConfigs) {
        if (config.name == name) {
            return &config;
        }
    }

    return nullptr;
}

void ConfigManager::RegisterFeatherAPI() {
    lua_newtable(m_state);

    int featherIndex = lua_absindex(m_state, -1);

    for (const auto& [name, tree] : m_rootTree->m_trees) {
        RegisterTree(m_state, tree.get());
        lua_setfield(m_state, featherIndex, name.c_str());
    }

    lua_pushlightuserdata(m_state, this);
    lua_pushcclosure(m_state, Monitor, 1);
    lua_setfield(m_state, featherIndex, "monitor");

    lua_pushlightuserdata(m_state, this);
    lua_pushcclosure(m_state, Exec, 1);
    lua_setfield(m_state, featherIndex, "exec");

    lua_setglobal(m_state, "feather");
}