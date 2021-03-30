#include "Gui.h"

int gui::Gui::CatchEvent(sf::Event& event, sf::RenderWindow& w, bool checkTextBoxes)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(w);
	sf::Vector2f pos = w.mapPixelToCoords(pixelPos);
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].isPressed(event, pos) && buttons[i].check) {
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
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetText(text);
		}
	}
}

void gui::Gui::SetTextColor(int ID, sf::Color color)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetTextColor(color);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetTextColor(color);
		}
	}
}

void gui::Gui::SetTextSize(int ID, int size)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetTextSize(size);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetTextSize(size);
		}
	}
}

void gui::Gui::SetFont(int ID, std::string filename)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetFont(filename);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetFont(filename);
		}
	}
}

void gui::Gui::SetTextOutlineColor(int ID, sf::Color color)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetTextOutlineColor(color);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetTextOutlineColor(color);
		}
	}
}

void gui::Gui::SetTextOutlineThickness(int ID, int outlineThickness)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetTextOutlineThickness(outlineThickness);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetTextOutlineThickness(outlineThickness);
		}
	}
}

void gui::Gui::SetTextPosition(int ID, float x, float y)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetTextPosition(x, y);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetTextPosition(x, y);
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
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetColor(color);
		}
	}
}

void gui::Gui::SetOutlineColor(int ID, sf::Color color)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetOutlineColor(color);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetOutlineColor(color);
		}
	}
}

void gui::Gui::SetOutlineThickness(int ID, int outlineThickness)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetOutlineThickness(outlineThickness);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetOutlineThickness(outlineThickness);
		}
	}
}

void gui::Gui::SetPosition(int ID, float x, float y)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetPosition(x, y);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetPosition(x, y);
		}
	}
}

void gui::Gui::SetSize(int ID, float w, float h)
{
	for (int i = 0; i < buttons.size(); i++) {
		if (buttons[i].ID == ID) {
			buttons[i].SetSize(w, h);
		}
	}
	for (int i = 0; i < textboxes.size(); i++) {
		if (textboxes[i].ID == ID) {
			textboxes[i].SetSize(w, h);
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
