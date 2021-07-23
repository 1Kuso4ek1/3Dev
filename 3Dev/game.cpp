#include <Camera.h>
#include <Model.h>
#include <Animation.h>
#include <Light.h>
#include <Shape.h>
#include <Gui.h>

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

int main()
{
    gui::Gui gui;
    gui::TextBox txbx(5, 30, 70, 20, 2, 12, 1);
    gui::Button bt(5, 5, 80, 20, "Enter shininess", 12, 2, 0), bt1(5, 55, 70, 20, "Teapot", 12, 2, 0), 
    bt2(5, 80, 70, 20, "Torus", 12, 3, 0), bt3(5, 105, 70, 20, "Cyborg", 12, 4, 0);
    bt.SetColor(sf::Color(0, 0, 0, 0));
    gui.Add(txbx); gui.Add(bt); gui.Add(bt1); gui.Add(bt2); gui.Add(bt3);
    
    sf::ContextSettings s;
    s.depthBits = 24;
    
    int width = 1280, height = 720;
    
    sf::RenderWindow w(sf::VideoMode(width, height), "3Dev", sf::Style::Default, s);
    
    w.setVerticalSyncEnabled(true);
    w.setFramerateLimit(120);
    
    GLsetup(width, height);
	
    GLuint skybox[6] = {
	LoadTexture("resources/skybox/skybox_front.bmp"),
	LoadTexture("resources/skybox/skybox_back.bmp"),
	LoadTexture("resources/skybox/skybox_left.bmp"),
	LoadTexture("resources/skybox/skybox_right.bmp"),
	LoadTexture("resources/skybox/skybox_bottom.bmp"),
	LoadTexture("resources/skybox/skybox_top.bmp")
    };
    
    Camera cam(5, 3, 15, 1);
    Model torus("resources/models/torus.obj", "resources/textures/texture.png", 0, 0, 0);
    Model teapot("resources/models/teapot.obj", "resources/textures/gold.jpg", 10, 0, 0);
    Animation cyborg("resources/animation/cyborg", "resources/textures/cyborg_diffuse.png", 20, 1);
    
    cyborg.SetPosition(-5, 0, 0);
    
    Light l(GL_LIGHT0, 0, 10, 10);
    
    l.SetParameters({ 2, 2, 2, 1 }, GL_AMBIENT);
    l.SetParameters({ 2, 2, 2, 1 }, GL_SPECULAR);
    l.SetParameters({ 0.3 }, GL_LINEAR_ATTENUATION);
    l.SetParameters({ 2, 2, 2, 1 }, GL_DIFFUSE);
    
    sf::Clock clock;
    
    glewInit();
    
    bool a = true;
    
    float teashininess = 20;
    float torshininess = 120;
    float cyborgshininess = 50;
    
    sf::Clock aa;
    
    Material cyborgm, torm, teapotm;
    
    cyborgm.SetParameters({ cyborgshininess }, GL_SHININESS);
    cyborgm.SetParameters({ 2, 2, 2, 1 }, GL_SPECULAR);
    
    torm.SetParameters({ torshininess }, GL_SHININESS);
    torm.SetParameters({ 2, 2, 2, 1 }, GL_SPECULAR);
    
    teapotm.SetParameters({ teashininess }, GL_SHININESS);
    teapotm.SetParameters({ 2, 2, 2, 1 }, GL_SPECULAR);
    
    torus.SetMaterial(torm); teapot.SetMaterial(teapotm); cyborg.SetMaterial(cyborgm);
    
    while(w.isOpen()) {
	sf::Event event;
	while(w.pollEvent(event)) {
	    int op = gui.CatchEvent(event, w);
	    if(op == bt1.ID) {
		teashininess = stoi(gui.GetTextBoxString(1));
		teapotm.SetParameters({ teashininess }, GL_SHININESS);
		teapot.SetMaterial(teapotm);
	    }
	    else if(op == bt2.ID) {
		torshininess = stoi(gui.GetTextBoxString(1));
		torm.SetParameters({ torshininess }, GL_SHININESS);
		torus.SetMaterial(torm);
	    }
	    else if(op == bt3.ID) {
		cyborgshininess = stoi(gui.GetTextBoxString(1));
		cyborgm.SetParameters({ cyborgshininess }, GL_SHININESS);
		cyborg.SetMaterial(cyborgm);
	    }
	    if(event.type == sf::Event::Closed) w.close();
	}
	
	float time = clock.getElapsedTime().asMilliseconds();
	clock.restart();
	time = time / 40;
	if (time > 5) time = 5;

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	if(sf::Mouse::isButtonPressed(sf::Mouse::Right)) a = !a;
			    
	if(a) cam.Mouse(w);
	cam.Move(time);
	cam.Look();
			    
	l.Update();

	torus.Draw();
	teapot.AddRotation(0, 0.4, 0);
	teapot.Draw();
	cyborg.DrawAnimation(time);

	glDisable(GL_LIGHTING);
	Shape::Draw(skybox, cam.x, cam.y, cam.z, 1000, 1000, 1000);
	glEnable(GL_LIGHTING);
				
	gui.Draw(w);
				
	w.display();
    }
}
