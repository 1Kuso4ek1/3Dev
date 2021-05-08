#define NOMINMAX
#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Skybox.h>
#include <Model.h>
#include <Animation.h>
#include <Shape.h>
#include <SFML/Network.hpp>
#include <chrono>
class Player {
public:
	std::string id;
	float x = 300, y = 0, z = 300, speed = 1, rotation = 0;
	void Move(float time, Camera& c) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			x += -sin(c.angleX / 180 * pi) * (speed * time);
			z += -cos(c.angleX / 180 * pi) * (speed * time);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			x += sin(c.angleX / 180 * pi) * (speed * time);
			z += cos(c.angleX / 180 * pi) * (speed * time);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			x += sin((c.angleX + 90) / 180 * pi) * (speed * time);
			z += cos((c.angleX + 90) / 180 * pi) * (speed * time);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			x += sin((c.angleX - 90) / 180 * pi) * (speed * time);
			z += cos((c.angleX - 90) / 180 * pi) * (speed * time);
		}
	}
};

int main() {
	Player you, mp;
	//Подключаемся к серверу
	sf::IpAddress ip;
	sf::TcpSocket socket;
	std::cout << "Server IP Adress: ";
	std::cin >> ip;
	std::cout << "Your name: ";
	std::cin >> you.id;
	if(socket.connect(ip, 2000) != sf::Socket::Done) {
		std::cout << "Error! Can't connect to the server" << std::endl;
		exit(1);
	}
	socket.setBlocking(false);
	//Settings for SFML window
	sf::ContextSettings s;
	s.depthBits = 24;
	//////////////////////////

	//SFML window
	sf::RenderWindow w(sf::VideoMode(640, 480), "3Dev", sf::Style::Default, s);
	w.setFramerateLimit(60);
	//OpenGL settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0);
	glDepthMask(GL_TRUE);
	glClearDepth(1.f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.f, 640.f / 480.f, 0.1, 50000.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	//////////////////
	//Loading some textures
	GLuint texture = LoadTexture("t.jpg");

	GLuint skybox[6];
	skybox[0] = LoadTexture("resources/skybox3/skybox_front.bmp");
	skybox[1] = LoadTexture("resources/skybox3/skybox_back.bmp");
	skybox[2] = LoadTexture("resources/skybox3/skybox_left.bmp");
	skybox[3] = LoadTexture("resources/skybox3/skybox_right.bmp");
	skybox[4] = LoadTexture("resources/skybox3/skybox_bottom.bmp");
	skybox[5] = LoadTexture("resources/skybox3/skybox_top.bmp");
	///////////////////////
	Camera cam(300, 20, 300, 2);
	/////////////////////////////////////////////////
	Model player("cyborg.obj", "cyborg_diffuse.png", 5, 0, 10); //Cyborg model
	Model hand("phnd.obj", "hand.jpg", 0, 0, 0);
	Shape map(1000, 2, 1000, 0, -2, 0);
	//Settings for lighting
	float amb[4] = { 2.5, 2.5, 2.5, 1 }; //Ambient
	float dif[4] = { 2, 2, 2, 1 }; //Diffuse
	float pos[4] = { 0, 20, 0, 1 }; //Position
	float c[1] = { 0.2 }; //Linear attenuation
	///////////////////////
	//Creating lights
	Light l((GLenum)GL_LIGHT0, 0, 0, 0);
	////////////////
	//Light parameters
	//l.SetParameters(c, GL_LINEAR_ATTENUATION); //Linear attenuation
	l.SetParameters(amb, GL_AMBIENT); //Ambient
	l.SetParameters(dif, GL_DIFFUSE); //Diffuse
	l.SetParameters(dif, GL_SPECULAR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, dif);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30);
	//////////////////
	sf::Clock clock;
	bool update = false, mouse = true;
	float rotation;
	std::vector<Player> ps;
	//Main cycle
	while (w.isOpen()) {
		sf::Event event;
		while (w.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				w.close();
			}
			if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) {
				w.close();
			}
			else if (event.type == sf::Event::GainedFocus)
				update = true;
			else if (event.type == sf::Event::LostFocus)
				update = false;
		}

		float time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		time = time / 40;
		if (time > 5) time = 5;
		
		l.Update();
		
		//l.SetParameters(pos, GL_POSITION);
		//Camera controls
		//cam.Move(time);
		if(update) cam.Mouse(w);
		/////////////////
		if(update) you.Move(time, cam);
		
		cam.x = you.x; cam.y = 3; cam.z = you.z;
		
		//Отправляем данные на сервер
		sf::Packet packet;
		
		packet << you.id << you.x << you.y << you.z << cam.angleX;
		
		socket.send(packet);
		
		//Принимаем данные
		while(socket.receive(packet) == sf::Socket::Done) {
			if(packet >> mp.id >> mp.x >> mp.y >> mp.z >> mp.rotation) {
				auto pred = [&mp](const Player & item) {
					return item.id == mp.id;
				};
				if(std::find_if(std::begin(ps), std::end(ps), pred) == std::end(ps)) {
					ps.push_back(mp);
				}
				else {
					for(int i = 0; i < ps.size(); i++) {
						if(mp.id == ps[i].id) {
							ps[i].x = mp.x;
							ps[i].y = mp.y;
							ps[i].z = mp.z;
							ps[i].rotation = mp.rotation;
						}
					}
				}
			}
		}
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		cam.Look();
		
		map.Draw(texture);
		hand.SetPosition(you.x, you.y + 2.5, you.z);
		hand.SetRotation(cam.angleY, cam.angleX, 0);
		hand.Draw();
		for(int i = 0; i < ps.size(); i++) {
			player.SetPosition(ps[i].x, ps[i].y, ps[i].z);
			player.SetRotation(0, ps[i].rotation + 180, 0);
			player.Draw();
		}
		glDisable(GL_LIGHTING);
		glTranslatef(cam.x, cam.y, cam.z);
		RenderSkybox(skybox, 2000); //Drawing skybox
		glTranslatef(-cam.x, -cam.y, -cam.z);
		glEnable(GL_LIGHTING);
		
		w.display();
	}
} 
