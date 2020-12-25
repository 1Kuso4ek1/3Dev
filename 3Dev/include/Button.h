#pragma once
#include "3Dev.h"
namespace gui {
	class Button
	{
	public:
		int ID;

		Button(float x, float y, float w, float h, std::string text, float fontSize, int id);

		void SetFont(std::string filename);
		void SetColor(sf::Color color);
		void SetText(std::string text);
		void Draw(sf::RenderWindow& w);

		bool isPressed(sf::Event event, sf::Vector2f pos);
	private:
		float x, y, w, h;

		sf::Font font;
		sf::Text text;
		sf::RectangleShape shape;
	};
}

