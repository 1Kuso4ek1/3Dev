#include <ScriptManager.hpp>

ScriptManager::ScriptManager() : engine(asCreateScriptEngine())
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, true);

    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFile(engine);
    RegisterVector2();
    RegisterVector3();
    RegisterQuaternion();
    RegisterTransform();
    RegisterRay();
    RegisterRigidBody();
    RegisterJoints();
    RegisterPhysicsManager();
    RegisterSoundManager();
    RegisterNode();
    RegisterCamera();
    RegisterMaterial();
    RegisterAnimation();
    RegisterShader();
    RegisterModel();
    RegisterLight();
    RegisterBone();
    RegisterSceneManager();
    RegisterSfKeyboard();
    RegisterSfMouse();
    RegisterRandom();
    RegisterClock();
    RegisterTGUI();
    RegisterEngine();
    RegisterNetwork();

    AddFunction("string to_string(int)", WRAP_FN_PR(std::to_string, (int), std::string));
    AddFunction("string to_string(float)", WRAP_FN_PR(std::to_string, (float), std::string));

    AddFunction("int stoi(const string& in, int = 10)", WRAP_FN(Stoi));
    AddFunction("float stof(const string& in)", WRAP_FN(Stof));

    AddFunction("float radians(float)", WRAP_FN(glm::radians<float>));
    AddFunction("float degrees(float)", WRAP_FN(glm::degrees<float>));

    AddFunction("Quaternion LookAt(const Vector3& in, const Vector3& in, const Vector3& in)", WRAP_FN(LookAt));

    SetDefaultNamespace("Log");
    AddEnum("Type", { "Critical", "Error", "Warning", "Info" });
    AddFunction("void Write(string, int = Info)", WRAP_FN(Log::Write));
    SetDefaultNamespace("");

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
        engine->RegisterObjectMethod(name.c_str(), i.first.c_str(), i.second, asCALL_GENERIC);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.c_str(), i.first.c_str(), i.second);
}

void ScriptManager::AddType(std::string name, int size, std::unordered_map<std::string, asSFuncPtr> methods, std::unordered_map<std::string, int> properties)
{
    engine->RegisterObjectType(name.c_str(), size, asOBJ_REF | asOBJ_NOCOUNT);
    for(auto& i : methods)
        engine->RegisterObjectMethod(name.c_str(), i.first.c_str(), i.second, asCALL_GENERIC);
    for(auto& i : properties)
        engine->RegisterObjectProperty(name.c_str(), i.first.c_str(), i.second);
}

void ScriptManager::AddTypeConstructor(std::string name, std::string declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_CONSTRUCT, declaration.c_str(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeDestructor(std::string name, std::string declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_DESTRUCT, declaration.c_str(), ptr, asCALL_GENERIC);
}

void ScriptManager::AddTypeFactory(std::string name, std::string declaration, const asSFuncPtr& ptr)
{
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_FACTORY, declaration.c_str(), ptr, asCALL_GENERIC);
}

void ScriptManager::SetDefaultNamespace(std::string name)
{
    engine->SetDefaultNamespace(name.c_str());
}

void ScriptManager::Save(std::string filename, bool relativePaths)
{
    Json::Value data;

    for(int i = 0; i < scripts.size(); i++)
        if(relativePaths)
            data["scripts"][i] = std::filesystem::relative(scripts[i], std::filesystem::path(filename).parent_path()).string();
        else data["scripts"][i] = scripts[i];

    std::ofstream file(filename);
    file << data.toStyledString();
    file.close();

    if(scripts.size() > 0)
    {
        BytecodeStream stream(fopen((std::filesystem::path(scripts[0]).parent_path().string() + "/bytecode").c_str(), "w"));
        builder.GetModule()->SaveByteCode(&stream, true);
    }
}

void ScriptManager::Load(std::string filename, bool loadBytecode)
{
    Json::Value data;
    Json::CharReaderBuilder rbuilder;

    std::ifstream file(filename);

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &data, &errors))
    {
        Log::Write("Json parsing failed: " + errors, Log::Type::Error);
        return;
    }

    for(int i = 0; i < data["scripts"].size(); i++)
        scripts.push_back(data["scripts"][i].asString());

    if(loadBytecode && scripts.size() > 0)
    {
        BytecodeStream stream(fopen((std::filesystem::path(scripts[0]).parent_path().string() + "/bytecode").c_str(), "r"));

        buildSucceded = (builder.GetModule()->LoadByteCode(&stream) >= 0);
    }
}

void ScriptManager::LoadScript(std::string filename)
{
    scripts.push_back(filename);
}

void ScriptManager::Build()
{
    sf::Clock time;

    engine->DiscardModule("module");
    builder.StartNewModule(engine, "module");

    for(auto& i : scripts)
        builder.AddSectionFromFile(i.c_str());

    int ret = builder.BuildModule();
    buildSucceded = (ret >= 0);

    if(buildSucceded)
    {
        Log::Write("Build completed successfully in " + std::to_string(time.restart().asSeconds()) + " s", Log::Type::Info);
        auto mod = builder.GetModule();
        for(int i = 0; i < mod->GetGlobalVarCount(); i++)
            globalVariables[std::string(mod->GetGlobalVarDeclaration(i))] = mod->GetAddressOfGlobalVar(i);
    }
    else
        Log::Write("Build failed", Log::Type::Error);
}

bool ScriptManager::IsBuildSucceded()
{
    return buildSucceded;
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
        buildSucceded = (context->Prepare(function.first) >= 0);
        buildSucceded = (context->Execute() >= 0);
    }
}

void ScriptManager::RemoveScript(std::string filename)
{
    std::remove(scripts.begin(), scripts.end(), filename);
}

std::vector<std::string> ScriptManager::GetScripts()
{
    return scripts;
}

std::unordered_map<std::string, void*> ScriptManager::GetGlobalVariables()
{
    return globalVariables;
}

void ScriptManager::RegisterVector2()
{
    AddValueType("Vector2", sizeof(rp3d::Vector2), asGetTypeTraits<rp3d::Vector2>(),
    {
        { "void normalize()", WRAP_MFN(rp3d::Vector2, normalize) },
        { "float length() const", WRAP_MFN(rp3d::Vector2, length) },
        { "float dot(const Vector2& in) const", WRAP_MFN(rp3d::Vector2, dot) },
        { "string to_string()", WRAP_MFN(rp3d::Vector2, to_string) },
        { "Vector2& opAssign(const Vector2& in)", WRAP_MFN_PR(rp3d::Vector2, operator=, (const rp3d::Vector2&), rp3d::Vector2&) },
        { "Vector2& opAddAssign(const Vector2& in)", WRAP_MFN_PR(rp3d::Vector2, operator+=, (const rp3d::Vector2&), rp3d::Vector2&) },
        { "Vector2& opSubAssign(const Vector2& in)", WRAP_MFN_PR(rp3d::Vector2, operator-=, (const rp3d::Vector2&), rp3d::Vector2&) },
        { "Vector2& opMulAssign(float)", WRAP_MFN_PR(rp3d::Vector2, operator*=, (float), rp3d::Vector2&) },
        { "Vector2& opDivAssign(float)", WRAP_MFN_PR(rp3d::Vector2, operator/=, (float), rp3d::Vector2&) },
        { "Vector2 opAdd(const Vector2& in)", WRAP_OBJ_LAST(AddVector2) },
        { "Vector2 opSub(const Vector2& in)", WRAP_OBJ_LAST(SubVector2) },
        { "Vector2 opMul(float)", WRAP_OBJ_LAST(MulVector2) },
        { "Vector2 opDiv(float)", WRAP_OBJ_LAST(DivVector2) },
        { "bool opEquals(const Vector2& in) const", WRAP_MFN_PR(rp3d::Vector2, operator==, (const rp3d::Vector2&) const, bool) }
    },
    {
        { "float x", asOFFSET(rp3d::Vector2, x) },
        { "float y", asOFFSET(rp3d::Vector2, y) }
    });

    AddTypeConstructor("Vector2", "void f()", WRAP_OBJ_LAST(MakeType<rp3d::Vector2>));
    AddTypeConstructor("Vector2", "void f(float, float)", WRAP_OBJ_LAST(MakeVector2));
    AddTypeConstructor("Vector2", "void f(const Vector2& in)", WRAP_OBJ_LAST(CopyType<rp3d::Vector2>));
    AddTypeDestructor("Vector2", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::Vector2>));
}

