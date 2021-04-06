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
		
		void SetText(int ID, sf::String text);
		void SetTextColor(int ID, sf::Color color);
		void SetTextSize(int ID, int size);
		void SetFont(int ID, std::string filename);
		void SetTextOutlineColor(int ID, sf::Color color);
		void SetTextOutlineThickness(int ID, int outlineThickness);
		void SetTextPosition(int ID, float x, float y);
		
		void SetColor(int ID, sf::Color color);
		void SetOutlineColor(int ID, sf::Color color);
		void SetOutlineThickness(int ID, int outlineThickness);
		
		void SetPosition(int ID, float x, float y);
		void SetSize(int ID, float w, float h);
		
		void Draw(sf::RenderWindow& w);
		
		std::string GetTextBoxString(int id);
	private:
		template<class T>
		void Find(int ID, std::function<void(gui::Button&, T)> f1, std::function<void(gui::TextBox&, T)> f2, T a);
		
		template<class T>
		void Find(int ID, std::function<void(gui::Button&, T, T)> f1, std::function<void(gui::TextBox&, T, T)> f2, T a, T b);
		
		std::vector<gui::Button> buttons;
		std::vector<gui::TextBox> textboxes;
	};
}

