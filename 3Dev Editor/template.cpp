#include <Gui.h>
#include <Model.h>
#include <Camera.h>
#include <Light.h>
#include <Shape.h>
#include <Shader.h>
#include <Physics.h>
#include <Animation.h>

void OpenGL_Setup(float w, float h)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0);
	glDepthMask(GL_TRUE);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.f, w / h, 0.1, 50000.f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	
	glewInit();
}

int main()
{
	//Settings for SFML window
	sf::ContextSettings s;
	s.depthBits = 24;
	//////////////////////////
	
	float width = 1280, height = 720;
	
	//SFML window
	sf::RenderWindow w(sf::VideoMode(width, height), "SUPER ULTRA MEGA COOL GAME", sf::Style::Default, s);
	w.setFramerateLimit(60);
	
	OpenGL_Setup(width, height);
	
	//Animations
	//Models
	//Lights
	Camera c(0, 0, 0, 1);
	sf::Clock clock;
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
		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		c.Mouse(w);
		c.Move(time);
		c.Look();
		
		//LightsUpdate
		//AnimationsDraw
		//ModelsDraw
		w.display();
	}
}
