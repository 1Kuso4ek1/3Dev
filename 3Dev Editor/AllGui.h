#include <Gui.h>

gui::Button modelButton(1, 1, 100, 20, "Model", 12, 0, 0);
gui::Button animationButton(1, 22, 100, 20, "Animation", 12, 5, 0);
gui::Button lightButton(1, 43, 100, 20, "Light", 12, 20, 0);

gui::Button editButton(1030, 20, 60, 20, "Edit", 12, 6, 0);
gui::Button plusButton(1100, 20, 15, 20, "+", 12, 1, 0);
gui::Button minusButton(1215, 20, 15, 20, "-", 12, 2, 0);
gui::Button infoButton(1115, 20, 100, 20, "NULL", 12, 3, 0);
gui::Button infoButton1(1115, 1, 100, 20, "Models", 12, 4, 0);

gui::Button editButton1(1030, 62, 60, 20, "Edit", 12, 7, 0);
gui::Button plusButton1(1100, 62, 15, 20, "+", 12, 8, 0);
gui::Button minusButton1(1215, 62, 15, 20, "-", 12, 9, 0);
gui::Button infoButton2(1115, 62, 100, 20, "NULL", 12, 10, 0);
gui::Button infoButton3(1115, 42, 100, 20, "Animations", 12, 11, 0);

gui::Button editButton2(1030, 103, 60, 20, "Edit", 12, 15, 0);
gui::Button plusButton2(1100, 103, 15, 20, "+", 12, 16, 0);
gui::Button minusButton2(1215, 103, 15, 20, "-", 12, 17, 0);
gui::Button infoButton4(1115, 103, 100, 20, "NULL", 12, 18, 0);
gui::Button infoButton5(1115, 83, 100, 20, "Lights", 12, 19, 0);

gui::Button saveButton(500, 1, 100, 20, "Save project", 12, 12, 0);
gui::Button loadButton(601, 1, 100, 20, "Load project", 12, 13, 0);
gui::Button exportButton(500, 22, 100, 20, "Export project", 12, 14, 0);

gui::Button background(400, 100, 400, 300, "", 0, 20, 0, false);
gui::Button ok(570, 370, 50, 20, "Ok", 12, 21, 0);

gui::Button filenameb(410, 110, 0, 0, "Filename", 12, 22, 0);
gui::TextBox filenamebtb(480, 110, 100, 20, 1, 12, 23);

gui::Button textureb(410, 140, 0, 0, "Texture", 12, 24, 0);
gui::TextBox texturebtb(480, 140, 100, 20, 1, 12, 25);

gui::Button xb(410, 170, 0, 0, "X", 12, 26, 0);
gui::TextBox xbtb(425, 170, 30, 20, 1, 12, 27, "0");

gui::Button yb(460, 170, 0, 0, "Y", 12, 28, 0);
gui::TextBox ybtb(475, 170, 30, 20, 1, 12, 29, "0");

gui::Button zb(510, 170, 0, 0, "Z", 12, 30, 0);
gui::TextBox zbtb(525, 170, 30, 20, 1, 12, 31, "0");

gui::Button axb(410, 170, 0, 0, "AR", 12, 54, 0);
gui::TextBox axbtb(435, 170, 30, 20, 1, 12, 55, "0");

gui::Button ayb(470, 170, 0, 0, "AG", 12, 56, 0);
gui::TextBox aybtb(495, 170, 30, 20, 1, 12, 57, "0");

gui::Button azb(530, 170, 0, 0, "AB", 12, 58, 0);
gui::TextBox azbtb(555, 170, 30, 20, 1, 12, 59, "0");

gui::Button rxb(410, 170, 0, 0, "RX", 12, 41, 0);
gui::TextBox rxbtb(435, 170, 30, 20, 1, 12, 42, "0");

gui::Button ryb(470, 170, 0, 0, "RY", 12, 43, 0);
gui::TextBox rybtb(495, 170, 30, 20, 1, 12, 44, "0");

gui::Button rzb(530, 170, 0, 0, "RZ", 12, 45, 0);
gui::TextBox rzbtb(555, 170, 30, 20, 1, 12, 46, "0");

gui::Button sxb(410, 200, 0, 0, "SX", 12, 47, 0);
gui::TextBox sxbtb(435, 200, 30, 20, 1, 12, 48, "1");

