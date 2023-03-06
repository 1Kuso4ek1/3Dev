#include <ScriptManager.hpp>

ScriptManager::ScriptManager() : engine(asCreateScriptEngine())
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMath(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFile(engine);
    RegisterVector3();
    RegisterQuaternion();
    RegisterTransform();
    RegisterRay();
    RegisterRigidBody();
    RegisterJoints();
    RegisterPhysicsManager();
    RegisterSoundManager();
    RegisterCamera();
    RegisterMaterial();
    RegisterModel();
    RegisterLight();
    RegisterSceneManager();
    RegisterSfKeyboard();
    RegisterSfMouse();
    RegisterRandom();
    RegisterClock();
    RegisterTGUI();
    RegisterEngine();

    AddFunction("string to_string(int)", WRAP_FN_PR(std::to_string, (int), std::string));
    AddFunction("string to_string(float)", WRAP_FN_PR(std::to_string, (float), std::string));

    AddFunction("int stoi(const string& in)", WRAP_FN_PR(std::stoi, (const std::string&, size_t*, int), int));
    AddFunction("float stof(const string& in)", WRAP_FN_PR(std::stof, (const std::string&, size_t*), float));

    AddFunction("float radians(float)", WRAP_FN(glm::radians<float>));
    AddFunction("float degrees(float)", WRAP_FN(glm::degrees<float>));

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
}

void ScriptManager::Load(std::string filename)
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
        scripts.push_back(std::filesystem::absolute(data["scripts"][i].asString()).string());
}

bool ScriptManager::LoadScript(std::string filename)
{
    CScriptBuilder temp;
    temp.StartNewModule(engine, "temp");
    temp.AddSectionFromFile(filename.c_str());
    bool ret = temp.BuildModule() >= 0;
    scripts.push_back(filename);
    engine->DiscardModule("temp");
    return ret;
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

void ScriptManager::RegisterVector3()
{
    AddValueType("Vector3", sizeof(rp3d::Vector3), asGetTypeTraits<rp3d::Vector3>(),
    {
        { "float length() const", WRAP_MFN(rp3d::Vector3, length) },
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
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Model, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Model, SetOrientation) },
        { "void SetSize(const Vector3& in)", WRAP_MFN(Model, SetSize) },
        { "void SetPhysicsManager(PhysicsManager@)", WRAP_MFN(Model, SetPhysicsManager) },
        { "void SetIsDrawable(bool)", WRAP_MFN(Model, SetIsDrawable) },
        { "void CreateRigidBody()", WRAP_MFN(Model, CreateRigidBody) },
        { "void Move(const Vector3& in)", WRAP_MFN(Model, Move) },
        { "void Rotate(const Quaternion& in)", WRAP_MFN(Model, Rotate) },
        { "void Expand(const Vector3& in)", WRAP_MFN(Model, Expand) },
        { "bool IsDrawable()", WRAP_MFN(Model, IsDrawable) },
        { "Vector3 GetPosition()", WRAP_MFN(Model, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Model, GetOrientation) },
        { "Vector3 GetSize()", WRAP_MFN(Model, GetSize) },
        { "RigidBody@ GetRigidBody()", WRAP_MFN(Model, GetRigidBody) },
        { "void PlayAnimation(int = 0)", WRAP_MFN(Model, PlayAnimation) },
        { "void StopAnimation(int = 0)", WRAP_MFN(Model, StopAnimation) },
        { "void PauseAnimation(int = 0)", WRAP_MFN(Model, PauseAnimation) },
        { "void AutoUpdateAnimation(bool = true)", WRAP_MFN(Model, AutoUpdateAnimation) },
        { "int GetMeshesCount()", WRAP_MFN(Model, GetMeshesCount) },
        { "int GetAnimationsCount()", WRAP_MFN(Model, GetAnimationsCount) }
    }, {});
}

