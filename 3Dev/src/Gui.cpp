#include "Gui.h"

int gui::Gui::CatchEvent(sf::Event& event, sf::RenderWindow& w, bool checkTextBoxes)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(w);
	sf::Vector2f pos = w.mapPixelToCoords(pixelPos);
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].isPressed(event, pos)) {
			return buttons[i].ID;
		}
	}
	if(checkTextBoxes) {
		for (int i = 0; i < textboxes.size(); i++) {
			textboxes[i].TextEntered(event);
			textboxes[i].isPressed(event, pos);
		}
	}
	return -1;
}

void gui::Gui::Add(gui::Button& button)
{
	buttons.push_back(button);
}

void gui::Gui::Add(gui::TextBox& textbox)
{
	textboxes.emplace_back(textbox);
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
	for (int i = 0; i < textboxes.size(); i++) {
		textboxes[i].Draw(w);
	}
}

std::string gui::Gui::GetTextBoxString(int id)
{
	for(auto i : textboxes) {
		if(i.ID == id) return i.GetText();
	}
	return "";
}
