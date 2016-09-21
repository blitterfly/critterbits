#pragma once
#ifndef CBSCRIPTING_HPP
#define CBSCRIPTING_HPP

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include <duktape/duktape.h>

#include <cb/entity.hpp>
#include <cb/sprite.hpp>

#define CB_SCRIPT_GLOBAL_ONCOLLISION "oncollision"
#define CB_SCRIPT_GLOBAL_START "start"
#define CB_SCRIPT_GLOBAL_UPDATE "update"

namespace Critterbits {
namespace Scripting {

class Script {
    friend class ScriptEngine;

  public:
    std::string script_path;

    void CallOnCollision(std::shared_ptr<Entity>, std::shared_ptr<Entity>);
    void CallStart(std::shared_ptr<Entity>);
    void CallUpdate(std::shared_ptr<Entity>, float);

  private:
    duk_context * context{nullptr};
    bool global_oncollision{false};
    bool global_start{false};
    bool global_update{false};

    void DiscoverGlobals();
    void PostCallRetrieveAllEntities();
};

class ScriptEngine {
  public:
    ScriptEngine();
    ~ScriptEngine();
    std::shared_ptr<Script> GetScriptHandle(const std::string &) const;
    std::shared_ptr<Script> LoadScript(const std::string &);

  private:
    duk_context * context{nullptr};
    std::vector<std::shared_ptr<Script>> loaded_scripts;

    void AddCommonScriptingFunctions(duk_context *) const;

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine(ScriptEngine &&) = delete;
    void operator=(ScriptEngine const &) = delete;
};
}
}
#endif