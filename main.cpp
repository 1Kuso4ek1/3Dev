/*
This file is an example of using the engine.
https://vk.com/idngames
*/
//include everything we need
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "Skybox.h"
#include "Model.h"
#include "Animation.h"

int main() {
	//Settings for SFML window
	sf::ContextSettings s;
	s.depthBits = 24;
	s.antialiasingLevel = 4;
	//////////////////////////

	//SFML window
	sf::RenderWindow w(sf::VideoMode(1366, 768), "3Dev", sf::Style::Default, s);
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
	gluPerspective(45.f, 1366.f / 768.f, 0.1, 50000.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	//////////////////
	//Loading some textures
	GLuint cyborg_texture = LoadTexture("cyborg_diffuse.png");

	GLuint skybox[6];
	skybox[0] = LoadTexture("resources/skybox3/skybox_front.bmp");
	skybox[1] = LoadTexture("resources/skybox3/skybox_back.bmp");
	skybox[2] = LoadTexture("resources/skybox3/skybox_left.bmp");
	skybox[3] = LoadTexture("resources/skybox3/skybox_right.bmp");
	skybox[4] = LoadTexture("resources/skybox3/skybox_bottom.bmp");
	skybox[5] = LoadTexture("resources/skybox3/skybox_top.bmp");
	///////////////////////
	//Creating camera. x = 15 y = 7 z = 15 speed = 4
	Camera cam(15, 7, 15, 4);
	/////////////////////////////////////////////////
	//Model cyborg; //Cyborg model
	//cyborg.Load("cyborg.obj", "cyborg_diffuse.png"); //Loading static model
	Animation cyborg_anim(20);
	cyborg_anim.LoadAnimation("Animation test/cyborg");
	//Settings for lighting
	float amb[4] = { 2, 2, 2, 1 }; //Ambient
	float dif[4] = { 3, 3, 3, 1 }; //Diffuse
	float pos[4] = { 1000, 500, 1000, 1 }; //Position
	///////////////////////
	//Creating light
	Light l(GL_LIGHT0);
	////////////////
	//Light parameters
	l.SetParameters(pos, GL_POSITION); //Position
	l.SetParameters(amb, GL_AMBIENT); //Ambient
	l.SetParameters(dif, GL_DIFFUSE); //Diffuse
	//////////////////
	sf::Clock clock;
	int frame = 1;
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
		}

		float time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		time = time / 40;
		if (time > 5) time = 5;

		//Camera controls
		cam.Move(time);
		cam.Mouse(w.getPosition().x, w.getPosition().y, 1366, 768);
		/////////////////

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		cam.Look();

		l.SetParameters(pos, GL_POSITION);
		
		glTranslatef(10, 0, 10);
		if (frame >= 19) {
			frame = 1;
		}
		frame++;
		cyborg_anim.DrawAnimation(frame, cyborg_texture); //Drawing frame
		glTranslatef(-10, -0, -10);

		glTranslatef(cam.x, cam.y, cam.z);
		RenderSkybox(skybox, 1000); //Drawing skybox
		glTranslatef(-cam.x, -cam.y, -cam.z);

		w.display();
	}
}
