#include <ScriptManager.hpp>

ScriptManager::ScriptManager() : engine(asCreateScriptEngine())
{
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    context = engine->CreateContext();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterVector3();
    RegisterQuaternion();
    RegisterTransform();
    RegisterPhysicsManager();
    RegisterRigidBody();
    RegisterModel();
    RegisterModelPtr();
    RegisterShape();
    RegisterShapePtr();
    RegisterCamera();
    RegisterSceneManager();
    RegisterSfKeyboard();

    AddFunction("string to_string(int)", asFUNCTIONPR(std::to_string, (int), std::string));
    AddFunction("string to_string(float)", asFUNCTIONPR(std::to_string, (float), std::string));

    SetDefaultNamespace("Log");
    AddEnum("Type", { "Critical", "Error", "Warning", "Info" });
    AddFunction("void Write(string, int)", asFUNCTION(Log::Write));
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

bool ScriptManager::LoadScript(std::string filename)
{
    CScriptBuilder temp;
    temp.StartNewModule(engine, "temp");
    temp.AddSectionFromFile(filename.c_str());
    bool ret = temp.BuildModule() >= 0;
    if(ret) scripts.push_back(filename);
    engine->DiscardModule("temp");
    return ret;
}

void ScriptManager::Build()
{
    engine->DiscardModule("module");
    builder.StartNewModule(engine, "module");

    for(auto& i : scripts)
        builder.AddSectionFromFile(i.c_str());

    int ret = builder.BuildModule();
    buildSucceded = (ret >= 0);
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

void ScriptManager::RegisterVector3()
{
    AddValueType("Vector3", sizeof(rp3d::Vector3), asGetTypeTraits<rp3d::Vector3>(),
    {
        { "float length()", asMETHOD(rp3d::Vector3, length) },
        { "string to_string()", asMETHOD(rp3d::Vector3, to_string) },
        { "Vector3& opAssign(Vector3& in)", asFUNCTION(AssignType<rp3d::Vector3>) },
        { "Vector3& opAddAssign(Vector3& in)", asMETHODPR(rp3d::Vector3, operator+=, (const rp3d::Vector3&), rp3d::Vector3&) },
        { "Vector3& opAdd(Vector3& in)", asFUNCTION(AddVector3) },
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

void ScriptManager::RegisterQuaternion()
{
    AddValueType("Quaternion", sizeof(rp3d::Quaternion), asGetTypeTraits<rp3d::Quaternion>(),
    {
        { "float length()", asMETHOD(rp3d::Quaternion, length) },
        { "Quaternion& opAssign(Quaternion& in)", asMETHODPR(rp3d::Quaternion, operator=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        { "Quaternion& opAddAssign(Quaternion& in)", asMETHODPR(rp3d::Quaternion, operator+=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        { "Quaternion& opSubAssign(Quaternion& in)", asMETHODPR(rp3d::Quaternion, operator-=, (const rp3d::Quaternion&), rp3d::Quaternion&) },
        //{ "Quaternion& opMul(float)", asMETHODPR(rp3d::Quaternion, operator*, (float), rp3d::Quaternion) },
        { "bool opEquals(Quaternion& in)", asMETHODPR(rp3d::Quaternion, operator==, (const rp3d::Quaternion&) const, bool) }
    },
    {
        { "float x", asOFFSET(rp3d::Quaternion, x) },
        { "float y", asOFFSET(rp3d::Quaternion, y) },
        { "float z", asOFFSET(rp3d::Quaternion, z) },
        { "float w", asOFFSET(rp3d::Quaternion, w) }
    });

    AddTypeConstructor("Quaternion", "void f()", asFUNCTION(MakeType<rp3d::Quaternion>));
    AddTypeConstructor("Quaternion", "void f(float x, float y, float z, float w)", asFUNCTION(MakeQuaternion));
    AddTypeConstructor("Quaternion", "void f(const Quaternion& in)", asFUNCTION(CopyType<rp3d::Quaternion>));
    AddTypeDestructor("Quaternion", "void f()", asFUNCTION(DestroyType<rp3d::Quaternion>));
}

void ScriptManager::RegisterModel()
{
    AddType("Model",
    {
        { "void SetPosition(Vector3)", asMETHOD(Model, SetPosition) },
        { "void SetOrientation(Quaternion)", asMETHOD(Model, SetOrientation) },
        { "void SetSize(Vector3)", asMETHOD(Model, SetSize) },
        { "void SetPhysicsManager(PhysicsManager@)", asMETHOD(Model, SetPhysicsManager) },
        { "void CreateRigidBody()", asMETHOD(Model, CreateRigidBody) },
        { "void Move(Vector3)", asMETHOD(Model, Move) },
        { "void Rotate(Quaternion)", asMETHOD(Model, Rotate) },
        { "void Expand(Vector3)", asMETHOD(Model, Expand) },
        { "Vector3 GetPosition()", asMETHOD(Model, GetPosition) },
        { "Quaternion GetOrientation()", asMETHOD(Model, GetOrientation) },
        { "Vector3 GetSize()", asMETHOD(Model, GetSize) },
        { "RigidBody@ GetRigidBody()", asMETHOD(Model, GetRigidBody) },
        { "int GetMeshesCount()", asMETHOD(Model, GetMeshesCount) }
    }, {});
}

void ScriptManager::RegisterModelPtr()
{
    AddValueType("ModelPtr", sizeof(std::shared_ptr<Model>), asGetTypeTraits<std::shared_ptr<Model>>(),
    {
        { "Model@ get()", asMETHOD(std::shared_ptr<Model>, get) },
        { "ModelPtr& opAssign(ModelPtr& in)", asMETHODPR(std::shared_ptr<Model>, operator=, (std::shared_ptr<Model>&&), std::shared_ptr<Model>&) },
        { "Model@ opCall()", asMETHOD(std::shared_ptr<Model>, get) }
    }, {});

    AddTypeConstructor("ModelPtr", "void f()", asFUNCTION(MakeType<std::shared_ptr<Model>>));
    AddTypeDestructor("ModelPtr", "void f()", asFUNCTION(DestroyType<std::shared_ptr<Model>>));
}

void ScriptManager::RegisterShape()
{
    AddType("Shape",
    {
        { "void SetPosition(Vector3)", asMETHOD(Shape, SetPosition) },
        { "void SetOrientation(Quaternion)", asMETHOD(Shape, SetOrientation) },
        { "void SetSize(Vector3)", asMETHOD(Shape, SetSize) },
        { "void SetPhysicsManager(PhysicsManager@)", asMETHOD(Shape, SetPhysicsManager) },
        { "void CreateRigidBody()", asMETHOD(Shape, CreateRigidBody) },
        { "void Move(Vector3)", asMETHOD(Shape, Move) },
        { "void Rotate(Quaternion)", asMETHOD(Shape, Rotate) },
        { "void Expand(Vector3)", asMETHOD(Shape, Expand) },
        { "Vector3 GetPosition()", asMETHOD(Shape, GetPosition) },
        { "Quaternion GetOrientation()", asMETHOD(Shape, GetOrientation) },
        { "Vector3 GetSize()", asMETHOD(Shape, GetSize) },
        { "RigidBody@ GetRigidBody()", asMETHOD(Shape, GetRigidBody) }
    }, {});
}

void ScriptManager::RegisterShapePtr()
{
    AddValueType("ShapePtr", sizeof(std::shared_ptr<Shape>), asGetTypeTraits<std::shared_ptr<Shape>>(),
    {
        { "Shape@ get()", asMETHOD(std::shared_ptr<Shape>, get) },
        { "ShapePtr& opAssign(ShapePtr& in)", asMETHODPR(std::shared_ptr<Shape>, operator=, (std::shared_ptr<Shape>&&), std::shared_ptr<Shape>&) },
        { "Shape@ opCall()", asMETHOD(std::shared_ptr<Shape>, get) }
    }, {});

    AddTypeConstructor("ShapePtr", "void f()", asFUNCTION(MakeType<std::shared_ptr<Shape>>));
    AddTypeDestructor("ShapePtr", "void f()", asFUNCTION(DestroyType<std::shared_ptr<Shape>>));
}

void ScriptManager::RegisterRigidBody()
{
    AddValueType("AABB", sizeof(rp3d::AABB), asGetTypeTraits<rp3d::AABB>(),
    {
        { "AABB& opAssign(AABB& in)", asFUNCTION(AssignAABB) }
    }, {});

    AddTypeConstructor("AABB", "void f()", asFUNCTION(MakeType<rp3d::AABB>));
    AddTypeConstructor("AABB", "void f(Vector3& in, Vector3& in)", asFUNCTION(MakeAABB));
    AddTypeConstructor("AABB", "void f(const AABB& in)", asFUNCTION(CopyType<rp3d::AABB>));
    AddTypeDestructor("AABB", "void f()", asFUNCTION(DestroyType<rp3d::AABB>));

    AddEnum("BodyType", { "STATIC", "KINEMATIC", "DYNAMIC" });
    AddType("RigidBody",
    {
        { "float getMass()", asMETHOD(rp3d::RigidBody, getMass) },
        { "void setMass(float)", asMETHOD(rp3d::RigidBody, setMass) },
        { "Vector3 getLinearVelocity()", asMETHOD(rp3d::RigidBody, getLinearVelocity) },
        { "void setLinearVelocity(const Vector3& in)", asMETHOD(rp3d::RigidBody, setLinearVelocity) },
        { "Vector3 getAngularVelocity()", asMETHOD(rp3d::RigidBody, getAngularVelocity) },
        { "void setAngularVelocity(const Vector3& in)", asMETHOD(rp3d::RigidBody, setAngularVelocity) },
        { "bool isActive()", asMETHOD(rp3d::RigidBody, isActive) },
        { "void setIsActive(bool)", asMETHOD(rp3d::RigidBody, setIsActive) },
        { "void setType(BodyType)", asMETHOD(rp3d::RigidBody, setType) },
        { "AABB getAABB()", asMETHOD(rp3d::RigidBody, getAABB) },
        { "bool testAABBOverlap(const AABB& in)", asMETHOD(rp3d::RigidBody, testAABBOverlap) }
    }, {});
}

void ScriptManager::RegisterCamera()
{
    AddType("Camera",
    {
        { "Vector3& Move(float)", asMETHOD(Camera, Move) },
        { "void SetPosition(Vector3)", asMETHOD(Camera, SetPosition) },
        { "void SetOrientation(Quaternion)", asMETHOD(Camera, SetOrientation) },
        { "void SetSpeed(float)", asMETHOD(Camera, SetSpeed) },
        { "void SetFOV(float)", asMETHOD(Camera, SetFOV) },
        { "void AlwaysUp(bool)", asMETHOD(Camera, AlwaysUp) },
        { "Vector3 GetPosition()", asMETHOD(Camera, GetPosition) },
        { "Quaternion GetOrientation()", asMETHOD(Camera, GetOrientation) },
        { "void Look()", asMETHODPR(Camera, Look, (), void) },
        { "void Look(Vector3)", asMETHODPR(Camera, Look, (rp3d::Vector3), void) }
    }, {});
}

void ScriptManager::RegisterSceneManager()
{
    AddType("SceneManager",
    {
        { "ModelPtr GetModel(string)", asMETHOD(SceneManager, GetModel) },
        { "ShapePtr GetShape(string)", asMETHOD(SceneManager, GetShape) },
        { "Camera@ GetCamera()", asMETHOD(SceneManager, GetCamera) }
    }, {});
}

void ScriptManager::RegisterSfKeyboard()
{
    SetDefaultNamespace("Keyboard");
    AddFunction("bool isKeyPressed(int)", asFUNCTION(sf::Keyboard::isKeyPressed));
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

void ScriptManager::RegisterPhysicsManager()
{
    AddType("PhysicsManager", {}, {});
}

void ScriptManager::RegisterTransform()
{
    rp3d::Transform::identity();
    AddValueType("Transform", sizeof(rp3d::Transform), asGetTypeTraits<rp3d::Transform>(), {}, {});
    AddTypeConstructor("Transform", "void f()", asFUNCTION(MakeType<rp3d::Transform>));
    AddTypeConstructor("Transform", "void f(const Transform& in)", asFUNCTION(CopyType<rp3d::Transform>));
    AddTypeDestructor("Transform", "void f()", asFUNCTION(DestroyType<rp3d::Transform>));
}