void ScriptManager::RegisterVector3()
{
    AddValueType("Vector3", sizeof(rp3d::Vector3), asGetTypeTraits<rp3d::Vector3>(),
    {
        { "void normalize()", WRAP_MFN(rp3d::Vector3, normalize) },
        { "float length() const", WRAP_MFN(rp3d::Vector3, length) },
        { "float dot(const Vector3& in) const", WRAP_MFN(rp3d::Vector3, dot) },
        { "Vector3 cross(const Vector3& in) const", WRAP_MFN(rp3d::Vector3, cross) },
        { "string to_string()", WRAP_MFN(rp3d::Vector3, to_string) },
        { "Vector3& opAssign(const Vector3& in)", WRAP_MFN_PR(rp3d::Vector3, operator=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opAddAssign(const Vector3& in)", WRAP_MFN_PR(rp3d::Vector3, operator+=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opSubAssign(const Vector3& in)", WRAP_MFN_PR(rp3d::Vector3, operator-=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opMulAssign(float)", WRAP_MFN_PR(rp3d::Vector3, operator*=, (float), rp3d::Vector3&) },
        { "Vector3& opDivAssign(float)", WRAP_MFN_PR(rp3d::Vector3, operator/=, (float), rp3d::Vector3&) },
        { "Vector3 opAdd(const Vector3& in)", WRAP_OBJ_LAST(AddVector3) },
        { "Vector3 opSub(const Vector3& in)", WRAP_OBJ_LAST(SubVector3) },
        { "Vector3 opMul(float)", WRAP_OBJ_LAST(MulVector3) },
        { "Vector3 opDiv(float)", WRAP_OBJ_LAST(DivVector3) },
        { "bool opEquals(const Vector3& in) const", WRAP_MFN_PR(rp3d::Vector3, operator==, (const rp3d::Vector3&) const, bool) }
    },
    {
        { "float x", asOFFSET(rp3d::Vector3, x) },
        { "float y", asOFFSET(rp3d::Vector3, y) },
        { "float z", asOFFSET(rp3d::Vector3, z) }
    });

    AddTypeConstructor("Vector3", "void f()", WRAP_OBJ_LAST(MakeType<rp3d::Vector3>));
    AddTypeConstructor("Vector3", "void f(float, float, float)", WRAP_OBJ_LAST(MakeVector3));
    AddTypeConstructor("Vector3", "void f(const Vector3& in)", WRAP_OBJ_LAST(CopyType<rp3d::Vector3>));
    AddTypeDestructor("Vector3", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::Vector3>));
}

void ScriptManager::RegisterQuaternion()
{
    AddValueType("Quaternion", sizeof(rp3d::Quaternion), asGetTypeTraits<rp3d::Quaternion>(),
    {
        { "float length()", WRAP_MFN(rp3d::Quaternion, length) },
        { "string to_string()", WRAP_MFN(rp3d::Quaternion, to_string) },
        { "void inverse()", WRAP_MFN(rp3d::Quaternion, inverse) },
        { "Quaternion getInverse()", WRAP_MFN(rp3d::Quaternion, getInverse) },
        { "Quaternion getConjugate()", WRAP_MFN(rp3d::Quaternion, getConjugate) },
        { "Quaternion& opAssign(const Quaternion& in)", WRAP_MFN_PR(rp3d::Quaternion, operator=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        { "Quaternion& opAddAssign(const Quaternion& in)", WRAP_MFN_PR(rp3d::Quaternion, operator+=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        { "Quaternion& opSubAssign(const Quaternion& in)", WRAP_MFN_PR(rp3d::Quaternion, operator-=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        { "Quaternion opAdd(const Quaternion& in)", WRAP_OBJ_LAST(AddQuaternion) },
        { "Quaternion opSub(const Quaternion& in)", WRAP_OBJ_LAST(SubQuaternion) },
        { "Quaternion opMul(const Quaternion& in)", WRAP_OBJ_LAST(MulQuaternion) },
        { "Quaternion opMul(float)", WRAP_MFN_PR(rp3d::Quaternion, operator*, (float) const, rp3d::Quaternion) },
        { "Quaternion opMul(const Quaternion& in)", WRAP_MFN_PR(rp3d::Quaternion, operator*, (const rp3d::Quaternion&) const, rp3d::Quaternion) },
        { "Vector3 opMul(const Vector3& in)", WRAP_MFN_PR(rp3d::Quaternion, operator*, (const rp3d::Vector3&) const, rp3d::Vector3) },
        { "bool opEquals(const Quaternion& in)", WRAP_MFN_PR(rp3d::Quaternion, operator==, (const rp3d::Quaternion&) const, bool) }
    },
    {
        { "float x", asOFFSET(rp3d::Quaternion, x) },
        { "float y", asOFFSET(rp3d::Quaternion, y) },
        { "float z", asOFFSET(rp3d::Quaternion, z) },
        { "float w", asOFFSET(rp3d::Quaternion, w) }
    });

    AddFunction("Quaternion QuaternionFromEuler(const Vector3& in)", WRAP_FN_PR(rp3d::Quaternion::fromEulerAngles, (const rp3d::Vector3&), rp3d::Quaternion));
    AddFunction("Vector3 EulerFromQuaternion(const Quaternion& in)", WRAP_FN(EulerFromQuaternion));
    AddFunction("Quaternion slerp(const Quaternion& in, const Quaternion& in, float)", WRAP_FN(rp3d::Quaternion::slerp));

    AddTypeConstructor("Quaternion", "void f()", WRAP_OBJ_LAST(MakeType<rp3d::Quaternion>));
    AddTypeConstructor("Quaternion", "void f(float, float, float, float)", WRAP_OBJ_LAST(MakeQuaternion));
    AddTypeConstructor("Quaternion", "void f(const Quaternion& in)", WRAP_OBJ_LAST(CopyType<rp3d::Quaternion>));
    AddTypeDestructor("Quaternion", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::Quaternion>));
}

void ScriptManager::RegisterModel()
{
    AddType("Model", sizeof(Model),
    {
        { "void Load()", WRAP_MFN_PR(Model, Load, (), void) },
        { "void Unload(bool)", WRAP_MFN(Model, Unload) },
        { "void SetTransform(const Transform& in)", WRAP_MFN(Model, SetTransform) },
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Model, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Model, SetOrientation) },
        { "void SetSize(const Vector3& in)", WRAP_MFN(Model, SetSize) },
        { "void SetMaterial(Material@, uint32 = 0)", WRAP_MFN(Model, SetMaterialSlot) },
        { "void SetPhysicsManager(PhysicsManager@)", WRAP_MFN(Model, SetPhysicsManager) },
        { "void SetShader(Shader@, bool = false)", WRAP_MFN(Model, SetShader) },
        { "void SetFilename(const string& in)", WRAP_MFN(Model, SetFilename) },
        { "void SetIsDrawable(bool)", WRAP_MFN(Model, SetIsDrawable) },
        { "void SetShadowBias(float)", WRAP_MFN(Model, SetShadowBias) },
        { "void DefaultPose()", WRAP_MFN(Model, DefaultPose) },
        { "void CreateRigidBody()", WRAP_MFN(Model, CreateRigidBody) },
        { "void Move(const Vector3& in)", WRAP_MFN(Model, Move) },
        { "void Rotate(const Quaternion& in)", WRAP_MFN(Model, Rotate) },
        { "void Expand(const Vector3& in)", WRAP_MFN(Model, Expand) },
        { "Node@ opCast()", WRAP_OBJ_LAST(CastModel) },
        { "bool IsLoaded()", WRAP_MFN(Model, IsLoaded) },
        { "bool IsDrawable()", WRAP_MFN(Model, IsDrawable) },
        { "Transform GetTransform()", WRAP_MFN(Model, GetTransform) },
        { "Vector3 GetPosition()", WRAP_MFN(Model, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Model, GetOrientation) },
        { "Vector3 GetSize()", WRAP_MFN(Model, GetSize) },
        { "Shader@ GetShader()", WRAP_MFN(Model, GetShader) },
        { "RigidBody@ GetRigidBody()", WRAP_MFN(Model, GetRigidBody) },
        { "int GetMeshesCount()", WRAP_MFN(Model, GetMeshesCount) }
    }, {});

    engine->RegisterObjectMethod("Node", "Model@ opCast()", WRAP_OBJ_LAST(CastToModel), asCALL_GENERIC);
}

void ScriptManager::RegisterLight()
{
    AddType("Light", sizeof(Light),
    {
        { "void SetTransform(const Transform& in)", WRAP_MFN(Light, SetTransform) },
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Light, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Light, SetOrientation) },
        { "void Move(const Vector3& in)", WRAP_MFN(Light, Move) },
        { "void Rotate(const Quaternion& in)", WRAP_MFN(Light, Rotate) },
        { "void SetColor(const Vector3& in)", WRAP_MFN(Light, SetColor) },
        { "void SetAttenuation(float, float, float)", WRAP_MFN(Light, SetAttenuation) },
        { "void SetCutoff(float)", WRAP_MFN(Light, SetCutoff) },
        { "void SetOuterCutoff(float)", WRAP_MFN(Light, SetOuterCutoff) },
        { "void CalcLightSpaceMatrix()", WRAP_MFN(Light, CalcLightSpaceMatrix) },
        { "void SetIsCastingShadows(bool)", WRAP_MFN(Light, SetIsCastingShadows) },
        { "void SetIsCastingPerspectiveShadows(bool)", WRAP_MFN(Light, SetIsCastingPerspectiveShadows) },
        { "Node@ opCast()", WRAP_OBJ_LAST(CastLight) },
        { "bool IsCastingShadows()", WRAP_MFN(Light, IsCastingShadows) },
        { "bool IsCastingPerspectiveShadows()", WRAP_MFN(Light, IsCastingPerspectiveShadows) },
        { "Transform GetTransform()", WRAP_MFN(Light, GetTransform) },
        { "Vector3 GetPosition()", WRAP_MFN(Light, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Light, GetOrientation) },
        { "Vector3 GetColor()", WRAP_MFN(Light, GetColor) },
        { "Vector3 GetAttenuation()", WRAP_MFN(Light, GetAttenuation) },
        { "float GetCutoff()", WRAP_MFN(Light, GetCutoff) },
        { "float GetOuterCutoff()", WRAP_MFN(Light, GetOuterCutoff) }
    }, {});

    engine->RegisterObjectMethod("Node", "Light@ opCast()", WRAP_OBJ_LAST(CastToLight), asCALL_GENERIC);
}

void ScriptManager::RegisterRigidBody()
{
    AddValueType("AABB", sizeof(rp3d::AABB), asGetTypeTraits<rp3d::AABB>() | asOBJ_POD, {}, {});

    AddTypeConstructor("AABB", "void f(const Vector3& in, const Vector3& in)", WRAP_OBJ_LAST(MakeAABB));

    AddValueType("PhysicalMaterial", sizeof(rp3d::Material), asGetTypeTraits<rp3d::Material>() | asOBJ_POD,
    {
        { "float getBounciness()", WRAP_MFN(rp3d::Material, getBounciness) },
        { "void setBounciness(float)", WRAP_MFN(rp3d::Material, setBounciness) },
        { "float getFrictionCoefficient()", WRAP_MFN(rp3d::Material, getFrictionCoefficient) },
        { "void setFrictionCoefficient(float)", WRAP_MFN(rp3d::Material, setFrictionCoefficient) },
        { "float getMassDensity()", WRAP_MFN(rp3d::Material, getMassDensity) },
        { "void setMassDensity(float)", WRAP_MFN(rp3d::Material, setMassDensity) },
        { "string to_string()", WRAP_MFN(rp3d::Material, to_string) }
    }, {});

    AddEnum("BodyType", { "STATIC", "KINEMATIC", "DYNAMIC" });
    AddType("RigidBody", sizeof(rp3d::RigidBody),
    {
        { "float getMass()", WRAP_MFN(rp3d::RigidBody, getMass) },
        { "void setMass(float)", WRAP_MFN(rp3d::RigidBody, setMass) },
        { "Vector3 getLinearVelocity()", WRAP_MFN(rp3d::RigidBody, getLinearVelocity) },
        { "void setLinearVelocity(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, setLinearVelocity) },
        { "Vector3 getAngularVelocity()", WRAP_MFN(rp3d::RigidBody, getAngularVelocity) },
        { "void setAngularVelocity(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, setAngularVelocity) },
        { "void setAngularLockAxisFactor(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, setAngularLockAxisFactor) },
        { "void setLinearLockAxisFactor(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, setLinearLockAxisFactor) },
        { "void setLocalCenterOfMass(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, setLocalCenterOfMass) },
        { "void applyLocalForceAtCenterOfMass(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyLocalForceAtCenterOfMass) },
        { "void applyWorldForceAtCenterOfMass(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyWorldForceAtCenterOfMass) },
        { "void applyLocalForceAtLocalPosition(const Vector3& in, const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyLocalForceAtLocalPosition) },
        { "void applyWorldForceAtLocalPosition(const Vector3& in, const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyWorldForceAtLocalPosition) },
        { "void applyLocalForceAtWorldPosition(const Vector3& in, const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyLocalForceAtWorldPosition) },
        { "void applyWorldForceAtWorldPosition(const Vector3& in, const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyWorldForceAtWorldPosition) },
        { "void applyWorldTorque(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyWorldTorque) },
        { "void applyLocalTorque(const Vector3& in)", WRAP_MFN(rp3d::RigidBody, applyLocalTorque) },
        { "bool isActive()", WRAP_MFN(rp3d::RigidBody, isActive) },
        { "void setIsActive(bool)", WRAP_MFN(rp3d::RigidBody, setIsActive) },
        { "void setType(BodyType)", WRAP_MFN(rp3d::RigidBody, setType) },
        { "void setMaterial(const PhysicalMaterial& in)", WRAP_OBJ_LAST(SetPhysicalMaterial) },
        { "AABB getAABB()", WRAP_MFN(rp3d::RigidBody, getAABB) },
        { "bool testAABBOverlap(const AABB& in)", WRAP_MFN(rp3d::RigidBody, testAABBOverlap) },
        { "bool raycast(const Ray& in, RaycastInfo& out)", WRAP_MFN(rp3d::RigidBody, raycast) }
    }, {});

    engine->RegisterObjectProperty("RaycastInfo", "RigidBody@ body", asOFFSET(rp3d::RaycastInfo, body));
}

void ScriptManager::RegisterCamera()
{
    AddType("Camera", sizeof(Camera),
    {
        { "Vector3 Move(float, bool = false)", WRAP_MFN(Camera, Move) },
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Camera, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Camera, SetOrientation) },
        { "void SetSpeed(float)", WRAP_MFN(Camera, SetSpeed) },
        { "void SetFOV(float)", WRAP_MFN(Camera, SetFOV) },
        { "void SetNear(float)", WRAP_MFN(Camera, SetNear) },
        { "void SetFar(float)", WRAP_MFN(Camera, SetFar) },
        { "void AlwaysUp(bool)", WRAP_MFN(Camera, AlwaysUp) },
        { "void Look()", WRAP_MFN_PR(Camera, Look, (), void) },
        { "void Look(const Vector3& in)", WRAP_MFN_PR(Camera, Look, (const rp3d::Vector3&), void) },
        { "Node@ opCast()", WRAP_OBJ_LAST(CastCamera) },
        { "Transform GetTransform()", WRAP_MFN(Camera, GetTransform) },
        { "Vector3 GetPosition(bool = false)", WRAP_MFN(Camera, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Camera, GetOrientation) },
        { "Vector2 WorldPositionToScreen(const Vector3& in, bool = true)", WRAP_MFN(Camera, WorldPositionToScreen) },
        { "Vector3 ScreenPositionToWorld(bool, const Vector2& in = Vector2(0, 0))", WRAP_MFN(Camera, ScreenPositionToWorld) },
        { "float GetSpeed()", WRAP_MFN(Camera, GetSpeed) },
        { "float GetFOV()", WRAP_MFN(Camera, GetFOV) },
        { "float GetNear()", WRAP_MFN(Camera, GetNear) },
        { "float GetFar()", WRAP_MFN(Camera, GetFar) }
    }, {});

    engine->RegisterObjectMethod("Node", "Camera@ opCast()", WRAP_OBJ_LAST(CastToCamera), asCALL_GENERIC);
}

void ScriptManager::RegisterSceneManager()
{
    AddValueType("ModelGroup", sizeof(Group<Model>), asGetTypeTraits<Group<Model>>() | asOBJ_POD,
    {
        { "uint Size()", WRAP_MFN(Group<Model>, Size) },
        { "Model@ opIndex(uint)", WRAP_MFN(Group<Model>, operator[]) }
    }, {});

    AddType("SceneManager", sizeof(SceneManager),
    {
        { "void Draw()", WRAP_MFN(SceneManager, Draw) },
        { "Model@ CreateModel(const string& in = \"\", bool = true, const string& in = \"model\")", WRAP_MFN(SceneManager, CreateModelPtr) },
        { "Light@ CreateLight(bool = true, const string& in = \"light\")", WRAP_MFN(SceneManager, CreateLightPtr) },
        { "Material@ CreateMaterial(const string& in = \"material\")", WRAP_MFN(SceneManager, CreateMaterialPtr) },
        { "Model@ GetModel(const string& in)", WRAP_MFN(SceneManager, GetModelPtr) },
        { "Light@ GetLight(const string& in)", WRAP_MFN(SceneManager, GetLightPtr) },
        { "Material@ GetMaterial(const string& in)", WRAP_MFN(SceneManager, GetMaterialPtr) },
        { "Bone@ GetBone(const string& in)", WRAP_MFN(SceneManager, GetBone) },
        { "Node@ GetNode(const string& in)", WRAP_MFN(SceneManager, GetNode) },
        { "Animation@ GetAnimation(const string& in)", WRAP_MFN(SceneManager, GetAnimationPtr) },
        { "Model@ CloneModel(Model@, bool = true, const string& in = \"model\")", WRAP_MFN(SceneManager, CloneModel) },
        { "Light@ CloneLight(Light@, bool = true, const string& in = \"light\")", WRAP_MFN(SceneManager, CloneLight) },
        { "Camera@ GetCamera()", WRAP_MFN(SceneManager, GetCamera) },
        { "PhysicsManager@ GetPhysicsManager()", WRAP_MFN(SceneManager, GetPhysicsManagerPtr) },
        { "SoundManager@ GetSoundManager()", WRAP_MFN(SceneManager, GetSoundManagerPtr) },
        { "void SetMainShader(Shader@, bool = false)", WRAP_MFN(SceneManager, SetMainShader) },
        { "void RemoveModel(Model@)", WRAP_MFN(SceneManager, RemoveModelPtr) },
        { "void RemoveLight(Light@)", WRAP_MFN(SceneManager, RemoveLightPtr) },
        { "void RemoveMaterial(Material@)", WRAP_MFN(SceneManager, RemoveMaterialPtr) },
        { "void RemoveAnimation(Animation@)", WRAP_MFN(SceneManager, RemoveAnimationPtr) },
        { "void Save(const string& in, bool = false)", WRAP_MFN(SceneManager, Save) },
        { "void Load(const string& in, bool = false)", WRAP_MFN(SceneManager, Load) },
        { "ModelGroup GetModelGroup(const string& in)", WRAP_MFN(SceneManager, GetModelPtrGroup) },
        { "void UpdatePhysics(bool)", WRAP_MFN(SceneManager, UpdatePhysics) },
        { "void SaveState()", WRAP_MFN(SceneManager, SaveState) },
        { "void LoadState()", WRAP_MFN(SceneManager, LoadState) },
        { "void LoadEnvironment(const string& in)", WRAP_MFN(SceneManager, LoadEnvironment) }
    }, {});
}

void ScriptManager::RegisterSfKeyboard()
{
    SetDefaultNamespace("Keyboard");
    AddFunction("bool isKeyPressed(int)", WRAP_FN_PR(sf::Keyboard::isKeyPressed, (sf::Keyboard::Key), bool));
    AddEnum("Key", { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
                     "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Num0", "Num1", "Num2", "Num3",
                     "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "Escape", "LControl", "LShift", "LAlt", "LSystem",
                     "RControl", "RShift", "RAlt", "RSystem", "Menu", "LBracket", "RBracket", "Semicolon", "Comma",
                     "Period", "Quote", "Slash", "Backslash", "Tilde", "Equal", "Hyphen", "Space", "Enter", "Backspace",
                     "Tab", "PageUp", "PageDown", "End", "Home", "Insert", "Delete", "Add", "Subtract", "Multiply", "Divide",
                     "Left", "Right", "Up", "Down", "Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5",
                     "Numpad6", "Numpad7", "Numpad8", "Numpad9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
                     "F10", "F11", "F12", "F13", "F14", "F15", "Pause" });
    SetDefaultNamespace("");
}

void ScriptManager::RegisterSfMouse()
{
    SetDefaultNamespace("Mouse");
    AddFunction("bool isButtonPressed(int)", WRAP_FN(sf::Mouse::isButtonPressed));
    AddEnum("Button", { "Left", "Right", "Middle", "XButton1", "XButton2" });
    SetDefaultNamespace("");
}

void ScriptManager::RegisterPhysicsManager()
{
    AddType("PhysicsManager", sizeof(PhysicsManager),
    {
        { "void SetTimeStep(float)", WRAP_MFN(PhysicsManager, SetTimeStep) },
        { "HingeJoint@ CreateHingeJoint(HingeJointInfo)", WRAP_MFN(PhysicsManager, CreateHingeJoint) },
        { "BallAndSocketJoint@ CreateBallAndSocketJoint(BallAndSocketJointInfo)", WRAP_MFN(PhysicsManager, CreateBallAndSocketJoint) },
        { "FixedJoint@ CreateFixedJoint(FixedJointInfo)", WRAP_MFN(PhysicsManager, CreateFixedJoint) },
        { "SliderJoint@ CreateSliderJoint(SliderJointInfo)", WRAP_MFN(PhysicsManager, CreateSliderJoint) }
    }, {});
}

void ScriptManager::RegisterTransform()
{
    AddValueType("Transform", sizeof(rp3d::Transform), asGetTypeTraits<rp3d::Transform>(),
    {
        { "void setPosition(const Vector3& in)", WRAP_MFN(rp3d::Transform, setPosition) },
        { "void setOrientation(const Quaternion& in)", WRAP_MFN(rp3d::Transform, setOrientation) },
        { "Vector3 getPosition()", WRAP_MFN(rp3d::Transform, getPosition) },
        { "Quaternion getOrientation()", WRAP_MFN(rp3d::Transform, getOrientation) }
    }, {});

    AddTypeConstructor("Transform", "void f()", WRAP_OBJ_LAST(MakeType<rp3d::Transform>));
    AddTypeConstructor("Transform", "void f(const Transform& in)", WRAP_OBJ_LAST(CopyType<rp3d::Transform>));
    AddTypeDestructor("Transform", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::Transform>));
}

void ScriptManager::RegisterRandom()
{
    AddValueType("random_device", sizeof(std::random_device), asGetTypeTraits<std::random_device>(),
    {
        { "uint opCall()", WRAP_MFN(std::random_device, operator()) }
    }, {});

    AddTypeConstructor("random_device", "void f()", WRAP_OBJ_LAST(MakeType<std::random_device>));
    AddTypeDestructor("random_device", "void f()", WRAP_OBJ_LAST(MakeType<std::random_device>));

    AddValueType("default_random_engine", sizeof(std::default_random_engine), asGetTypeTraits<std::default_random_engine>(),
    {
        { "double opCall(double, double)", WRAP_OBJ_LAST(GenerateRandomNumber) }
    }, {});

    AddTypeConstructor("default_random_engine", "void f(uint)", WRAP_OBJ_LAST(MakeDefaultRandomEngine));
    AddTypeDestructor("default_random_engine", "void f()", WRAP_OBJ_LAST(MakeType<std::default_random_engine>));
}

void ScriptManager::RegisterRay()
{
    AddValueType("Ray", sizeof(rp3d::Ray), asGetTypeTraits<rp3d::Ray>(),
    {
    	{ "Ray& opAssign(const Ray& in)", WRAP_OBJ_LAST(AssignType<rp3d::Ray>) }
    }, {});

    AddTypeConstructor("Ray", "void f(const Vector3& in, const Vector3& in)", WRAP_OBJ_LAST(MakeRay));
    AddTypeDestructor("Ray", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::Ray>));

    AddValueType("RaycastInfo", sizeof(rp3d::RaycastInfo), asGetTypeTraits<rp3d::RaycastInfo>(),
    {
        { "RaycastInfo& opAssign(const RaycastInfo& in)", WRAP_OBJ_LAST(AssignRaycastInfo) }
    },
    {
        { "Vector3 worldPoint", asOFFSET(rp3d::RaycastInfo, worldPoint) },
        { "Vector3 worldNormal", asOFFSET(rp3d::RaycastInfo, worldNormal) },
        { "float hitFraction", asOFFSET(rp3d::RaycastInfo, hitFraction) },
        { "int meshSubpart", asOFFSET(rp3d::RaycastInfo, meshSubpart) },
        { "int triangleIndex", asOFFSET(rp3d::RaycastInfo, triangleIndex) }
    });

    AddTypeConstructor("RaycastInfo", "void f()", WRAP_OBJ_LAST(MakeType<rp3d::RaycastInfo>));
    AddTypeDestructor("RaycastInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::RaycastInfo>));
}

void ScriptManager::RegisterJoints()
{
    AddValueType("HingeJointInfo", sizeof(rp3d::HingeJointInfo), asGetTypeTraits<rp3d::HingeJointInfo>() | asOBJ_POD, {}, {});

    AddTypeConstructor("HingeJointInfo", "void f(RigidBody@, RigidBody@, const Vector3& in, const Vector3& in)", WRAP_OBJ_LAST(MakeHingeJointInfo));
    AddTypeDestructor("HingeJointInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::HingeJointInfo>));

    AddType("HingeJoint", sizeof(rp3d::HingeJoint), {}, {});

    AddValueType("BallAndSocketJointInfo", sizeof(rp3d::BallAndSocketJointInfo), asGetTypeTraits<rp3d::BallAndSocketJointInfo>() | asOBJ_POD, {},
    {
        { "bool isUsingLocalSpaceAnchors", asOFFSET(rp3d::BallAndSocketJointInfo, isUsingLocalSpaceAnchors) },
        { "Vector3 anchorPointBody1LocalSpace", asOFFSET(rp3d::BallAndSocketJointInfo, anchorPointBody1LocalSpace) },
        { "Vector3 anchorPointBody2LocalSpace", asOFFSET(rp3d::BallAndSocketJointInfo, anchorPointBody2LocalSpace) }
    });

    AddTypeConstructor("BallAndSocketJointInfo", "void f(RigidBody@, RigidBody@, const Vector3& in)", WRAP_OBJ_LAST(MakeBallAndSocketJointInfo));
    AddTypeDestructor("BallAndSocketJointInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::BallAndSocketJointInfo>));

    AddType("BallAndSocketJoint", sizeof(rp3d::BallAndSocketJoint), {}, {});

    AddValueType("FixedJointInfo", sizeof(rp3d::FixedJointInfo), asGetTypeTraits<rp3d::FixedJointInfo>() | asOBJ_POD, {}, {});

    AddTypeConstructor("FixedJointInfo", "void f(RigidBody@, RigidBody@, const Vector3& in)", WRAP_OBJ_LAST(MakeFixedJointInfo));
    AddTypeDestructor("FixedJointInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::FixedJointInfo>));

    AddType("FixedJoint", sizeof(rp3d::FixedJoint), {}, {});

    AddValueType("SliderJointInfo", sizeof(rp3d::SliderJointInfo), asGetTypeTraits<rp3d::SliderJointInfo>() | asOBJ_POD, {}, {});

    AddTypeConstructor("SliderJointInfo", "void f(RigidBody@, RigidBody@, const Vector3& in, const Vector3& in)", WRAP_OBJ_LAST(MakeSliderJointInfo));
    AddTypeDestructor("SliderJointInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::SliderJointInfo>));

    AddType("SliderJoint", sizeof(rp3d::SliderJoint), {}, {});
}

void ScriptManager::RegisterClock()
{
	AddValueType("Time", sizeof(sf::Time), asGetTypeTraits<sf::Time>(),
	{
		{ "float asSeconds()", WRAP_MFN(sf::Time, asSeconds) },
		{ "int asMilliseconds()", WRAP_MFN(sf::Time, asMilliseconds) },
		{ "int64 asMicroseconds()", WRAP_MFN(sf::Time, asMicroseconds) }
	}, {});

	AddTypeConstructor("Time", "void f()", WRAP_OBJ_LAST(MakeType<sf::Time>));
    AddTypeConstructor("Time", "void f(const Time& in)", WRAP_OBJ_LAST(CopyType<sf::Time>));
	AddTypeDestructor("Time", "void f()", WRAP_OBJ_LAST(DestroyType<sf::Time>));

    AddFunction("Time seconds(float)", WRAP_FN(sf::seconds));

	AddValueType("Clock", sizeof(sf::Clock), asGetTypeTraits<sf::Clock>(),
	{
		{ "Time restart()", WRAP_MFN(sf::Clock, restart) },
		{ "Time getElapsedTime()", WRAP_MFN(sf::Clock, getElapsedTime) }
	}, {});

	AddTypeConstructor("Clock", "void f()", WRAP_OBJ_LAST(MakeType<sf::Clock>));
	AddTypeDestructor("Clock", "void f()", WRAP_OBJ_LAST(DestroyType<sf::Clock>));
}

void ScriptManager::RegisterSoundManager()
{
    SetDefaultNamespace("Listener");
    AddFunction("void SetPosition(const Vector3& in)", WRAP_FN(ListenerWrapper::SetPosition));
    AddFunction("void SetUpVector(const Vector3& in)", WRAP_FN(ListenerWrapper::SetUpVector));
    AddFunction("void SetOrientation(const Quaternion& in)", WRAP_FN(ListenerWrapper::SetOrientation));
    AddFunction("void SetGlobalVolume(float)", WRAP_FN(ListenerWrapper::SetGlobalVolume));
    SetDefaultNamespace("");

    AddType("SoundManager", sizeof(SoundManager),
    {
        { "void Play(const string& in, int = 0)", WRAP_MFN(SoundManager, Play) },
        { "void PlayAt(const string& in, int = 0, const Vector3& in = const Vector3& in(0, 0, 0))", WRAP_MFN(SoundManager, PlayAt) },
        { "void PlayMono(const string& in, int = 0)", WRAP_MFN(SoundManager, PlayMono) },
        { "void Stop(const string& in, int = 0)", WRAP_MFN(SoundManager, Stop) },
        { "void Pause(const string& in, int = 0)", WRAP_MFN(SoundManager, Pause) },
        { "void SetPosition(const Vector3& in, const string& in, int = 0)", WRAP_MFN(SoundManager, SetPosition) },
        { "void SetRelativeToListener(bool, const string& in, int = 0)", WRAP_MFN(SoundManager, SetRelativeToListener) },
        { "void SetLoop(bool, const string& in, int = 0)", WRAP_MFN(SoundManager, SetLoop) },
        { "void SetVolume(float, const string& in, int = 0)", WRAP_MFN(SoundManager, SetVolume) },
        { "void SetMinDistance(float, const string& in, int = 0)", WRAP_MFN(SoundManager, SetMinDistance) },
        { "void SetAttenuation(float, const string& in, int = 0)", WRAP_MFN(SoundManager, SetAttenuation) },
        { "void SetPitch(float, const string& in, int = 0)", WRAP_MFN(SoundManager, SetPitch) },
        { "Vector3 GetPosition(const string& in, int = 0)", WRAP_MFN(SoundManager, GetPosition) },
        { "bool GetRelativeToListener(const string& in, int = 0)", WRAP_MFN(SoundManager, GetRelativeToListener) },
        { "bool GetLoop(const string& in, int = 0)", WRAP_MFN(SoundManager, GetLoop) },
        { "float GetVolume(const string& in, int = 0)", WRAP_MFN(SoundManager, GetVolume) },
        { "float GetMinDistance(const string& in, int = 0)", WRAP_MFN(SoundManager, GetMinDistance) },
        { "float GetAttenuation(const string& in, int = 0)", WRAP_MFN(SoundManager, GetAttenuation) },
        { "float GetPitch(const string& in, int = 0)", WRAP_MFN(SoundManager, GetPitch) },
        { "void UpdateAll()", WRAP_MFN_PR(SoundManager, UpdateAll, (), void) },
        { "void UpdateAll(const string& in)", WRAP_MFN_PR(SoundManager, UpdateAll, (const std::string&), void) }
    }, {});
}

void ScriptManager::RegisterEngine()
{
    AddType("Engine", sizeof(Engine),
    {
        { "tgui::Gui@ CreateGui(string)", WRAP_MFN(Engine, CreateGui) },
        { "void RemoveGui()", WRAP_MFN(Engine, RemoveGui) },
        { "void Close()", WRAP_MFN(Engine, Close) }
    }, {});
}

void ScriptManager::RegisterTGUI()
{
    SetDefaultNamespace("tgui");

    AddValueType("String", sizeof(tgui::String), asGetTypeTraits<tgui::String>() | asOBJ_POD,
    {
        { "string toStdString() const", WRAP_MFN(tgui::String, toStdString) }
    }, {});
    AddTypeConstructor("String", "void f(const string& in)", WRAP_OBJ_LAST(MakeString));
    AddTypeConstructor("String", "void f(String& in)", WRAP_OBJ_LAST(CopyType<tgui::String>));

    AddValueType("Duration", sizeof(tgui::Duration), asGetTypeTraits<tgui::Duration>() | asOBJ_POD, {}, {});
    AddTypeConstructor("Duration", "void f(const Time& in)", WRAP_OBJ_LAST(MakeDuration));

    AddValueType("Color", sizeof(tgui::Color), asGetTypeTraits<tgui::Color>() | asOBJ_POD, {}, {});
    AddTypeConstructor("Color", "void f(uint8, uint8, uint8, uint8 = 255)", WRAP_OBJ_LAST(MakeColor));

    AddEnum("ShowEffectType", { "Fade", "Scale", "SlideToRight", "SlideToLeft", "SlideToBottom", "SlideToTop", "SlideFromLeft", "SlideFromRight", "SlideTFromTop", "SlideFromBottom" });

    engine->RegisterFuncdef("void OnPress()");

    // It's probably better to rewrite this whole mess
    AddType("BitmapButton", sizeof(tgui::BitmapButton),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void onPress(OnPress@)", WRAP_OBJ_LAST(OnPress) },
        { "void setImageScaling(float)", WRAP_MFN(tgui::BitmapButton, setImageScaling) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::BitmapButton, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::BitmapButton, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::BitmapButton, setFocused) },
        { "void setText(const String& in)", WRAP_MFN(tgui::BitmapButton, setText) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::BitmapButton, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::BitmapButton, hideWithEffect) }
    }, {});

    AddType("Button", sizeof(tgui::Button),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void onPress(OnPress@)", WRAP_OBJ_LAST(OnPress) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Button, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Button, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Button, setFocused) },
        { "void setText(const String& in)", WRAP_MFN(tgui::Button, setText) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Button, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Button, hideWithEffect) }
    }, {});

    AddType("ChatBox", sizeof(tgui::ChatBox),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void addLine(const String& in)", WRAP_MFN_PR(tgui::ChatBox, addLine, (const tgui::String&), void) },
        { "void addLine(const String& in, Color)", WRAP_MFN_PR(tgui::ChatBox, addLine, (const tgui::String&, tgui::Color), void) },
        { "bool removeLine(uint)", WRAP_MFN(tgui::ChatBox, removeLine) },
        { "void removeAllLines()", WRAP_MFN(tgui::ChatBox, removeAllLines) },
        { "uint getLineAmount()", WRAP_MFN(tgui::ChatBox, getLineAmount) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ChatBox, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ChatBox, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ChatBox, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ChatBox, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ChatBox, hideWithEffect) }
    }, {});

    AddType("CheckBox", sizeof(tgui::CheckBox),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setChecked(bool)", WRAP_MFN(tgui::CheckBox, setChecked) },
        { "bool isChecked()", WRAP_MFN(tgui::CheckBox, isChecked) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::CheckBox, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::CheckBox, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::CheckBox, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::CheckBox, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::CheckBox, hideWithEffect) }
    }, {});

    AddType("ChildWindow", sizeof(tgui::ChildWindow),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ChildWindow, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ChildWindow, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ChildWindow, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ChildWindow, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ChildWindow, hideWithEffect) }
    }, {});

    AddType("ComboBox", sizeof(tgui::ComboBox),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void addItem(const String& in, const String& in = \"\")", WRAP_MFN(tgui::ComboBox, addItem) },
        { "bool setSelectedItem(const String& in)", WRAP_MFN(tgui::ComboBox, setSelectedItem) },
        { "bool setSelectedItemById(const String& in)", WRAP_MFN(tgui::ComboBox, setSelectedItemById) },
        { "bool setSelectedItemByIndex(uint)", WRAP_MFN(tgui::ComboBox, setSelectedItemByIndex) },
        { "String getSelectedItem()", WRAP_MFN(tgui::ComboBox, getSelectedItem) },
        { "void deselectItem()", WRAP_MFN(tgui::ComboBox, deselectItem) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ComboBox, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ComboBox, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ComboBox, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ComboBox, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ComboBox, hideWithEffect) }
    }, {});

    AddType("EditBox", sizeof(tgui::EditBox),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setText(const String& in)", WRAP_MFN(tgui::EditBox, setText) },
        { "const String& getText() const", WRAP_MFN(tgui::EditBox, getText) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::EditBox, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::EditBox, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::EditBox, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::EditBox, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::EditBox, hideWithEffect) }
    }, {});

    AddType("Group", sizeof(tgui::Group),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Group, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Group, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Group, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Group, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Group, hideWithEffect) }
    }, {});

    AddType("Knob", sizeof(tgui::Knob),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Knob, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Knob, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Knob, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Knob, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Knob, hideWithEffect) }
    }, {});

    AddType("Label", sizeof(tgui::Label),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setText(const String& in)", WRAP_MFN(tgui::Label, setText) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Label, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Label, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Label, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Label, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Label, hideWithEffect) }
    }, {});

    AddType("ListBox", sizeof(tgui::ListBox),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ListBox, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ListBox, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ListBox, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ListBox, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ListBox, hideWithEffect) }
    }, {});

    AddType("ListView", sizeof(tgui::ListView),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ListView, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ListView, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ListView, setFocused) },
        { "uint64 addItem(const String& in)", WRAP_MFN_PR(tgui::ListView, addItem, (const tgui::String&), size_t) },
        { "uint64 addItem(const array<string>& in)", WRAP_OBJ_LAST(AddListViewItem) },
        { "void insertItem(uint64, const String& in)", WRAP_MFN_PR(tgui::ListView, insertItem, (size_t, const tgui::String&), void) },
        { "void insertItem(uint64, const array<string>& in)", WRAP_OBJ_LAST(InsertListViewItem) },
        { "bool changeItem(uint64, const array<string>& in)", WRAP_OBJ_LAST(ChangeListViewItem) },
        { "bool removeItem(uint64)", WRAP_MFN(tgui::ListView, removeItem) },
        { "void removeAllItems()", WRAP_MFN(tgui::ListView, removeAllItems) },
        { "int getSelectedItemIndex()", WRAP_MFN(tgui::ListView, getSelectedItemIndex) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ListView, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ListView, hideWithEffect) }
    }, {});

    AddType("Panel", sizeof(tgui::Panel),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Panel, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Panel, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Panel, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Panel, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Panel, hideWithEffect) }
    }, {});

    AddType("Picture", sizeof(tgui::Picture),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Picture, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Picture, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Picture, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Picture, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Picture, hideWithEffect) }
    }, {});

    AddType("ProgressBar", sizeof(tgui::ProgressBar),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setValue(uint)", WRAP_MFN(tgui::ProgressBar, setValue) },
        { "uint getValue()", WRAP_MFN(tgui::ProgressBar, getValue) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ProgressBar, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::ProgressBar, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::ProgressBar, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ProgressBar, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::ProgressBar, hideWithEffect) }
    }, {});

    AddType("RadioButton", sizeof(tgui::RadioButton),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setChecked(bool)", WRAP_MFN(tgui::RadioButton, setChecked) },
        { "bool isChecked()", WRAP_MFN(tgui::RadioButton, isChecked) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::RadioButton, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::RadioButton, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::RadioButton, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RadioButton, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RadioButton, hideWithEffect) }
    }, {});

    AddType("RangeSlider", sizeof(tgui::RangeSlider),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setSelectionStart(float)", WRAP_MFN(tgui::RangeSlider, setSelectionStart) },
        { "void setSelectionEnd(float)", WRAP_MFN(tgui::RangeSlider, setSelectionEnd) },
        { "float getSelectionStart()", WRAP_MFN(tgui::RangeSlider, getSelectionStart) },
        { "float getSelectionEnd()", WRAP_MFN(tgui::RangeSlider, getSelectionEnd) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::RangeSlider, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::RangeSlider, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::RangeSlider, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RangeSlider, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RangeSlider, hideWithEffect) }
    }, {});

    AddType("RichTextLabel", sizeof(tgui::RichTextLabel),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setText(const String& in)", WRAP_MFN(tgui::RichTextLabel, setText) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::RichTextLabel, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::RichTextLabel, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::RichTextLabel, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RichTextLabel, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::RichTextLabel, hideWithEffect) }
    }, {});

    AddType("Slider", sizeof(tgui::Slider),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setValue(float)", WRAP_MFN(tgui::Slider, setValue) },
        { "float getValue()", WRAP_MFN(tgui::Slider, getValue) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Slider, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Slider, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Slider, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Slider, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Slider, hideWithEffect) }
    }, {});

    AddType("SpinButton", sizeof(tgui::SpinButton),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::SpinButton, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::SpinButton, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::SpinButton, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::SpinButton, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::SpinButton, hideWithEffect) }
    }, {});

    AddType("SpinControl", sizeof(tgui::SpinControl),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::SpinControl, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::SpinControl, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::SpinControl, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::SpinControl, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::SpinControl, hideWithEffect) }
    }, {});

    AddType("Tabs", sizeof(tgui::Tabs),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Tabs, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::Tabs, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::Tabs, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Tabs, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::Tabs, hideWithEffect) }
    }, {});

    AddType("TextArea", sizeof(tgui::TextArea),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "String getText() const", WRAP_MFN(tgui::TextArea, getText) },
        { "void setText(const String& in)", WRAP_MFN(tgui::TextArea, setText) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::TextArea, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::TextArea, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::TextArea, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::TextArea, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::TextArea, hideWithEffect) }
    }, {});

    AddType("TreeView", sizeof(tgui::TreeView),
    {
        { "void setPosition(float, float)", WRAP_OBJ_LAST(SetPosition) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::TreeView, setEnabled) },
        { "void setVisible(bool)", WRAP_MFN(tgui::TreeView, setVisible) },
        { "void setFocused(bool)", WRAP_MFN(tgui::TreeView, setFocused) },
        { "void showWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::TreeView, showWithEffect) },
        { "void hideWithEffect(ShowEffectType, Duration)", WRAP_MFN(tgui::TreeView, hideWithEffect) }
    }, {});

    AddType("Gui", sizeof(tgui::Gui),
    {
        { "void setOpacity(float)", WRAP_MFN(tgui::Gui, setOpacity) },
        { "float getOpacity() const", WRAP_MFN(tgui::Gui, getOpacity) },
        { "BitmapButton@ getBitmapButton(string)", WRAP_OBJ_LAST(GetWidget<tgui::BitmapButton>) },
        { "Button@ getButton(string)", WRAP_OBJ_LAST(GetWidget<tgui::Button>) },
        { "ChatBox@ getChatBox(string)", WRAP_OBJ_LAST(GetWidget<tgui::ChatBox>) },
        { "CheckBox@ getCheckBox(string)", WRAP_OBJ_LAST(GetWidget<tgui::CheckBox>) },
        { "ChildWindow@ getChildWindow(string)", WRAP_OBJ_LAST(GetWidget<tgui::ChildWindow>) },
        { "ComboBox@ getComboBox(string)", WRAP_OBJ_LAST(GetWidget<tgui::ComboBox>) },
        { "EditBox@ getEditBox(string)", WRAP_OBJ_LAST(GetWidget<tgui::EditBox>) },
        { "Group@ getGroup(string)", WRAP_OBJ_LAST(GetWidget<tgui::Group>) },
        { "Knob@ getKnob(string)", WRAP_OBJ_LAST(GetWidget<tgui::Knob>) },
        { "Label@ getLabel(string)", WRAP_OBJ_LAST(GetWidget<tgui::Label>) },
        { "ListBox@ getListBox(string)", WRAP_OBJ_LAST(GetWidget<tgui::ListBox>) },
        { "ListView@ getListView(string)", WRAP_OBJ_LAST(GetWidget<tgui::ListView>) },
        { "Panel@ getPanel(string)", WRAP_OBJ_LAST(GetWidget<tgui::Panel>) },
        { "Picture@ getPicture(string)", WRAP_OBJ_LAST(GetWidget<tgui::Picture>) },
        { "ProgressBar@ getProgressBar(string)", WRAP_OBJ_LAST(GetWidget<tgui::ProgressBar>) },
        { "RadioButton@ getRadioButton(string)", WRAP_OBJ_LAST(GetWidget<tgui::RadioButton>) },
        { "RangeSlider@ getRangeSlider(string)", WRAP_OBJ_LAST(GetWidget<tgui::RangeSlider>) },
        { "RichTextLabel@ getRichTextLabel(string)", WRAP_OBJ_LAST(GetWidget<tgui::RichTextLabel>) },
        { "Slider@ getSlider(string)", WRAP_OBJ_LAST(GetWidget<tgui::Slider>) },
        { "SpinButton@ getSpinButton(string)", WRAP_OBJ_LAST(GetWidget<tgui::SpinButton>) },
        { "SpinControl@ getSpinControl(string)", WRAP_OBJ_LAST(GetWidget<tgui::SpinControl>) },
        { "Tabs@ getTabs(string)", WRAP_OBJ_LAST(GetWidget<tgui::Tabs>) },
        { "TextArea@ getTextArea(string)", WRAP_OBJ_LAST(GetWidget<tgui::TextArea>) },
        { "TreeView@ getTreeView(string)", WRAP_OBJ_LAST(GetWidget<tgui::TreeView>) },
        { "void copyBitmapButton(BitmapButton@, string)", WRAP_OBJ_LAST(CopyWidget) },
        { "void copyButton(Button@, string)", WRAP_OBJ_LAST(CopyWidget) },
        { "void copyLabel(Label@, string)", WRAP_OBJ_LAST(CopyWidget) },
        { "void copyPicture(Picture@, string)", WRAP_OBJ_LAST(CopyWidget) },
        { "void copyProgressBar(ProgressBar@, string)", WRAP_OBJ_LAST(CopyWidget) },
        { "void copyRichTextLabel(RichTextLabel@, string)", WRAP_OBJ_LAST(CopyWidget) }
    }, {});
    
    SetDefaultNamespace("");
}

