#include <3Dev.h>

namespace gui {
	class TextBox 
	{
	public:
		int ID;
	
		TextBox(float x, float y, float w, float h, float outlineThickness, float fontSize, int id);
		
		void SetFont(std::string filename);
		void SetTextColor(sf::Color color);
		void SetText(std::string text);
		void SetTextSize(int size);
		void SetTextOutlineColor(sf::Color color);
		void SetTextOutlineThickness(int outlineThickness);
		void SetTextPosition(float x, float y);
		
		void SetColor(sf::Color color);
		void SetOutlineColor(sf::Color color);
		void SetOutlineThickness(int outlineThickness);
		
		void SetPosition(float x, float y);
		void SetSize(float w, float h);
		
		void Draw(sf::RenderWindow& w);
		
		void isPressed(sf::Event& event, sf::Vector2f pos);
		void TextEntered(sf::Event& event);
		
		std::string GetText();
	private:
		float x, y, w, h;
		bool inFocus = false;
		std::string enteredText;
				
		sf::Font font;
		sf::Text text;
		sf::RectangleShape shape;
	};
}
