#include <3Dev.h>

namespace gui {
	class TextBox 
	{
	public:
		int ID;
	
		TextBox(float x, float y, float w, float h, float outlineThickness, float fontSize, int id);
		
		void SetFont(std::string filename);
		void SetColor(sf::Color color);
		void SetText(std::string text);
		
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