void ScriptManager::RegisterMaterial()
{
    SetDefaultNamespace("MaterialParameter");
    AddEnum("Type", { "Color", "Normal", "AmbientOcclusion", "Metalness",
                      "Emission", "EmissionStrength", "Roughness", "Opacity" });
    SetDefaultNamespace("");
    AddType("Material", sizeof(Material),
    {
        { "void SetParameter(Vector3, int)", WRAP_OBJ_LAST(SetMaterialParameter) },
        { "Vector3 GetParameter(int)", WRAP_OBJ_LAST(GetMaterialParameter) },
        { "bool Contains(int)", WRAP_MFN(Material, Contains) }
    }, {});
}

void ScriptManager::RegisterNetwork()
{
    AddValueType("IpAddress", sizeof(sf::IpAddress), asGetTypeTraits<sf::IpAddress>(),
    {
        { "string toString()", WRAP_MFN(sf::IpAddress, toString) }
    }, {});

    AddTypeConstructor("IpAddress", "void f(const IpAddress& in)", WRAP_OBJ_LAST(CopyType<sf::IpAddress>));
    AddTypeDestructor("IpAddress", "void f()", WRAP_OBJ_LAST(DestroyType<sf::IpAddress>));

    AddFunction("IpAddress ResolveIp(string)", WRAP_FN(ResolveIp));

    AddValueType("Packet", sizeof(sf::Packet), asGetTypeTraits<sf::Packet>(),
    {
        { "void clear()", WRAP_MFN(sf::Packet, clear) },
        { "Packet& opShl(const string& in)", WRAP_MFN_PR(sf::Packet, operator<<, (const std::string&), sf::Packet&) },
        { "Packet& opShl(int)", WRAP_MFN_PR(sf::Packet, operator<<, (int), sf::Packet&) },
        { "Packet& opShl(float)", WRAP_MFN_PR(sf::Packet, operator<<, (float), sf::Packet&) },
        { "Packet& opShl(bool)", WRAP_MFN_PR(sf::Packet, operator<<, (bool), sf::Packet&) },
        { "Packet& opShr(string& out)", WRAP_MFN_PR(sf::Packet, operator>>, (std::string&), sf::Packet&) },
        { "Packet& opShr(int& out)", WRAP_MFN_PR(sf::Packet, operator>>, (int&), sf::Packet&) },
        { "Packet& opShr(float& out)", WRAP_MFN_PR(sf::Packet, operator>>, (float&), sf::Packet&) },
        { "Packet& opShr(bool& out)", WRAP_MFN_PR(sf::Packet, operator>>, (bool&), sf::Packet&) },
        { "Packet& opAssign(const Packet& in)", WRAP_MFN_PR(sf::Packet, operator=, (const sf::Packet&), sf::Packet&) },
        { "bool opImplConv()", WRAP_MFN(sf::Packet, operator bool) }
    }, {});

    AddTypeConstructor("Packet", "void f()", WRAP_OBJ_LAST(MakeType<sf::Packet>));
    AddTypeConstructor("Packet", "void f(const Packet& in)", WRAP_OBJ_LAST(CopyType<sf::Packet>));
    AddTypeDestructor("Packet", "void f()", WRAP_OBJ_LAST(DestroyType<sf::Packet>));

    SetDefaultNamespace("Socket");
    AddEnum("Status", { "Done", "NotReady", "Partial", "Disconnected", "Error" });
    SetDefaultNamespace("");

    AddValueType("TcpSocket", sizeof(sf::TcpSocket), asGetTypeTraits<sf::TcpSocket>(),
    {
        { "Socket::Status connect(const IpAddress& in, uint16, Time)", WRAP_MFN(sf::TcpSocket, connect) },
        { "void disconnect()", WRAP_MFN(sf::TcpSocket, disconnect) },
        { "Socket::Status send(Packet& in)", WRAP_MFN_PR(sf::TcpSocket, send, (sf::Packet&), sf::Socket::Status) },
        { "Socket::Status receive(Packet& out)", WRAP_MFN_PR(sf::TcpSocket, receive, (sf::Packet&), sf::Socket::Status) },
        { "void setBlocking(bool)", WRAP_MFN(sf::TcpSocket, setBlocking) },
        { "bool isBlocking()", WRAP_MFN(sf::TcpSocket, isBlocking) }
    }, {});

    AddTypeConstructor("TcpSocket", "void f()", WRAP_OBJ_LAST(MakeType<sf::TcpSocket>));
    AddTypeDestructor("TcpSocket", "void f()", WRAP_OBJ_LAST(DestroyType<sf::TcpSocket>));

    AddValueType("UdpSocket", sizeof(sf::UdpSocket), asGetTypeTraits<sf::UdpSocket>() | asOBJ_POD,
    {
        { "Socket::Status bind(uint16, const IpAddress& in)", WRAP_MFN(sf::UdpSocket, bind) },
        { "void unbind()", WRAP_MFN(sf::UdpSocket, unbind) },
        { "Socket::Status send(Packet& in, const IpAddress& in, uint)", WRAP_MFN_PR(sf::UdpSocket, send, (sf::Packet&, const sf::IpAddress&, unsigned short), sf::Socket::Status) },
        { "Socket::Status receive(Packet& out, const IpAddress& in, uint)", WRAP_MFN_PR(sf::UdpSocket, receive, (sf::Packet&, std::optional<sf::IpAddress>&, unsigned short&), sf::Socket::Status) },
        { "void setBlocking(bool)", WRAP_MFN(sf::UdpSocket, setBlocking) },
        { "bool isBlocking()", WRAP_MFN(sf::UdpSocket, isBlocking) }
    }, {});

    AddTypeConstructor("UdpSocket", "void f()", WRAP_OBJ_LAST(MakeType<sf::UdpSocket>));
    AddTypeDestructor("UdpSocket", "void f()", WRAP_OBJ_LAST(DestroyType<sf::UdpSocket>));
}

