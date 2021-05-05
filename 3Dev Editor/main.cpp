#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Skybox.h>
#include <Model.h>
#include <Animation.h>
#include <Shape.h>
#include <AllGui.h>
#include <filesystem>

Camera cam(0, 0, 0, 1);
bool right = false;

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
std::string filename, texture, id;

int selected = -1, selectedAnim = -1, selectedLight = -1;
int frames = 0, lightNum = 0;
int choice = 0;

float speed = 0;
float x, y, z, rotx, roty, rotz, sizex, sizey, sizez, temp;

bool move = false, resize = false, rotate = false;
bool modelmove = false, animmove = false, lightmove = false;
bool showcursor = true;
bool modeldialog = false, animationdialog = false, lightdialog = false, modeleditdialog = false, animationeditdialog = false, lighteditdialog = false, savedialog = false, loaddialog = false;

Shape lightShape(0.3, 0.3, 0.3, 0, 0, 0);

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

int main() {
	sf::ContextSettings s;
	s.depthBits = 24;

	int width = 1280, height = 720;

	sf::RenderWindow w(sf::VideoMode(width, height), "3Dev", sf::Style::Default, s);
	w.setVerticalSyncEnabled(true);
	w.setFramerateLimit(60);
	
	GLsetup(width, height);

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
			if (gui.CatchEvent(event, w) == modelButton.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog) modeldialog = !modeldialog;
			if (gui.CatchEvent(event, w) == animationButton.ID && !modeldialog && !lightdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog) animationdialog = !animationdialog;
			if (gui.CatchEvent(event, w) == lightButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog && !loaddialog) lightdialog = !lightdialog;
			if (gui.CatchEvent(event, w) == editButton.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && selected != -1) modeleditdialog = !modeleditdialog;
			if (gui.CatchEvent(event, w) == editButton1.ID && !modeleditdialog && !animationdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && selectedAnim != -1) animationeditdialog = !animationeditdialog;
			if (gui.CatchEvent(event, w) == editButton2.ID && !animationdialog && !animationeditdialog && !lightdialog && !modeldialog && !lighteditdialog && !savedialog && !loaddialog && !modeleditdialog && selectedLight != -1) lighteditdialog = !lighteditdialog;
			if (gui.CatchEvent(event, w) == saveButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !loaddialog) savedialog = !savedialog;
			if (gui.CatchEvent(event, w) == loadButton.ID && !modeldialog && !animationdialog && !animationeditdialog && !modeleditdialog && !lighteditdialog && !savedialog) loaddialog = !loaddialog;
			
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
					if(std::filesystem::exists(filename))
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
					float ads[4] = { 0, 0, 0, 1 };
					float la[1] = { 0 };
					if(!lightEditDialog.GetTextBoxString(idbtb.ID).empty()) lights[selectedLight]->SetID(lightEditDialog.GetTextBoxString(idbtb.ID));
					lights[selectedLight]->SetPosition(stof(lightEditDialog.GetTextBoxString(xbtb.ID)), stof(lightEditDialog.GetTextBoxString(ybtb.ID)), stof(lightEditDialog.GetTextBoxString(zbtb.ID)));
					ads[0] = stof(lightEditDialog.GetTextBoxString(axbtb.ID)); ads[1] = stof(lightEditDialog.GetTextBoxString(aybtb.ID)); ads[2] = stof(lightEditDialog.GetTextBoxString(azbtb.ID));
					lights[selectedLight]->SetParameters(ads, GL_AMBIENT);
					ads[0] = stof(lightEditDialog.GetTextBoxString(rxbtb.ID)); ads[1] = stof(lightEditDialog.GetTextBoxString(rybtb.ID)); ads[2] = stof(lightEditDialog.GetTextBoxString(rzbtb.ID));
					lights[selectedLight]->SetParameters(ads, GL_DIFFUSE);
					ads[0] = stof(lightEditDialog.GetTextBoxString(sxbtb.ID)); ads[1] = stof(lightEditDialog.GetTextBoxString(sybtb.ID)); ads[2] = stof(lightEditDialog.GetTextBoxString(szbtb.ID));
					lights[selectedLight]->SetParameters(ads, GL_SPECULAR);
					la[0] = stof(lightEditDialog.GetTextBoxString(labtb.ID));
					lights[selectedLight]->SetParameters(la, GL_LINEAR_ATTENUATION);
					gui.SetText(infoButton4.ID, lights[selectedLight]->GetID());
					lighteditdialog = false;
				}
				else if (temp == deleteb.ID)
				{
					lights.erase(lights.begin() + selectedLight);
					selectedLight -= 1;
					gui.SetText(infoButton.ID, (selectedLight < 0) ? "NULL" : lights[selectedLight]->GetID());
					lighteditdialog = false;
				}
			}
			
			if(savedialog)
			{
				if (saveLoadDialog.CatchEvent(event, w) == ok.ID)
				{
					filename = saveLoadDialog.GetTextBoxString(pnamebtb.ID);
					std::ofstream output(filename);
					output << models.size() << std::endl;
					for(int i = 0; i < models.size(); i++)
					{
						output << models[i]->GetID() << " " << models[i]->GetFilename() << " " << (models[i]->GetTextureFilename().empty() ? "notex" : models[i]->GetTextureFilename()) << " " << models[i]->GetPosition().x << " " << models[i]->GetPosition().y << " " << models[i]->GetPosition().z << " " << models[i]->GetRotation().x << " " << models[i]->GetRotation().y << " " << models[i]->GetRotation().z << " " << models[i]->GetSize().x << " " << models[i]->GetSize().y << " " << models[i]->GetSize().z << std::endl;
					}
					output << animations.size() << std::endl;
					for(int i = 0; i < animations.size(); i++)
					{
						output << animations[i]->GetID() << " " << animations[i]->GetFilename() << " " << (animations[i]->GetTextureFilename().empty() ? "notex" : animations[i]->GetTextureFilename()) << " " << animations[i]->GetFrames() << "" << animations[i]->GetSpeed() << " " << animations[i]->GetPosition().x << " " << animations[i]->GetPosition().y << " " << animations[i]->GetPosition().z << " " << animations[i]->GetRotation().x << " " << animations[i]->GetRotation().y << " " << animations[i]->GetRotation().z << " " << animations[i]->GetSize().x << " " << animations[i]->GetSize().y << " " << animations[i]->GetSize().z << std::endl;
					}
					output << lights.size() << std::endl;
					for(int i = 0; i < lights.size(); i++)
					{
						float la[1] = { 0 };
						float ads[4] = { 0, 0, 0, 1 };
						output << lights[i]->GetID() << " " << std::hex << lights[i]->GetLightNum() << " " << lights[i]->GetPosition().x << " " << lights[i]->GetPosition().y << " " << lights[i]->GetPosition().z << " ";
						lights[i]->GetParameters(GL_LINEAR_ATTENUATION, la);
						output << la[0] << " ";
						lights[i]->GetParameters(GL_AMBIENT, ads);
						output << ads[0] << " " << ads[1] << " " << ads[2] << " ";
						lights[i]->GetParameters(GL_DIFFUSE, ads);
						output << ads[0] << " " << ads[1] << " " << ads[2] << " ";
						lights[i]->GetParameters(GL_SPECULAR, ads);
						output << ads[0] << " " << ads[1] << " " << ads[2] << std::endl;
					}
					output.close();
					savedialog = false;
				}
			}
			
			if(loaddialog)
			{
				if (saveLoadDialog.CatchEvent(event, w) == ok.ID)
				{
					filename = saveLoadDialog.GetTextBoxString(pnamebtb.ID);
					std::ifstream input(filename);
					if(input.is_open())
					{
						int it = 0;
						input >> it;
						for(int i = 0; i < it; i++)
						{
							input >> id >> filename >> texture >> x >> y >> z >> rotx >> roty >> rotz >> sizex >> sizey >> sizez;
							if(texture != "notex") models.emplace_back(std::make_shared<Model>(filename, texture, id, x, y, z, 0, 0, 0, 1, 1, 1));
							else models.emplace_back(std::make_shared<Model>(filename, GLuint(0), id, x, y, z, 0, 0, 0, 1, 1, 1));
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
							float ads[4] = { 0, 0, 0, 1 };
							input >> id >> std::hex >> lightNum >> x >> y >> z;
							lights.emplace_back(std::make_shared<Light>(lightNum, x, y, z, id));
							input >> la[0];
							lights[i]->SetParameters(la, GL_LINEAR_ATTENUATION);
							input >> ads[0] >> ads[1] >> ads[2];
							lights[i]->SetParameters(ads, GL_AMBIENT);
							input >> ads[0] >> ads[1] >> ads[2];
							lights[i]->SetParameters(ads, GL_DIFFUSE);
							input >> ads[0] >> ads[1] >> ads[2];
							lights[i]->SetParameters(ads, GL_SPECULAR);
						}
						input.close();
						loaddialog = false;
					}
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
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) move = true; resize = false; rotate = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) move = false; resize = false; rotate = true;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) move = false; resize = true; rotate = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) move = resize = rotate = false;
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) choice = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) choice = 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) choice = 2;

		if (move) {
			if (modelmove && selected >= 0) 
			{
				switch (choice)
				{
				case 0: models[selected]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: models[selected]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: models[selected]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
			if (animmove && selectedAnim >= 0) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: animations[selectedAnim]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
			if (lightmove && selectedLight >= 0) {
				switch (choice)
				{
				case 0: lights[selectedLight]->AddPosition((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: lights[selectedLight]->AddPosition(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: lights[selectedLight]->AddPosition(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
		}
		if (rotate) {
			if (modelmove) {
				switch (choice)
				{
				case 0: models[selected]->AddRotation((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: models[selected]->AddRotation(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: models[selected]->AddRotation(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
			if (animmove) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddRotation((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddRotation(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: animations[selectedAnim]->AddRotation(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
		}
		if (resize) {
			if (modelmove) {
				switch (choice)
				{
				case 0: models[selected]->AddSize((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: models[selected]->AddSize(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: models[selected]->AddSize(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
				}
			}
			if (animmove) {
				switch (choice)
				{
				case 0: animations[selectedAnim]->AddSize((sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0, 0); break;
				case 1: animations[selectedAnim]->AddSize(0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0, 0); break;
				case 2: animations[selectedAnim]->AddSize(0, 0, (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) ? 0.1 : (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) ? -0.1 : 0); break;
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
		if(savedialog || loaddialog) saveLoadDialog.Draw(w);
		
		gui.Draw(w);
					
		w.display();
	}
}