void ScriptManager::RegisterLight()
{
    AddType("Light", sizeof(Light),
    {
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Light, SetPosition) },
        { "void SetDirection(const Vector3& in)", WRAP_MFN(Light, SetDirection) },
        { "void SetColor(const Vector3& in)", WRAP_MFN(Light, SetColor) },
        { "void SetAttenuation(float, float, float)", WRAP_MFN(Light, SetAttenuation) },
        { "void SetCutoff(float)", WRAP_MFN(Light, SetCutoff) },
        { "void SetOuterCutoff(float)", WRAP_MFN(Light, SetCutoff) },
        { "void SetIsCastingShadows(bool)", WRAP_MFN(Light, SetIsCastingShadows) },
        { "bool IsCastingShadows()", WRAP_MFN(Light, IsCastingShadows) },
        { "Vector3 GetPosition()", WRAP_MFN(Light, GetPosition) },
        { "Vector3 GetDirection()", WRAP_MFN(Light, GetDirection) },
        { "Vector3 GetColor()", WRAP_MFN(Light, GetColor) },
        { "Vector3 GetAttenuation()", WRAP_MFN(Light, GetAttenuation) },
        { "float GetCutoff()", WRAP_MFN(Light, GetCutoff) },
        { "float GetOuterCutoff()", WRAP_MFN(Light, GetOuterCutoff) }
    }, {});
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
        { "void AlwaysUp(bool)", WRAP_MFN(Camera, AlwaysUp) },
        { "Vector3 GetPosition()", WRAP_MFN(Camera, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Camera, GetOrientation) },
        { "void GetSpeed(float)", WRAP_MFN(Camera, GetSpeed) },
        { "void GetFOV(float)", WRAP_MFN(Camera, GetFOV) },
        { "void Look()", WRAP_MFN_PR(Camera, Look, (), void) },
        { "void Look(const Vector3& in)", WRAP_MFN_PR(Camera, Look, (const rp3d::Vector3&), void) }
    }, {});
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
        { "Model@ GetModel(string)", WRAP_MFN(SceneManager, GetModelPtr) },
        { "Light@ GetLight(string)", WRAP_MFN(SceneManager, GetLight) },
        { "Material@ GetMaterial(string)", WRAP_MFN(SceneManager, GetMaterialPtr) },
        { "Model@ CloneModel(Model@, bool = true, string = \"model\")", WRAP_MFN(SceneManager, CloneModel) },
        { "Camera@ GetCamera()", WRAP_MFN(SceneManager, GetCamera) },
        { "PhysicsManager@ GetPhysicsManager()", WRAP_MFN(SceneManager, GetPhysicsManagerPtr) },
        { "SoundManager@ GetSoundManager()", WRAP_MFN(SceneManager, GetSoundManagerPtr) },
        { "void RemoveModel(Model@)", WRAP_MFN(SceneManager, RemoveModel) },
        { "void RemoveLight(Light@)", WRAP_MFN(SceneManager, RemoveModel) },
        { "void Save(string filename)", WRAP_MFN(SceneManager, Save) },
        { "void Load(string filename)", WRAP_MFN(SceneManager, Load) },
        { "ModelGroup GetModelGroup(string)", WRAP_MFN(SceneManager, GetModelPtrGroup) },
        { "void UpdatePhysics(bool)", WRAP_MFN(SceneManager, UpdatePhysics) },
        { "void SaveState()", WRAP_MFN(SceneManager, SaveState) },
        { "void LoadState()", WRAP_MFN(SceneManager, LoadState) }
    }, {});
}

void ScriptManager::RegisterSfKeyboard()
{
    SetDefaultNamespace("Keyboard");
    AddFunction("bool isKeyPressed(int)", WRAP_FN(sf::Keyboard::isKeyPressed));
    AddEnum("Key", { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
                     "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Num0", "Num1", "Num2", "Num3",
                     "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "Escape", "LControl", "LShift", "LAlt", "LSystem",
                     "RControl", "RShift", "RAlt", "RSystem", "Menu", "LBracket", "RBracket", "Semicolon", "Comma",
                     "Period", "Quote", "Slash", "Backslash", "Tilde", "Equal", "Hyphen", "Space", "Enter", "Backspace",
                     "Tab", "PageUp", "PageDown", "End", "Home", "Insert", "Delete", "Add", "Substract", "Multiply", "Divide",
                     "Left", "Right", "Up", "Down", "Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5",
                     "Numpad6", "Numpad7", "Numpad8", "Numpad9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
                     "F10", "F11", "F12", "F13", "F14", "F15", "Pause" });
    SetDefaultNamespace("");
}