gui::Button syb(470, 200, 0, 0, "SY", 12, 49, 0);
gui::TextBox sybtb(495, 200, 30, 20, 1, 12, 50, "1");

gui::Button szb(530, 200, 0, 0, "SZ", 12, 51, 0);
gui::TextBox szbtb(555, 200, 30, 20, 1, 12, 52, "1");

gui::Button idb(410, 200, 0, 0, "ID", 12, 32, 0);
gui::TextBox idbtb(440, 200, 100, 20, 1, 12, 33);

gui::Button framesb(410, 230, 0, 0, "Frames", 12, 34, 0);
gui::TextBox framesbtb(470, 230, 50, 20, 1, 12, 35, "0");

gui::Button speedb(410, 260, 0, 0, "Speed", 12, 36, 0);
gui::TextBox speedbtb(460, 260, 30, 20, 1, 12, 37, "1");

gui::Button lab(410, 260, 0, 0, "Linear attenuation", 12, 60, 0);
gui::TextBox labtb(520, 260, 30, 20, 1, 12, 61, "0");

gui::Button lightNumberb(410, 140, 0, 0, "Light number", 12, 38, 0);
gui::TextBox lightNumberbtb(500, 140, 30, 20, 1, 12, 39, "0");

gui::Button warning(410, 110, 0, 0, "In \"Light number\" field enter number only between 0 and 8!", 12, 40, 0);

gui::Button deleteb(410, 230, 50, 20, "Delete", 12, 53, 0);

gui::Button pnameb(410, 110, 0, 0, "Project name", 12, 62, 0);
gui::TextBox pnamebtb(510, 110, 100, 20, 1, 12, 63, ".dev");

gui::Gui SetupGui() {
	gui::Gui gui;

	infoButton1.SetColor(sf::Color(0, 0, 0, 0));
	infoButton.SetColor(sf::Color(70, 70, 70));

	infoButton3.SetColor(sf::Color(0, 0, 0, 0));
	infoButton2.SetColor(sf::Color(70, 70, 70));
	
	infoButton5.SetColor(sf::Color(0, 0, 0, 0));
	infoButton4.SetColor(sf::Color(70, 70, 70));
	
	gui.Add(modelButton);
	gui.Add(animationButton);
	gui.Add(lightButton);
	
	gui.Add(plusButton);
	gui.Add(minusButton);
	gui.Add(infoButton);
	gui.Add(infoButton1);
	gui.Add(editButton);

	gui.Add(plusButton1);
	gui.Add(minusButton1);
	gui.Add(infoButton3);
	gui.Add(infoButton2);
	gui.Add(editButton1);

	gui.Add(plusButton2);
	gui.Add(minusButton2);
	gui.Add(infoButton4);
	gui.Add(infoButton5);
	gui.Add(editButton2);

	gui.Add(saveButton);
	gui.Add(loadButton);
	gui.Add(exportButton);
	
	return gui;
}

