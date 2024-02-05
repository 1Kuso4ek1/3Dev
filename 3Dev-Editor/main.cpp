#include <Engine.hpp>

#ifdef _WIN32
    std::string homeFolder = std::string(getenv("HOMEPATH")) + "/.3Dev-Editor/";
#else
    std::string homeFolder = std::string(getenv("HOME")) + "/.3Dev-Editor/";
#endif

sf::Clock shortcutDelay;

std::string lastPath = std::filesystem::current_path().string();
std::string projectDir;

bool disableShortcuts = false;

std::unordered_map<std::string, std::string> code;
std::string currentFile;

struct
{
    float time = 0.0;
    std::string name;
    std::shared_ptr<Animation> ptr;
} lastAnimation;

void SaveProperties(Json::Value data)
{
    std::ofstream file(homeFolder + "properties.json");
    file << data.toStyledString();
    file.close();
}

Json::Value DefaultProperties(const Json::Value& recentProjects = "", const Json::Value& recentProjectsPaths = "")
{
    Json::Value p;

    p["version"] = EDITOR_VERSION;

    p["logFilename"] = homeFolder + "log/EditorLog.txt";
    p["defaultResorces"] = homeFolder + "default/";
    p["enableMultithreading"] = true;

    p["renderer"]["shadersDir"] = std::filesystem::absolute(std::string(SHADERS_DIRECTORY)).string();
    p["renderer"]["hdriPath"] = homeFolder + "default/hdri.hdr";
    p["renderer"]["skyboxSideSize"] = 256;
    p["renderer"]["irradianceSideSize"] = 32;
    p["renderer"]["prefilteredSideSize"] = 256;
    p["renderer"]["shadowMapResolution"] = 4096;
    p["renderer"]["bloomResolutionScale"] = 8;
    p["renderer"]["ssaoSamples"] = 64;
    p["renderer"]["ssaoStrength"] = 3.0;
    p["renderer"]["ssaoRadius"] = 0.5;
    p["renderer"]["exposure"] = 0.8;
    p["renderer"]["ssrRayStep"] = 0.1;
    p["renderer"]["ssrMaxSteps"] = 100;
    p["renderer"]["ssrMaxBinarySearchSteps"] = 20;
    p["renderer"]["useRGBA16F"] = true;
    p["renderer"]["ssrEnabled"] = true;

    if(!recentProjects.isArray())
    {
        p["recentProjects"][0] = recentProjects;
        p["recentProjectsPaths"][0] = recentProjectsPaths;
    }
    else
    {
        p["recentProjects"] = recentProjects;
        p["recentProjectsPaths"] = recentProjectsPaths;
    }

    SaveProperties(p);

    return p;
}

