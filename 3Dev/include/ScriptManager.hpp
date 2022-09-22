#pragma once
#include "Engine.hpp"
#include "AngelscriptUtils.hpp"

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    void AddFunction(std::string declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_CDECL);
    void AddProperty(std::string declaration, void* ptr);
    void AddValueType(std::string name, int size, int traits, 
                      std::unordered_map<std::string, asSFuncPtr> methods, 
                      std::unordered_map<std::string, int> properties);
    void AddType(std::string name, std::unordered_map<std::string, asSFuncPtr> methods, std::unordered_map<std::string, int> properties);
    void AddTypeConstructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddTypeDestructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddEnum(std::string name, std::vector<std::string> values);
    void SetDefaultNamespace(std::string name);

    void LoadScript(std::string filename);
    void Build();

    void ExecuteFunction(std::string declaration);

private:
    void RegisterVector3();
    void RegisterModel();
    void RegisterModelPtr();
    void RegisterSceneManager();

    bool buildSucceded = false;

    asIScriptEngine* engine;
    asIScriptContext* context;
    std::pair<asIScriptFunction*, std::string> function = { nullptr, "" };

    CScriptBuilder builder;
};
