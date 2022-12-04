#include <Engine.hpp>
#include <filesystem>
#define TGUI_USE_STD_FILESYSTEM
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-OpenGL3.hpp>
#include <unistd.h>

#ifdef _WIN32
    std::string homeFolder = std::string(getenv("HOMEPATH")) + "/.3Dev-Editor/";
#else
    std::string homeFolder = std::string(getenv("HOME")) + "/.3Dev-Editor/";
#endif

std::string lastPath = std::filesystem::current_path().string();

void SaveProperties(Json::Value data)
{
    std::ofstream file(homeFolder + "properties.json");
    file << data.toStyledString();
    file.close();
}

void DefaultProperties()
{
    Json::Value p;
    p["logFilename"] = homeFolder + "log/EditorLog.txt";
    p["defaultResorces"] = homeFolder + "default/";

    SaveProperties(p);
}

Json::Value ParseProperties()
{
	Json::Value ret;
	Json::CharReaderBuilder rbuilder;

    std::ifstream file(homeFolder + "properties.json");

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &ret, &errors))
        Log::Write("Json parsing failed: " + errors, Log::Type::Critical);
    return ret;
}

std::shared_ptr<tgui::FileDialog> CreateFileDialog(std::string title, int fileType, std::string buttonTitle = "Open", bool fileMustExist = true)
{
	auto dialog = tgui::FileDialog::create(title, buttonTitle);
	if(fileType == 0)
		dialog->setFileTypeFilters(
		{
			{ "3D model files", { "*.obj", "*.fbx", "*.dae", "*.gltf", "*.glb", "*.blend", "*.x3d", "*.ply", "*.stl" } }
		});
    else if(fileType == 1)
		dialog->setFileTypeFilters(
		{
			{ "Angelscript files", { "*.as" } }
		});
    else if(fileType == 2)
        dialog->setFileTypeFilters(
		{
			{ "JSON", { "*.json" } }
		});
    else if(fileType == 3)
        dialog->setFileTypeFilters(
		{
			{ "Images", { "*.jpg", "*.jpeg", "*.png", "*.bmp",  } }
		});
    dialog->setPath(lastPath);
	dialog->setFileMustExist(fileMustExist);
	dialog->setVisible(false);
	dialog->showWithEffect(tgui::ShowEffectType::Fade, tgui::Duration(sf::seconds(0.3)));

	/*auto panel = tgui::Panel::create({ "100%", "100%" });
    panel->getRenderer()->setBackgroundColor({ 0, 0, 0, 150 });
    gui->add(panel);*/

    dialog->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
    dialog->setFocused(true);

    return dialog;
}

std::shared_ptr<tgui::ColorPicker> CreateColorPicker(std::string title, tgui::Color color)
{
    auto picker = tgui::ColorPicker::create(title, color);
	picker->setVisible(false);
	picker->showWithEffect(tgui::ShowEffectType::Fade, tgui::Duration(sf::seconds(0.3)));

	/*auto panel = tgui::Panel::create({ "100%", "100%" });
    panel->getRenderer()->setBackgroundColor({ 0, 0, 0, 150 });
    gui->add(panel);*/

    picker->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
    picker->setFocused(true);

    return picker;
}