Json::Value ParseProperties()
{
	Json::Value ret;
	Json::CharReaderBuilder rbuilder;

    std::ifstream file(homeFolder + "properties.json");

    std::string errors;
    if(!Json::parseFromStream(rbuilder, file, &ret, &errors))
        Log::Write("Json parsing failed: " + errors, Log::Type::Critical);

    if(ret["version"].asString() != EDITOR_VERSION)
    {
        Log::Write("The editor's version has changed, updating properties", Log::Type::Info);

        std::filesystem::remove(homeFolder + "default/hdri.hdr");
        std::filesystem::remove(homeFolder + "gui/menu.txt");
        std::filesystem::remove(homeFolder + "gui/loading.txt");
        std::filesystem::remove(homeFolder + "gui/editor.txt");
        std::filesystem::remove(homeFolder + "gui/themes/Black.txt");
        std::filesystem::remove(homeFolder + "gui/themes/Black.png");
	    std::filesystem::remove(homeFolder + "gui/themes/SourceCodePro-Regular.ttf");
        std::filesystem::remove(homeFolder + "icon.png");

        std::filesystem::copy("../default/hdri.hdr", homeFolder + "default/hdri.hdr");
        std::filesystem::copy("../gui/menu.txt", homeFolder + "gui/menu.txt");
        std::filesystem::copy("../gui/loading.txt", homeFolder + "gui/loading.txt");
        std::filesystem::copy("../gui/editor.txt", homeFolder + "gui/editor.txt");
        std::filesystem::copy("../gui/themes/Black.txt", homeFolder + "gui/themes/Black.txt");
        std::filesystem::copy("../gui/themes/Black.png", homeFolder + "gui/themes/Black.png");
	    std::filesystem::copy("../gui/themes/SourceCodePro-Regular.ttf", homeFolder + "gui/themes/SourceCodePro-Regular.ttf");
        std::filesystem::copy("../icon.png", homeFolder + "icon.png");

        return DefaultProperties(ret["recentProjects"], ret["recentProjectsPaths"]);
    }

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
			{ "Images", { "*.jpg", "*.jpeg", "*.png", "*.bmp"  } }
		});
    else if(fileType == 4)
        dialog->setFileTypeFilters(
		{
			{ "Sounds", { "*.ogg", "*.wav", "*.mp3"  } }
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

bool Shortcut(std::vector<sf::Keyboard::Key> keys, float delay = 0.3)
{
	bool ret = true && shortcutDelay.getElapsedTime().asSeconds() > delay && !disableShortcuts;
	for(auto& i : keys)
		ret &= sf::Keyboard::isKeyPressed(i);
	if(ret)
		shortcutDelay.restart();
	return ret;
}

struct
{
	std::string name;
	std::shared_ptr<Model> object;
} buffer;

int main()
{
    Json::Value properties;

    if(std::filesystem::exists(homeFolder + "properties.json"))
        properties = ParseProperties();
    else
    {
    	std::filesystem::create_directory(homeFolder);
    	std::filesystem::create_directory(homeFolder + "log");
    	std::filesystem::create_directory(homeFolder + "gui");
        std::filesystem::create_directory(homeFolder + "gui/themes");
        std::filesystem::create_directory(homeFolder + "default");

        std::filesystem::copy("../default/hdri.hdr", homeFolder + "default/hdri.hdr", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../gui/menu.txt", homeFolder + "gui/menu.txt", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../gui/loading.txt", homeFolder + "gui/loading.txt", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../gui/editor.txt", homeFolder + "gui/editor.txt", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../gui/themes/Black.txt", homeFolder + "gui/themes/Black.txt", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../gui/themes/Black.png", homeFolder + "gui/themes/Black.png", std::filesystem::copy_options::overwrite_existing);
	    std::filesystem::copy("../gui/themes/SourceCodePro-Regular.ttf", homeFolder + "gui/themes/SourceCodePro-Regular.ttf", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy("../icon.png", homeFolder + "icon.png", std::filesystem::copy_options::overwrite_existing);

        DefaultProperties();
        Log::Write("properties.json created.", Log::Type::Info);
        properties = ParseProperties();
    }

    Engine engine(false);
    Log::Init(properties["logFilename"].asString(), false, true);
    engine.CreateWindow(1600, 900, "3Dev Editor");
    engine.Init();

    engine.GetWindow().setFramerateLimit(60);

    sf::Image icon;
    if(icon.loadFromFile(homeFolder + "icon.png"))
        engine.GetWindow().setIcon(icon.getSize(), icon.getPixelsPtr());

    std::filesystem::current_path(homeFolder + "gui");

    tgui::Gui menu{ engine.GetWindow() };
    tgui::Gui loading{ engine.GetWindow() };
    tgui::Gui editor{ engine.GetWindow() };

    menu.loadWidgetsFromFile(homeFolder + "gui/menu.txt");
    loading.loadWidgetsFromFile(homeFolder + "gui/loading.txt");
    editor.loadWidgetsFromFile(homeFolder + "gui/editor.txt");

    auto newProjectButton = menu.get<tgui::Button>("newProject");
    auto loadProjectButton = menu.get<tgui::Button>("loadProject");

    auto newProjectWindow = menu.get<tgui::ChildWindow>("newProjectWindow");
    auto loadProjectWindow = menu.get<tgui::ChildWindow>("loadWindow");

    auto recentBox = menu.get<tgui::ListBox>("recent");
    auto pathEdit = menu.get<tgui::EditBox>("path");
    auto openFileDialogButton = menu.get<tgui::Button>("openFileDialog");
    auto loadButton = menu.get<tgui::Button>("load");
    auto cancelLoadButton = menu.get<tgui::Button>("cancelLoad");

    auto projectNameEdit = menu.get<tgui::EditBox>("name");
    auto folderEdit = menu.get<tgui::EditBox>("folder");
    auto createButton = menu.get<tgui::Button>("create");
    auto cancelNewButton = menu.get<tgui::Button>("cancelNew");
    
    auto progressBar = loading.get<tgui::ProgressBar>("progressBar");

	auto viewportWindow = editor.get<tgui::ChildWindow>("viewport");
    auto viewport = tgui::CanvasOpenGL3::create({ "100%", "100%" });

    viewportWindow->add(viewport);

    auto codeEditor = editor.get<tgui::ChildWindow>("codeEditor");
    auto codeArea = editor.get<tgui::TextArea>("code");
    auto fileTabs = editor.get<tgui::Tabs>("fileTabs");

	auto messagesBox = editor.get<tgui::ChatBox>("messages");

    auto modelButton = editor.get<tgui::Button>("createModel");
    auto shapeButton = editor.get<tgui::Button>("createShape");
    auto lightButton = editor.get<tgui::Button>("createLight");
    auto materialButton = editor.get<tgui::Button>("createMaterial");
    auto boxColliderButton = editor.get<tgui::Button>("boxCollider");
    auto sphereColliderButton = editor.get<tgui::Button>("sphereCollider");
    auto capsuleColliderButton = editor.get<tgui::Button>("capsuleCollider");
    auto concaveColliderButton = editor.get<tgui::Button>("concaveCollider");
    auto soundButton = editor.get<tgui::Button>("createSound");
	auto scriptButton = editor.get<tgui::Button>("createScript");
    auto animationButton = editor.get<tgui::Button>("createAnimation");

    auto sceneTree = editor.get<tgui::TreeView>("scene");

    auto objectEditorGroup = editor.get<tgui::Group>("objectEditor");
    auto lightEditorGroup = editor.get<tgui::Group>("lightEditor");
    auto materialEditorGroup = editor.get<tgui::Group>("materialEditor");
    auto boneEditorGroup = editor.get<tgui::Group>("boneEditor");
    auto animationEditorGroup = editor.get<tgui::Group>("animationEditor");
    auto soundEditorGroup = editor.get<tgui::Group>("soundEditor");
	auto scriptsGroup = editor.get<tgui::Group>("scriptsGroup");
	auto sceneGroup = editor.get<tgui::Group>("sceneGroup");

    std::vector<tgui::Group::Ptr> groups = { objectEditorGroup, lightEditorGroup,
                                             materialEditorGroup, soundEditorGroup,
                                             scriptsGroup, sceneGroup, boneEditorGroup,
                                             animationEditorGroup };

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

    auto bodyTypeBox = editor.get<tgui::ComboBox>("bodyType");

    auto isDrawableBox = editor.get<tgui::CheckBox>("isDrawable");
    auto immLoadBox = editor.get<tgui::CheckBox>("immLoad");

    auto openFileButton = editor.get<tgui::Button>("openFile");
    auto reloadButton = editor.get<tgui::Button>("reload");
    auto deleteButton = editor.get<tgui::Button>("delete");

    auto materialNameEdit = editor.get<tgui::EditBox>("matName");

    auto colorPickerButton = editor.get<tgui::Button>("colorPicker");
    auto emissionPickerButton = editor.get<tgui::Button>("emissionPicker");

    std::shared_ptr<tgui::ColorPicker> colorPicker = nullptr, emissionPicker = nullptr;

    auto metalEdit = editor.get<tgui::EditBox>("mtlbox");
    auto roughEdit = editor.get<tgui::EditBox>("rghbox");
    auto opacityEdit = editor.get<tgui::EditBox>("opcbox");
    auto emissionStrengthEdit = editor.get<tgui::EditBox>("emissionStrength");

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

    auto lightNameEdit = editor.get<tgui::EditBox>("lightName");

    auto lposEditX = editor.get<tgui::EditBox>("lposX");
    auto lposEditY = editor.get<tgui::EditBox>("lposY");
    auto lposEditZ = editor.get<tgui::EditBox>("lposZ");

    auto lrotEditX = editor.get<tgui::EditBox>("lrotX");
    auto lrotEditY = editor.get<tgui::EditBox>("lrotY");
    auto lrotEditZ = editor.get<tgui::EditBox>("lrotZ");

    auto rEdit = editor.get<tgui::EditBox>("r");
    auto gEdit = editor.get<tgui::EditBox>("g");
    auto bEdit = editor.get<tgui::EditBox>("b");

    auto constAttEdit = editor.get<tgui::EditBox>("constAtt");
    auto linAttEdit = editor.get<tgui::EditBox>("linAtt");
    auto quadAttEdit = editor.get<tgui::EditBox>("quadAtt");

    auto innerCutoffEdit = editor.get<tgui::EditBox>("innerCutoff");
    auto outerCutoffEdit = editor.get<tgui::EditBox>("outerCutoff");

    auto castShadowsBox = editor.get<tgui::CheckBox>("castShadows");
    auto perspectiveShadowsBox = editor.get<tgui::CheckBox>("perspectiveShadows");

    auto ldeleteButton = editor.get<tgui::Button>("ldelete");

    auto animNameEdit = editor.get<tgui::EditBox>("animName");

    auto durationEdit = editor.get<tgui::EditBox>("duration");
    auto tpsEdit = editor.get<tgui::EditBox>("tps");
    auto kfNameEdit = editor.get<tgui::EditBox>("kfName");
    auto addKfButton = editor.get<tgui::Button>("addKf");
    auto timelineSlider = editor.get<tgui::Slider>("timeline");
    auto timeEdit = editor.get<tgui::EditBox>("time");
    auto repeatBox = editor.get<tgui::CheckBox>("repeat");
    auto addActionButton = editor.get<tgui::Button>("addAction");
    auto deleteAnimOrKFButton = editor.get<tgui::Button>("deleteAnimOrKF");

    auto aplayButton = editor.get<tgui::Button>("aplay");
    auto apauseButton = editor.get<tgui::Button>("apause");
    auto astopButton = editor.get<tgui::Button>("astop");

    auto soundNameEdit = editor.get<tgui::EditBox>("soundName");

    auto sposEditX = editor.get<tgui::EditBox>("sposX");
    auto sposEditY = editor.get<tgui::EditBox>("sposY");
    auto sposEditZ = editor.get<tgui::EditBox>("sposZ");
    auto volumeSlider = editor.get<tgui::Slider>("volume");
    auto attenuationEdit = editor.get<tgui::EditBox>("attenuation");
    auto minDistEdit = editor.get<tgui::EditBox>("minDist");

    auto loopBox = editor.get<tgui::CheckBox>("loop");

    auto playButton = editor.get<tgui::Button>("play");
    auto pauseButton = editor.get<tgui::Button>("pause");
    auto stopButton = editor.get<tgui::Button>("stop");

    auto sdeleteButton = editor.get<tgui::Button>("sdelete");

    auto boneNameEdit = editor.get<tgui::EditBox>("boneName");

    auto bposEditX = editor.get<tgui::EditBox>("bposX");
    auto bposEditY = editor.get<tgui::EditBox>("bposY");
    auto bposEditZ = editor.get<tgui::EditBox>("bposZ");

    auto brotEditX = editor.get<tgui::EditBox>("brotX");
    auto brotEditY = editor.get<tgui::EditBox>("brotY");
    auto brotEditZ = editor.get<tgui::EditBox>("brotZ");

    auto bsizeEditX = editor.get<tgui::EditBox>("bsizeX");
    auto bsizeEditY = editor.get<tgui::EditBox>("bsizeY");
    auto bsizeEditZ = editor.get<tgui::EditBox>("bsizeZ");

	auto buildButton = editor.get<tgui::Button>("build");
    auto startStopButton = editor.get<tgui::Button>("startStop");
    auto variablesList = editor.get<tgui::ListBox>("variables");
    auto valueEdit = editor.get<tgui::EditBox>("value");
    auto setValue = editor.get<tgui::Button>("setValue");
    auto editScriptButton = editor.get<tgui::Button>("editScript");
    auto removeScriptButton = editor.get<tgui::Button>("removeScript");

    auto filenameEdit = editor.get<tgui::EditBox>("filename");
    auto fileDialogButton = editor.get<tgui::Button>("openFileDialog");
    auto saveButton = editor.get<tgui::Button>("save");

    auto modeLabel = editor.get<tgui::Label>("mode");
    modeLabel->moveToFront();

    codeArea->enableMonospacedFontOptimization();

    std::shared_ptr<tgui::FileDialog> openFileDialog = nullptr;

    #ifdef _WIN32
        folderEdit->setText(std::string(getenv("HOMEPATH")));
    #else
        folderEdit->setText(std::string(getenv("HOME")));
    #endif

    bool objectMode = false;
    int axis = -1, param = 0;

    tgui::Color matColor = tgui::Color::White;

    if(!properties["recentProjects"].empty())
    {
        for(auto i : properties["recentProjects"])
            if(!i.asString().empty()) recentBox->addItem(i.asString());

        if(recentBox->getItemCount() > 0)
            recentBox->setSelectedItemByIndex(0);
        if(recentBox->getSelectedItemIndex() >= 0)
            pathEdit->setText(properties["recentProjectsPaths"]
                                        [recentBox->getSelectedItemIndex()].asString());
    }

    std::string projectFilename = "";

    recentBox->onItemSelect([&]()
    {
        if(recentBox->getSelectedItemIndex() >= 0)
            pathEdit->setText(properties["recentProjectsPaths"]
                                        [recentBox->getSelectedItemIndex()].asString());
    });

    newProjectButton->onPress([&]()
    {
        newProjectWindow->setEnabled(true);
        newProjectWindow->setVisible(true);
    });

    loadProjectButton->onPress([&]()
    {
        loadProjectWindow->setEnabled(true);
        loadProjectWindow->setVisible(true);
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

    loadButton->onPress([&]()
    {
        if(!properties["recentProjectsPaths"].empty() && recentBox->getSelectedItemIndex() >= 0)
            projectFilename = properties["recentProjectsPaths"]
                                        [recentBox->getSelectedItemIndex()].asString();
        if(projectFilename != pathEdit->getText().toStdString())
            projectFilename = pathEdit->getText().toStdString();
        projectDir = projectFilename.substr(0, projectFilename.find_last_of("/"));
        if(!std::filesystem::exists(projectDir + "/assets"))
        {
            std::filesystem::create_directory(projectDir + "/assets");
            std::filesystem::create_directory(projectDir + "/assets/models");
            std::filesystem::create_directory(projectDir + "/assets/textures");
            std::filesystem::create_directory(projectDir + "/assets/sounds");
            std::filesystem::create_directory(projectDir + "/assets/scripts");
        }
        engine.Close();
    });

    createButton->onPress([&]()
    {
        if(!std::filesystem::exists(folderEdit->getText().toStdString()))
            std::filesystem::create_directory(folderEdit->getText().toStdString());
        lastPath = folderEdit->getText().toStdString() + "/" + projectNameEdit->getText().toStdString();
        projectDir = lastPath;
        std::filesystem::create_directory(lastPath);
        std::filesystem::create_directory(lastPath + "/assets");
        std::filesystem::create_directory(lastPath + "/assets/models");
        std::filesystem::create_directory(lastPath + "/assets/textures");
        std::filesystem::create_directory(projectDir + "/assets/sounds");
        std::filesystem::create_directory(lastPath + "/assets/scripts");
        engine.Close();
    });

    cancelNewButton->onPress([&]()
    {
        newProjectWindow->setEnabled(false);
        newProjectWindow->setVisible(false);
    });

    cancelLoadButton->onPress([&]()
    {
        loadProjectWindow->setEnabled(false);
        loadProjectWindow->setVisible(false);
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

    engine.Launch(false);
    
    progressBar->setText("Initializing Renderer");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();
        
    Renderer::GetInstance()->SetShadersDirectory(properties["renderer"]["shadersDir"].asString());
    if(properties["renderer"]["ssaoSamples"].asInt() > 0)
        Renderer::GetInstance()->SetSSAOSamples(properties["renderer"]["ssaoSamples"].asInt());
    Renderer::GetInstance()->SetIsSSREnabled(properties["renderer"]["ssrEnabled"].asBool());
    Renderer::GetInstance()->SetSSRRayStep(properties["renderer"]["ssrRayStep"].asFloat());
    Renderer::GetInstance()->SetSSRMaxSteps(properties["renderer"]["ssrMaxSteps"].asInt());
    Renderer::GetInstance()->SetSSRMaxBinarySearchSteps(properties["renderer"]["ssrMaxBinarySearchSteps"].asInt());
    
    Renderer::GetInstance()->SetIsSSGIEnabled(properties["renderer"]["ssgiEnabled"].asBool());
    Renderer::GetInstance()->SetSSGIStrength(properties["renderer"]["ssgiStrength"].asFloat());

    Renderer::GetInstance()->Init({ (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y },
                                    properties["renderer"]["hdriPath"].asString(),
                                    properties["renderer"]["skyboxSideSize"].asInt(),
                                    properties["renderer"]["irradianceSideSize"].asInt(),
                                    properties["renderer"]["prefilteredSideSize"].asInt(),
                                    properties["renderer"]["bloomResolutionScale"].asFloat(),
                                    properties["renderer"]["useRGBA16F"].asBool());
    
    progressBar->setValue(20);
    progressBar->setText("Setting up defaults");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();

    Camera cam(&engine.GetWindow());
    cam.SetViewportSize({ (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y });
    cam.SetGuiSize({ (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y });

    auto shadowSource = std::make_shared<Light>(rp3d::Vector3(0, 0, 0), rp3d::Vector3(30.1, 50.0, -30.1), true);

    Material skyboxMaterial(
    {
        { Renderer::GetInstance()->GetTexture(Renderer::TextureType::Skybox), Material::Type::Cubemap }
    });

    auto defaultMaterial = std::make_shared<Material>();

    auto skybox = std::make_shared<Model>(true);
    skybox->SetMaterial({ &skyboxMaterial });

	rp3d::PhysicsWorld::WorldSettings st;
    auto man = std::make_shared<PhysicsManager>(st);
    auto gizmosMan = std::make_shared<PhysicsManager>(st);

    progressBar->setValue(40);
    progressBar->setText("Loading scene");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();

    engine.SetGuiView({ 0, 0, viewportWindow->getSize().x * 1.65f, viewportWindow->getSize().y * 1.65f });
    engine.SetGuiViewport({ 221, 26, viewportWindow->getSize().x - 1, viewportWindow->getSize().y - 28 });

    auto sman = std::make_shared<SoundManager>();

    auto saveRecent = [&](std::string path)
    {
        auto filename = std::filesystem::path(path).filename().string();
        auto it = std::find(properties["recentProjects"].begin(), properties["recentProjects"].end(), filename);
        if(it == properties["recentProjects"].end())
        {
            properties["recentProjects"].insert(0, filename);
            properties["recentProjectsPaths"].insert(0, path);
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
            properties["recentProjectsPaths"].insert(0, path);
        }
        SaveProperties(properties);
        Log::Write(path + " saved", Log::Type::Info);
    };

    Multithreading::GetInstance()->SetIsEnabled(properties["enableMultithreading"].asBool());

    // Gizmos
    sf::Vector2i prevMPos;
    auto gizmosFb = std::make_shared<Framebuffer>(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post), (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y);

    Material gizmoXMat(
    {
        { glm::vec3(1.0, 0.0, 0.0), Material::Type::Color },
        { glm::vec3(0.0), Material::Type::Metalness },
        { glm::vec3(1.0), Material::Type::Roughness },
        { glm::vec3(1.0, 0.0, 0.0), Material::Type::Emission },
        { glm::vec3(1.0), Material::Type::EmissionStrength },
        { glm::vec3(1.0), Material::Type::Opacity }
    });

    Material gizmoYMat(
    {
        { glm::vec3(0.0, 1.0, 0.0), Material::Type::Color },
        { glm::vec3(0.0), Material::Type::Metalness },
        { glm::vec3(1.0), Material::Type::Roughness },
        { glm::vec3(0.0, 1.0, 0.0), Material::Type::Emission },
        { glm::vec3(1.0), Material::Type::EmissionStrength },
        { glm::vec3(1.0), Material::Type::Opacity }
    });

    Material gizmoZMat(
    {
        { glm::vec3(0.0, 0.0, 1.0), Material::Type::Color },
        { glm::vec3(0.0), Material::Type::Metalness },
        { glm::vec3(1.0), Material::Type::Roughness },
        { glm::vec3(0.0, 0.0, 1.0), Material::Type::Emission },
        { glm::vec3(1.0), Material::Type::EmissionStrength },
        { glm::vec3(1.0), Material::Type::Opacity }
    });

    auto gizmoX = std::make_shared<Model>(true);
    gizmoX->SetShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Forward));
    gizmoX->SetMaterial({ &gizmoXMat });
    gizmoX->SetPhysicsManager(gizmosMan.get());
    gizmoX->CreateRigidBody();
    gizmoX->CreateBoxShape();
    gizmoX->GetRigidBody()->setType(rp3d::BodyType::STATIC);

    auto gizmoY = std::make_shared<Model>(true);
    gizmoY->SetShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Forward));
    gizmoY->SetMaterial({ &gizmoYMat });
    gizmoY->SetPhysicsManager(gizmosMan.get());
    gizmoY->CreateRigidBody();
    gizmoY->CreateBoxShape();
    gizmoY->GetRigidBody()->setType(rp3d::BodyType::STATIC);

    auto gizmoZ = std::make_shared<Model>(true);
    gizmoZ->SetShader(Renderer::GetInstance()->GetShader(Renderer::ShaderType::Forward));
    gizmoZ->SetMaterial({ &gizmoZMat });
    gizmoZ->SetPhysicsManager(gizmosMan.get());
    gizmoZ->CreateRigidBody();
    gizmoZ->CreateBoxShape();
    gizmoZ->GetRigidBody()->setType(rp3d::BodyType::STATIC);

    SceneManager scene;

	scene.SetPhysicsManager(man);
	scene.SetCamera(&cam);
    scene.SetSkybox(skybox);
    scene.UpdatePhysics(false);
    scene.SetSoundManager(sman);

	if(!projectFilename.empty())
	{
        lastPath = projectFilename.substr(0, projectFilename.find_last_of("/"));
        scene.Load(projectFilename, true);
        if(scene.GetNames()[2].empty())
            scene.AddLight(shadowSource, "shadowSource");
        
        filenameEdit->setText(projectFilename.empty() ? "scene.json" : projectFilename);
    }
    else
    {
        scene.AddLight(shadowSource, "shadowSource");
        scene.AddMaterial(defaultMaterial, "default");
        materialBox->addItem("default");
        sceneTree->addItem({ "Scene", "Materials", "default" });
        sceneTree->addItem({ "Scene", "Objects", "shadowSource" });
        projectFilename = lastPath + "/" + projectNameEdit->getText().toStdString() + ".json";
        scene.Save(projectFilename, true);
        saveRecent(projectFilename);
        filenameEdit->setText(projectFilename.empty() ? "scene.json" : projectFilename);
        projectFilename.clear();
    }

    progressBar->setValue(70);
    progressBar->setText("Loading scripts");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();

	bool manageCameraMovement = true, manageCameraLook = true,
         manageCameraMouse = true, manageSceneRendering = true,
         updateShadows = true, mouseCursorGrabbed = true,
         mouseCursorVisible = false;

    bool scriptLaunched = false;

    float exposure = properties["renderer"]["exposure"].asFloat();
    float bloomStrength = 0.3;
    float mouseSensitivity = 1.0;

    float dofMinDistance = 1.0;
    float dofMaxDistance = 1.0;
    float dofFocusDistance = 1.0;

    float fogStart = 0.0;
    float fogEnd = 0.0;
    float fogHeight = 0.0;
    
    float ssaoStrength = properties["renderer"]["ssaoStrength"].asFloat();
    float ssaoRadius = properties["renderer"]["ssaoRadius"].asFloat();

    int blurIterations = 8;

    ScriptManager scman;
    scman.AddProperty("Engine engine", &engine);
    scman.SetDefaultNamespace("Game");
    scman.AddProperty("SceneManager scene", &scene);
    scman.AddProperty("Camera camera", scene.GetCamera());
    scman.AddProperty("bool mouseCursorGrabbed", &mouseCursorGrabbed);
    scman.AddProperty("bool mouseCursorVisible", &mouseCursorVisible);
    scman.AddProperty("bool updateShadows", &updateShadows);
    scman.AddProperty("bool manageSceneRendering", &manageSceneRendering);
    scman.AddProperty("bool manageCameraMovement", &manageCameraMovement);
    scman.AddProperty("bool manageCameraLook", &manageCameraLook);
    scman.AddProperty("bool manageCameraMouse", &manageCameraMouse);
    scman.AddProperty("float mouseSensitivity", &mouseSensitivity);
    scman.AddProperty("float exposure", &exposure);
    scman.AddProperty("float bloomStrength", &bloomStrength);
    scman.AddProperty("int blurIterations", &blurIterations);
    scman.AddProperty("float ssaoStrength", &ssaoStrength);
    scman.AddProperty("float ssaoRadius", &ssaoRadius);
    scman.AddProperty("float dofMinDistance", &dofMinDistance);
    scman.AddProperty("float dofMaxDistance", &dofMaxDistance);
    scman.AddProperty("float dofFocusDistance", &dofFocusDistance);
    scman.AddProperty("float fogStart", &fogStart);
    scman.AddProperty("float fogEnd", &fogEnd);
    scman.AddProperty("float fogHeight", &fogHeight);
    scman.SetDefaultNamespace("");
	std::string startDecl = "void Start()", loopDecl = "void Loop()";

    if(!projectFilename.empty())
    {
        auto scPath = projectFilename;
        scPath.insert(scPath.find_last_of('.'), "_scripts");
        scman.Load(scPath);
        auto scripts = scman.GetScripts();
        for(auto& i : scripts)
        {
            std::string filename = std::filesystem::path(i).filename().string();
            std::ifstream file(i);
            std::copy(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>(),
                      std::back_inserter(code[filename]));
            codeArea->setText(code[filename]);
            fileTabs->add(filename);
        }
    }
    else scman.Save(lastPath + "/" + projectNameEdit->getText().toStdString() + "_scripts.json");

    auto readSceneTree = [&]()
    {
        sceneTree->removeAllItems();
        materialBox->removeAllItems();
        auto names = scene.GetNames();
        auto sounds = sman->GetSounds();
        std::vector<std::pair<std::string, std::vector<tgui::String>>> used;
        std::vector<std::string> pending;
        for(auto& i : names[3])
            if(!scene.GetNode(i)->GetParent())
                pending.push_back(i);

        while(!pending.empty())
        {
            std::vector<std::string> next;
            for(auto& i : pending)
            {
                auto node = scene.GetNode(i);

                if(!node->GetParent())
                {
                    sceneTree->addItem({ "Scene", "Objects", i });
                    used.push_back({ i, { "Scene", "Objects", i } });
                }
                else
                {
                    auto it = std::find_if(used.begin(), used.end(), [&](auto& a)
                                   { return scene.GetNodeName(node->GetParent()) == a.first; });
                    auto item = it->second;
                    item.push_back(i);
                    sceneTree->addItem(item);
                    sceneTree->collapse(item);
                    used.push_back({ i, item });
                }
                auto children = node->GetChildren();
                for(auto j : children)
                    if(!scene.GetNodeName(j).empty())
                        next.push_back(scene.GetNodeName(j));
            }
            pending = next;
        }
        for(auto& i : names[1])
        {
            materialBox->addItem(i);
            sceneTree->addItem({ "Scene", "Materials", i });
        }
        for(auto& i : sounds) sceneTree->addItem({ "Scene", "Sounds", i });
        for(auto& i : names[5])
        {
            sceneTree->addItem({ "Scene", "Animations", i });

            for(auto& [name, kf] : scene.GetAnimation(i)->GetKeyframes())
                sceneTree->addItem({ "Scene", "Animations", i, name });
        }

        auto scripts = scman.GetScripts();
        for(auto& i : scripts)
            sceneTree->addItem({ "Scripts", std::filesystem::path(i).filename().string() });
    };

    readSceneTree();

    Log::SetCrashHandle([&]()
    {
        auto path = filenameEdit->getText().toStdString(), scPath = path;
        scene.Save(path, true);
        scPath.insert(scPath.find_last_of('.'), "_scripts");
        scman.Save(scPath, true);
    });
    
    progressBar->setValue(90);
    progressBar->setText("Finishing");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();

    ShadowManager shadows(&scene, glm::ivec2(properties["renderer"]["shadowMapResolution"].asInt()));

    std::vector<std::vector<tgui::String>> selectedWithShift;
    std::vector<Framebuffer*> pingPongBuffers = 
    {
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0),
        Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)
    };

    modelButton->onPress([&]()
    {
    	auto model = std::make_shared<Model>();
    	model->SetMaterial({ scene.GetMaterial(scene.GetNames()[1][0]).get() });
		model->SetPhysicsManager(man.get());
		model->CreateRigidBody();
    	scene.AddModel(model);
    	readSceneTree();
        sceneTree->selectItem({ "Scene", "Objects", scene.GetLastAdded() });
    });

    shapeButton->onPress([&]()
    {
        auto model = std::make_shared<Model>(true);
		model->SetMaterial({ scene.GetMaterial(scene.GetNames()[1][0]).get() });
		model->SetPhysicsManager(man.get());
		model->CreateRigidBody();
        model->CreateBoxShape();
        scene.AddModel(model, "cube");
    	readSceneTree();
        sceneTree->selectItem({ "Scene", "Objects", scene.GetLastAdded() });
    });

    lightButton->onPress([&]()
    {
        auto light = std::make_shared<Light>(rp3d::Vector3::zero(), rp3d::Vector3::zero());
        scene.AddLight(light);
        readSceneTree();
        sceneTree->selectItem({ "Scene", "Objects", scene.GetLastAdded() });
    });

    materialButton->onPress([&]()
    {
    	scene.AddMaterial(std::make_shared<Material>());
    	readSceneTree();
    	materialBox->addItem(scene.GetLastAdded());
        sceneTree->selectItem({ "Scene", "Materials", scene.GetLastAdded() });
    });

    boxColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Objects")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateBoxShape(i);
			}
    });

    sphereColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Objects")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateSphereShape(i);
			}
    });

    capsuleColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Objects")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateCapsuleShape(i);
			}
    });

    concaveColliderButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 2)
			if(sceneTree->getSelectedItem()[1] == "Objects")
			{
			    auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
			    for(int i = 0; i < model->GetMeshesCount(); i++)
                    model->CreateConcaveShape(i);
			}
    });

    soundButton->onPress([&]()
    {
        openFileDialog = CreateFileDialog("Open file", 4);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                std::string filename = path.substr(path.find_last_of("/") + 1, path.size());
                if(!std::filesystem::exists(projectDir + "/assets/sounds/" + filename))
                    std::filesystem::copy(path, projectDir + "/assets/sounds/" + filename);
                path = projectDir + "/assets/sounds/" + filename;
                sman->LoadSound(path);
                sceneTree->addItem({ "Scene", "Sounds", sman->GetSounds().back() });
                sceneTree->selectItem({ "Scene", "Sounds", sman->GetSounds().back() });
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    playButton->onPress([&]()
    {
        sman->Play(sceneTree->getSelectedItem().back().toStdString());
    });

    pauseButton->onPress([&]()
    {
        sman->Pause(sceneTree->getSelectedItem().back().toStdString());
    });

    stopButton->onPress([&]()
    {
        sman->Stop(sceneTree->getSelectedItem().back().toStdString());
    });

    sdeleteButton->onPress([&]()
    {
        sman->RemoveSound(sceneTree->getSelectedItem().back().toStdString());
    	sceneTree->removeItem(sceneTree->getSelectedItem(), false);
    });

    ldeleteButton->onPress([&]()
    {
        auto light = scene.GetLight(sceneTree->getSelectedItem().back().toStdString());
    	scene.RemoveLight(light);
    	readSceneTree();
    });

    auto addKf = [&](std::string kf)
    {
        if(scene.GetNode(kf))
        {
            lastAnimation.ptr->AddKeyframe(kf, Keyframe());
            sceneTree->addItem({ "Scene", "Animations", lastAnimation.name, kf });
        }
    };

    addKfButton->onPress([&]()
    {
        addKf(kfNameEdit->getText().toStdString());
    });

    auto addAction = [&](std::string kfName)
    {
        auto& kf = lastAnimation.ptr->GetKeyframes()[kfName];
        auto node = scene.GetNode(kfName);

        kf.positions.push_back(toglm(node->GetTransform().getPosition()));
        kf.rotations.push_back(toglm(node->GetTransform().getOrientation()));
        kf.scales.push_back(toglm(node->GetSize()));

        kf.posStamps.push_back(lastAnimation.time);
        kf.rotStamps.push_back(lastAnimation.time);
        kf.scaleStamps.push_back(lastAnimation.time);
    };

    addActionButton->onPress([&]()
    {
        addAction(sceneTree->getSelectedItem()[3].toStdString());
    });

    deleteAnimOrKFButton->onPress([&]()
    {
        if(sceneTree->getSelectedItem().size() > 3)
            scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString())->GetKeyframes().erase(sceneTree->getSelectedItem()[3].toStdString());
        else scene.RemoveAnimation(scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString()));
        readSceneTree();
        if(sceneTree->getSelectedItem().size() > 3)
            sceneTree->selectItem({ "Scene", "Animations", sceneTree->getSelectedItem()[2].toStdString() });
    });

    aplayButton->onPress([&]()
    {
        scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString())->Play();
    });

    apauseButton->onPress([&]()
    {
        scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString())->Pause();
    });

    astopButton->onPress([&]()
    {
        scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString())->Stop();
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

    auto loadTexture = [&](Material::Type type)
    {
        openFileDialog = CreateFileDialog("Open file", 3);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&](Material::Type matType)
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                std::string filename = path.substr(path.find_last_of("/") + 1, path.size());
                if(!std::filesystem::exists(projectDir + "/assets/textures/" + filename))
                    std::filesystem::copy(path, projectDir + "/assets/textures/" + filename);
                path = projectDir + "/assets/textures/" + filename;
                auto mat = scene.GetMaterial(sceneTree->getSelectedItem().back().toStdString());
                auto p = mat->GetParameter(matType);
                std::string name = "texture";
                if(std::holds_alternative<GLuint>(p))
                {
                    name = TextureManager::GetInstance()->GetName(std::get<1>(p));
                    TextureManager::GetInstance()->DeleteTexture(name);
                }
                mat->SetParameter(TextureManager::GetInstance()->LoadTexture(path, name), matType);
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
                
                switch (matType)
                {
                case Material::Type::Metalness:
                    metalSlider->setValue(0);
                    break;
                case Material::Type::Roughness:
                    roughSlider->setValue(0);
                    break;
                case Material::Type::Opacity:
                    opacitySlider->setValue(0);
                    break;
                default:
                    break;
                }
            }
  	    	openFileDialog = nullptr;
  	    }, type);
    };

    colorTextureButton->onPress(loadTexture, Material::Type::Color);
    metalTextureButton->onPress(loadTexture, Material::Type::Metalness);
    roughTextureButton->onPress(loadTexture, Material::Type::Roughness);
    normalTextureButton->onPress(loadTexture, Material::Type::Normal);
    aoTextureButton->onPress(loadTexture, Material::Type::AmbientOcclusion);
    emissionTextureButton->onPress(loadTexture, Material::Type::Emission);
    opacityTextureButton->onPress(loadTexture, Material::Type::Opacity);

	scriptButton->onPress([&]()
	{
		openFileDialog = CreateFileDialog("Save file", 1, "Save", false);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
            {
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                std::string filename = std::filesystem::path(path).filename().string();
                if(!std::filesystem::exists(path))
                {
                    code[filename] = "";
                    fileTabs->add(filename);
                }
                else
                {
                    std::ifstream file(openFileDialog->getSelectedPaths()[0].asString().toStdString());
                    std::copy(std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>(),
                              std::back_inserter(code[filename]));
                    fileTabs->add(filename);
                    codeArea->setText(code[filename]);
                }
                scman.LoadScript(path);
                readSceneTree();
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
                currentFile = filename;

                codeEditor->setVisible(true);
                codeEditor->setEnabled(true);
            }
  	    	openFileDialog = nullptr;
  	    });
	});

    animationButton->onPress([&]()
    {
        auto anim = std::make_shared<Animation>("");
        scene.AddAnimation(anim);
        readSceneTree();
        sceneTree->selectItem({ "Scene", "Animations", scene.GetLastAdded() });
    });

    sceneTree->onItemSelect([&]()
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        {
            selectedWithShift.push_back(sceneTree->getSelectedItem());
            if(selectedWithShift.size() == 2)
            {
                auto parentNode = scene.GetNode(selectedWithShift[1].back().toStdString());
                auto childNode = scene.GetNode(selectedWithShift[0].back().toStdString());

                if(parentNode && childNode)
                {
                    parentNode->AddChild(childNode);
                    childNode->SetParent(parentNode);
                    readSceneTree();
                }
                selectedWithShift.clear();
            }
        }
        else selectedWithShift.clear();
    });

    openFileButton->onPress([&]()
    {
    	openFileDialog = CreateFileDialog("Open file", 0);
    	editor.add(openFileDialog);
    	openFileDialog->onClose([&]()
  	    {
            if(!openFileDialog->getSelectedPaths().empty())
			{
                auto path = openFileDialog->getSelectedPaths()[0].asString().toStdString();
                std::string filename = path.substr(path.find_last_of("/") + 1, path.size());
                if(!std::filesystem::exists(projectDir + "/assets/models/" + filename))
                    std::filesystem::copy(path, projectDir + "/assets/models/" + filename);
                path = projectDir + "/assets/models/" + filename;
                auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
                scene.RemoveBones(model);
                model->Load(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
                scene.StoreBones(model);
                readSceneTree();
                materialsList->removeAllItems();
                auto mtl = model->GetMaterial();
                for(int i = 0; i < mtl.size(); i++)
                    materialsList->addItem(scene.GetMaterialName(mtl[i]), tgui::String(i));
                lastPath = openFileDialog->getSelectedPaths()[0].getParentPath().asString().toStdString();
            }
  	    	openFileDialog = nullptr;
  	    });
    });

    reloadButton->onPress([&]()
    {
        auto model = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
        if(!model->GetFilename().empty())
            model->Load(model->GetFilename(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    });

    deleteButton->onPress([&]()
    {
    	scene.RemoveModel(scene.GetModel(sceneTree->getSelectedItem().back().toStdString()));
    	readSceneTree();
    });

	buildButton->onPress([&]()
    {
    	scman.Build();
        variablesList->removeAllItems();
        auto variables = scman.GetGlobalVariables();
        for(auto& i : variables)
            variablesList->addItem(i.first);
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
                engine.RemoveGui();
			    scene.UpdatePhysics(false);
			    scene.LoadState();
			}
		}
    });

    setValue->onPress([&]()
    {
        auto selected = variablesList->getSelectedItem().toStdString();
        void* var = scman.GetGlobalVariables()[selected];

        auto type = selected.substr(0, selected.find(' '));
        if(type == "int") *(int32_t*)(var) = valueEdit->getText().toInt();
        if(type == "int8") *(int8_t*)(var) = valueEdit->getText().toInt();
        if(type == "int16") *(int16_t*)(var) = valueEdit->getText().toInt();
        if(type == "int64") *(int64_t*)(var) = valueEdit->getText().toInt();
        if(type == "uint") *(uint32_t*)(var) = valueEdit->getText().toUInt();
        if(type == "uint8") *(uint8_t*)(var) = valueEdit->getText().toUInt();
        if(type == "uint16") *(uint16_t*)(var) = valueEdit->getText().toUInt();
        if(type == "uint64") *(uint64_t*)(var) = valueEdit->getText().toUInt();
        if(type == "float") *(float*)(var) = valueEdit->getText().toFloat();
        if(type == "double") *(double*)(var) = valueEdit->getText().toFloat();
        if(type == "bool") *(bool*)(var) = valueEdit->getText() == "true" ? true : false;
        if(type == "string") *(std::string*)(var) = valueEdit->getText().toStdString();
    });

    auto saveFile = [&]()
    {
        if(currentFile.empty())
            currentFile = fileTabs->getSelected().toStdString();
        std::ofstream file(projectDir + "/assets/scripts/" + currentFile);
        file << codeArea->getText().toStdString();
        file.close();
        code[currentFile] = codeArea->getText().toStdString();
    };

    fileTabs->onTabSelect([&]()
    {
        if(!currentFile.empty())
            saveFile();
        currentFile = fileTabs->getSelected().toStdString();
        codeArea->setText(code[currentFile]);
    });

    editScriptButton->onPress([&]()
	{
        if(sceneTree->getSelectedItem().size() > 1)
        {
            codeEditor->setVisible(true);
            codeEditor->setEnabled(true);
            codeEditor->moveToFront();
            fileTabs->select(sceneTree->getSelectedItem().back().toStdString());
        }
	});

    removeScriptButton->onPress([&]()
	{
        if(sceneTree->getSelectedItem().size() > 1)
        {
            scman.RemoveScript(std::filesystem::absolute("assets/scripts/" + sceneTree->getSelectedItem().back().toStdString()).string());
            fileTabs->remove(sceneTree->getSelectedItem().back().toStdString());
            code[sceneTree->getSelectedItem().back().toStdString()] = "";
            sceneTree->removeItem(sceneTree->getSelectedItem(), false);
            if(fileTabs->getTabsCount() == 0 && codeEditor->isEnabled())
            {
                codeEditor->setVisible(false);
                codeEditor->setEnabled(false);
            }
        }
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

	auto saveProject = [&]()
	{
		if(!filenameEdit->getText().empty())
        {
            auto path = filenameEdit->getText().toStdString(), scPath = path;
            scPath.insert(scPath.find_last_of('.'), "_scripts");

            scene.Save(path, true);
            scman.Save(scPath, true);

            saveRecent(path);
        }
	};

    saveButton->onPress(saveProject);

    codeEditor->onEscapeKeyPress([&]()
    {
        codeEditor->setVisible(false);
        codeEditor->setEnabled(false);
    });

    engine.EventLoop([&](sf::Event& event)
    {
        bool focusCodeArea = false;
        if(codeArea->isFocused())
        {
            if(Shortcut({ sf::Keyboard::Key::Tab }, 0.1))
            {
                auto pos = codeArea->getCaretPosition();
                auto text = codeArea->getText().toStdString();
                text.insert(pos, "    ");
                codeArea->setText(text);
                codeArea->setCaretPosition(pos + 4);
                focusCodeArea = true;
            }
        }
    	editor.handleEvent(event);
        if(focusCodeArea)
            codeArea->setFocused(true);
        if(event.type == sf::Event::Resized)
        {
            auto p = editor.get<tgui::ChildWindow>("objectEditorWindow");
            auto s = editor.get<tgui::ChildWindow>("sceneTreeWindow");
            auto m = editor.get<tgui::ChildWindow>("messagesWindow");
            auto c = editor.get<tgui::ChildWindow>("createWindow");

            p->setPosition({ event.size.width - p->getSize().x, event.size.height - p->getSize().y });

            s->setPosition({ event.size.width - s->getSize().x, 0 });
            s->setSize({ 270, p->getPosition().y });

            m->setPosition({ m->getPosition().x, event.size.height - m->getSize().y });
            m->setSize({ p->getPosition().x - m->getPosition().x, m->getSize().y });

            c->setSize({ c->getSize().x, event.size.height });

            viewportWindow->setSize({ p->getPosition().x - c->getSize().x, event.size.height - m->getSize().y });
            editor.get<tgui::ChildWindow>("codeEditor")->setSize(viewportWindow->getSize());

            engine.SetGuiView({ 0, 0, viewportWindow->getSize().x * 1.65f, viewportWindow->getSize().y * 1.65f });
            engine.SetGuiViewport({ 221, 26, viewportWindow->getSize().x - 1, viewportWindow->getSize().y - 28 });

            cam.SetViewportSize({ (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y });
            cam.SetGuiSize({ (uint32_t)viewport->getSize().x, (uint32_t)viewport->getSize().y });

            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::GBuffer)->Resize(viewport->getSize().x, viewport->getSize().y);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::DecalsGBuffer)->Resize(viewport->getSize().x, viewport->getSize().y);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Main)->Resize(viewport->getSize().x, viewport->getSize().y);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::Transparency)->Resize(viewport->getSize().x, viewport->getSize().y);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::SSAO)->Resize(viewport->getSize().x / 2.0, viewport->getSize().y / 2.0);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::SSR)->Resize(viewport->getSize().x, viewport->getSize().y);
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong0)->Resize(viewport->getSize().x / properties["renderer"]["bloomResolutionScale"].asFloat(), viewport->getSize().y / properties["renderer"]["bloomResolutionScale"].asFloat());
            Renderer::GetInstance()->GetFramebuffer(Renderer::FramebufferType::BloomPingPong1)->Resize(viewport->getSize().x / properties["renderer"]["bloomResolutionScale"].asFloat(), viewport->getSize().y / properties["renderer"]["bloomResolutionScale"].asFloat());
        }
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

        disableShortcuts = !engine.GetWindow().hasFocus();

    	sceneTree->setEnabled(!openFileDialog && !colorPicker);
    	openFileButton->setEnabled(!openFileDialog);
        
        auto findNode = [&](std::string name, std::vector<std::string> arr) -> bool
        {
            return std::find(arr.begin(), arr.end(), name) != arr.end();
        };

		if(sceneTree->getSelectedItem().size() > 2)
		{
			////////////// OBJECTS //////////////
			if(findNode(sceneTree->getSelectedItem().back().toStdString(), scene.GetNames()[0]) && sceneTree->getSelectedItem()[1] != "Animations")
			{
                std::shared_ptr<Model> object;
                
                object = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());

                if(!object)
                {
                    sceneTree->removeItem(sceneTree->getSelectedItem(), false);
                    sceneTree->selectItem({ "Scene", "Objects" });
                }
                else
                {
                    std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == objectEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
                    objectEditorGroup->setEnabled(true);
                    objectEditorGroup->setVisible(true);

                    if((!nameEdit->isFocused() && nameEdit->getText() != sceneTree->getSelectedItem().back()) || objectMode)
                    {
                        rp3d::Vector3 position = object->GetPosition();
                        rp3d::Quaternion orientation = object->GetOrientation();
                        rp3d::Vector3 size = object->GetSize();

                        if(!nameEdit->isFocused() && nameEdit->getText() != sceneTree->getSelectedItem().back())
                        {
                            materialBox->deselectItem();
                            materialsList->removeAllItems();
                            auto mtl = object->GetMaterial();
                            for(int i = 0; i < mtl.size(); i++)
                                materialsList->addItem(scene.GetMaterialName(mtl[i]), tgui::String(i));
                            bodyTypeBox->setSelectedItemByIndex((int)object->GetRigidBody()->getType());
                            isDrawableBox->setChecked(object->IsDrawable());
                            immLoadBox->setChecked(object->IsLoadingImmediatelly());
                        }

                        nameEdit->setText(sceneTree->getSelectedItem().back());
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

                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && nameEdit->isFocused() && nameEdit->getText() != sceneTree->getSelectedItem().back())
                    {
                        scene.SetModelName(sceneTree->getSelectedItem().back().toStdString(), nameEdit->getText().toStdString());
                        readSceneTree();
                    
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

                    if(!objectMode)
                    {
                        object->SetPosition(pos);
                        object->SetOrientation(rp3d::Quaternion::fromEulerAngles(euler));
                        object->SetSize(size);
                    }

                    if(!materialBox->getSelectedItem().empty())
                    {
                        if(materialBox->getSelectedItem() != materialsList->getSelectedItem())
                        {
                            object->GetMaterial()[materialsList->getSelectedItemId().toInt()] = scene.GetMaterial(materialBox->getSelectedItem().toStdString()).get();
                            materialsList->changeItemById(materialsList->getSelectedItemId(), materialBox->getSelectedItem());
                        }
                    }
                    if((int)object->GetRigidBody()->getType() != bodyTypeBox->getSelectedItemIndex())
                        object->GetRigidBody()->setType(rp3d::BodyType(bodyTypeBox->getSelectedItemIndex()));
                    object->SetIsDrawable(isDrawableBox->isChecked());
                    object->SetIsLoadingImmediatelly(immLoadBox->isChecked());

                    if(!materialsList->getSelectedItem().empty())
                        materialBox->setSelectedItem(materialsList->getSelectedItem());
                    else materialBox->deselectItem();
                }
		    }
		    /////////////////////////////////////

            /////////////// LIGHTS //////////////
            else if(findNode(sceneTree->getSelectedItem().back().toStdString(), scene.GetNames()[2]) && sceneTree->getSelectedItem()[1] != "Animations")
		    {
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == lightEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
                lightEditorGroup->setEnabled(true);
				lightEditorGroup->setVisible(true);

                auto light = scene.GetLight(sceneTree->getSelectedItem().back().toStdString());
                if(!light)
                {
                    sceneTree->removeItem(sceneTree->getSelectedItem(), false);
                    sceneTree->selectItem({ "Scene", "Objects" });
                }
                if(light)
                {
                    if((!lightNameEdit->isFocused() && lightNameEdit->getText() != sceneTree->getSelectedItem().back()) || objectMode)
                    {
                        rp3d::Vector3 position = light->GetPosition();
                        rp3d::Quaternion orientation = light->GetOrientation();
                        rp3d::Vector3 color = light->GetColor();
                        rp3d::Vector3 attenuation = light->GetAttenuation();
                        float innerCutoff = light->GetCutoff();
                        float outerCutoff = light->GetOuterCutoff();

                        if(!lightNameEdit->isFocused() && lightNameEdit->getText() != sceneTree->getSelectedItem().back())
                        {
                            rEdit->setText(tgui::String(color.x));
                            gEdit->setText(tgui::String(color.y));
                            bEdit->setText(tgui::String(color.z));

                            constAttEdit->setText(tgui::String(attenuation.x));
                            linAttEdit->setText(tgui::String(attenuation.y));
                            quadAttEdit->setText(tgui::String(attenuation.z));

                            innerCutoffEdit->setText(tgui::String(innerCutoff));
                            outerCutoffEdit->setText(tgui::String(outerCutoff));

                            castShadowsBox->setChecked(light->IsCastingShadows());
                            perspectiveShadowsBox->setChecked(light->IsCastingPerspectiveShadows());
                        }

                        lightNameEdit->setText(sceneTree->getSelectedItem().back());

                        lposEditX->setText(tgui::String(position.x));
                        lposEditY->setText(tgui::String(position.y));
                        lposEditZ->setText(tgui::String(position.z));

                        glm::vec3 euler = glm::eulerAngles(toglm(orientation));

                        lrotEditX->setText(tgui::String(glm::degrees(euler.x)));
                        lrotEditY->setText(tgui::String(glm::degrees(euler.y)));
                        lrotEditZ->setText(tgui::String(glm::degrees(euler.z)));
                    }

                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && lightNameEdit->isFocused() && lightNameEdit->getText() != sceneTree->getSelectedItem().back())
                    {
                        scene.SetLightName(sceneTree->getSelectedItem().back().toStdString(), lightNameEdit->getText().toStdString());
                        readSceneTree();
                    
                        lightNameEdit->setFocused(false);
                    }

                    rp3d::Vector3 pos;
                    pos.x = lposEditX->getText().toFloat();
                    pos.y = lposEditY->getText().toFloat();
                    pos.z = lposEditZ->getText().toFloat();

                    rp3d::Vector3 euler;
                    euler.x = glm::radians(lrotEditX->getText().toFloat());
                    euler.y = glm::radians(lrotEditY->getText().toFloat());
                    euler.z = glm::radians(lrotEditZ->getText().toFloat());

                    rp3d::Vector3 color;
                    color.x = rEdit->getText().toInt();
                    color.y = gEdit->getText().toInt();
                    color.z = bEdit->getText().toInt();

                    rp3d::Vector3 att;
                    att.x = constAttEdit->getText().toFloat();
                    att.y = linAttEdit->getText().toFloat();
                    att.z = quadAttEdit->getText().toFloat();

                    if(!objectMode)
                    {
                        light->SetPosition(pos);
                        light->SetOrientation(rp3d::Quaternion::fromEulerAngles(euler));
                    }
                    
                    light->SetColor(color);
                    light->SetAttenuation(att.x, att.y, att.z);
                    light->SetCutoff(innerCutoffEdit->getText().toFloat());
                    light->SetOuterCutoff(outerCutoffEdit->getText().toFloat());
                    light->SetIsCastingShadows(castShadowsBox->isChecked());
                    light->SetIsCastingPerspectiveShadows(perspectiveShadowsBox->isChecked());
                }
            }
            /////////////////////////////////////

            /////////////// BONES ///////////////
            else if(findNode(sceneTree->getSelectedItem().back().toStdString(), scene.GetNames()[4]) && sceneTree->getSelectedItem()[1] != "Animations")
            {
                auto bone = scene.GetBone(sceneTree->getSelectedItem().back().toStdString());
			
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == boneEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
				boneEditorGroup->setEnabled(true);
				boneEditorGroup->setVisible(true);

				if((!boneNameEdit->isFocused() && boneNameEdit->getText() != sceneTree->getSelectedItem().back()) || objectMode)
				{
					rp3d::Vector3 position = bone->GetTransform().getPosition();
					rp3d::Quaternion orientation = bone->GetTransform().getOrientation();
					rp3d::Vector3 size = bone->GetSize();

					boneNameEdit->setText(sceneTree->getSelectedItem().back());
					bposEditX->setText(tgui::String(position.x));
				    bposEditY->setText(tgui::String(position.y));
				    bposEditZ->setText(tgui::String(position.z));

					glm::vec3 euler = glm::eulerAngles(toglm(orientation));

				    brotEditX->setText(tgui::String(glm::degrees(euler.x)));
	  			    brotEditY->setText(tgui::String(glm::degrees(euler.y)));
	  			    brotEditZ->setText(tgui::String(glm::degrees(euler.z)));

					bsizeEditX->setText(tgui::String(size.x));
				    bsizeEditY->setText(tgui::String(size.y));
				    bsizeEditZ->setText(tgui::String(size.z));
			    }

			    rp3d::Vector3 pos;
			    pos.x = bposEditX->getText().toFloat();
			    pos.y = bposEditY->getText().toFloat();
			    pos.z = bposEditZ->getText().toFloat();

			    rp3d::Vector3 euler;
			    euler.x = glm::radians(brotEditX->getText().toFloat());
			    euler.y = glm::radians(brotEditY->getText().toFloat());
			    euler.z = glm::radians(brotEditZ->getText().toFloat());

			    rp3d::Vector3 size;
			    size.x = bsizeEditX->getText().toFloat();
			    size.y = bsizeEditY->getText().toFloat();
			    size.z = bsizeEditZ->getText().toFloat();

                if(!objectMode)
                {
                    bone->SetPosition(pos);
                    bone->SetOrientation(rp3d::Quaternion::fromEulerAngles(euler));
                    bone->SetSize(size);
                }
            }
            /////////////////////////////////////

		    ///////////// MATERIALS /////////////
		    else if(sceneTree->getSelectedItem()[1] == "Materials")
		    {
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == materialEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
				materialEditorGroup->setEnabled(true);
				materialEditorGroup->setVisible(true);

				auto material = scene.GetMaterial(sceneTree->getSelectedItem().back().toStdString());
				if(materialNameEdit->getText() != sceneTree->getSelectedItem().back() && !materialNameEdit->isFocused())
				{
					materialNameEdit->setText(sceneTree->getSelectedItem().back());

					std::optional<glm::vec3> color;
					float metal = -1.0;
					float rough = -1.0;
					float opacity = -1.0;
                    float emissionStrength = 1.0;

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
                        case Material::Type::EmissionStrength:
                            emissionStrength = std::get<0>(i.first).x;
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
                    emissionStrengthEdit->setText(tgui::String(emissionStrength));
			    }

			    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && materialNameEdit->isFocused() && materialNameEdit->getText() != sceneTree->getSelectedItem().back())
			    {
		    		scene.SetMaterialName(sceneTree->getSelectedItem().back().toStdString(), materialNameEdit->getText().toStdString());
		    		sceneTree->removeItem(sceneTree->getSelectedItem(), false);
		    		sceneTree->addItem({ "Scene", "Materials", materialNameEdit->getText() });
		    		sceneTree->selectItem({ "Scene", "Materials", materialNameEdit->getText() });
				    materialNameEdit->setFocused(false);
				    materialBox->removeAllItems();
				    auto names = scene.GetNames();
                    for(auto& i : names[1]) materialBox->addItem(i);
			    }

			    std::optional<glm::vec3> color, ctmp, emission, etmp;
			    float metal, rough, opacity, emissionStrength;

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
                emissionStrength = emissionStrengthEdit->getText().toFloat();

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
                    case Material::Type::EmissionStrength:
                        i.first = glm::vec3(emissionStrength);
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

            ///////////// ANIMATIONS ////////////
            else if(sceneTree->getSelectedItem()[1] == "Animations" && sceneTree->getSelectedItem().size() >= 3)
            {
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == animationEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
                animationEditorGroup->setEnabled(true);
                animationEditorGroup->setVisible(true);

                auto anim = scene.GetAnimation(sceneTree->getSelectedItem()[2].toStdString());
                lastAnimation.ptr = anim;
                lastAnimation.name = sceneTree->getSelectedItem()[2].toStdString();
                if(sceneTree->getSelectedItem().size() > 3)
                {
                    addActionButton->setVisible(true);
                    addActionButton->setEnabled(true);
                    deleteAnimOrKFButton->setText("Delete keyframe");
                }
                else
                {
                    addActionButton->setVisible(false);
                    addActionButton->setEnabled(false);
                    deleteAnimOrKFButton->setText("Delete animation");
                }

                if((!animNameEdit->isFocused() && animNameEdit->getText() != sceneTree->getSelectedItem()[2]) || objectMode)
                {
                    animNameEdit->setText(sceneTree->getSelectedItem()[2]);

                    durationEdit->setText(tgui::String(anim->GetDuration()));
                    tpsEdit->setText(tgui::String(anim->GetTPS()));
                    repeatBox->setChecked(anim->IsRepeated());
                }

                timelineSlider->setMaximum(anim->GetDuration());

                if(anim->GetState() == Animation::State::Playing)
                    timelineSlider->setValue(anim->GetTime());
                else anim->SetLastTime(timelineSlider->getValue());

                lastAnimation.time = timelineSlider->getValue();

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && animNameEdit->isFocused() && animNameEdit->getText() != sceneTree->getSelectedItem().back())
                {
                    scene.SetAnimationName(sceneTree->getSelectedItem()[2].toStdString(), animNameEdit->getText().toStdString());
                    readSceneTree();
                    sceneTree->selectItem({ "Scene", "Animations", animNameEdit->getText() });
                    animNameEdit->setFocused(false);
                }

                anim->SetDuration(durationEdit->getText().toFloat());
                anim->SetTPS(tpsEdit->getText().toFloat());
                anim->SetIsRepeated(repeatBox->isChecked());
                timeEdit->setText(tgui::String(timelineSlider->getValue()));
            }
            /////////////////////////////////////

            ////////////// SOUNDS ///////////////
            else if(sceneTree->getSelectedItem()[1] == "Sounds")
            {
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == soundEditorGroup) return; g->setEnabled(false); g->setVisible(false); });
                soundEditorGroup->setEnabled(true);
                soundEditorGroup->setVisible(true);

                auto sound = sceneTree->getSelectedItem().back().toStdString();

                if((!soundNameEdit->isFocused() && soundNameEdit->getText() != sceneTree->getSelectedItem().back()) || objectMode)
				{
					rp3d::Vector3 position = sman->GetPosition(sound);
                    float attenuation = sman->GetAttenuation(sound);
                    float minDistance = sman->GetMinDistance(sound);
                    float volume = sman->GetVolume(sound);

					soundNameEdit->setText(sceneTree->getSelectedItem().back());

					sposEditX->setText(tgui::String(position.x));
				    sposEditY->setText(tgui::String(position.y));
				    sposEditZ->setText(tgui::String(position.z));

                    volumeSlider->setValue(volume);

                    attenuationEdit->setText(tgui::String(attenuation));
	  			    minDistEdit->setText(tgui::String(minDistance));
                    
                    loopBox->setChecked(sman->GetLoop(sound));
			    }

			    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && soundNameEdit->isFocused() && soundNameEdit->getText() != sceneTree->getSelectedItem().back())
			    {
                    sman->SetName(sceneTree->getSelectedItem().back().toStdString(), soundNameEdit->getText().toStdString());
                    sceneTree->removeItem(sceneTree->getSelectedItem(), false);
                    sceneTree->addItem({ "Scene", "Sounds", soundNameEdit->getText() });
                    sceneTree->selectItem({ "Scene", "Sounds", soundNameEdit->getText() });
                
				    soundNameEdit->setFocused(false);
			    }

			    rp3d::Vector3 pos;
			    pos.x = sposEditX->getText().toFloat();
			    pos.y = sposEditY->getText().toFloat();
			    pos.z = sposEditZ->getText().toFloat();

                float attenuation = attenuationEdit->getText().toFloat();
			    float minDist = minDistEdit->getText().toFloat();
                float volume = volumeSlider->getValue();

                if(!objectMode)
                {
                    sman->SetPosition(pos, sound);
                    sman->SetVolume(volume, sound);
                    sman->SetAttenuation(attenuation, sound);
                    sman->SetMinDistance(minDist, sound);
                    sman->SetLoop(loopBox->isChecked(), sound);
                }
            }
            /////////////////////////////////////
            else
            {
                nameEdit->setText("");
                std::for_each(groups.begin(), groups.end(), [&](auto g) { g->setEnabled(false); g->setVisible(false); });
            }
		}
        ////////////// SCRIPTS //////////////
        else if(!sceneTree->getSelectedItem().empty())
        {
            if(sceneTree->getSelectedItem().size() > 1 &&
               sceneTree->getSelectedItem()[0] == "Scripts")
            {
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == scriptsGroup) return; g->setEnabled(false); g->setVisible(false); });
                scriptsGroup->setEnabled(true);
                scriptsGroup->setVisible(true);
            }
            /////////////////////////////////////
            else if(sceneTree->getSelectedItem()[0] == "Scene")
            {
                nameEdit->setText("");
                std::for_each(groups.begin(), groups.end(), [&](auto g) { if(g == sceneGroup) return; g->setEnabled(false); g->setVisible(false); });
                sceneGroup->setEnabled(true);
                sceneGroup->setVisible(true);
            }
        }
        else
        {
            nameEdit->setText("");
            std::for_each(groups.begin(), groups.end(), [&](auto g) { g->setEnabled(false); g->setVisible(false); });
        }

		if(scriptLaunched)
			scman.ExecuteFunction(loopDecl);

		cam.Update();
		if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && engine.GetWindow().hasFocus())
		{
			engine.GetWindow().setMouseCursorVisible(false);
			engine.GetWindow().setMouseCursorGrabbed(true);

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
                cam.SetSpeed(2.0);
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
                cam.SetSpeed(0.5);
            else cam.SetSpeed(1.0);

            modeLabel->setText("Camera movement");

	        if(manageCameraMovement) cam.Move(1);
	        if(manageCameraMouse) cam.Mouse(mouseSensitivity);
        }
        else
        {
            if(!objectMode) modeLabel->setText("");
            else modeLabel->setText(std::string("Object mode") + (param == 0 ? " (Move)" : (param == 1 ? " (Rotate)" : " (Scale)")));

        	engine.GetWindow().setMouseCursorVisible(true);
        	engine.GetWindow().setMouseCursorGrabbed(false);

        	if(Shortcut({ sf::Keyboard::Key::LControl, sf::Keyboard::Key::S }))
            {
                if(!codeEditor->isFocused())
        		    saveProject();
                else
                    saveFile();
            }
        }
        if(manageCameraLook) cam.Look();

        if(Shortcut({ sf::Keyboard::Key::LControl, sf::Keyboard::Key::G }))
            objectMode = !objectMode;

        if(objectMode)
        {
            if(Shortcut({ sf::Keyboard::Key::LAlt, sf::Keyboard::Key::M })) param = 0;
            if(Shortcut({ sf::Keyboard::Key::LAlt, sf::Keyboard::Key::R })) param = 1;
            if(Shortcut({ sf::Keyboard::Key::LAlt, sf::Keyboard::Key::S })) param = 2;

            if(sceneTree->getSelectedItem().size() > 2)
            {
                if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
                   sceneTree->getSelectedItem()[1] == "Objects" && !nameEdit->isFocused())
                {
                    auto vpPos = viewport->getAbsolutePosition();
                    auto mPos = sf::Mouse::getPosition(engine.GetWindow());
                    auto mouse = cam.ScreenPositionToWorld(false, rp3d::Vector2(float(mPos.x) - vpPos.x, float(mPos.y) - vpPos.y));
                    auto selectedNode = scene.GetNode(sceneTree->getSelectedItem().back().toStdString());
                    auto selectedNodeTransform = selectedNode->GetTransform();
                    auto camVec = cam.GetPosition() - selectedNodeTransform.getPosition();
                    auto camDist = camVec.length();

                    rp3d::Vector3 delta;

                    rp3d::Ray ray(cam.GetPosition(true), cam.GetPosition(true) + mouse * 10000);
                    rp3d::RaycastInfo info;

                    if((gizmoX->GetRigidBody()->raycast(ray, info) && axis == -1) || axis == 0)
                    {
                        axis = 0;
                        delta.x = -float(prevMPos.x - mPos.x) * (camDist / 300) * (camVec.z > 0 ? 1 : -1);
                    }
                    if((gizmoY->GetRigidBody()->raycast(ray, info) && axis == -1) || axis == 1)
                    {
                        axis = 1;
                        delta.y = float(prevMPos.y - mPos.y) * (camDist / 300);
                    }
                    if((gizmoZ->GetRigidBody()->raycast(ray, info) && axis == -1) || axis == 2)
                    {
                        axis = 2;
                        delta.z = float(prevMPos.x - mPos.x) * (camDist / 300) * (camVec.x > 0 ? 1 : -1);   
                    }

                    switch(param)
                    {
                    case 0:
                        selectedNode->SetTransform({ selectedNodeTransform.getPosition() + delta, selectedNodeTransform.getOrientation() }); break;
                    case 1:
                        selectedNode->SetTransform({ selectedNodeTransform.getPosition(), selectedNodeTransform.getOrientation() * rp3d::Quaternion::fromEulerAngles(delta / 10) }); break;
                    case 2:
                        selectedNode->SetSize(selectedNode->GetSize() + (delta / 5)); break;
                    }

                    prevMPos = mPos;
                }
                else
                {
                    axis = -1;
                    prevMPos = sf::Mouse::getPosition(engine.GetWindow());
                }
            }
        }

        if(lastAnimation.ptr && sceneTree->getSelectedItem().size() > 2)
        {
            if(sceneTree->getSelectedItem()[1] == "Objects" && !nameEdit->isFocused())
            {
                if(Shortcut({ sf::Keyboard::Key::K }))
                    addKf(sceneTree->getSelectedItem().back().toStdString());
                if(Shortcut({ sf::Keyboard::Key::I }))
                    addAction(sceneTree->getSelectedItem().back().toStdString());
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Period) || 
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Comma))
                {
                    float timeChangeSpeed = 0.1 + (0.4 * sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift));
                    if(Shortcut({ sf::Keyboard::Key::Period }, 0.1))
                        lastAnimation.time += timeChangeSpeed;
                    if(Shortcut({ sf::Keyboard::Key::Comma }, 0.1))
                        lastAnimation.time -= timeChangeSpeed;
                    
                    if(lastAnimation.time < 0) lastAnimation.time = 0;
                    if(lastAnimation.time > lastAnimation.ptr->GetDuration()) lastAnimation.time = lastAnimation.ptr->GetDuration();

                    modeLabel->setText("Animation: " + lastAnimation.name + "; time: " + std::to_string(lastAnimation.time));
                }
            }
        }

        if(viewport->isFocused() || sceneTree->isFocused())
        {
            if(Shortcut({ sf::Keyboard::Key::LControl, sf::Keyboard::Key::C }))
            {
                if(findNode(sceneTree->getSelectedItem().back().toStdString(), scene.GetNames()[0]))
                {
                    buffer.name = sceneTree->getSelectedItem().back().toStdString();
                    buffer.object = scene.GetModel(sceneTree->getSelectedItem().back().toStdString());
                }
            }

            if(Shortcut({ sf::Keyboard::Key::LControl, sf::Keyboard::Key::V }))
            {
                if(buffer.object)
                {
                    auto a = scene.CloneModel(buffer.object.get(), false, buffer.name + "-copy");
                    std::string name = scene.GetLastAdded();
                    readSceneTree();
                }
            }
        }

        gizmoX->SetIsDrawable(false);
        gizmoY->SetIsDrawable(false);
        gizmoZ->SetIsDrawable(false);

        if(sceneTree->getSelectedItem().size() > 2 && objectMode)
        {
            if(sceneTree->getSelectedItem()[1] == "Objects" && !nameEdit->isFocused())
            {
                auto selectedNode = scene.GetNode(sceneTree->getSelectedItem().back().toStdString());
                auto selectedNodeTransform = Node::GetFinalTransform(selectedNode, selectedNode->GetTransform());
                auto camDist = (cam.GetPosition() - selectedNodeTransform.getPosition()).length();

                gizmoX->SetPosition(selectedNodeTransform.getPosition() + rp3d::Vector3(1.1 * camDist / 10.0, 0, 0));
                gizmoY->SetPosition(selectedNodeTransform.getPosition() + rp3d::Vector3(0, 1.1 * camDist / 10.0, 0));
                gizmoZ->SetPosition(selectedNodeTransform.getPosition() + rp3d::Vector3(0, 0, 1.1 * camDist / 10.0));

                gizmoX->SetSize(rp3d::Vector3(1.0, 0.1, 0.1) * camDist / 10.0);
                gizmoY->SetSize(rp3d::Vector3(0.1, 1.0, 0.1) * camDist / 10.0);
                gizmoZ->SetSize(rp3d::Vector3(0.1, 0.1, 1.0) * camDist / 10.0);

                switch(axis)
                {
                case -1:
                case 0: gizmoX->SetIsDrawable(true); if(axis != -1) break;
                case 1: gizmoY->SetIsDrawable(true); if(axis != -1) break;
                case 2: gizmoZ->SetIsDrawable(true); if(axis != -1) break;
                }
            }
        }

		ListenerWrapper::SetPosition(cam.GetPosition());
		ListenerWrapper::SetOrientation(cam.GetOrientation());

        Renderer::GetInstance()->SetExposure(exposure);
        Renderer::GetInstance()->SetBloomStrength(bloomStrength);
        Renderer::GetInstance()->SetBlurIterations(blurIterations);
        Renderer::GetInstance()->SetSSAOStrength(ssaoStrength);
        Renderer::GetInstance()->SetSSAORadius(ssaoRadius);
        Renderer::GetInstance()->SetDOFMinDistance(dofMinDistance);
        Renderer::GetInstance()->SetDOFMaxDistance(dofMaxDistance);
        Renderer::GetInstance()->SetDOFFocusDistance(dofFocusDistance);
        Renderer::GetInstance()->SetFogStart(fogStart);
        Renderer::GetInstance()->SetFogEnd(fogEnd);
        Renderer::GetInstance()->SetFogHeight(fogHeight);

		if(engine.GetWindow().hasFocus())
            engine.GetWindow().setFramerateLimit(60);
        else engine.GetWindow().setFramerateLimit(5);

        if(manageSceneRendering) scene.Draw(nullptr, nullptr, updateShadows ? shadows.Update() : true);

        gizmosFb->Bind();
        auto size = gizmosFb->GetSize();
        glViewport(0, 0, size.x, size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gizmoX->Draw(&cam, {});
        gizmoY->Draw(&cam, {});
        gizmoZ->Draw(&cam, {});

		viewport->bindFramebuffer();
        Renderer::GetInstance()->DrawFramebuffers();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->Bind();
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("rawColor", true);
        Renderer::GetInstance()->GetShader(Renderer::ShaderType::Post)->SetUniform1i("transparentBuffer", false);
        gizmosFb->Draw();
        Framebuffer::Unbind();

        editor.draw();
    });
    
    progressBar->setValue(100);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    loading.draw();
    engine.GetWindow().display();

    engine.Launch(false);

    std::ofstream out(homeFolder + "/properties.json");
    out << properties.toStyledString();
    out.close();
}
