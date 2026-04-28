#include "ConfigManager.hpp"
#include "../debug/Debug.hpp"
#include <fstream>
#include <cstdlib>
#include <filesystem>

static constexpr unsigned char FEATHER_DEFAULT_CONFIG_BYTES[] = {
    #embed "../../examples/feather.lua"
};

void Leaf::SetFromLua(lua_State* L, int idx) {
    switch (m_Type) {
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
    std::unordered_map<std::string, std::unique_ptr<Tree>>::iterator it = m_Trees.find(key);

    if (it != m_Trees.end()) {
        return it->second.get();
    }

    return nullptr;
}

Leaf* Tree::GetLeaf(const std::string& key) {
    std::unordered_map<std::string, std::unique_ptr<Leaf>>::iterator it = m_Leaves.find(key);

    if (it != m_Leaves.end()) {
        return it->second.get();
    }

    return nullptr;
}

Tree* Tree::AddTree(const std::string& key) {
    std::unique_ptr<Tree>& ref = m_Trees[key];

    if (!ref) {
        ref = std::make_unique<Tree>();
    }

    return ref.get();
}

Leaf* Tree::AddLeaf(const std::string& key, Leaf leaf) {
    std::unique_ptr<Leaf>& ref = m_Leaves[key];

    if (!ref) {
        ref = std::make_unique<Leaf>(leaf);
    }

    return ref.get();
}

ConfigManager::ConfigManager() {
    m_State = luaL_newstate();
    luaL_openlibs(m_State);
}

Tree* ConfigManager::Root() {
    return &m_RootTree;
}

void ConfigManager::Initialize() {
    const char* home = std::getenv("HOME");

    if (!home) {
        log_error("HOME not found");
        return;
    }

    m_ConfigPath = std::string(home) + "/.config/feather/feather.lua";

    EnsureUserConfigExists();

    Tree* input = m_RootTree.AddTree("input");

    input->AddLeaf("layout", Leaf(std::string("us")));
    input->AddLeaf("repeat_rate", Leaf(25));
    input->AddLeaf("repeat_delay", Leaf(600));

    Load(m_ConfigPath);
}

void ConfigManager::Cleanup() {
    if (m_State) {
        lua_close(m_State);
        m_State = nullptr;
    }
}

bool ConfigManager::Load(const std::string& path) {
    log_info("Loading config: %s", path.c_str());

    if (luaL_loadfile(m_State, path.c_str()) != LUA_OK) {
        const char* err = lua_tostring(m_State, -1);
        log_error("Loadfile failed: %s", err ? err : "unknown");
        lua_pop(m_State, 1);
        return false;
    }

    if (lua_pcall(m_State, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(m_State, -1);
        log_error("Runtime failed: %s", err ? err : "unknown");
        lua_pop(m_State, 1);
        return false;
    }

    if (!lua_istable(m_State, -1)) {
        log_error("Config must return a table");
        lua_pop(m_State, 1);
        return false;
    }

    ParseTable(-1, &m_RootTree);
    lua_pop(m_State, 1);

    log_info("Config loaded successfully");
    return true;
}

Leaf* ConfigManager::GetLeafFromPath(const std::string& path) {
    Tree* node = &m_RootTree;

    size_t start = 0;
    size_t end = 0;

    while ((end = path.find('.', start)) != std::string::npos) {
        std::string key = path.substr(start, end - start);

        node = node->GetTree(key);

        if (!node) {
            log_error("Invalid config path: %s", path.c_str());
            return nullptr;
        }

        start = end + 1;
    }

    std::string leafKey = path.substr(start);
    return node->GetLeaf(leafKey);
}

int ConfigManager::GetInt(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_Type != Leaf::INT) {
        return 0;
    }

    return leaf->m_I;
}

float ConfigManager::GetFloat(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_Type != Leaf::FLOAT) {
        return 0.f;
    }

    return leaf->m_F;
}

bool ConfigManager::GetBool(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_Type != Leaf::BOOL) {
        return false;
    }

    return leaf->m_B;
}

std::string ConfigManager::GetString(const std::string& path) {
    Leaf* leaf = GetLeafFromPath(path);

    if (!leaf || leaf->m_Type != Leaf::STRING) {
        return "";
    }

    return leaf->m_S;
}

void ConfigManager::ParseTable(int index, Tree* node) {
    lua_pushnil(m_State);

    while (lua_next(m_State, index < 0 ? index - 1 : index)) {
        if (!lua_isstring(m_State, -2)) {
            lua_pop(m_State, 1);
            continue;
        }

        std::string key = lua_tostring(m_State, -2);
        Leaf* leaf = node->GetLeaf(key);

        if (leaf) {
            ParseValue(-1, leaf);
            lua_pop(m_State, 1);
            continue;
        }

        Tree* tree = node->GetTree(key);

        if (tree) {
            if (lua_istable(m_State, -1)) {
                ParseTable(-1, tree);
            }

            lua_pop(m_State, 1);
            continue;
        }

        log_debug("Unknown config key ignored: %s", key.c_str());
        lua_pop(m_State, 1);
    }
}

void ConfigManager::ParseValue(int index, Leaf* leaf) {
    leaf->SetFromLua(m_State, index);
}

void ConfigManager::EnsureUserConfigExists() {
    namespace fs = std::filesystem;

    fs::path dir = fs::path(std::getenv("HOME")) / ".config" / "feather";
    fs::path file = dir / "feather.lua";

    std::error_code ec;
    fs::create_directories(dir, ec);

    if (ec) {
        log_error("Failed creating config dir");
        return;
    }

    if (fs::exists(file)) {
        return;
    }

    log_info("Creating feather config file");

    std::ofstream ofs(file, std::ios::binary);

    if (!ofs.is_open()) {
        log_error("Failed opening config file");
        return;
    }

    ofs.write(reinterpret_cast<const char*>(FEATHER_DEFAULT_CONFIG_BYTES), sizeof(FEATHER_DEFAULT_CONFIG_BYTES));
}