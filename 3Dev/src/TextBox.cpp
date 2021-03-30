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

void gui::TextBox::SetTextColor(sf::Color color)
{
	text.setFillColor(color);
}

void gui::TextBox::SetText(sf::String text)
{
	this->text.setString(text);
}

void gui::TextBox::SetTextSize(int size)
{
	text.setCharacterSize(size);
}

void gui::TextBox::SetTextOutlineColor(sf::Color color)
{
	text.setOutlineColor(color);
}

void gui::TextBox::SetTextOutlineThickness(int outlineThickness)
{
	text.setOutlineThickness(outlineThickness);
}

void gui::TextBox::SetTextPosition(float x, float y)
{
	text.setPosition(x, y);
}

void gui::TextBox::SetColor(sf::Color color)
{
	shape.setFillColor(color);
}

void gui::TextBox::SetOutlineColor(sf::Color color)
{
	shape.setOutlineColor(color);
}

void gui::TextBox::SetOutlineThickness(int outlineThickness)
{
	shape.setOutlineThickness(outlineThickness);
}

void gui::TextBox::SetPosition(float x, float y)
{
	shape.setPosition(x, y);
	text.setPosition(x + 2, y);
}

void gui::TextBox::SetSize(float w, float h)
{
	shape.setSize(sf::Vector2f(w, h));	
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
				if(enteredText.getSize() > 0) enteredText.erase(enteredText.getSize() - 1);
			}
		}
		SetText(enteredText);
		if(enteredText.getSize() * text.getCharacterSize() > w * 1.8) {
			w += text.getCharacterSize() / 2;
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
	if (event.type == sf::Event::KeyPressed)
	{
		if(event.key.code == sf::Keyboard::Enter)
		{
			inFocus = false;
		}
	}
}

std::string gui::TextBox::GetText()
{
	return enteredText;
}
