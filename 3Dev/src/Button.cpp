#include "Button.h"

gui::Button::Button(float x, float y, float w, float h, std::string text, float fontSize, int id)
{
	ID = id;
	font.loadFromFile("arial.ttf");
	this->text.setFont(font);
	this->text.setCharacterSize(fontSize);
	this->text.setString(text);
	this->text.setPosition(x + 2, y);
	shape.setFillColor(sf::Color::Black);
	shape.setSize(sf::Vector2f(w, h));
	shape.setPosition(x, y);
}

void gui::Button::SetFont(std::string filename)
{
	font.loadFromFile(filename);
	text.setFont(font);
}

void gui::Button::SetColor(sf::Color color)
{
	shape.setFillColor(color);
}

void gui::Button::SetText(std::string text)
{
	this->text.setString(text);
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
