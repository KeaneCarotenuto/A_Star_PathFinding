#include "CTile.h"

CTile::CTile(TileType _type, sf::Vector2f _pos)
{
	sprite = new sf::Sprite();
	SetType(_type);
	sprite->setPosition(_pos);
}

void CTile::SetType(TileType _type)
{
	type = _type;

	switch (type)
	{
	case TileType::Empty:
		if (!texture.loadFromFile("Sprites/Empty.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	case TileType::EmptySearching:
		if (!texture.loadFromFile("Sprites/Searching.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	case TileType::EmptySearched:
		if (!texture.loadFromFile("Sprites/Searched.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	case TileType::Wall:
		if (!texture.loadFromFile("Sprites/Wall.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	case TileType::Start:
		if (!texture.loadFromFile("Sprites/Start.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	case TileType::End:
		if (!texture.loadFromFile("Sprites/End.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;

	default:
		if (!texture.loadFromFile("Sprites/Empty.png")) std::cout << "Failed to Load Empty Sprite\n";
		break;
	}

	sprite->setTexture(texture);
}

void CTile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(*sprite);
}
