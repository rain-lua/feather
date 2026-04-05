#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <lua.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "../debug/Debug.hpp"

namespace FeatherConfig {

enum class LeafType {
    INT,
    FLOAT,
    BOOL,
    STRING
};

class Leaf {
public:
    Leaf(int v) : Type_(LeafType::INT), IntValue_(v) {}
    Leaf(float v) : Type_(LeafType::FLOAT), FloatValue_(v) {}
    Leaf(bool v) : Type_(LeafType::BOOL), BoolValue_(v) {}
    Leaf(const std::string& v) : Type_(LeafType::STRING), StrValue_(v) {}

    int GetInt() const {
        if(Type_ != LeafType::INT) { log_error("Leaf type mismatch: expected INT"); std::abort(); }
        return IntValue_;
    }

    float GetFloat() const {
        if(Type_ != LeafType::FLOAT) { log_error("Leaf type mismatch: expected FLOAT"); std::abort(); }
        return FloatValue_;
    }

    bool GetBool() const {
        if(Type_ != LeafType::BOOL) { log_error("Leaf type mismatch: expected BOOL"); std::abort(); }
        return BoolValue_;
    }

    const std::string& GetString() const {
        if(Type_ != LeafType::STRING) { log_error("Leaf type mismatch: expected STRING"); std::abort(); }
        return StrValue_;
    }

    void SetValueFromLua(lua_State* L, int index);

private:
    LeafType Type_;
    int IntValue_;
    float FloatValue_;
    bool BoolValue_;
    std::string StrValue_;
};

class Tree : public std::enable_shared_from_this<Tree> {
public:
    Tree() = default;

    std::shared_ptr<Tree> DefineTree(const std::string& key) {
        auto it = Trees_.find(key);
        if(it != Trees_.end()) return it->second;
        auto tree = std::make_shared<Tree>();
        Trees_[key] = tree;
        return tree;
    }

    std::shared_ptr<Leaf> DefineLeaf(const std::string& key, const Leaf& leaf) {
        auto it = Leaves_.find(key);
        if(it != Leaves_.end()) return it->second;
        auto l = std::make_shared<Leaf>(leaf);
        Leaves_[key] = l;
        return l;
    }

    std::shared_ptr<Tree> GetTree(const std::string& key) const {
        auto it = Trees_.find(key);
        return it != Trees_.end() ? it->second : nullptr;
    }

    std::shared_ptr<Leaf> GetLeafDirect(const std::string& key) const {
        auto it = Leaves_.find(key);
        return it != Leaves_.end() ? it->second : nullptr;
    }

    std::shared_ptr<Leaf> GetLeaf(const std::string& key) const {
        auto leaf = GetLeafDirect(key);
        if(leaf) return leaf;
        for(auto& [_, subtree] : Trees_) {
            leaf = subtree->GetLeaf(key);
            if(leaf) return leaf;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Tree>> Trees_;
    std::unordered_map<std::string, std::shared_ptr<Leaf>> Leaves_;
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    void Define();

    bool Load(const std::string& path);
    std::shared_ptr<Tree> GetRootTree() const { return RootTree_; }

    static void EnsureUserConfigExists();

private:
    lua_State* LuaState_;
    std::shared_ptr<Tree> RootTree_;

    void ParseLuaTable(int index, std::shared_ptr<Tree> node);
    void ParseLuaValue(int index, std::shared_ptr<Leaf> leaf);
};

}

#endif