#include "CButton.h"

CButton::CButton(sf::RectangleShape* _rect, sf::Text* _text, void(*_function)()) :
	rect(_rect), text(_text), function(_function)
{
	function = *_function;
}

CButton::~CButton()
{
}
