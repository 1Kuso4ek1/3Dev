#pragma once
#include <Engine.hpp>

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    void AddFunction(std::string declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_CDECL);
    void AddProperty(std::string declaration, void* ptr);
    void AddObject(std::string name, std::unordered_map<std::string, asSFuncPtr&> methods, std::unordered_map<std::string, int> properties);
    void AddEnum(std::string name, std::vector<std::string> values);
    void SetDefaultNamespace(std::string name);

    void LoadScript(std::string filename);

    void ExecuteFunction(std::string declaration);

private:
    asIScriptEngine* engine;
    asIScriptContext* context;
    std::pair<asIScriptFunction*, std::string> function = { nullptr, "" };

    CScriptBuilder builder;

    static void MessageCallback(const asSMessageInfo *msg, void *param);
};
