#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Skybox.h>
#include <Model.h>
#include <Animation.h>
#include <Shape.h>
#include "AllGui.h"
#include <filesystem>
#include <memory>

gui::Gui modelDialog(SetupModelDialog());
gui::Gui animationDialog(SetupAnimationDialog());
gui::Gui lightDialog(SetupLightDialog());
gui::Gui modelEditDialog(SetupModelEditDialog());
gui::Gui animationEditDialog(SetupAnimationEditDialog());
gui::Gui lightEditDialog(SetupLightEditDialog());
gui::Gui saveLoadDialog(SetupSaveLoadDialog());

std::vector<std::shared_ptr<Model>> models;
std::vector<std::shared_ptr<Animation>> animations;
std::vector<std::shared_ptr<Light>> lights;

GLenum lightnumber;
GLuint skybox[6];
std::string filename, texture, id, templatefile;

int selected = -1, selectedAnim = -1, selectedLight = -1;
int frames = 0, lightNum = 0;
int choice = 0;

float speed = 0;
float x, y, z, rotx, roty, rotz, sizex, sizey, sizez, temp, m_move = 0.1, m_rotate = 0.3, m_resize = 0.01;

bool right = false;
bool move = false, resize = false, rotate = false;
bool modelmove = false, animmove = false, lightmove = false;
bool showcursor = true;
bool modeldialog = false, animationdialog = false, lightdialog = false, modeleditdialog = false, animationeditdialog = false, lighteditdialog = false, savedialog = false, loaddialog = false, exportdialog = false;

Shape lightShape(0.3, 0.3, 0.3, 0, 0, 0);

Camera cam(0, 0, 0, 1);

void SaveProject(std::string filename)
{
	std::ofstream output(filename);
	output << models.size() << std::endl;
	for(int i = 0; i < models.size(); i++)
	{
		output << models[i]->GetID() << " " << models[i]->GetFilename() << " " << (models[i]->GetTextureFilename().empty() ? "notex" : models[i]->GetTextureFilename()) << " " << models[i]->GetPosition().x << " " << models[i]->GetPosition().y << " " << models[i]->GetPosition().z << " " << models[i]->GetRotation().x << " " << models[i]->GetRotation().y << " " << models[i]->GetRotation().z << " " << models[i]->GetSize().x << " " << models[i]->GetSize().y << " " << models[i]->GetSize().z << std::endl;
	}
	output << animations.size() << std::endl;
	for(int i = 0; i < animations.size(); i++)
	{
		output << animations[i]->GetID() << " " << animations[i]->GetFilename() << " " << (animations[i]->GetTextureFilename().empty() ? "notex" : animations[i]->GetTextureFilename()) << " " << animations[i]->GetFrames() << " " << animations[i]->GetSpeed() << " " << animations[i]->GetPosition().x << " " << animations[i]->GetPosition().y << " " << animations[i]->GetPosition().z << " " << animations[i]->GetRotation().x << " " << animations[i]->GetRotation().y << " " << animations[i]->GetRotation().z << " " << animations[i]->GetSize().x << " " << animations[i]->GetSize().y << " " << animations[i]->GetSize().z << std::endl;
	}
	output << lights.size() << std::endl;
	for(int i = 0; i < lights.size(); i++)
	{
		float la[1] = { 0 };
		std::vector<float> ads = { 0, 0, 0, 1 };
		output << lights[i]->GetID() << " " << std::hex << lights[i]->GetLightNum() << " " << lights[i]->GetPosition().x << " " << lights[i]->GetPosition().y << " " << lights[i]->GetPosition().z << " ";
		la[0] = lights[i]->GetParameters(GL_LINEAR_ATTENUATION)[0];
		output << la[0] << " ";
		ads = lights[i]->GetParameters(GL_AMBIENT);
		output << ads[0] << " " << ads[1] << " " << ads[2] << " ";
		ads = lights[i]->GetParameters(GL_DIFFUSE);
		output << ads[0] << " " << ads[1] << " " << ads[2] << " ";
		ads = lights[i]->GetParameters(GL_SPECULAR);
		output << ads[0] << " " << ads[1] << " " << ads[2] << std::endl;
	}
	output.close();
}