void ScriptManager::RegisterSfMouse()
{
    SetDefaultNamespace("Mouse");
    AddFunction("bool isButtonPressed(int)", WRAP_FN(sf::Keyboard::isKeyPressed));
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
    AddValueType("Transform", sizeof(rp3d::Transform), asGetTypeTraits<rp3d::Transform>(), {}, {});
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
		{ "float asMilliseconds()", WRAP_MFN(sf::Time, asMilliseconds) },
		{ "float asMicroseconds()", WRAP_MFN(sf::Time, asMicroseconds) }
	}, {});

	AddTypeConstructor("Time", "void f()", WRAP_OBJ_LAST(MakeType<sf::Time>));
	AddTypeDestructor("Time", "void f()", WRAP_OBJ_LAST(DestroyType<sf::Time>));

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
    AddFunction("void SetPosition(Vector3)", WRAP_FN(ListenerWrapper::SetPosition));
    AddFunction("void SetUpVector(Vector3)", WRAP_FN(ListenerWrapper::SetUpVector));
    AddFunction("void SetOrientation(Quaternion)", WRAP_FN(ListenerWrapper::SetOrientation));
    AddFunction("void SetGlobalVolume(float)", WRAP_FN(ListenerWrapper::SetGlobalVolume));
    SetDefaultNamespace("");

    AddType("SoundManager", sizeof(SoundManager),
    {
        { "void Play(string, int = 0)", WRAP_MFN(SoundManager, Play) },
        { "void PlayAt(string, int = 0, Vector3 = Vector3(0, 0, 0))", WRAP_MFN(SoundManager, PlayAt) },
        { "void PlayMono(string, int = 0)", WRAP_MFN(SoundManager, PlayMono) },
        { "void Stop(string, int = 0)", WRAP_MFN(SoundManager, Stop) },
        { "void Pause(string, int = 0)", WRAP_MFN(SoundManager, Pause) },
        { "void SetPosition(Vector3, string, int = 0)", WRAP_MFN(SoundManager, SetPosition) },
        { "void SetRelativeToListener(bool, string, int = 0)", WRAP_MFN(SoundManager, SetRelativeToListener) },
        { "void SetLoop(bool, string, int = 0)", WRAP_MFN(SoundManager, SetLoop) },
        { "void SetVolume(float, string, int = 0)", WRAP_MFN(SoundManager, SetVolume) },
        { "void SetMinDistance(float, string, int = 0)", WRAP_MFN(SoundManager, SetMinDistance) },
        { "void SetAttenuation(float, string, int = 0)", WRAP_MFN(SoundManager, SetAttenuation) },
        { "void SetPitch(float, string, int = 0)", WRAP_MFN(SoundManager, SetPitch) },
        { "Vector3 GetPosition(string, int = 0)", WRAP_MFN(SoundManager, GetPosition) },
        { "bool GetRelativeToListener(string, int = 0)", WRAP_MFN(SoundManager, GetRelativeToListener) },
        { "bool GetLoop(string, int = 0)", WRAP_MFN(SoundManager, GetLoop) },
        { "float GetVolume(string, int = 0)", WRAP_MFN(SoundManager, GetVolume) },
        { "float GetMinDistance(string, int = 0)", WRAP_MFN(SoundManager, GetMinDistance) },
        { "float GetAttenuation(string, int = 0)", WRAP_MFN(SoundManager, GetAttenuation) },
        { "float GetPitch(string, int = 0)", WRAP_MFN(SoundManager, GetPitch) },
        { "void UpdateAll()", WRAP_MFN_PR(SoundManager, UpdateAll, (), void) },
        { "void UpdateAll(string)", WRAP_MFN_PR(SoundManager, UpdateAll, (std::string), void) }
    }, {});
}

void ScriptManager::RegisterEngine()
{
    AddType("Engine", sizeof(Engine),
    {
        { "tgui::Gui@ CreateGui(string)", WRAP_MFN(Engine, CreateGui) },
        { "void RemoveGui()", WRAP_MFN(Engine, RemoveGui) }
    }, {});
}