int main()
{
    Json::Value properties;

    if(std::filesystem::exists(homeFolder + "properties.json"))
        properties = ParseProperties();
    else
    {
        DefaultProperties();
        Log::Write("properties.json created.", Log::Type::Info);
        properties = ParseProperties();
    }

    Engine engine(false);
    Log::Init(properties["logFilename"].asString(), false, true);
    engine.CreateWindow(1280, 720, "3Dev Editor");
    engine.Init();

    engine.GetWindow().setFramerateLimit(120);

    std::filesystem::current_path(homeFolder + "gui");

    tgui::Gui menu{ engine.GetWindow() };
    tgui::Gui editor{ engine.GetWindow() };

    menu.loadWidgetsFromFile(homeFolder + "gui/menu.txt");
    editor.loadWidgetsFromFile(homeFolder + "gui/editor.txt");

    auto openButton = menu.get<tgui::Button>("open");
    auto pathEdit = menu.get<tgui::EditBox>("path");
    auto openFileDialogButton = menu.get<tgui::Button>("openFileDialog");
    auto projectsComboBox = menu.get<tgui::ComboBox>("projects");

	auto viewportWindow = editor.get<tgui::ChildWindow>("viewport");
    auto viewport = tgui::CanvasOpenGL3::create({ viewportWindow->getSize().x, viewportWindow->getSize().y - 28 });

    viewportWindow->add(viewport);

	auto messagesBox = editor.get<tgui::ChatBox>("messages");

    auto modelButton = editor.get<tgui::Button>("createModel");
    auto shapeButton = editor.get<tgui::Button>("createShape");
    auto lightButton = editor.get<tgui::Button>("createLight");
    auto materialButton = editor.get<tgui::Button>("createMaterial");
    auto boxColliderButton = editor.get<tgui::Button>("boxCollider");
    auto sphereColliderButton = editor.get<tgui::Button>("sphereCollider");
    auto capsuleColliderButton = editor.get<tgui::Button>("capsuleCollider");
    auto concaveColliderButton = editor.get<tgui::Button>("concaveCollider");
	auto scriptButton = editor.get<tgui::Button>("createScript");

    auto sceneTree = editor.get<tgui::TreeView>("scene");

    sceneTree->addItem({ "Scene", "Models" });
    sceneTree->addItem({ "Scene", "Shapes" });
    sceneTree->addItem({ "Scene", "Materials" });
    sceneTree->addItem({ "Scene", "Lights" });
	sceneTree->addItem({ "Scene", "Scripts" });

    auto objectEditorGroup = editor.get<tgui::Group>("objectEditor");
    auto materialEditorGroup = editor.get<tgui::Group>("materialEditor");
	auto scriptsGroup = editor.get<tgui::Group>("scriptsGroup");
	auto sceneGroup = editor.get<tgui::Group>("sceneGroup");

	auto nameEdit = editor.get<tgui::EditBox>("name");

    auto posEditX = editor.get<tgui::EditBox>("posX");
    auto posEditY = editor.get<tgui::EditBox>("posY");
    auto posEditZ = editor.get<tgui::EditBox>("posZ");

    auto rotEditX = editor.get<tgui::EditBox>("rotX");
    auto rotEditY = editor.get<tgui::EditBox>("rotY");
    auto rotEditZ = editor.get<tgui::EditBox>("rotZ");

    auto sizeEditX = editor.get<tgui::EditBox>("sizeX");
    auto sizeEditY = editor.get<tgui::EditBox>("sizeY");
    auto sizeEditZ = editor.get<tgui::EditBox>("sizeZ");

    auto materialsList = editor.get<tgui::ListBox>("materials");
    auto materialBox = editor.get<tgui::ComboBox>("material");

    auto openFileButton = editor.get<tgui::Button>("openFile");
    auto deleteButton = editor.get<tgui::Button>("delete");

    auto materialNameEdit = editor.get<tgui::EditBox>("matName");

    auto colorPickerButton = editor.get<tgui::Button>("colorPicker");
    auto emissionPickerButton = editor.get<tgui::Button>("emissionPicker");

    std::shared_ptr<tgui::ColorPicker> colorPicker = nullptr, emissionPicker = nullptr;

    auto metalEdit = editor.get<tgui::EditBox>("mtlbox");
    auto roughEdit = editor.get<tgui::EditBox>("rghbox");
    auto opacityEdit = editor.get<tgui::EditBox>("opcbox");

    auto metalSlider = editor.get<tgui::Slider>("metal");
    auto roughSlider = editor.get<tgui::Slider>("rough");
    auto opacitySlider = editor.get<tgui::Slider>("opacity");

    auto colorTextureButton = editor.get<tgui::Button>("loadColor");
    auto metalTextureButton = editor.get<tgui::Button>("loadMetalness");
    auto roughTextureButton = editor.get<tgui::Button>("loadRoughness");
    auto normalTextureButton = editor.get<tgui::Button>("loadNormal");
    auto aoTextureButton = editor.get<tgui::Button>("loadAo");
    auto emissionTextureButton = editor.get<tgui::Button>("loadEmission");
    auto opacityTextureButton = editor.get<tgui::Button>("loadOpacity");

	auto buildButton = editor.get<tgui::Button>("build");
    auto startStopButton = editor.get<tgui::Button>("startStop");
    auto removeScriptButton = editor.get<tgui::Button>("removeScript");

    auto filenameEdit = editor.get<tgui::EditBox>("filename");
    auto fileDialogButton = editor.get<tgui::Button>("openFileDialog");
    auto saveButton = editor.get<tgui::Button>("save");

    auto modeLabel = editor.get<tgui::Label>("mode");
    modeLabel->moveToFront();

    std::shared_ptr<tgui::FileDialog> openFileDialog = nullptr;

    bool objectMovement = false;
    sf::Clock changeMode;

    tgui::Color matColor = tgui::Color::White;

    for(int i = 0; i < (properties["recentProjects"].size() >= 9 ? 9 : properties["recentProjects"].size()); i++)
        if(!properties["recentProjects"][i].empty())
            projectsComboBox->addItem(properties["recentProjects"][i].asString());

    projectsComboBox->addItem("New project");
    projectsComboBox->setSelectedItemByIndex(0);
    pathEdit->setText(properties["recentProjectsPaths"]
                                [projectsComboBox->getSelectedItemIndex()].asString());

    std::string projectFilename = "";

    projectsComboBox->onItemSelect([&]()
    {
        if(projectsComboBox->getSelectedItem() != "New project")
            pathEdit->setText(properties["recentProjectsPaths"]
                                        [projectsComboBox->getSelectedItemIndex()].asString());
        else pathEdit->setText("");
    });

    openFileDialogButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 2);
    	menu.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
                pathEdit->setText(openFileDialog->getSelectedPaths()[0].asString());
  	    	openFileDialog = nullptr;
  	    });
    });

    openButton->onPress([&]()
    {
    	if(projectsComboBox->getSelectedItem() == "New project" && !pathEdit->getText().empty())
    		engine.Close();
        else
        {
            if(projectsComboBox->getSelectedItem() != "New project")
                projectFilename = properties["recentProjectsPaths"]
                                            [projectsComboBox->getSelectedItemIndex()].asString();
            if(projectFilename != pathEdit->getText().toStdString())
                projectFilename = pathEdit->getText().toStdString();
            engine.Close();
        }
    });

    engine.EventLoop([&](sf::Event& event)
    {
    	menu.handleEvent(event);
    	if(event.type == sf::Event::Closed)
    	{
    		engine.Close();
    		exit(0);
    	}
    });

    engine.Loop([&]()
    {
        menu.draw();
    });

    engine.Launch();

    Renderer::GetInstance()->Init({ 840, 492 }, properties["defaultResorces"].asString() + "hdri.hdr");

    Camera cam(&engine.GetWindow());
    cam.SetViewportSize({ 840, 492 });

    Light shadowSource({ 0, 0, 0 }, { 50.1, 100.0, 50.1 });
    shadowSource.SetDirection({ 0.0, -1.0, 0.0 });

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto defaultMaterial = std::make_shared<Material>();

    auto skybox = std::make_shared<Shape>(rp3d::Vector3{ 1, 1, 1 }, &skyboxMaterial);

	rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);

    SceneManager scene;

	scene.AddPhysicsManager(man);
	scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.UpdatePhysics(false);

	if(!projectFilename.empty())
	{
        scene.Load(projectFilename);
        auto names = scene.GetNames();
        for(auto& i : names[0]) sceneTree->addItem({ "Scene", "Models", i });
        for(auto& i : names[1]) sceneTree->addItem({ "Scene", "Shapes", i });
        for(auto& i : names[2])
        {
            materialBox->addItem(i);
            sceneTree->addItem({ "Scene", "Materials", i });
        }
    }
    else
    {
        scene.AddMaterial(defaultMaterial, "default");
        materialBox->addItem("default");
        sceneTree->addItem({ "Scene", "Materials", "default" });
    }

    filenameEdit->setText(projectFilename.empty() ? "scene.json" : projectFilename);

	ScriptManager scman;
	bool manageCameraMovement = true, manageCameraLook = true, manageCameraMouse = true;
	bool scriptLaunched = false;
    scman.SetDefaultNamespace("Game");
    scman.AddProperty("SceneManager scene", &scene);
    scman.AddProperty("Camera camera", &cam);
    scman.AddProperty("bool manageCameraMovement", &manageCameraMovement);
    scman.AddProperty("bool manageCameraLook", &manageCameraLook);
    scman.AddProperty("bool manageCameraMouse", &manageCameraMouse);
    scman.SetDefaultNamespace("");
	std::string startDecl = "void Start()", loopDecl = "void Loop()";

    if(!projectFilename.empty())
    {
        auto scPath = projectFilename;
        scPath.insert(scPath.find_last_of('.'), "_scripts");
        scman.Load(scPath);
        auto scripts = scman.GetScripts();
        for(auto& i : scripts)
            sceneTree->addItem({ "Scene", "Scripts", i });
    }

    ShadowManager shadows(&scene, { &shadowSource }, glm::ivec2(2048, 2048));

    modelButton->onPress([&]()
    {
    	auto model = std::make_shared<Model>();
    	model->SetMaterial({ scene.GetMaterial(scene.GetNames()[2][0]).get() });
		model->SetPhysicsManager(man.get());
		model->CreateRigidBody();
		//model->GetRigidBody()->setIsActive(false);
    	scene.AddObject(model);
    	std::string name = scene.GetNames()[0][0];
    	sceneTree->addItem({ "Scene", "Models", name });
    	sceneTree->selectItem({ "Scene", "Models", name });
    	/*materialsNames[model] = { "" };
    	materialsNames[model].resize(model->GetMeshesCount(), "empty");
    	materialsNames[model][0] = "default";*/
    });

    shapeButton->onPress([&]()
    {
		auto shape = std::make_shared<Shape>(rp3d::Vector3{ 1, 1, 1 }, scene.GetMaterial(scene.GetNames()[2][0]).get(), man.get());
		//shape->GetRigidBody()->setIsActive(false);
    	scene.AddObject(shape);
    	std::string name = scene.GetNames()[1][0];
    	sceneTree->addItem({ "Scene", "Shapes", name });
    	sceneTree->selectItem({ "Scene", "Shapes", name });
    	//materialsNames[scene.GetShape(name)] = { "default" };
    });

    materialButton->onPress([&]()
    {
    	scene.AddMaterial(std::make_shared<Material>());
    	std::string name = scene.GetNames()[2][0];
    	sceneTree->addItem({ "Scene", "Materials", name });
    	sceneTree->selectItem({ "Scene", "Materials", name });
    	materialBox->addItem(name);
    });

    boxColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Models")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateBoxShape(i);
			}
    });

    sphereColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Models")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateSphereShape(i);
			}
    });

    capsuleColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Models")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateCapsuleShape(i);
			}
    });

    concaveColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Models")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateConcaveShape(i);
			}
    });

    colorPickerButton->onPress([&]()
    {
        colorPicker = CreateColorPicker("Color", matColor);
        editor.add(colorPicker);
        colorPicker->onClose([&]()
  	    {
  	    	colorPicker->destroy();
  	    	colorPicker = nullptr;
  	    });
    });

    emissionPickerButton->onPress([&]()
    {
        emissionPicker = CreateColorPicker("Color", tgui::Color::Black);
        editor.add(emissionPicker);
        emissionPicker->onClose([&]()
  	    {
  	    	emissionPicker->destroy();
  	    	emissionPicker = nullptr;
  	    });
    });

    // TODO: Add a universal LoadTexture function or smth
    colorTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Color);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Color);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    metalTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Metalness);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Metalness);
                metalSlider->setValue(0.0);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    roughTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Roughness);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Roughness);
                roughSlider->setValue(0.0);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    normalTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Normal);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Normal);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    aoTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::AmbientOcclusion);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::AmbientOcclusion);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    emissionTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Emission);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Emission);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    opacityTextureButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
                auto p = mat->GetParameter(Material::Type::Opacity);
                if(std::holds_alternative<GLuint>(p))
                {
                    auto name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path), Material::Type::Opacity);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

	scriptButton->onPress([&]()
	{
		openFileDialog = CreateFileDialog("Open file", 1);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                scman.LoadScript(openFileDialog->getSelectedPaths()[0].asString().toStdString());
                sceneTree->addItem({ "Scene", "Scripts", openFileDialog->getSelectedPaths()[0].getFilename() });
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
	});

    openFileButton->onPress([&]()
    {
    	openFileDialog = CreateFileDialog("Open file", 0);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
			{
                auto model = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
                model->Load(openFileDialog->getSelectedPaths()[0].asString().toStdString(),
                            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);

                materialsList->removeAllItems();
                auto mtl = model->GetMaterial();
                for(int i = 0; i < mtl.size(); i++)
                    materialsList->addItem(scene.GetName(mtl[i]), tgui::String(i));
                if(model->GetAnimationsCount())
                    model->PlayAnimation();
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    deleteButton->onPress([&]()
    {
    	if(sceneTree->getSelectedItem()[1] == "Models")
    	{
    		scene.RemoveObject(scene.GetModel(sceneTree->getSelectedItem()[2].toStdString()));
    	}
    	else if(sceneTree->getSelectedItem()[1] == "Shapes")
    	{
    	    scene.RemoveObject(scene.GetShape(sceneTree->getSelectedItem()[2].toStdString()));
    	}
    	sceneTree->removeItem(sceneTree->getSelectedItem(), false);
    });

	buildButton->onPress([&]()
    {
    	scman.Build();
    });

	startStopButton->onPress([&]()
    {
    	if(scman.IsBuildSucceded())
		{
			scriptLaunched = !scriptLaunched;
			if(scriptLaunched)
            {
                scene.SaveState();
                scene.UpdatePhysics(true);
				scman.ExecuteFunction(startDecl);
            }
			else
			{
			    manageCameraMovement = manageCameraLook = manageCameraMouse = true;
			    scene.UpdatePhysics(false);
			    scene.LoadState();
			}
		}
    });

    removeScriptButton->onPress([&]()
	{
        scman.RemoveScript(sceneTree->getSelectedItem()[2].toStdString());
        sceneTree->removeItem(sceneTree->getSelectedItem(), false);
	});

    fileDialogButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Save file", 2, "Save", false);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
				if(openFileDialog->getSelectedPaths()[0].asString().find(".json") != std::string::npos)
                    filenameEdit->setText(openFileDialog->getSelectedPaths()[0].asString().toStdString());
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    saveButton->onPress([&]()
    {
        if(!filenameEdit->getText().empty())
        {
            auto path = filenameEdit->getText().toStdString(), scPath = path;
            scene.Save(path, true);
            scPath.insert(scPath.find_last_of('.'), "_scripts");
            scman.Save(scPath, true);

            auto filename = std::filesystem::path(path).filename().string();
            auto it = std::find(properties["recentProjects"].begin(), properties["recentProjects"].end(), filename);
            if(it == properties["recentProjects"].end())
            {
                properties["recentProjects"].insert(0, filename);
                properties["recentProjectsPaths"].insert(0, filenameEdit->getText().toStdString());
            }
            else
            {
                for(int i = 0; i < properties["recentProjects"].size(); i++)
                {
                    if(properties["recentProjects"][i] == filename)
                    {
                        properties["recentProjects"].removeIndex(i, 0);
                        properties["recentProjectsPaths"].removeIndex(i, 0);
                    }
                }
                properties["recentProjects"].insert(0, filename);
                properties["recentProjectsPaths"].insert(0, filenameEdit->getText().toStdString());
            }
            SaveProperties(properties);
            Log::Write(filenameEdit->getText().toStdString() + " saved", Log::Type::Info);
        }
    });

    engine.EventLoop([&](sf::Event& event)
    {
    	editor.handleEvent(event);
    	if(event.type == sf::Event::Closed)
    		engine.Close();
    });

    engine.Loop([&]()
    {
		if(!Log::GetMessages().empty())
		{
			for(auto& i : Log::GetMessages())
				switch(i.second)
				{
				case Log::Type::Error:
					messagesBox->addLine(i.first, { 255, 40, 0, 255 }); break;
				case Log::Type::Warning:
					messagesBox->addLine(i.first, { 255, 165, 0, 255 }); break;
				case Log::Type::Info:
					messagesBox->addLine(i.first, { 0, 185, 255, 255 }); break;
				}
			Log::ClearMessagesList();
		}

    	sceneTree->setEnabled(!openFileDialog);
    	openFileButton->setEnabled(!openFileDialog);

		if(sceneTree->getSelectedItem().size() > 2)
		{
			////////////// OBJECTS //////////////
			int variantIndex = -1;
			std::variant<std::shared_ptr<Model>, std::shared_ptr<Shape>> object;

			if(sceneTree->getSelectedItem()[1] == "Models")
			{
				variantIndex = 0;
				openFileButton->setEnabled(true);
				openFileButton->setVisible(true);
				object = scene.GetModel(sceneTree->getSelectedItem()[2].toStdString());
			}
			else if(sceneTree->getSelectedItem()[1] == "Shapes")
			{
				variantIndex = 1;
				openFileButton->setEnabled(false);
				openFileButton->setVisible(false);
				object = scene.GetShape(sceneTree->getSelectedItem()[2].toStdString());
			}
			if(variantIndex != -1)
			{
				objectEditorGroup->setEnabled(true);
				objectEditorGroup->setVisible(true);
				materialEditorGroup->setEnabled(false);
				materialEditorGroup->setVisible(false);
				scriptsGroup->setEnabled(false);
				scriptsGroup->setVisible(false);
				sceneGroup->setEnabled(false);
				sceneGroup->setVisible(false);

				if(!nameEdit->isFocused() && nameEdit->getText() != sceneTree->getSelectedItem()[2])
				{
					rp3d::Vector3 position;
					rp3d::Quaternion orientation;
					rp3d::Vector3 size;

					materialsList->removeAllItems();
					if(variantIndex == 0)
					{
						position = std::get<0>(object)->GetPosition();
						orientation = std::get<0>(object)->GetOrientation();
						size = std::get<0>(object)->GetSize();
						auto mtl = std::get<0>(object)->GetMaterial();
						for(int i = 0; i < mtl.size(); i++)
							materialsList->addItem(scene.GetName(mtl[i]), tgui::String(i));
					}
					else
					{
						position = std::get<1>(object)->GetPosition();
						orientation = std::get<1>(object)->GetOrientation();
						size = std::get<1>(object)->GetSize();

						materialsList->addItem(scene.GetName(std::get<1>(object)->GetMaterial()), "0");
					}

					nameEdit->setText(sceneTree->getSelectedItem()[2]);
					posEditX->setText(tgui::String(position.x));
				    posEditY->setText(tgui::String(position.y));
				    posEditZ->setText(tgui::String(position.z));

					glm::vec3 euler = glm::eulerAngles(toglm(orientation));

				    rotEditX->setText(tgui::String(glm::degrees(euler.x)));
	  			    rotEditY->setText(tgui::String(glm::degrees(euler.y)));
	  			    rotEditZ->setText(tgui::String(glm::degrees(euler.z)));

					sizeEditX->setText(tgui::String(size.x));
				    sizeEditY->setText(tgui::String(size.y));
				    sizeEditZ->setText(tgui::String(size.z));
			    }
			    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && nameEdit->isFocused() && nameEdit->getText() != sceneTree->getSelectedItem()[2])
			    {
			    	if(variantIndex == 0)
			    	{
			    		scene.SetModelName(sceneTree->getSelectedItem()[2].toStdString(), nameEdit->getText().toStdString());
			    		sceneTree->removeItem(sceneTree->getSelectedItem(), false);
			    		sceneTree->addItem({ "Scene", "Models", nameEdit->getText() });
			    		sceneTree->selectItem({ "Scene", "Models", nameEdit->getText() });
			    	}
			    	else if(variantIndex == 1)
			    	{
				    	scene.SetShapeName(sceneTree->getSelectedItem()[2].toStdString(), nameEdit->getText().toStdString());
				    	sceneTree->removeItem(sceneTree->getSelectedItem(), false);
				    	sceneTree->addItem({ "Scene", "Shapes", nameEdit->getText() });
				    	sceneTree->selectItem({ "Scene", "Shapes", nameEdit->getText() });
				    }
				    nameEdit->setFocused(false);
			    }

			    rp3d::Vector3 pos;
			    pos.x = posEditX->getText().toFloat();
			    pos.y = posEditY->getText().toFloat();
			    pos.z = posEditZ->getText().toFloat();

			    rp3d::Vector3 euler;
			    euler.x = glm::radians(rotEditX->getText().toFloat());
			    euler.y = glm::radians(rotEditY->getText().toFloat());
			    euler.z = glm::radians(rotEditZ->getText().toFloat());

			    rp3d::Vector3 size;
			    size.x = sizeEditX->getText().toFloat();
			    size.y = sizeEditY->getText().toFloat();
			    size.z = sizeEditZ->getText().toFloat();

			    if(variantIndex == 0)
				{
					std::get<0>(object)->SetPosition(pos);
					std::get<0>(object)->SetOrientation(rp3d::Quaternion::fromEulerAngles(euler));
					std::get<0>(object)->SetSize(size);

					if(!materialBox->getSelectedItem().empty())
					{
						if(materialBox->getSelectedItem() != materialsList->getSelectedItem())
						{
							std::get<0>(object)->GetMaterial()[materialsList->getSelectedItemId().toInt()] = scene.GetMaterial(materialBox->getSelectedItem().toStdString()).get();
							materialsList->changeItemById(materialsList->getSelectedItemId(), materialBox->getSelectedItem());
						}
					}
					std::get<0>(object)->CheckOpacity();
				}
				else
				{
					std::get<1>(object)->SetPosition(pos);
					std::get<1>(object)->SetOrientation(rp3d::Quaternion::fromEulerAngles(euler));
					std::get<1>(object)->SetSize(size);

					if(!materialBox->getSelectedItem().empty())
					{
						if(materialBox->getSelectedItem() != materialsList->getSelectedItem())
						{
							std::get<1>(object)->SetMaterial(scene.GetMaterial(materialBox->getSelectedItem().toStdString()).get());
							materialsList->changeItemById(materialsList->getSelectedItemId(), materialBox->getSelectedItem());
						}
					}
					std::get<1>(object)->CheckOpacity();
				}

				if(!materialsList->getSelectedItem().empty())
					materialBox->setSelectedItem(materialsList->getSelectedItem());
				else materialBox->deselectItem();
		    }
		    /////////////////////////////////////

		    ///////////// MATERIALS /////////////
		    if(sceneTree->getSelectedItem()[1] == "Materials")
		    {
				materialEditorGroup->setEnabled(true);
				materialEditorGroup->setVisible(true);
				objectEditorGroup->setEnabled(false);
				objectEditorGroup->setVisible(false);
				scriptsGroup->setEnabled(false);
				scriptsGroup->setVisible(false);
				sceneGroup->setEnabled(false);
				sceneGroup->setVisible(false);

				auto material = scene.GetMaterial(sceneTree->getSelectedItem()[2].toStdString());
				if(materialNameEdit->getText() != sceneTree->getSelectedItem()[2] && !materialNameEdit->isFocused())
				{
					materialNameEdit->setText(sceneTree->getSelectedItem()[2]);

					std::optional<glm::vec3> color;
					float metal = -1.0;
					float rough = -1.0;
					float opacity = -1.0;

					auto params = material->GetParameters();
                    for(auto& i : params)
                    {
                        switch(i.second)
                        {
                        case Material::Type::Color:
                            if(std::holds_alternative<glm::vec3>(i.first))
                                color = std::get<0>(i.first);
                            break;
                        case Material::Type::Metalness:
                            if(std::holds_alternative<glm::vec3>(i.first))
                                metal = std::get<0>(i.first).x;
                            break;
                        case Material::Type::Roughness:
                            if(std::holds_alternative<glm::vec3>(i.first))
                                rough = std::get<0>(i.first).x;
                            break;
                        case Material::Type::Opacity:
                            if(std::holds_alternative<glm::vec3>(i.first))
                                opacity = std::get<0>(i.first).x;
                            break;
                        default: break;
                        }
                    }

                    if(color.has_value())
                        matColor = tgui::Color(color.value().x * 255, color.value().y * 255, color.value().z * 255);

				    metalEdit->setText(tgui::String(metal));   metalSlider->setValue(metal < 0.0 ? 0.0 : metal);
				    roughEdit->setText(tgui::String(rough));   roughSlider->setValue(rough < 0.0 ? 0.0 : rough);
				    opacityEdit->setText(tgui::String(opacity));   opacitySlider->setValue(opacity < 0.0 ? 0.0 : opacity);
			    }

			    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && materialNameEdit->isFocused() && materialNameEdit->getText() != sceneTree->getSelectedItem()[2])
			    {
		    		scene.SetMaterialName(sceneTree->getSelectedItem()[2].toStdString(), materialNameEdit->getText().toStdString());
		    		sceneTree->removeItem(sceneTree->getSelectedItem(), false);
		    		sceneTree->addItem({ "Scene", "Materials", materialNameEdit->getText() });
		    		sceneTree->selectItem({ "Scene", "Materials", materialNameEdit->getText() });
				    materialNameEdit->setFocused(false);
				    materialBox->removeAllItems();
				    auto names = scene.GetNames();
                    for(auto& i : names[2]) materialBox->addItem(i);
			    }

			    std::optional<glm::vec3> color, ctmp, emission, etmp;
			    float metal, rough, opacity;

			    if(std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Color)))
                    color = std::get<0>(material->GetParameter(Material::Type::Color));
                if(std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Emission)))
                    emission = std::get<0>(material->GetParameter(Material::Type::Emission));

			    if(colorPicker)
			    {
                    tgui::Color c = colorPicker->getColor();

                    ctmp = glm::vec3(float(c.getRed()) / 255, float(c.getGreen()) / 255, float(c.getBlue()) / 255);
			    }

			    if(emissionPicker)
			    {
                    tgui::Color c = emissionPicker->getColor();

                    etmp = glm::vec3(float(c.getRed()) / 255, float(c.getGreen()) / 255, float(c.getBlue()) / 255);
			    }

			    metal = metalSlider->getValue();
			    rough = roughSlider->getValue();
			    opacity = opacitySlider->getValue();

			    metalEdit->setText(tgui::String(metal));
			    roughEdit->setText(tgui::String(rough));
			    opacityEdit->setText(tgui::String(opacity));

                auto& params = material->GetParameters();
                for(auto& i : params)
                {
                    switch(i.second)
                    {
                    case Material::Type::Color:
                        if(!color.has_value() && ctmp.has_value())
                        {
                            auto name = TextureManager::GetInstance()->GetName(std::get<1>(i.first));
                            TextureManager::GetInstance()->DeleteTexture(name);
                            i.first = ctmp.value();
                        }
                        if(color.has_value() && ctmp.has_value())
                            i.first = ctmp.value();
                        break;
                    case Material::Type::Metalness:
                        if(!std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Metalness)) && metal > 0.0)
                        {
                            auto name = TextureManager::GetInstance()->GetName(std::get<1>(i.first));
                            TextureManager::GetInstance()->DeleteTexture(name);
                            i.first = glm::vec3(metal);
                        }
                        if(std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Metalness)))
                            i.first = glm::vec3(metal);
                        break;
                    case Material::Type::Roughness:
                        if(!std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Roughness)) && rough > 0.0)
                        {
                            auto name = TextureManager::GetInstance()->GetName(std::get<1>(i.first));
                            TextureManager::GetInstance()->DeleteTexture(name);
                            i.first = glm::vec3(rough);
                        }
                        if(std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Roughness)))
                            i.first = glm::vec3(rough);
                        break;
                    case Material::Type::Emission:
                        if(!emission.has_value() && etmp.has_value())
                        {
                            auto name = TextureManager::GetInstance()->GetName(std::get<1>(i.first));
                            TextureManager::GetInstance()->DeleteTexture(name);
                            i.first = etmp.value();
                        }
                        if(emission.has_value() && etmp.has_value())
                            i.first = etmp.value();
                        break;
                    case Material::Type::Opacity:
                        if(!std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Opacity)) && opacity > 0.0)
                        {
                            auto name = TextureManager::GetInstance()->GetName(std::get<1>(i.first));
                            TextureManager::GetInstance()->DeleteTexture(name);
                            i.first = glm::vec3(opacity);
                        }
                        if(std::holds_alternative<glm::vec3>(material->GetParameter(Material::Type::Opacity)))
                            i.first = glm::vec3(opacity);
                        break;
                    default: break;
                    }
                }
		    }
		    /////////////////////////////////////

			////////////// SCRIPTS //////////////
			if(sceneTree->getSelectedItem()[1] == "Scripts")
			{
				materialEditorGroup->setEnabled(false);
				materialEditorGroup->setVisible(false);
				objectEditorGroup->setEnabled(false);
				objectEditorGroup->setVisible(false);
				scriptsGroup->setEnabled(true);
				scriptsGroup->setVisible(true);
				sceneGroup->setEnabled(false);
				sceneGroup->setVisible(false);
			}
		}
		else
		{
            nameEdit->setText("");
			objectEditorGroup->setEnabled(false);
			objectEditorGroup->setVisible(false);
			materialEditorGroup->setEnabled(false);
			materialEditorGroup->setVisible(false);
			scriptsGroup->setEnabled(false);
			scriptsGroup->setVisible(false);
			sceneGroup->setEnabled(true);
			sceneGroup->setVisible(true);
		}
		if(scriptLaunched)
			scman.ExecuteFunction(loopDecl);

		cam.Update();
		if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
		{
			engine.GetWindow().setMouseCursorVisible(false);
			engine.GetWindow().setMouseCursorGrabbed(true);

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                cam.SetSpeed(2.0);
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
                cam.SetSpeed(0.5);
            else cam.SetSpeed(1.0);

            modeLabel->setText("Mode: Camera movement");

	        if(manageCameraMovement) cam.Move(1);
	        if(manageCameraMouse) cam.Mouse();
        }
        else
        {
            if(!objectMovement)
                modeLabel->setText("Mode: None");
            else modeLabel->setText("Mode: Object movement");

        	engine.GetWindow().setMouseCursorVisible(true);
        	engine.GetWindow().setMouseCursorGrabbed(false);
        }
        if(manageCameraLook) cam.Look();

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::G) && changeMode.getElapsedTime().asSeconds() > 0.3)
        {
            objectMovement = !objectMovement;
            changeMode.restart();
        }

		ListenerWrapper::SetPosition(cam.GetPosition());
		ListenerWrapper::SetOrientation(cam.GetOrientation());

		if(engine.GetWindow().hasFocus())
            engine.GetWindow().setFramerateLimit(60);
        else engine.GetWindow().setFramerateLimit(5);

		shadows.Update();

        scene.Draw(Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main));

        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1f("exposure", 1.0);

		viewport->bindFramebuffer();
		glClear(GL_COLOR_BUFFER_BIT);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Draw();
        glDisable(GL_DEPTH_TEST);
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Draw();
        glEnable(GL_DEPTH_TEST);
        Framebuffer::Unbind();

        editor.draw();
    });

    engine.Launch();

    std::ofstream out(homeFolder + "/properties.json");
    out << properties.toStyledString();
    out.close();
}