bool LoadProject(std::string filename)
{
	std::ifstream input(filename);
	if(input.is_open())
	{
		int it = 0;
		input >> it;
		for(int i = 0; i < it; i++)
		{
			input >> id >> filename >> texture >> x >> y >> z >> rotx >> roty >> rotz >> sizex >> sizey >> sizez;
			if(texture != "notex") models.emplace_back(std::make_shared<Model>(filename, texture, id, x, y, z, rotx, roty, rotz, sizex, sizey, sizez));
			else models.emplace_back(std::make_shared<Model>(filename, GLuint(0), id, x, y, z, rotx, roty, rotz, sizex, sizey, sizez));
		}
		input >> it;
		for(int i = 0; i < it; i++)
		{
			input >> id >> filename >> texture >> frames >> speed >> x >> y >> z >> rotx >> roty >> rotz >> sizex >> sizey >> sizez;
			if(texture != "notex") animations.emplace_back(std::make_shared<Animation>(filename, texture, frames, speed, id, x, y, z, 0, 0, 0, 1, 1, 1));
			else animations.emplace_back(std::make_shared<Animation>(filename, GLuint(0), frames, speed, id, x, y, z, 0, 0, 0, 1, 1, 1));
		}
		input >> it;
		for(int i = 0; i < it; i++)
		{
			float la[1] = { 0 };
			std::vector<float> ads = { 0, 0, 0, 1 };
			input >> id >> std::hex >> lightNum >> x >> y >> z;
			lights.emplace_back(std::make_shared<Light>(lightNum, x, y, z, id));
			input >> la[0];
			lights[i]->SetParameters({ la[0] }, GL_LINEAR_ATTENUATION);
			input >> ads[0] >> ads[1] >> ads[2];
			lights[i]->SetParameters(ads, GL_AMBIENT);
			input >> ads[0] >> ads[1] >> ads[2];
			lights[i]->SetParameters(ads, GL_DIFFUSE);
			input >> ads[0] >> ads[1] >> ads[2];
			lights[i]->SetParameters(ads, GL_SPECULAR);
		}
		input.close();
		return true;
	}
	return false;
}

void ReadConfig(bool loaddefproj = true)
{
	std::ifstream input("editor.cfg");
	std::string param, val;
	if(input.is_open())
	{
		while(input >> param >> val)
		{
			if(param == "template_file" && val != "NULL") templatefile = val;
			if(param == "default_project" && val != "NULL" && loaddefproj)
			{ 
				LoadProject(val); 
				saveLoadDialog.SetEnteredText(pnamebtb.ID, val);
			}
			if(param == "m_move" && val != "NULL") m_move = stof(val);
			if(param == "m_rotate" && val != "NULL") m_rotate = stof(val);
			if(param == "m_resize" && val != "NULL") m_resize = stof(val);
			if(param == "cam_speed" && val != "NULL") cam.speed = stof(val);
		}
	}
}