gui::Gui SetupModelDialog()
{
	gui::Gui gui;
	
	background.SetColor(sf::Color(255, 255, 255, 200));
	ok.SetColor(sf::Color(70, 70, 70, 200));
	filenameb.SetTextColor(sf::Color::Black);
	
	textureb.SetTextColor(sf::Color::Black);
	
	xb.SetTextColor(sf::Color::Black);
	yb.SetTextColor(sf::Color::Black);
	zb.SetTextColor(sf::Color::Black);
	
	idb.SetTextColor(sf::Color::Black);
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(filenameb);
	gui.Add(filenamebtb);
	
	gui.Add(textureb);
	gui.Add(texturebtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	return gui;
}

gui::Gui SetupAnimationDialog()
{
	gui::Gui gui;
	
	speedb.SetTextColor(sf::Color::Black);
	
	framesb.SetTextColor(sf::Color::Black);
		
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(filenameb);
	gui.Add(filenamebtb);
	
	gui.Add(textureb);
	gui.Add(texturebtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	gui.Add(speedb);
	gui.Add(speedbtb);
	
	gui.Add(framesb);
	gui.Add(framesbtb);
	
	return gui;
}

gui::Gui SetupLightDialog()
{
	gui::Gui gui;
	
	lightNumberb.SetTextColor(sf::Color::Black);
	
	warning.SetTextColor(sf::Color::Black);
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(warning);
	
	gui.Add(lightNumberb);
	gui.Add(lightNumberbtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	return gui;
}

gui::Gui SetupModelEditDialog()
{
	gui::Gui gui;
	
	idb.SetPosition(410, 110);
	idbtb.SetPosition(440, 110);
	
	xb.SetPosition(410, 140);
	xbtb.SetPosition(425, 140);
	
	yb.SetPosition(460, 140);
	ybtb.SetPosition(475, 140);
	
	zb.SetPosition(510, 140);
	zbtb.SetPosition(525, 140);
	
	rxb.SetTextColor(sf::Color::Black);
	ryb.SetTextColor(sf::Color::Black);
	rzb.SetTextColor(sf::Color::Black);
	
	sxb.SetTextColor(sf::Color::Black);
	syb.SetTextColor(sf::Color::Black);
	szb.SetTextColor(sf::Color::Black);
	
	deleteb.SetColor(sf::Color(70, 70, 70, 200));
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(rxb);
	gui.Add(rxbtb);
	
	gui.Add(ryb);
	gui.Add(rybtb);
	
	gui.Add(rzb);
	gui.Add(rzbtb);
	
	gui.Add(sxb);
	gui.Add(sxbtb);
	
	gui.Add(syb);
	gui.Add(sybtb);
	
	gui.Add(szb);
	gui.Add(szbtb);
	
	gui.Add(deleteb);
	
	return gui;
}

gui::Gui SetupAnimationEditDialog()
{
	gui::Gui gui;
	
	speedb.SetPosition(410, 230);
	speedbtb.SetPosition(460, 230);
	
	deleteb.SetPosition(410, 260);
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(rxb);
	gui.Add(rxbtb);
	
	gui.Add(ryb);
	gui.Add(rybtb);
	
	gui.Add(rzb);
	gui.Add(rzbtb);
	
	gui.Add(sxb);
	gui.Add(sxbtb);
	
	gui.Add(syb);
	gui.Add(sybtb);
	
	gui.Add(szb);
	gui.Add(szbtb);
	
	gui.Add(speedb);
	gui.Add(speedbtb);
	
	gui.Add(deleteb);
	
	return gui;
}

gui::Gui SetupLightEditDialog()
{
	gui::Gui gui;
	
	axb.SetTextColor(sf::Color::Black);
	ayb.SetTextColor(sf::Color::Black);
	azb.SetTextColor(sf::Color::Black);
	
	lab.SetTextColor(sf::Color::Black);
	
	rxb.SetText("DR");
	ryb.SetText("DG");
	rzb.SetText("DB");
	
	sxb.SetText("SR");
	syb.SetText("SG");
	szb.SetText("SB");

	rxb.SetPosition(410, 200);
	rxbtb.SetPosition(435, 200);
	
	ryb.SetPosition(470, 200);
	rybtb.SetPosition(495, 200);
	
	rzb.SetPosition(530, 200);
	rzbtb.SetPosition(555, 200);
	
	sxb.SetPosition(410, 230);
	sxbtb.SetPosition(435, 230);
	
	syb.SetPosition(470, 230);
	sybtb.SetPosition(495, 230);
	
	szb.SetPosition(530, 230);
	szbtb.SetPosition(555, 230);
	
	deleteb.SetPosition(410, 290);
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(idb);
	gui.Add(idbtb);
	
	gui.Add(xb);
	gui.Add(xbtb);
	
	gui.Add(yb);
	gui.Add(ybtb);
	
	gui.Add(zb);
	gui.Add(zbtb);
	
	gui.Add(axb);
	gui.Add(axbtb);
	
	gui.Add(ayb);
	gui.Add(aybtb);
	
	gui.Add(azb);
	gui.Add(azbtb);
	
	gui.Add(rxb);
	gui.Add(rxbtb);
	
	gui.Add(ryb);
	gui.Add(rybtb);
	
	gui.Add(rzb);
	gui.Add(rzbtb);
	
	gui.Add(sxb);
	gui.Add(sxbtb);
	
	gui.Add(syb);
	gui.Add(sybtb);
	
	gui.Add(szb);
	gui.Add(szbtb);
	
	gui.Add(lab);
	gui.Add(labtb);
	
	gui.Add(deleteb);
	
	return gui;
}

gui::Gui SetupSaveLoadDialog()
{
	gui::Gui gui;
	
	pnameb.SetTextColor(sf::Color::Black);
	
	gui.Add(background);
	gui.Add(ok);
	
	gui.Add(pnameb);
	gui.Add(pnamebtb);
	
	return gui;
}
