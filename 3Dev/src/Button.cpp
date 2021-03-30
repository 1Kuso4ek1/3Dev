#include "Button.h"

gui::Button::Button(float x, float y, float w, float h, std::string text, float fontSize, int id, int outlineThickness, bool check) : ID(id), check(check)
{
	font.loadFromFile("arial.ttf");
	this->text.setFont(font);
	this->text.setCharacterSize(fontSize);
	this->text.setString(text);
	this->text.setPosition(x + 2, y);
	shape.setFillColor(sf::Color::Black);
	shape.setSize(sf::Vector2f(w, h));
	shape.setPosition(x, y);
	shape.setOutlineThickness(outlineThickness);
	shape.setOutlineColor(sf::Color::Black);
}

void gui::Button::SetFont(std::string filename)
{
	font.loadFromFile(filename);
	text.setFont(font);
}

void gui::Button::SetTextColor(sf::Color color)
{
	text.setFillColor(color);
}

void gui::Button::SetText(std::string text)
{
	this->text.setString(text);
}

void gui::Button::SetTextSize(int size)
{
	text.setCharacterSize(size);
}

void gui::Button::SetTextOutlineColor(sf::Color color)
{
	text.setOutlineColor(color);
}

void gui::Button::SetTextOutlineThickness(int outlineThickness)
{
	text.setOutlineThickness(outlineThickness);
}

void gui::Button::SetTextPosition(float x, float y)
{
	text.setPosition(x, y);
}

void gui::Button::SetColor(sf::Color color)
{
	shape.setFillColor(color);
}

void gui::Button::SetOutlineColor(sf::Color color)
{
	shape.setOutlineColor(color);
}

void gui::Button::SetOutlineThickness(int outlineThickness)
{
	shape.setOutlineThickness(outlineThickness);
}

void gui::Button::SetPosition(float x, float y)
{
	shape.setPosition(x, y);
	text.setPosition(x + 2, y);
}

void gui::Button::SetSize(float w, float h)
{
	shape.setSize(sf::Vector2f(w, h));	
}

void gui::Button::Draw(sf::RenderWindow& w)
{
	w.pushGLStates();
	w.draw(shape);
	w.draw(text);
	w.popGLStates();
}

bool gui::Button::isPressed(sf::Event event, sf::Vector2f pos)
{
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.key.code == sf::Mouse::Left) {
			if (shape.getGlobalBounds().contains(pos.x, pos.y)) {
				return true;
			}
		}
	}
	return false;
}
