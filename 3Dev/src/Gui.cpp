#include "Gui.h"

int gui::Gui::CatchEvent(sf::Event& event, sf::RenderWindow& w)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(w);
	sf::Vector2f pos = w.mapPixelToCoords(pixelPos);
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].isPressed(event, pos)) {
			return buttons[i].ID;
		}
	}
	return -1;
}

void gui::Gui::AddButton(gui::Button& button)
{
	buttons.push_back(button);
}

void gui::Gui::SetText(int ID, std::string text)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetText(text);
		}
	}
}

void gui::Gui::SetColor(int ID, sf::Color color)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetColor(color);
		}
	}
}

void gui::Gui::Draw(sf::RenderWindow& w)
{
	for (int i = 0; i < buttons.size(); i++) {
		buttons[i].Draw(w);
	}
}
