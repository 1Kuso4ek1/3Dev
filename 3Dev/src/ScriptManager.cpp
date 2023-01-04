#include <ScriptManager.hpp>

ScriptManager::ScriptManager() : engine(asCreateScriptEngine())
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

    engine->SetEngineProperty(asEP_COMPILER_WARNINGS, true);

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
    RegisterHingeJoint();
    RegisterPhysicsManager();
    RegisterCamera();
    RegisterModel();
    RegisterLight();
    RegisterSceneManager();
    RegisterSfKeyboard();
    RegisterSfMouse();
    RegisterRandom();
    RegisterClock();

    AddFunction("string to_string(int)", WRAP_FN_PR(std::to_string, (int), std::string));
    AddFunction("string to_string(float)", WRAP_FN_PR(std::to_string, (float), std::string));

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
        { "Vector3 opAdd(const Vector3& in)", WRAP_OBJ_LAST(AddVector3) },
        { "Vector3& opAddAssign(const Vector3& in)", WRAP_MFN_PR(rp3d::Vector3, operator+=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opSubAssign(const Vector3& in)", WRAP_MFN_PR(rp3d::Vector3, operator-=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opMulAssign(float)", WRAP_MFN_PR(rp3d::Vector3, operator*=, (float), rp3d::Vector3&) },
        { "Vector3& opDivAssign(float)", WRAP_MFN_PR(rp3d::Vector3, operator/=, (float), rp3d::Vector3&) },
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
        { "Vector3 Move(float)", WRAP_MFN(Camera, Move) },
        { "void SetPosition(const Vector3& in)", WRAP_MFN(Camera, SetPosition) },
        { "void SetOrientation(const Quaternion& in)", WRAP_MFN(Camera, SetOrientation) },
        { "void SetSpeed(float)", WRAP_MFN(Camera, SetSpeed) },
        { "void SetFOV(float)", WRAP_MFN(Camera, SetFOV) },
        { "void AlwaysUp(bool)", WRAP_MFN(Camera, AlwaysUp) },
        { "Vector3 GetPosition()", WRAP_MFN(Camera, GetPosition) },
        { "Quaternion GetOrientation()", WRAP_MFN(Camera, GetOrientation) },
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
        { "Model@ GetModel(string)", WRAP_MFN(SceneManager, GetModelPtr) },
        { "Light@ GetLight(string)", WRAP_MFN(SceneManager, GetLight) },
        { "Model@ CloneModel(Model@, bool = true, string = \"model\")", WRAP_MFN(SceneManager, CloneModel) },
        { "Camera@ GetCamera()", WRAP_MFN(SceneManager, GetCamera) },
        { "PhysicsManager@ GetPhysicsManager()", WRAP_MFN(SceneManager, GetPhysicsManagerPtr) },
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
        { "HingeJoint@ CreateHingeJoint(HingeJointInfo)", WRAP_MFN(PhysicsManager, CreateHingeJoint) }
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

void ScriptManager::RegisterHingeJoint()
{
    AddValueType("HingeJointInfo", sizeof(rp3d::HingeJointInfo), asGetTypeTraits<rp3d::HingeJointInfo>() | asOBJ_POD, {}, {});

    AddTypeConstructor("HingeJointInfo", "void f(RigidBody@, RigidBody@, const Vector3& in, const Vector3& in)", WRAP_OBJ_LAST(MakeHingeJointInfo));
    AddTypeDestructor("HingeJointInfo", "void f()", WRAP_OBJ_LAST(DestroyType<rp3d::HingeJointInfo>));

    AddType("HingeJoint", sizeof(rp3d::HingeJoint), {}, {});
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
