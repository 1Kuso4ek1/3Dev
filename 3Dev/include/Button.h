#pragma once
#include "3Dev.h"
namespace gui {
	class Button
	{
	public:
		int ID;
		bool check;
		
		Button(float x, float y, float w, float h, std::string text, float fontSize, int id, int outlineThickness, bool check = true);

		void SetFont(std::string filename);
		void SetTextColor(sf::Color color);
		void SetText(std::string text);
		void SetTextSize(int size);
		
		void SetColor(sf::Color color);
		void SetOutlineColor(sf::Color color);
		void SetOutlineThickness(int outlineThickness);
		
		void SetPosition(float x, float y);
		void SetSize(float w, float h);
		
		void Draw(sf::RenderWindow& w);

		bool isPressed(sf::Event event, sf::Vector2f pos);
	private:
		sf::Font font;
		sf::Text text;
		sf::RectangleShape shape;
	};
}

