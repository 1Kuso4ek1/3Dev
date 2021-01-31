#include <Camera.h>
#include <Model.h>
#include <Light.h>
#include <Skybox.h>

int main() {
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
	Light l(GL_LIGHT0, 5, 5, 20);
	l.SetParameters(amb, GL_AMBIENT);
	l.SetParameters(dif, GL_SPECULAR);
	l.SetParameters(la, GL_LINEAR_ATTENUATION); 
	l.SetParameters(dif, GL_DIFFUSE);
	sf::Clock clock;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, dif);
    while(w.isOpen()) {
        sf::Event event;
        while(w.pollEvent(event)) {
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
		
		cam.Mouse(w);
		cam.Move(time);
        cam.Look();
		
		l.Update();
		
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120);
		torus.Draw();
		
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20);
		teapot.AddRotation(0, 0.4, 0);
		teapot.Draw();
		
        glDisable(GL_LIGHTING);
		glTranslatef(cam.x, cam.y, cam.z);
		RenderSkybox(skybox, 1000);
		glTranslatef(-cam.x, -cam.y, -cam.z);
		glEnable(GL_LIGHTING);

        w.display();
    }
}
