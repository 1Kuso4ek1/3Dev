#pragma once
#include "3Dev.h"
#include "Button.h"
namespace gui {
	class Gui
	{
	public:
		int CatchEvent(sf::Event& event, sf::RenderWindow& w);

		void AddButton(gui::Button& button);
		void SetText(int ID, std::string text);
		void SetColor(int ID, sf::Color color);
		void Draw(sf::RenderWindow& w);
	private:
		std::vector<gui::Button> buttons;
	};
}

