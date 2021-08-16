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

void gui::Gui::SetText(int ID, sf::String text)
{
	Find(ID, std::function<void(gui::Button&, sf::String)>(&gui::Button::SetText), std::function<void(gui::TextBox&, sf::String)>(&gui::TextBox::SetText), text);
}

void gui::Gui::SetEnteredText(int ID, sf::String text)
{
	for(auto& i : textboxes) {
		if(i.ID == ID) i.SetEnteredText(text);
	}
}

void gui::Gui::SetTextColor(int ID, sf::Color color)
{
	Find(ID, std::function<void(gui::Button&, sf::Color)>(&gui::Button::SetTextColor), std::function<void(gui::TextBox&, sf::Color)>(&gui::TextBox::SetTextColor), color);
}

void gui::Gui::SetTextSize(int ID, int size)
{
	Find(ID, std::function<void(gui::Button&, int)>(&gui::Button::SetTextSize), std::function<void(gui::TextBox&, int)>(&gui::TextBox::SetTextSize), size);
}

void gui::Gui::SetFont(int ID, std::string filename)
{
	Find(ID, std::function<void(gui::Button&, std::string)>(&gui::Button::SetFont), std::function<void(gui::TextBox&, std::string)>(&gui::TextBox::SetFont), filename);
}

void gui::Gui::SetTextOutlineColor(int ID, sf::Color color)
{
	Find(ID, std::function<void(gui::Button&, sf::Color)>(&gui::Button::SetTextOutlineColor), std::function<void(gui::TextBox&, sf::Color)>(&gui::TextBox::SetTextOutlineColor), color);
}

void gui::Gui::SetTextOutlineThickness(int ID, int outlineThickness)
{
	Find(ID, std::function<void(gui::Button&, int)>(&gui::Button::SetTextOutlineThickness), std::function<void(gui::TextBox&, int)>(&gui::TextBox::SetTextOutlineThickness), outlineThickness);
}

void gui::Gui::SetTextPosition(int ID, float x, float y)
{
	Find(ID, std::function<void(gui::Button&, float, float)>(&gui::Button::SetTextPosition), std::function<void(gui::TextBox&, float, float)>(&gui::TextBox::SetTextPosition), x, y);
}

void gui::Gui::SetColor(int ID, sf::Color color)
{
	Find(ID, std::function<void(gui::Button&, sf::Color)>(&gui::Button::SetColor), std::function<void(gui::TextBox&, sf::Color)>(&gui::TextBox::SetColor), color);
}

void gui::Gui::SetOutlineColor(int ID, sf::Color color)
{
	Find(ID, std::function<void(gui::Button&, sf::Color)>(&gui::Button::SetOutlineColor), std::function<void(gui::TextBox&, sf::Color)>(&gui::TextBox::SetOutlineColor), color);
}

void gui::Gui::SetOutlineThickness(int ID, int outlineThickness)
{
	Find(ID, std::function<void(gui::Button&, int)>(&gui::Button::SetOutlineThickness), std::function<void(gui::TextBox&, int)>(&gui::TextBox::SetOutlineThickness), outlineThickness);
}

void gui::Gui::SetPosition(int ID, float x, float y)
{
	Find(ID, std::function<void(gui::Button&, float, float)>(&gui::Button::SetPosition), std::function<void(gui::TextBox&, float, float)>(&gui::TextBox::SetPosition), x, y);
}

void gui::Gui::SetSize(int ID, float w, float h)
{
	Find(ID, std::function<void(gui::Button&, float, float)>(&gui::Button::SetSize), std::function<void(gui::TextBox&, float, float)>(&gui::TextBox::SetSize), w, h);
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

sf::String gui::Gui::GetTextBoxSfString(int id)
{
	for(auto i : textboxes) {
		if(i.ID == id) return i.GetSfString();
	}
	return "";
}

template<class T>
void gui::Gui::Find(int ID, std::function<void(gui::Button&, T)> f1, std::function<void(gui::TextBox&, T)> f2, T a)
{
	for (auto& i : buttons)
		if (i.ID == ID) f1(i, a);
		
	for (auto& i : textboxes)
		if (i.ID == ID) f2(i, a);
}

template<class T>
void gui::Gui::Find(int ID, std::function<void(gui::Button&, T, T)> f1, std::function<void(gui::TextBox&, T, T)> f2, T a, T b)
{
	for (auto& i : buttons)
		if (i.ID == ID) f1(i, a, b);
		
	for (auto& i : textboxes)
		if (i.ID == ID) f2(i, a, b);
}
