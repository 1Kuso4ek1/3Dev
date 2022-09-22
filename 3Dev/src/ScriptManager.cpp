#include <ScriptManager.hpp>

ScriptManager::ScriptManager() : engine(asCreateScriptEngine())
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    context = engine->CreateContext();
    
    RegisterStdString(engine);
    RegisterVector3();
    RegisterModel();
    RegisterModelPtr();
    RegisterSceneManager();

    AddFunction("string to_string(float)", asFUNCTIONPR(std::to_string, (float), std::string));
    AddFunction("void Write(string, int)", asFUNCTION(Log::Write));
    
    builder.StartNewModule(engine, "module");
}

ScriptManager::~ScriptManager()
{
    context->Release();
    engine->ShutDownAndRelease();
}

void ScriptManager::AddFunction(std::string declaration, const asSFuncPtr& ptr, asECallConvTypes callType)
{
    engine->RegisterGlobalFunction(declaration.c_str(), ptr, callType);
}

void ScriptManager::AddProperty(std::string declaration, void* ptr)
{
    engine->RegisterGlobalProperty(declaration.c_str(), ptr);
}

void ScriptManager::AddEnum(std::string name, std::vector<std::string> values)
{
    engine->RegisterEnum(name.c_str());
    for(int i = 0; i < values.size(); i++)
        engine->RegisterEnumValue(name.c_str(), values[i].c_str(), i);
}

void ScriptManager::AddValueType(std::string name, int size, int traits,
                                 std::unordered_map<std::string, asSFuncPtr> methods,
                                 std::unordered_map<std::string, int> properties)
{
    engine->RegisterObjectType(name.c_str(), size, asOBJ_VALUE | traits);
    for(auto& i : methods)
        engine->RegisterObjectMethod(name.c_str(), i.first.c_str(), i.second, i.second.flag == 3 ? asCALL_THISCALL : asCALL_CDECL_OBJFIRST);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.c_str(), i.first.c_str(), i.second);
}

void ScriptManager::AddType(std::string name, std::unordered_map<std::string, asSFuncPtr> methods, std::unordered_map<std::string, int> properties)
{
    engine->RegisterObjectType(name.c_str(), 0, asOBJ_REF | asOBJ_NOCOUNT);
    for(auto& i : methods)
        engine->RegisterObjectMethod(name.c_str(), i.first.c_str(), i.second, asCALL_THISCALL);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.c_str(), i.first.c_str(), i.second);
}

void ScriptManager::AddTypeConstructor(std::string name, std::string declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_CONSTRUCT, declaration.c_str(), ptr, asCALL_CDECL_OBJLAST);
}

void ScriptManager::AddTypeDestructor(std::string name, std::string declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_DESTRUCT, declaration.c_str(), ptr, asCALL_CDECL_OBJLAST);
}

void ScriptManager::SetDefaultNamespace(std::string name)
{
    engine->SetDefaultNamespace(name.c_str());
}

void ScriptManager::LoadScript(std::string filename)
{
    builder.AddSectionFromFile(filename.c_str());
}

void ScriptManager::Build()
{
    int ret = builder.BuildModule();
    buildSucceded = (ret >= 0);
}

void ScriptManager::ExecuteFunction(std::string declaration)
{
    if(buildSucceded)
    {
        if(declaration != function.second)
        {
            function.first = builder.GetModule()->GetFunctionByDecl(declaration.c_str());
            function.second = declaration;
        }
        context->Prepare(function.first);
        context->Execute();
    }
}

void ScriptManager::RegisterVector3()
{
    AddValueType("Vector3", sizeof(rp3d::Vector3), asGetTypeTraits<rp3d::Vector3>(),
    {
        { "float length()", asMETHOD(rp3d::Vector3, length) },
        { "Vector3& opAssign(Vector3& in)", asFUNCTION(AssignType<rp3d::Vector3>) },
        { "Vector3& opAddAssign(Vector3& in)", asMETHODPR(rp3d::Vector3, operator+=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opSubAssign(Vector3& in)", asMETHODPR(rp3d::Vector3, operator-=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opMulAssign(float)", asMETHODPR(rp3d::Vector3, operator*=, (float), rp3d::Vector3&) },
        { "Vector3& opDivAssign(float)", asMETHODPR(rp3d::Vector3, operator/=, (float), rp3d::Vector3&) },
        { "bool opEquals(Vector3& in)", asMETHODPR(rp3d::Vector3, operator==, (const rp3d::Vector3&) const, bool) },
        { "bool opEquals(Vector3& in)", asMETHODPR(rp3d::Vector3, operator!=, (const rp3d::Vector3&) const, bool) },
        { "bool opCmp(Vector3& in)", asMETHODPR(rp3d::Vector3, operator<, (const rp3d::Vector3&) const, bool) }
    },
    {
        { "float x", asOFFSET(rp3d::Vector3, x) },
        { "float y", asOFFSET(rp3d::Vector3, y) },
        { "float z", asOFFSET(rp3d::Vector3, z) }
    });

    AddTypeConstructor("Vector3", "void f()", asFUNCTION(MakeType<rp3d::Vector3>));
    AddTypeConstructor("Vector3", "void f(float x, float y, float z)", asFUNCTION(MakeVector3));
    AddTypeConstructor("Vector3", "void f(const Vector3& in)", asFUNCTION(CopyType<rp3d::Vector3>));
    AddTypeDestructor("Vector3", "void f()", asFUNCTION(DestroyType<rp3d::Vector3>));
}

void ScriptManager::RegisterModel()
{
    AddType("Model",
    {
        { "void SetPosition(Vector3 position)", asMETHOD(Model, SetPosition) },
        { "void Move(Vector3 vec)", asMETHOD(Model, Move) },
        { "Vector3& GetPosition()", asMETHOD(Model, GetPosition) }
    }, {});
}

void ScriptManager::RegisterModelPtr()
{
    AddValueType("ModelPtr", sizeof(std::shared_ptr<Model>), asGetTypeTraits<std::shared_ptr<Model>>(),
    {
        { "Model@ get()", asMETHOD(std::shared_ptr<Model>, get) },
        { "ModelPtr& opAssign(ModelPtr& in)", asMETHODPR(std::shared_ptr<Model>, operator=, (std::shared_ptr<Model>&&), std::shared_ptr<Model>&) }
    }, {});

    AddTypeConstructor("ModelPtr", "void f()", asFUNCTION(MakeType<std::shared_ptr<Model>>));
    AddTypeDestructor("ModelPtr", "void f()", asFUNCTION(DestroyType<std::shared_ptr<Model>>));
}

void ScriptManager::RegisterSceneManager()
{
    AddType("SceneManager",
    {
        { "ModelPtr GetModel(string)", asMETHOD(SceneManager, GetModel) }
    }, {});
}
