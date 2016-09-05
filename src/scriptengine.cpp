#include <critterbits.h>

#include <duktape/duktape.h>

namespace Critterbits {

namespace {
void duktape_fatal_error(duk_context * ctx, duk_errcode_t code, const char * msg) {
    LOG_ERR("duktape runtime encountered fatal error " + std::to_string(code) + " " + std::string(msg));
}
}

ScriptEngine::ScriptEngine() {
    this->context = duk_create_heap(NULL, NULL, NULL, NULL, duktape_fatal_error);
    if (this->context == nullptr) {
        LOG_ERR("ScriptEngine::ScriptEngine unable to create duktape context");
    }
}

ScriptEngine::~ScriptEngine() {
    if (this->context != nullptr) {
        duk_destroy_heap(this->context);
    }
}

std::shared_ptr<Script> ScriptEngine::GetScriptHandle(const std::string & script_name) {
    for (auto & script : this->loaded_scripts) {
        if (script->script_name == script_name) {
            return script;
        }
    }
    return nullptr;
}

bool ScriptEngine::LoadScript(const std::string & script_name) {
    if (this->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript no scripting runtime");
        return false;
    }
    std::string script_path =
        Engine::GetInstance().config->asset_path + CB_SCRIPT_PATH + PATH_SEP + script_name + CB_SCRIPT_EXT;
    LOG_INFO("ScriptEngine::LoadScript about to load " + script_path);

    std::shared_ptr<Script> new_script(new Script());
    new_script->script_name = script_name;
    new_script->script_path = script_path;
    duk_push_thread_new_globalenv(this->context);
    new_script->context = duk_get_context(this->context, 0);
    if (new_script->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript unable to create new script context");
        return false;
    }
    if (duk_peval_file(new_script->context, script_path.c_str()) != 0) {
        const char * error = duk_safe_to_string(new_script->context, 0);
        LOG_ERR("ScriptEngine::LoadScript unable to compile script " + std::string(error));
        return false;
    }
    duk_pop(new_script->context); // ignore result of peval

    // prepare the script object
    new_script->DiscoverGlobals();

    this->loaded_scripts.push_back(std::move(new_script));
    return true;
}
}