bool ExportProject(std::string filename)
{
	if(!templatefile.empty())
	{
		std::ofstream output(filename);
		std::ifstream input(templatefile);
		std::string code;
		
		std::istreambuf_iterator<char> inputit(input), emptyit;
		std::back_insert_iterator<std::string> strin(code);
		std::copy(inputit, emptyit, strin);
		
		size_t modelspos = code.find("//Models");
		if(modelspos != std::string::npos)
		{
			code.erase(modelspos, std::strlen("//Models"));
			for(int i = 0; i < models.size(); i++)
			{
				std::string mdlstr("Model " + models[i]->GetID() + "(\"" + models[i]->GetFilename() + "\", " + (models[i]->GetTextureFilename().empty() ? "GLuint(0)" : "\"" + models[i]->GetTextureFilename() + "\"") + ", " + std::to_string(models[i]->GetPosition().x) + ", " + std::to_string(models[i]->GetPosition().y) + ", " + std::to_string(models[i]->GetPosition().z)
				+ ", " + std::to_string(models[i]->GetRotation().x) + ", " + std::to_string(models[i]->GetRotation().y) + ", " + std::to_string(models[i]->GetRotation().z)+ ", " + std::to_string(models[i]->GetSize().x) + ", " + std::to_string(models[i]->GetSize().y) + ", " + std::to_string(models[i]->GetSize().z) + ");\n");
				code.insert(modelspos, mdlstr);
				modelspos += mdlstr.size();
			}
		}
		size_t modelsdrawpos = code.find("//ModelsDraw");
		if(modelsdrawpos != std::string::npos)
		{
			code.erase(modelsdrawpos, std::strlen("//ModelsDraw"));
			for(int i = 0; i < models.size(); i++)
			{
				std::string mdldrwstr(models[i]->GetID() + ".Draw();\n");
				code.insert(modelsdrawpos, mdldrwstr);
				modelsdrawpos += mdldrwstr.size();
			}
		}
		size_t animationspos = code.find("//Animations");
		if(animationspos != std::string::npos)
		{
			code.erase(animationspos, std::strlen("//Animations"));
			for(int i = 0; i < animations.size(); i++)
			{
				std::string animstr("Animation " + animations[i]->GetID() + "(\"" + animations[i]->GetFilename() + "\", " + (animations[i]->GetTextureFilename().empty() ? "GLuint(0)" : "\"" + animations[i]->GetTextureFilename() + "\"") + ", " + std::to_string(animations[i]->GetFrames()) + ", " + std::to_string(animations[i]->GetSpeed()) + ", \"" + animations[i]->GetID() + "\", " + std::to_string(animations[i]->GetPosition().x) + ", " + std::to_string(animations[i]->GetPosition().y) + ", " + std::to_string(animations[i]->GetPosition().z)
				+ ", " + std::to_string(animations[i]->GetRotation().x) + ", " + std::to_string(animations[i]->GetRotation().y) + ", " + std::to_string(animations[i]->GetRotation().z)+ ", " + std::to_string(animations[i]->GetSize().x) + ", " + std::to_string(animations[i]->GetSize().y) + ", " + std::to_string(animations[i]->GetSize().z) + ");\n");
				code.insert(animationspos, animstr);
				animationspos += animstr.size();
			}
		}
		size_t animationsdrawpos = code.find("//AnimationsDraw");
		if(animationsdrawpos != std::string::npos)
		{
			code.erase(animationsdrawpos, std::strlen("//AnimationsDraw"));
			for(int i = 0; i < animations.size(); i++)
			{
				std::string animdrwstr(animations[i]->GetID() + ".DrawAnimation(time);\n");
				code.insert(animationsdrawpos, animdrwstr);
				animationsdrawpos += animdrwstr.size();
			}
		}
		size_t lightspos = code.find("//Lights");
		if(lightspos != std::string::npos)
		{
			code.erase(lightspos, std::strlen("//Lights"));
			for(int i = 0; i < lights.size(); i++)
			{
				auto paramtostr = [&](std::vector<float> vec) 
				{ 
					std::string output;
					for(auto& i : vec)
					{
						output += std::to_string(i) + ", ";
					}
					output.erase(output.end() - 2, output.end());
					return output;
				};
				
				std::stringstream ss;
				ss << std::hex << lights[i]->GetLightNum();
				
				std::string lghstr = std::string("Light " + lights[i]->GetID() + "(0x" + ss.str() + ", " + std::to_string(lights[i]->GetPosition().x) + ", " + std::to_string(lights[i]->GetPosition().y) + ", " + std::to_string(lights[i]->GetPosition().z) + ");\n\n");
				code.insert(lightspos, lghstr);
				lightspos += lghstr.size();
				
				lghstr = std::string(lights[i]->GetID() + ".SetParameters({ " + std::to_string(lights[i]->GetParameters(GL_LINEAR_ATTENUATION)[0]) + " }, GL_LINEAR_ATTENUATION);\n");
				code.insert(lightspos, lghstr);
				lightspos += lghstr.size();
				
				lghstr = std::string(lights[i]->GetID() + ".SetParameters({ " + paramtostr(lights[i]->GetParameters(GL_AMBIENT)) + " }, GL_AMBIENT);\n");
				code.insert(lightspos, lghstr);
				lightspos += lghstr.size();
				
				lghstr = std::string(lights[i]->GetID() + ".SetParameters({ " + paramtostr(lights[i]->GetParameters(GL_DIFFUSE)) + " }, GL_DIFFUSE);\n");
				code.insert(lightspos, lghstr);
				lightspos += lghstr.size();
				
				lghstr = std::string(lights[i]->GetID() + ".SetParameters({ " + paramtostr(lights[i]->GetParameters(GL_SPECULAR)) + " }, GL_SPECULAR);\n\n");
				code.insert(lightspos, lghstr);
				lightspos += lghstr.size();
			}
		}
		size_t lightsdrawpos = code.find("//LightsUpdate");
		if(lightsdrawpos != std::string::npos)
		{
			code.erase(lightsdrawpos, std::strlen("//LightsUpdate"));
			for(int i = 0; i < lights.size(); i++)
			{
				std::string lghtdrwstr(lights[i]->GetID() + ".Update();\n");
				code.insert(lightsdrawpos, lghtdrwstr);
				lightsdrawpos += lghtdrwstr.size();
			}
		}
		output << code;
		input.close();
		return true;
	}
	else return false;
} 