void ScriptManager::RegisterBone()
{
    AddType("Bone", sizeof(Bone),
    {
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Bone, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Bone, SetOrientation) },
        { "void SetSize(const Vector3& in)", WRAP_MFN(Bone, SetSize) },
        { "void Move(const Vector3& in)", WRAP_MFN(Bone, Move) },
        { "void Rotate(const Quaternion& in)", WRAP_MFN(Bone, Rotate) },
        { "void Expand(const Vector3& in)", WRAP_MFN(Bone, Expand) },
        { "Node@ opCast()", WRAP_OBJ_LAST(CastBone) },
        { "Vector3 GetPosition()", WRAP_MFN(Bone, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Bone, GetOrientation) },
        { "Vector3 GetSize()", WRAP_MFN(Bone, GetSize) }
    }, {});

    engine->RegisterObjectMethod("Node", "Bone@ opCast()", WRAP_OBJ_LAST(CastToBone), asCALL_GENERIC);
}

void ScriptManager::RegisterAnimation()
{
    AddEnum("AnimationState", { "Stopped", "Playing", "Paused" });

    AddType("Animation", sizeof(Animation),
    {
        { "void Play()", WRAP_MFN(Animation, Play) },
        { "void Pause()", WRAP_MFN(Animation, Pause) },
        { "void Stop()", WRAP_MFN(Animation, Stop) },
        { "void SetIsRepeated(bool)", WRAP_MFN(Animation, SetIsRepeated) },
        { "void SetTPS(float)", WRAP_MFN(Animation, SetTPS) },
        { "AnimationState GetState()", WRAP_MFN(Animation, GetState) }
    }, {});
}

void ScriptManager::RegisterShader()
{
    AddType("Shader", sizeof(Shader),
    {
        { "void SetUniform1i(const string& in, int)", WRAP_MFN(Shader, SetUniform1i) },
        { "void SetUniform1f(const string& in, float)", WRAP_MFN(Shader, SetUniform1f) },
        { "void SetUniform2f(const string& in, float, float)", WRAP_MFN(Shader, SetUniform2f) },
        { "void SetUniform3f(const string& in, float, float, float)", WRAP_MFN(Shader, SetUniform3f) }
    }, {});

    AddTypeFactory("Shader", "Shader@ f(const string& in, const string& in, bool = true)", WRAP_FN(MakeShader));
}

void ScriptManager::RegisterNode()
{
    AddType("Node", sizeof(Node),
    {
        { "void AddChild(Node@)", WRAP_MFN(Node, AddChild) },
        { "void SetParent(Node@)", WRAP_MFN(Node, SetParent) }
    }, {});

    AddFunction("Transform GetFinalTransform(Node@, Transform = Transform())", WRAP_FN(Node::GetFinalTransform));
}
