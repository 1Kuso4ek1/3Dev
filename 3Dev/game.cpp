#include <Camera.h>
#include <Model.h>
#include <Animation.h>
#include <Light.h>
#include <Skybox.h>
#include <Shader.h>
#include <Gui.h>

int main() {
	gui::Gui gui;
	
	gui::TextBox txbx(5, 30, 70, 20, 2, 12, 1);
	gui::Button bt(5, 5, 80, 20, "Enter shininess", 12, 2, 0);
	bt.SetColor(sf::Color(0, 0, 0, 0));
	gui::Button bt1(5, 55, 70, 20, "Teapot", 12, 2, 0);
	gui::Button bt2(5, 80, 70, 20, "Torus", 12, 3, 0);
	gui::Button bt3(5, 105, 70, 20, "Cyborg", 12, 4, 0);
	gui.Add(txbx);
	gui.Add(bt);
	gui.Add(bt1);
	gui.Add(bt2);
	gui.Add(bt3);
	
	sf::ContextSettings s;
	s.depthBits = 24;
	
	int width = 1280, height = 720;
	
	sf::RenderWindow w(sf::VideoMode(width, height), "3Dev", sf::Style::Default, s);
	
	w.setVerticalSyncEnabled(true);
	w.setFramerateLimit(120);
	
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
	
	GLuint skybox[6];
	skybox[0] = LoadTexture("resources/skybox/skybox_front.bmp");
	skybox[1] = LoadTexture("resources/skybox/skybox_back.bmp");
	skybox[2] = LoadTexture("resources/skybox/skybox_left.bmp");
	skybox[3] = LoadTexture("resources/skybox/skybox_right.bmp");
	skybox[4] = LoadTexture("resources/skybox/skybox_bottom.bmp");
	skybox[5] = LoadTexture("resources/skybox/skybox_top.bmp");
	
	float amb[4] = { 2, 2, 2, 1 };
	float dif[4] = { 2, 2, 2, 1 };
	float la[1] = { 0.3 }; 
	
	Camera cam(5, 3, 15, 1);
	Model torus("resources/models/torus.obj", "resources/textures/texture.png", 0, 0, 0);
	Model teapot("resources/models/teapot.obj", "resources/textures/gold.jpg", 10, 0, 0);
	Animation cyborg("resources/animation/cyborg", "resources/textures/cyborg_diffuse.png", 20, 1);
	cyborg.SetPosition(-5, 0, 0);
	Light l(GL_LIGHT0, 0, 10, 10);
	l.SetParameters(amb, GL_AMBIENT);
	l.SetParameters(dif, GL_SPECULAR);
	l.SetParameters(la, GL_LINEAR_ATTENUATION); 
	l.SetParameters(dif, GL_DIFFUSE);
	sf::Clock clock;
	glewInit();
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, dif);
    bool a = true;
    int teashininess = 20;
    int torshininess = 120;
    int remyshininess = 50;
    sf::Clock aa;
    
    while(w.isOpen()) {
        sf::Event event;
        while(w.pollEvent(event)) {
			if(gui.CatchEvent(event, w) == bt1.ID) {
				teashininess = stoi(gui.GetTextBoxString(1));
			}
			else if(gui.CatchEvent(event, w, false) == bt2.ID) {
				torshininess = stoi(gui.GetTextBoxString(1));
			}
			else if(gui.CatchEvent(event, w, false) == bt3.ID) {
				remyshininess = stoi(gui.GetTextBoxString(1));
			}
            if(event.type == sf::Event::Closed) {
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
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) a = !a;
		
		if(a) cam.Mouse(w);
		cam.Move(time);
        cam.Look();
		
		l.Update();
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, torshininess);
		torus.Draw();
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, teashininess);
		teapot.AddRotation(0, 0.4, 0);
		teapot.Draw();
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, remyshininess);
		cyborg.DrawAnimation(time);
        glDisable(GL_LIGHTING);
		glTranslatef(cam.x, cam.y, cam.z);
		RenderSkybox(skybox, 1000);
		glTranslatef(-cam.x, -cam.y, -cam.z);
		glEnable(GL_LIGHTING);
		
		gui.Draw(w);

        w.display();
    }
}
