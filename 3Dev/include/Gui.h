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
		void SetTextColor(int ID, sf::Color color);
		void SetTextSize(int ID, int size);
		void SetFont(int ID, std::string filename);
		
		void SetColor(int ID, sf::Color color);
		void SetOutlineColor(int ID, sf::Color color);
		void SetOutlineThickness(int ID, int outlineThickness);
		
		void SetPosition(int ID, float x, float y);
		void SetSize(int ID, float w, float h);
		
		void Draw(sf::RenderWindow& w);
		
		std::string GetTextBoxString(int id);
	private:
		std::vector<gui::Button> buttons;
		std::vector<gui::TextBox> textboxes;
	};
}

