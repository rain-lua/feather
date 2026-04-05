#include "ConfigManager.hpp"
#include <fstream>
#include <cstdlib>
#include <unistd.h>

#if LUA_VERSION_NUM < 503
    #define LUA_TO_INTEGER(L, index) static_cast<int>(lua_tonumber(L, index))
#else
    #define LUA_TO_INTEGER(L, index) (lua_isinteger(L, index) ? static_cast<int>(lua_tointeger(L, index)) : static_cast<int>(lua_tonumber(L, index)))
#endif

namespace FeatherConfig {

void Leaf::SetValueFromLua(lua_State* L, int index) {
    switch(Type_) {
        case LeafType::INT:
            IntValue_ = LUA_TO_INTEGER(L, index);
            break;
        case LeafType::FLOAT:
            FloatValue_ = static_cast<float>(lua_tonumber(L, index));
            break;
        case LeafType::BOOL:
            BoolValue_ = lua_toboolean(L, index) != 0;
            break;
        case LeafType::STRING: {
            const char* s = lua_tostring(L, index);
            StrValue_ = s ? s : "";
            break;
        }
    }
}

ConfigManager::ConfigManager() {
    LuaState_ = luaL_newstate();
    luaL_openlibs(LuaState_);
    RootTree_ = std::make_shared<Tree>();
    Define();
}

ConfigManager::~ConfigManager() {
    if(LuaState_) lua_close(LuaState_);
}

void ConfigManager::Define() {
    auto input = RootTree_->DefineTree("input");
    input->DefineLeaf("layout", Leaf(std::string("us")));
    input->DefineLeaf("repeat_rate", Leaf(25));
    input->DefineLeaf("repeat_delay", Leaf(600));
}

bool ConfigManager::Load(const std::string& path) {
    if(luaL_dofile(LuaState_, path.c_str()) != LUA_OK) {
        log_error("[ConfigManager] Failed to load Lua config: %s", lua_tostring(LuaState_, -1));
        lua_pop(LuaState_, 1);
        return false;
    }

    #if LUA_VERSION_NUM >= 502
        lua_pushglobaltable(LuaState_);
    #else
        lua_pushvalue(LuaState_, LUA_GLOBALSINDEX);
    #endif

    ParseLuaTable(-1, RootTree_);
    lua_pop(LuaState_, 1);

    log_info("[ConfigManager] Successfully loaded config from %s", path.c_str());
    return true;
}

void ConfigManager::ParseLuaTable(int index, std::shared_ptr<Tree> node) {
    lua_pushnil(LuaState_);
    while(lua_next(LuaState_, index < 0 ? index - 1 : index)) {
        std::string key = lua_tostring(LuaState_, -2);

        auto leaf = node->GetLeafDirect(key);
        if(leaf) { ParseLuaValue(-1, leaf); lua_pop(LuaState_, 1); continue; }

        auto tree = node->GetTree(key);
        if(tree) { ParseLuaTable(-1, tree); lua_pop(LuaState_, 1); continue; }

        lua_pop(LuaState_, 1);
    }
}

void ConfigManager::ParseLuaValue(int index, std::shared_ptr<Leaf> leaf) {
    leaf->SetValueFromLua(LuaState_, index);
}

constexpr char Bytes[] = {
    #embed "../examples/feather.lua"
};

constexpr std::string_view cfg = {Bytes, sizeof(Bytes)};

void ConfigManager::EnsureUserConfigExists() {
    namespace fs = std::filesystem;

    const fs::path configDir = fs::path(std::getenv("HOME")) / ".config" / "feather";
    const fs::path configFile = configDir / "feather.lua";

    try {
        if (!fs::exists(configDir))
            fs::create_directories(configDir);

        if (!fs::exists(configFile)) {
            std::ofstream ofs(configFile, std::ios::binary);
            if (!ofs) {
                log_error("[ConfigManager] Failed to open config file for writing: %s", configFile.string().c_str());
                return;
            }
            ofs.write(cfg.data(), cfg.size());
            ofs.close();

            log_info("[ConfigManager] Created example config at %s", configFile.string().c_str());
        }
    } catch (const fs::filesystem_error& e) {
        log_error("Failed to ensure config: %s", e.what());
    }
}

}