void GLsetup(float width, float height)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0);
	glDepthMask(GL_TRUE);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.f, (float)width / (float)height, 0.1, 5000.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);	
}

int main(int argc, char* argv[]) {
	sf::ContextSettings s;
	s.depthBits = 24;

	int width = 1280, height = 720;

	sf::RenderWindow w(sf::VideoMode(width, height), "3Dev Editor", sf::Style::Default, s);
	w.setVerticalSyncEnabled(true);
	w.setFramerateLimit(60);
	
	GLsetup(width, height);
	
	ReadConfig(argv[1] == nullptr);
	if(argv[1] != nullptr) LoadProject(argv[1]);
	
	skybox[0] = LoadTexture("resources/skybox3/skybox_front.bmp");
	skybox[1] = LoadTexture("resources/skybox3/skybox_back.bmp");
	skybox[2] = LoadTexture("resources/skybox3/skybox_left.bmp");
	skybox[3] = LoadTexture("resources/skybox3/skybox_right.bmp");
	skybox[4] = LoadTexture("resources/skybox3/skybox_bottom.bmp");
	skybox[5] = LoadTexture("resources/skybox3/skybox_top.bmp");
	
	gui::Gui gui(SetupGui());
	
	sf::Clock clock;

	while (w.isOpen()) {
		sf::Event event;
		while (w.pollEvent(event)) {			
			if (gui.CatchEvent(event, w) == modelButton.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog) modeldialog = !modeldialog;
			if (gui.CatchEvent(event, w) == animationButton.ID && !modeldialog && !lightdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog) animationdialog = !animationdialog;
			if (gui.CatchEvent(event, w) == lightButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog) lightdialog = !lightdialog;
			if (gui.CatchEvent(event, w) == editButton.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog && selected != -1) 
			{
				modelEditDialog.SetEnteredText(idbtb.ID, models[selected]->GetID());
				modelEditDialog.SetEnteredText(xbtb.ID, std::to_string(models[selected]->GetPosition().x)); modelEditDialog.SetEnteredText(ybtb.ID, std::to_string(models[selected]->GetPosition().y)); modelEditDialog.SetEnteredText(zbtb.ID, std::to_string(models[selected]->GetPosition().z));
				modelEditDialog.SetEnteredText(rxbtb.ID, std::to_string(models[selected]->GetRotation().x)); modelEditDialog.SetEnteredText(rybtb.ID, std::to_string(models[selected]->GetRotation().y)); modelEditDialog.SetEnteredText(rzbtb.ID, std::to_string(models[selected]->GetRotation().z));
				modelEditDialog.SetEnteredText(sxbtb.ID, std::to_string(models[selected]->GetSize().x)); modelEditDialog.SetEnteredText(sybtb.ID, std::to_string(models[selected]->GetSize().y)); modelEditDialog.SetEnteredText(szbtb.ID, std::to_string(models[selected]->GetSize().z));
				modeleditdialog = !modeleditdialog;
			}
			if (gui.CatchEvent(event, w) == editButton1.ID && !modeleditdialog && !animationdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog && selectedAnim != -1) 
			{
				animationEditDialog.SetEnteredText(idbtb.ID, animations[selectedAnim]->GetID());
				animationEditDialog.SetEnteredText(xbtb.ID, std::to_string(animations[selectedAnim]->GetPosition().x)); animationEditDialog.SetEnteredText(ybtb.ID, std::to_string(animations[selectedAnim]->GetPosition().y)); animationEditDialog.SetEnteredText(zbtb.ID, std::to_string(animations[selectedAnim]->GetPosition().z));
				animationEditDialog.SetEnteredText(rxbtb.ID, std::to_string(animations[selectedAnim]->GetRotation().x)); animationEditDialog.SetEnteredText(rybtb.ID, std::to_string(animations[selectedAnim]->GetRotation().y)); animationEditDialog.SetEnteredText(rzbtb.ID, std::to_string(animations[selectedAnim]->GetRotation().z));
				animationEditDialog.SetEnteredText(sxbtb.ID, std::to_string(animations[selectedAnim]->GetSize().x)); animationEditDialog.SetEnteredText(sybtb.ID, std::to_string(animations[selectedAnim]->GetSize().y)); animationEditDialog.SetEnteredText(szbtb.ID, std::to_string(animations[selectedAnim]->GetSize().z));
				animationEditDialog.SetEnteredText(speedbtb.ID, std::to_string(animations[selectedAnim]->GetSpeed()));
				animationeditdialog = !animationeditdialog;
			}
			if (gui.CatchEvent(event, w) == editButton2.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && !exportdialog && !modeleditdialog && selectedLight != -1) 
			{
				lightEditDialog.SetEnteredText(idbtb.ID, lights[selectedLight]->GetID());
				lightEditDialog.SetEnteredText(xbtb.ID, std::to_string(lights[selectedLight]->GetPosition().x)); lightEditDialog.SetEnteredText(ybtb.ID, std::to_string(lights[selectedLight]->GetPosition().y)); lightEditDialog.SetEnteredText(zbtb.ID, std::to_string(lights[selectedLight]->GetPosition().z));
				lightEditDialog.SetEnteredText(axbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_AMBIENT)[0])); lightEditDialog.SetEnteredText(aybtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_AMBIENT)[1])); lightEditDialog.SetEnteredText(azbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_AMBIENT)[2]));
				lightEditDialog.SetEnteredText(rxbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_DIFFUSE)[0])); lightEditDialog.SetEnteredText(rybtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_DIFFUSE)[1])); lightEditDialog.SetEnteredText(rzbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_DIFFUSE)[2]));
				lightEditDialog.SetEnteredText(sxbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_SPECULAR)[0])); lightEditDialog.SetEnteredText(sybtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_SPECULAR)[1])); lightEditDialog.SetEnteredText(szbtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_SPECULAR)[2]));
				lightEditDialog.SetEnteredText(labtb.ID, std::to_string(lights[selectedLight]->GetParameters(GL_LINEAR_ATTENUATION)[0]));
				lighteditdialog = !lighteditdialog;
			}
			if (gui.CatchEvent(event, w) == saveButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !loaddialog && !exportdialog) savedialog = !savedialog;
			if (gui.CatchEvent(event, w) == loadButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !exportdialog) loaddialog = !loaddialog;
			if (gui.CatchEvent(event, w) == exportButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog) exportdialog = !exportdialog;
			
			if(modeldialog)
			{
				if (modelDialog.CatchEvent(event, w) == ok.ID)
				{
					filename = modelDialog.GetTextBoxString(filenamebtb.ID);
					if(std::filesystem::exists(filename))
					{
						texture = modelDialog.GetTextBoxString(texturebtb.ID);
						id = modelDialog.GetTextBoxString(idbtb.ID).empty() ? "nn" : modelDialog.GetTextBoxString(idbtb.ID);
						x = stof(modelDialog.GetTextBoxString(xbtb.ID));
						y = stof(modelDialog.GetTextBoxString(ybtb.ID));
						z = stof(modelDialog.GetTextBoxString(zbtb.ID));
						if(!texture.empty()) models.emplace_back(std::make_shared<Model>(filename, texture, id, x, y, z, 0, 0, 0, 1, 1, 1));
						else models.emplace_back(std::make_shared<Model>(filename, GLuint(0), id, x, y, z, 0, 0, 0, 1, 1, 1));
						modeldialog = false;
					}
					else modelDialog.SetEnteredText(filenamebtb.ID, "Error!");
				}
			}
			
			if(animationdialog)
			{
				if (animationDialog.CatchEvent(event, w) == ok.ID)
				{
					filename = animationDialog.GetTextBoxString(filenamebtb.ID);
					if(std::filesystem::exists(filename + "_000000.obj"))
					{
						texture = animationDialog.GetTextBoxString(texturebtb.ID);
						id = animationDialog.GetTextBoxString(idbtb.ID).empty() ? "nn" : animationDialog.GetTextBoxString(idbtb.ID);
						x = stof(animationDialog.GetTextBoxString(xbtb.ID));
						y = stof(animationDialog.GetTextBoxString(ybtb.ID));
						z = stof(animationDialog.GetTextBoxString(zbtb.ID));
						frames = stoi(animationDialog.GetTextBoxString(framesbtb.ID));
						speed = stof(animationDialog.GetTextBoxString(speedbtb.ID));
						if(!texture.empty()) animations.emplace_back(std::make_shared<Animation>(filename, texture, frames, speed, id, x, y, z, 0, 0, 0, 1, 1, 1));
						else animations.emplace_back(std::make_shared<Animation>(filename, GLuint(0), frames, speed, id, x, y, z, 0, 0, 0, 1, 1, 1));
						animationdialog = false;
					}
					else animationDialog.SetEnteredText(filenamebtb.ID, "Error!");
				}
			}
			
			if(lightdialog)
			{
				if (lightDialog.CatchEvent(event, w) == ok.ID)
				{
					lightNum = stoi(lightDialog.GetTextBoxString(lightNumberbtb.ID));
					id = lightDialog.GetTextBoxString(idbtb.ID).empty() ? "nn" : lightDialog.GetTextBoxString(idbtb.ID);
					x = stof(lightDialog.GetTextBoxString(xbtb.ID));
					y = stof(lightDialog.GetTextBoxString(ybtb.ID));
					z = stof(lightDialog.GetTextBoxString(zbtb.ID));
					lights.emplace_back(std::make_shared<Light>(GL_LIGHT0 + lightNum, x, y, z, id));
					lightdialog = false;
				}
			}
			
			if(modeleditdialog)
			{
				int temp = modelEditDialog.CatchEvent(event, w);
				if (temp == ok.ID)
				{
					if(!modelEditDialog.GetTextBoxString(idbtb.ID).empty()) models[selected]->SetID(modelEditDialog.GetTextBoxString(idbtb.ID));
					models[selected]->SetPosition(stof(modelEditDialog.GetTextBoxString(xbtb.ID)), stof(modelEditDialog.GetTextBoxString(ybtb.ID)), stof(modelEditDialog.GetTextBoxString(zbtb.ID)));
					models[selected]->SetRotation(stof(modelEditDialog.GetTextBoxString(rxbtb.ID)), stof(modelEditDialog.GetTextBoxString(rybtb.ID)), stof(modelEditDialog.GetTextBoxString(rzbtb.ID)));
					models[selected]->SetSize(stof(modelEditDialog.GetTextBoxString(sxbtb.ID)), stof(modelEditDialog.GetTextBoxString(sybtb.ID)), stof(modelEditDialog.GetTextBoxString(szbtb.ID)));
					gui.SetText(infoButton.ID, models[selected]->GetID());
					modeleditdialog = false;
				}
				else if (temp == deleteb.ID)
				{
					models.erase(models.begin() + selected);
					selected -= 1;
					gui.SetText(infoButton.ID, (selected < 0) ? "NULL" : models[selected]->GetID());
					modeleditdialog = false;
				}
			}

			if(animationeditdialog)
			{
				int temp = animationEditDialog.CatchEvent(event, w);
				if (temp == ok.ID)
				{
					if(!animationEditDialog.GetTextBoxString(idbtb.ID).empty()) animations[selectedAnim]->SetID(animationEditDialog.GetTextBoxString(idbtb.ID));
					animations[selectedAnim]->SetPosition(stof(animationEditDialog.GetTextBoxString(xbtb.ID)), stof(animationEditDialog.GetTextBoxString(ybtb.ID)), stof(animationEditDialog.GetTextBoxString(zbtb.ID)));
					animations[selectedAnim]->SetRotation(stof(animationEditDialog.GetTextBoxString(rxbtb.ID)), stof(animationEditDialog.GetTextBoxString(rybtb.ID)), stof(animationEditDialog.GetTextBoxString(rzbtb.ID)));
					animations[selectedAnim]->SetSize(stof(animationEditDialog.GetTextBoxString(sxbtb.ID)), stof(animationEditDialog.GetTextBoxString(sybtb.ID)), stof(animationEditDialog.GetTextBoxString(szbtb.ID)));
					animations[selectedAnim]->SetSpeed(stof(animationEditDialog.GetTextBoxString(speedbtb.ID)));
					gui.SetText(infoButton2.ID, animations[selectedAnim]->GetID());
					animationeditdialog = false;
				}
				else if (temp == deleteb.ID)
				{
					animations.erase(animations.begin() + selectedAnim);
					selected -= 1;
					gui.SetText(infoButton2.ID, (selectedAnim < 0) ? "NULL" : animations[selectedAnim]->GetID());
					animationeditdialog = false;
				}
			}
			
			if(lighteditdialog)
			{
				int temp = lightEditDialog.CatchEvent(event, w);
				if (temp == ok.ID)
				{
					if(!lightEditDialog.GetTextBoxString(idbtb.ID).empty()) lights[selectedLight]->SetID(lightEditDialog.GetTextBoxString(idbtb.ID));
					lights[selectedLight]->SetPosition(stof(lightEditDialog.GetTextBoxString(xbtb.ID)), stof(lightEditDialog.GetTextBoxString(ybtb.ID)), stof(lightEditDialog.GetTextBoxString(zbtb.ID)));
					lights[selectedLight]->SetParameters({ stof(lightEditDialog.GetTextBoxString(axbtb.ID)), stof(lightEditDialog.GetTextBoxString(aybtb.ID)), stof(lightEditDialog.GetTextBoxString(azbtb.ID)), 1 }, GL_AMBIENT);
					lights[selectedLight]->SetParameters({ stof(lightEditDialog.GetTextBoxString(rxbtb.ID)), stof(lightEditDialog.GetTextBoxString(rybtb.ID)), stof(lightEditDialog.GetTextBoxString(rzbtb.ID)), 1 }, GL_DIFFUSE);
					lights[selectedLight]->SetParameters({ stof(lightEditDialog.GetTextBoxString(sxbtb.ID)), stof(lightEditDialog.GetTextBoxString(sybtb.ID)), stof(lightEditDialog.GetTextBoxString(szbtb.ID)), 1 }, GL_SPECULAR);
					lights[selectedLight]->SetParameters({ stof(lightEditDialog.GetTextBoxString(labtb.ID)) }, GL_LINEAR_ATTENUATION);
					gui.SetText(infoButton4.ID, lights[selectedLight]->GetID());
					lighteditdialog = false;
				}
				else if (temp == deleteb.ID)
				{
					lights.erase(lights.begin() + selectedLight);
					selectedLight -= 1;
					gui.SetText(infoButton4.ID, (selectedLight < 0) ? "NULL" : lights[selectedLight]->GetID());
					lighteditdialog = false;
				}
			}
			
			if(savedialog)
			{
				if (saveLoadDialog.CatchEvent(event, w) == ok.ID)
				{
					SaveProject(saveLoadDialog.GetTextBoxString(pnamebtb.ID));
					savedialog = false;
				}
			}
			
			if(loaddialog)
			{
				if (saveLoadDialog.CatchEvent(event, w) == ok.ID)
				{
					if(LoadProject(saveLoadDialog.GetTextBoxString(pnamebtb.ID))) loaddialog = false;
					else saveLoadDialog.SetEnteredText(pnamebtb.ID, "Error!");
				}
			}
			
			if(exportdialog)
			{
				if (saveLoadDialog.CatchEvent(event, w) == ok.ID)
				{
					if(ExportProject(saveLoadDialog.GetTextBoxString(pnamebtb.ID))) exportdialog = false;
					else saveLoadDialog.SetEnteredText(pnamebtb.ID, "Error!");
				}
			}
			
			if (gui.CatchEvent(event, w) == plusButton.ID) {
				if (selected != models.size() - 1) {
					selected++;
					gui.SetText(infoButton.ID, models[selected]->GetID());
				}
			}
			
			if (gui.CatchEvent(event, w) == minusButton.ID) {
				if (selected != 0 && selected > -1) {
					selected--;
					gui.SetText(infoButton.ID, models[selected]->GetID());
				}
			}

			if (gui.CatchEvent(event, w) == plusButton1.ID) {
				if (selectedAnim != animations.size() - 1) {
					selectedAnim++;
					gui.SetText(infoButton2.ID, animations[selectedAnim]->GetID());
				}
			}

			if (gui.CatchEvent(event, w) == minusButton1.ID) {
				if (selectedAnim != 0 && selectedAnim > -1) {
					selectedAnim--;
					gui.SetText(infoButton2.ID, animations[selectedAnim]->GetID());
				}
			}
			
			if (gui.CatchEvent(event, w) == plusButton2.ID) {
				if (selectedLight != lights.size() - 1) {
					selectedLight++;
					gui.SetText(infoButton4.ID, lights[selectedLight]->GetID());
				}
			}
			
			if (gui.CatchEvent(event, w) == minusButton2.ID) {
				if (selectedLight != 0 && selectedLight > -1) {
					selectedLight--;
					gui.SetText(infoButton4.ID, lights[selectedLight]->GetID());
				}
			}

			if (gui.CatchEvent(event, w) == infoButton.ID) {
				modelmove = true;
				animmove = lightmove = false;
			}
			else if (gui.CatchEvent(event, w) == infoButton2.ID) {
				modelmove = lightmove = false;
				animmove = true;
			}
			else if (gui.CatchEvent(event, w) == infoButton4.ID) {
				modelmove = animmove = false;
				lightmove = true;
			}

			if (event.type == sf::Event::Closed) {
				w.close();
			}
			if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) {
				w.close();
			}
			if ((event.type == sf::Event::MouseButtonPressed) && (event.key.code == sf::Mouse::Right)) {
				right = !right;
			}
		}

		float time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		time = time / 40;
		if (time > 5) time = 5;
		
		cam.Move(time);
		
		if (right) {
			if(showcursor) w.setMouseCursorVisible(!showcursor);
			showcursor = false;
			cam.Mouse(w);
		}
		else if (!showcursor) {
			w.setMouseCursorVisible(!showcursor);
			showcursor = true;
		}
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) { move = true; resize = false; rotate = false; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { move = false; resize = false; rotate = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { move = false; resize = true; rotate = false; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) { move = resize = rotate = false; }
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) choice = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) choice = 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) choice = 2;

		if (move) {
			if (modelmove && selected >= 0) 
			{
				switch (choice)
				{
				case 0: models[selected]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0, 0); break;
				case 1: models[selected]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0); break;
				case 2: models[selected]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0); break;
				}
			}
			if (animmove && selectedAnim >= 0) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0); break;
				case 2: animations[selectedAnim]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0); break;
				}
			}
			if (lightmove && selectedLight >= 0) {
				switch (choice)
				{
				case 0: lights[selectedLight]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0, 0); break;
				case 1: lights[selectedLight]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0, 0); break;
				case 2: lights[selectedLight]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_move * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_move * time : 0); break;
				}
			}
		}
		else if (rotate) {
			if (modelmove && selected >= 0) {
				switch (choice)
				{
				case 0: models[selected]->AddRotation((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0, 0, 0); break;
				case 1: models[selected]->AddRotation(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0, 0); break;
				case 2: models[selected]->AddRotation(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0); break;
				}
			}
			if (animmove && selectedAnim >= 0) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddRotation((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddRotation(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0, 0); break;
				case 2: animations[selectedAnim]->AddRotation(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_rotate * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_rotate * time : 0); break;
				}
			}
		}
		else if (resize) {
			if (modelmove && selected >= 0) {
				switch (choice)
				{
				case 0: models[selected]->AddSize((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0, 0, 0); break;
				case 1: models[selected]->AddSize(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0, 0); break;
				case 2: models[selected]->AddSize(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0); break;
				}
			}
			if (animmove && selectedAnim >= 0) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddSize((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddSize(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0, 0); break;
				case 2: animations[selectedAnim]->AddSize(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? m_resize * time : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -m_resize * time : 0); break;
				}
			}
		}
		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		cam.Look();
				
		if(!lights.empty()) {
			for(int i = 0; i < lights.size(); i++) { 
				lights[i]->Update();
				lightShape.SetPosition(lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
				glDisable(GL_LIGHTING);
				lightShape.Draw(GLuint(0));
				glEnable(GL_LIGHTING);
			}
		}
		
		if (!models.empty()) {
			for (int i = 0; i < models.size(); i++) {
				models[i]->Draw();
			}
		}

		if (!animations.empty()) {
			for (int i = 0; i < animations.size(); i++) {
				animations[i]->DrawAnimation(time);
			}
		}
		
		glDisable(GL_LIGHTING);
		Shape::Draw(skybox, cam.x, cam.y, cam.z, 1000, 1000, 1000);
		glEnable(GL_LIGHTING);
		
		if(modeldialog) modelDialog.Draw(w);		
		if(animationdialog) animationDialog.Draw(w);
		if(lightdialog) lightDialog.Draw(w);
		if(modeleditdialog) modelEditDialog.Draw(w);		
		if(animationeditdialog) animationEditDialog.Draw(w);		
		if(lighteditdialog) lightEditDialog.Draw(w);
		if(savedialog || loaddialog || exportdialog) saveLoadDialog.Draw(w);
		
		gui.Draw(w);
					
		w.display();
	}
}
