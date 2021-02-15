#include <TextBox.h>

gui::TextBox::TextBox(float x, float y, float w, float h, float outlineThickness, float fontSize, int id) : ID(id), x(x), y(y), w(w), h(h)
{
	font.loadFromFile("arial.ttf");
	text.setFillColor(sf::Color::Black);
	text.setFont(font);
	text.setCharacterSize(fontSize);
	text.setPosition(x + 2, y);
	shape.setFillColor(sf::Color::White);
	shape.setOutlineThickness(outlineThickness);
	shape.setOutlineColor(sf::Color::Black);
	shape.setSize(sf::Vector2f(w, h));
	shape.setPosition(x, y);
}

void gui::TextBox::SetFont(std::string filename)
{
	font.loadFromFile(filename);
	text.setFont(font);
}

void gui::TextBox::SetColor(sf::Color color)
{
	shape.setFillColor(color);
}

void gui::TextBox::SetText(std::string text)
{
	this->text.setString(text);
}

void gui::TextBox::Draw(sf::RenderWindow& w)
{
	w.pushGLStates();
	w.draw(shape);
	w.draw(text);
	w.popGLStates();
}

void gui::TextBox::TextEntered(sf::Event& event)
{
	if(inFocus) {
		if (event.type == sf::Event::TextEntered) {
			if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Backspace)) {
				enteredText += event.text.unicode;
			}
			else {
				if(enteredText.size() > 0) enteredText.erase(enteredText.size() - 1);
			}
		}
		SetText(enteredText);
		if(enteredText.size() * text.getCharacterSize() > w * 2) {
			w += text.getCharacterSize();
			shape.setSize(sf::Vector2f(w, h));
		}
	}
}

void gui::TextBox::isPressed(sf::Event& event, sf::Vector2f pos)
{
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.key.code == sf::Mouse::Left) {
			if (shape.getGlobalBounds().contains(pos.x, pos.y)) {
				inFocus = true;
			}
			else {
				inFocus = false;
			}
		}
	}
}

std::string gui::TextBox::GetText()
{
	return enteredText;
}
