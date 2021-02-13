#pragma once
#include "3Dev.h"
#include "Button.h"
#include "TextBox.h"
namespace gui {
	class Gui
	{
	public:
		int CatchEvent(sf::Event& event, sf::RenderWindow& w, bool checkTextBoxes = true);

		void Add(gui::Button& button);
		void Add(gui::TextBox& textbox);
		void SetText(int ID, std::string text);
		void SetColor(int ID, sf::Color color);
		void Draw(sf::RenderWindow& w);
		
		std::string GetTextBoxString(int id);
	private:
		std::vector<gui::Button> buttons;
		std::vector<gui::TextBox> textboxes;
	};
}

