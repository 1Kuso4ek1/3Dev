#pragma once
#include "Engine.hpp"
#include "AngelscriptUtils.hpp"

#include <cstdio>

// https://www.angelcode.com/angelscript/sdk/docs/manual/doc_adv_precompile.html
class BytecodeStream : public asIBinaryStream
{
public:
    BytecodeStream(FILE *fp) : f(fp) {}
    ~BytecodeStream() { fclose(f); }

    int Write(const void *ptr, asUINT size) override
    {
        if(size == 0) return -1;
        fwrite(ptr, size, 1, f);
        return 0;
    }

    int Read(void *ptr, asUINT size) override
    {
        if(size == 0) return -1;
        fread(ptr, size, 1, f);
        return 0;
    }

protected:
    FILE *f;
};

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    void AddFunction(std::string declaration, const asSFuncPtr& ptr, asECallConvTypes callType = asCALL_GENERIC);
    void AddProperty(std::string declaration, void* ptr);
    void AddValueType(std::string name, int size, int traits,
                      std::unordered_map<std::string, asSFuncPtr> methods,
                      std::unordered_map<std::string, int> properties);
    void AddType(std::string name, int size, std::unordered_map<std::string, asSFuncPtr> methods,
                 std::unordered_map<std::string, int> properties);
    void AddTypeConstructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddTypeDestructor(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddTypeFactory(std::string name, std::string declaration, const asSFuncPtr& ptr);
    void AddEnum(std::string name, std::vector<std::string> values);
    void SetDefaultNamespace(std::string name);

    void Save(std::string filename, bool relativePaths = false);
    void Load(std::string filename, bool loadBytecode = false);

    void LoadScript(std::string filename);
    void Build();
    bool IsBuildSucceded();

    void ExecuteFunction(std::string declaration);

    void RemoveScript(std::string filename);

    std::vector<std::string> GetScripts();

    std::unordered_map<std::string, void*> GetGlobalVariables();

private:
    void RegisterVector2();
    void RegisterVector3();
    void RegisterQuaternion();
    void RegisterModel();
    void RegisterLight();
    void RegisterRigidBody();
    void RegisterCamera();
    void RegisterSceneManager();
    void RegisterSfKeyboard();
    void RegisterSfMouse();
    void RegisterPhysicsManager();
    void RegisterTransform();
    void RegisterRandom();
    void RegisterRay();
    void RegisterJoints();
    void RegisterClock();
    void RegisterSoundManager();
    void RegisterEngine();
    void RegisterTGUI();
    void RegisterMaterial();
    void RegisterNetwork();
    void RegisterBone();
    void RegisterAnimation();
    void RegisterShader();
    void RegisterNode();
    void RegisterRenderer();

    bool buildSucceded = false;

    asIScriptEngine* engine;
    asIScriptContext* context;
    std::pair<asIScriptFunction*, std::string> function = { nullptr, "" };

    std::unordered_map<std::string, void*> globalVariables;

    CScriptBuilder builder;

    std::vector<std::string> scripts;
};
