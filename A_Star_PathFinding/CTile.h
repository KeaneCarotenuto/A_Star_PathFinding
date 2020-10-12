#pragma once
#include "SFML/Graphics.hpp"
#include <iostream>

enum class TileType
{
	Empty,
	EmptySearching,
	EmptySearched,
	Wall,
	Start,
	End
};

struct ArrayPos
{
public:
	int x;
	int y;
};

class CTile:
	public sf::Drawable

	
{
public:
	sf::Sprite* sprite;
	sf::Texture texture;
	TileType type;

	ArrayPos arrayPos;

	float value = 0;

	CTile* previous = nullptr;
	std::vector<CTile*> next;

	CTile(TileType _type, sf::Vector2f _pos);

	void SetType(TileType _type);

	static bool IsSmaller (const CTile* _tile, const CTile* _tile2)
	{
		return (_tile->value < _tile2->value);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states)const;
};