void ScriptManager::RegisterTGUI()
{
    SetDefaultNamespace("tgui");

    AddValueType("String", sizeof(tgui::String), asGetTypeTraits<tgui::String>() | asOBJ_POD,
    {
        { "string toStdString()", WRAP_MFN(tgui::String, toStdString) }
    }, {});
    AddTypeConstructor("String", "void f(const string& in)", WRAP_OBJ_LAST(MakeString));

    AddValueType("Color", sizeof(tgui::Color), asGetTypeTraits<tgui::Color>() | asOBJ_POD, {}, {});
    AddTypeConstructor("Color", "void f(uint8, uint8, uint8, uint8 = 255)", WRAP_OBJ_LAST(MakeColor));

    engine->RegisterFuncdef("void OnPress()");
    AddType("BitmapButton", sizeof(tgui::BitmapButton),
    {
        { "void onPress(OnPress@)", WRAP_OBJ_LAST(OnPress) },
        { "void setImageScaling(float)", WRAP_MFN(tgui::BitmapButton, setImageScaling) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::BitmapButton, setEnabled) },
        { "void setText(const String& in)", WRAP_MFN(tgui::BitmapButton, setText) }
    }, {});

    AddType("Button", sizeof(tgui::Button),
    {
        { "void onPress(OnPress@)", WRAP_OBJ_LAST(OnPress) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Button, setEnabled) },
        { "void setText(const String& in)", WRAP_MFN(tgui::Button, setText) }
    }, {});

    AddType("ChatBox", sizeof(tgui::ChatBox),
    {
        { "void addLine(const String& in)", WRAP_MFN_PR(tgui::ChatBox, addLine, (const tgui::String&), void) },
        { "void addLine(const String& in, Color)", WRAP_MFN_PR(tgui::ChatBox, addLine, (const tgui::String&, tgui::Color), void) },
        { "bool removeLine(uint)", WRAP_MFN(tgui::ChatBox, removeLine) },
        { "void removeAllLines()", WRAP_MFN(tgui::ChatBox, removeAllLines) },
        { "uint getLineAmount()", WRAP_MFN(tgui::ChatBox, getLineAmount) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ChatBox, setEnabled) }
    }, {});

    AddType("CheckBox", sizeof(tgui::CheckBox),
    {
        { "void setChecked(bool)", WRAP_MFN(tgui::CheckBox, setChecked) },
        { "bool isChecked()", WRAP_MFN(tgui::CheckBox, isChecked) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::CheckBox, setEnabled) }
    }, {});

    AddType("ChildWindow", sizeof(tgui::ChildWindow),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::ChildWindow, setEnabled) }
    }, {});

    AddType("ComboBox", sizeof(tgui::ComboBox),
    {
        { "void addItem(const String& in, const string& in = \"\")", WRAP_MFN(tgui::ComboBox, addItem) },
        { "bool setSelectedItem(const String& in)", WRAP_MFN(tgui::ComboBox, setSelectedItem) },
        { "bool setSelectedItemById(const String& in)", WRAP_MFN(tgui::ComboBox, setSelectedItemById) },
        { "bool setSelectedItemByIndex(uint)", WRAP_MFN(tgui::ComboBox, setSelectedItemByIndex) },
        { "String getSelectedItem()", WRAP_MFN(tgui::ComboBox, getSelectedItem) },
        { "void deselectItem()", WRAP_MFN(tgui::ComboBox, deselectItem) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::ComboBox, setEnabled) }
    }, {});

    AddType("EditBox", sizeof(tgui::EditBox),
    {
        { "void setText(const String& in)", WRAP_MFN(tgui::EditBox, setText) },
        { "String getText()", WRAP_MFN(tgui::EditBox, getText) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::EditBox, setEnabled) }
    }, {});

    AddType("Group", sizeof(tgui::Group),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::Group, setEnabled) }
    }, {});

    AddType("Knob", sizeof(tgui::Knob),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::Knob, setEnabled) }
    }, {});

    AddType("Label", sizeof(tgui::Label),
    {
        { "void setText(const String& in)", WRAP_MFN(tgui::Label, setText) },
    }, {});

    AddType("ListBox", sizeof(tgui::ListBox),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::ListBox, setEnabled) }
    }, {});

    AddType("ListView", sizeof(tgui::ListView),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::ListView, setEnabled) }
    }, {});

    AddType("Panel", sizeof(tgui::Panel),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::Panel, setEnabled) }
    }, {});

    AddType("Picture", sizeof(tgui::Picture),
    {

    }, {});

    AddType("ProgressBar", sizeof(tgui::ProgressBar),
    {
        { "void setValue(float)", WRAP_MFN(tgui::ProgressBar, setValue) },
        { "float getValue()", WRAP_MFN(tgui::ProgressBar, getValue) }
    }, {});

    AddType("RadioButton", sizeof(tgui::RadioButton),
    {
        { "void setChecked(bool)", WRAP_MFN(tgui::RadioButton, setChecked) },
        { "bool isChecked()", WRAP_MFN(tgui::RadioButton, isChecked) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::RadioButton, setEnabled) }
    }, {});

    AddType("RangeSlider", sizeof(tgui::RangeSlider),
    {
        { "void setSelectionStart(float)", WRAP_MFN(tgui::RangeSlider, setSelectionStart) },
        { "void setSelectionEnd(float)", WRAP_MFN(tgui::RangeSlider, setSelectionEnd) },
        { "float getSelectionStart()", WRAP_MFN(tgui::RangeSlider, getSelectionStart) },
        { "float getSelectionEnd()", WRAP_MFN(tgui::RangeSlider, getSelectionEnd) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::RangeSlider, setEnabled) }
    }, {});

    AddType("Slider", sizeof(tgui::Slider),
    {
        { "void setValue(float)", WRAP_MFN(tgui::Slider, setValue) },
        { "float getValue()", WRAP_MFN(tgui::Slider, getValue) },
        { "void setEnabled(bool)", WRAP_MFN(tgui::Slider, setEnabled) }
    }, {});

    AddType("SpinButton", sizeof(tgui::SpinButton),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::SpinButton, setEnabled) }
    }, {});

    AddType("SpinControl", sizeof(tgui::SpinControl),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::SpinControl, setEnabled) }
    }, {});

    AddType("Tabs", sizeof(tgui::Tabs),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::Tabs, setEnabled) }
    }, {});

    AddType("TextArea", sizeof(tgui::TextArea),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::TextArea, setEnabled) }
    }, {});

    AddType("TreeView", sizeof(tgui::TreeView),
    {
        { "void setEnabled(bool)", WRAP_MFN(tgui::TreeView, setEnabled) }
    }, {});

    AddType("Gui", sizeof(tgui::Gui),
    {
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
        { "Slider@ getSlider(string)", WRAP_OBJ_LAST(GetWidget<tgui::Slider>) },
        { "SpinButton@ getSpinButton(string)", WRAP_OBJ_LAST(GetWidget<tgui::SpinButton>) },
        { "SpinControl@ getSpinControl(string)", WRAP_OBJ_LAST(GetWidget<tgui::SpinControl>) },
        { "Tabs@ getTabs(string)", WRAP_OBJ_LAST(GetWidget<tgui::Tabs>) },
        { "TextArea@ getTextArea(string)", WRAP_OBJ_LAST(GetWidget<tgui::TextArea>) },
        { "TreeView@ getTreeView(string)", WRAP_OBJ_LAST(GetWidget<tgui::TreeView>) }
    }, {});
    SetDefaultNamespace("");
}

void ScriptManager::RegisterMaterial()
{
    SetDefaultNamespace("MaterialParameter");
    AddEnum("Type", { "Color", "Normal", "AmbientOcclusion", "Metalness",
                      "Emission", "Roughness", "Opacity" });
    SetDefaultNamespace("");
    AddType("Material", sizeof(Material),
    {
        { "void SetParameter(Vector3, int)", WRAP_OBJ_LAST(SetMaterialParameter) },
        { "Vector3 GetParameter(int)", WRAP_OBJ_LAST(GetMaterialParameter) },
        { "bool Contains(int)", WRAP_MFN(Material, Contains) }
    }, {});
}
