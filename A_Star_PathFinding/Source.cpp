#include <vector>
#include <string>
#include <iostream>
#include <time.h>
#include <math.h>
#include <typeinfo>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <Windows.h>

#include "CManager.h"
#include "CTile.h"



void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding);

int FixedUpdate();
void BreadthFirst();
void AStar();
void ProcessTile(CTile* _tile, std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd, std::vector<CTile*>& toRemove);
bool AreAllNeighboursDone(std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd);
void AddToSearch(std::vector<CTile*>& toAdd, CTile* _tile, CTile* _neighbour);
void RemoveTileFromVector(CTile* _tile, std::vector<CTile*>& _vector);
bool DoesTileExistInVector(CTile* _tile, std::vector<CTile*>& _vector);
std::vector<CTile*> GetNeighbours(CTile* _tile);
void CreateLine(CTile* _tile, CTile* tempTile);
float FindDistanceToTile(CTile * start, CTile * end);
void CheckButtonsPressed();
void Draw();

CManager manager;

void StartStopSearch() {
	manager.startFinding = !manager.startFinding;
	manager.Buttons[0]->text->setString(manager.startFinding ? "Stop" : "Start");
	manager.Buttons[0]->rect->setFillColor(manager.startFinding ? sf::Color::Red : sf::Color::Color(0, 150, 0));
	//manager.step = (manager.startFinding ? (1.0f / 5.0f) : (1.0f / 60.0f));
}

void ClearSearch() {
	for (int x = 0; x < 40; x++) {
		for (int y = 0; y < 40; y++) {
			if (manager.tiles[x][y] == nullptr) {
				CTile* myTile = new CTile(TileType::Empty, { (float)x * 20,(float)y * 20 });
				manager.tiles[x][y] = myTile;
			}
			else {
				if (manager.tiles[x][y]->type == TileType::Wall) continue;
				manager.tiles[x][y]->SetType(TileType::Empty);
			}
			manager.tiles[x][y]->arrayPos = { x,y };
		}
	}

	manager.start = manager.tiles[10][20];
	manager.end = manager.tiles[30][20];

	manager.start->SetType(TileType::Start);
	manager.end->SetType(TileType::End);

	manager.searchStack.clear();
	manager.tempStack.clear();
	manager.doneStack.clear();
	manager.searchStack.push_back(manager.start);

	for (sf::VertexArray* _vert : manager.lines)
	{
		delete _vert;
	}
	manager.lines.clear();

	manager.foundRoute = false;
	manager.drawnRoute = false;
}

void ClearWalls() {
	for (int x = 0; x < 40; x++) {
		for (int y = 0; y < 40; y++) {
			if (manager.tiles[x][y]->type == TileType::Wall) manager.tiles[x][y]->SetType(TileType::Empty);
		}
	}
}

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 800), "A* Pathfinding - By Keane Carotenuto");
	manager.window = &window;

	if (!manager.font.loadFromFile("Fonts/Roboto.ttf")) std::cout << "Failed to load Roboto\n";

	CreateButton(&StartStopSearch, "Start", 25, sf::Color::White, sf::Text::Style::Regular, 0, 0, sf::Color::Color(0,150,0), 5);
	CreateButton(&ClearSearch, "Clear Search", 25, sf::Color::White, sf::Text::Style::Regular, 0, 40, sf::Color::Red, 5);
	CreateButton(&ClearWalls, "Clear Walls", 25, sf::Color::White, sf::Text::Style::Regular, 0, 80, sf::Color::Red, 5);
	
	ClearSearch();

	float stepTime = 0;
	bool drawn = false;
	

	while (window.isOpen() == true)
	{
		stepTime += manager.clock.getElapsedTime().asSeconds();
		manager.clock.restart();

		while (stepTime >= manager.step)
		{
			//Main Loop of Game
			if (FixedUpdate() == 0) return 0;

			stepTime -= manager.step;
			drawn = false;
		}

		//Draws After Updates
		if (drawn)
		{
			//sf::sleep(sf::seconds(0.01f));
		}
		else
		{
			Draw();

			drawn = true;
		}

		CheckButtonsPressed();


		sf::Event newEvent;

		//Quit
		while (window.pollEvent(newEvent))
		{
			if (newEvent.type == sf::Event::Closed)
			{
				window.close();
			}
		}
	}

	return 0;
}

int FixedUpdate()
{
	manager.currentStep ++;
	manager.ToDrawList.clear();

	//Breadth First Search
	BreadthFirst();
	
	//AStar();

	

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
	{
		for (int x = 0; x < 40; x++) {
			for (int y = 0; y < 40; y++) {
				if (manager.tiles[x][y]->type == TileType::Start || manager.tiles[x][y]->type == TileType::End) continue;

				sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);

				if (manager.tiles[x][y]->sprite->getGlobalBounds().contains(mousePos)) {
					manager.tiles[x][y]->SetType(
						sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
						manager.tiles[x][y]->type == TileType::Empty ?
						TileType::Wall : 
						(sf::Mouse::isButtonPressed(sf::Mouse::Right) &&
						manager.tiles[x][y]->type == TileType::Wall ? TileType::Empty : manager.tiles[x][y]->type));
				}
			}
		}
	}


	for (int x = 0; x < 40; x++) {
		for (int y = 0; y < 40; y++) {
			manager.ToDrawList.push_back(manager.tiles[x][y]);
		}
	}

	if (manager.foundRoute && !manager.drawnRoute) {
		manager.drawnRoute = true;

		CTile* currentTile = manager.end;

		while (currentTile->previous != nullptr) {

			sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
			lines->operator[](0).position = sf::Vector2f(currentTile->sprite->getPosition().x + 10, currentTile->sprite->getPosition().y + 10);
			lines->operator[](0).color = sf::Color::Green;
			lines->operator[](1).position = sf::Vector2f(currentTile->previous->sprite->getPosition().x + 10, currentTile->previous->sprite->getPosition().y + 10);
			lines->operator[](1).color = sf::Color::Green;

			manager.lines.push_back(lines);

			currentTile = currentTile->previous;
		}


	}

	for (sf::VertexArray* _line : manager.lines) {
		manager.ToDrawList.push_back(_line);
	}

	return 1;
}

void BreadthFirst()
{
	if (!manager.foundRoute && manager.startFinding && manager.currentStep % 6 == 0) {
		std::vector<CTile*> toUpdate;

		for (int x = 0; x < 40; x++) {
			for (int y = 0; y < 40; y++) {

				if (manager.tiles[x][y]->type == TileType::Start || manager.tiles[x][y]->type == TileType::EmptySearched) {

					int tempx = 0;
					int tempy = 0;
					for (int i = 0; i < 4; i++) {
						//Sleep(100);

						switch (i)
						{

						case 0:
							tempx = x + 1;
							tempy = y;
							break;

						case 1:
							tempx = x;
							tempy = y + 1;
							break;

						case 2:
							tempx = x - 1;
							tempy = y;
							break;

						case 3:
							tempx = x;
							tempy = y - 1;
							break;
						}

						if (tempx < 0 || tempx > 39 || tempy < 0 || tempy > 39 || manager.tiles[tempx][tempy]->type == TileType::EmptySearching) continue;

						if (manager.tiles[tempx][tempy]->type == TileType::Empty || manager.tiles[tempx][tempy]->type == TileType::End) {
							manager.tiles[x][y]->next.push_back(manager.tiles[tempx][tempy]);

							manager.tiles[tempx][tempy]->previous = manager.tiles[x][y];

							sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
							lines->operator[](0).position = sf::Vector2f(manager.tiles[x][y]->sprite->getPosition().x + 10, manager.tiles[x][y]->sprite->getPosition().y + 10);
							lines->operator[](0).color = sf::Color::Red;
							lines->operator[](1).position = sf::Vector2f(manager.tiles[tempx][tempy]->sprite->getPosition().x + 10, manager.tiles[tempx][tempy]->sprite->getPosition().y + 10);
							lines->operator[](1).color = sf::Color::Red;

							manager.lines.push_back(lines);

							if (manager.tiles[tempx][tempy]->type == TileType::End) {
								manager.foundRoute = true;
								goto update;
							}
							else {
								toUpdate.push_back(manager.tiles[tempx][tempy]);
								manager.tiles[tempx][tempy]->SetType(TileType::EmptySearching);
							}
						}
					}
				}
			}
		}

	update:
		for (CTile* _tile : toUpdate) {
			_tile->SetType(TileType::EmptySearched);
		}
	}
}

void AStar() {
	//if finding, and step is multiple of 6
	if (!manager.foundRoute && manager.startFinding && manager.currentStep % 1 == 0) {

		std::vector<CTile*> toAdd;
		std::vector<CTile*> toRemove;

		for (CTile* _tile : manager.searchStack) {
			
			std::vector<CTile*> neighbours = GetNeighbours(_tile);

			if (_tile == manager.searchStack[0]) {
				ProcessTile(_tile, neighbours, toAdd, toRemove);
			}
			else if (AreAllNeighboursDone(neighbours, toAdd)) {
				ProcessTile(_tile, neighbours, toAdd, toRemove);
			}

			
		}

		done:

		for (CTile* _tile : toAdd) {
			manager.searchStack.push_back(_tile);
		}
		for (CTile* _tile : toRemove) {
			RemoveTileFromVector(_tile, manager.searchStack);
		}

		std::sort(manager.searchStack.begin(), manager.searchStack.end(), CTile::IsSmaller);
		//std::reverse(manager.searchStack.begin(), manager.searchStack.end());
	}
}

void ProcessTile(CTile* _tile, std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd, std::vector<CTile*>& toRemove)
{
	

	for (CTile* _neighbour : neighbours) {
		if (!DoesTileExistInVector(_neighbour, manager.searchStack) && !DoesTileExistInVector(_neighbour, manager.doneStack) && !DoesTileExistInVector(_neighbour, toAdd)) {
			AddToSearch(toAdd, _tile, _neighbour);
		}

		if (_neighbour == manager.end) {
			manager.foundRoute = true;

			return;
		}
	}

	bool allNeighbhoursDone = AreAllNeighboursDone(neighbours, toAdd);;
	

	if (allNeighbhoursDone || _tile) {
		manager.doneStack.push_back(_tile);
		_tile->SetType(TileType::EmptySearched);
		toRemove.push_back(_tile);
	}
}



















///////////////////////////////////////////////////////// DOOOOOOOOOOOOOOOO A DISTANCE CHECK OR SOMETHING HERE SO CLOSER NEIGHBHOURS CAN BE OVER RIDDEN BY BETTER ONES


bool AreAllNeighboursDone(std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd)
{
	bool done = true;
	for (CTile* _neighbour : neighbours) {
		if (/*!DoesTileExistInVector(_neighbour, manager.searchStack) &&*/ !DoesTileExistInVector(_neighbour, manager.doneStack) /*&& !DoesTileExistInVector(_neighbour, toAdd)*/) {
			done = false;
		}
	}

	return done;
}

void AddToSearch(std::vector<CTile*>& toAdd, CTile* _tile, CTile* _neighbour)
{
	_neighbour->value = 14 * FindDistanceToTile(_neighbour, manager.end) + 10 * FindDistanceToTile(_neighbour, manager.start);
	toAdd.push_back(_neighbour);
	CreateLine(_tile, _neighbour);

	_tile->next.push_back(_neighbour);
	_neighbour->previous = _tile;

}

void RemoveTileFromVector(CTile* _tile, std::vector<CTile*>& _vector)
{
	std::vector<CTile*>::iterator pos = std::find(_vector.begin(), _vector.end(), _tile);
	if (pos != _vector.end()) {
		_vector.erase(pos);
	}
}

bool DoesTileExistInVector(CTile* _tile, std::vector<CTile*>& _vector)
{
	if (_vector.empty()) return false;
	std::vector<CTile*>::iterator pos = std::find(_vector.begin(), _vector.end(), _tile);
	if (pos != _vector.end()) {
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<CTile*> GetNeighbours(CTile* _tile) {
	std::vector<CTile*> neigh;

	for (int i = 0; i < (manager.allowDiagonal ? 8 : 4); i++) {

		int tempx = 0;
		int tempy = 0;
		switch (i)
		{
		case 0:
			tempx = _tile->arrayPos.x + 1;
			tempy = _tile->arrayPos.y;
			break;

		case 1:
			tempx = _tile->arrayPos.x;
			tempy = _tile->arrayPos.y + 1;
			break;

		case 2:
			tempx = _tile->arrayPos.x - 1;
			tempy = _tile->arrayPos.y;
			break;

		case 3:
			tempx = _tile->arrayPos.x;
			tempy = _tile->arrayPos.y - 1;
			break;

		case 4:
			tempx = _tile->arrayPos.x + 1;
			tempy = _tile->arrayPos.y + 1;
			break;

		case 5:
			tempx = _tile->arrayPos.x - 1;
			tempy = _tile->arrayPos.y + 1;
			break;

		case 6:
			tempx = _tile->arrayPos.x - 1;
			tempy = _tile->arrayPos.y - 1;
			break;

		case 7:
			tempx = _tile->arrayPos.x + 1;
			tempy = _tile->arrayPos.y - 1;
			break;
		}

		if (tempx < 0 || tempx > 39 || tempy < 0 || tempy > 39) continue;
		if (manager.tiles[tempx][tempy]->type == TileType::Wall) continue;

		neigh.push_back(manager.tiles[tempx][tempy]);
	}

	return neigh;
}

void CreateLine(CTile* _tile, CTile* _tile2)
{
	sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
	lines->operator[](0).position = sf::Vector2f(_tile->sprite->getPosition().x + 10, _tile->sprite->getPosition().y + 10);
	lines->operator[](0).color = sf::Color::Red;
	lines->operator[](1).position = sf::Vector2f(_tile2->sprite->getPosition().x + 10, _tile2->sprite->getPosition().y + 10);
	lines->operator[](1).color = sf::Color::Red;
	manager.lines.push_back(lines);
}

float FindDistanceToTile(CTile * start, CTile * end)
{
	float dist = 0;
	if (manager.allowDiagonal) {
		dist = std::sqrt((start->arrayPos.x - end->arrayPos.x) * (start->arrayPos.x - end->arrayPos.x) + (start->arrayPos.y - end->arrayPos.y) * (start->arrayPos.y - end->arrayPos.y));
	}
	else {
		dist = std::abs(start->arrayPos.x - end->arrayPos.x) + std::abs(start->arrayPos.y - end->arrayPos.y);
	}
	
	return dist;
}



void CheckButtonsPressed()
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
		
		if (!manager.frozenClick) {
			std::cout << "Click\n";
			for (CButton* _button : manager.Buttons)
			{

				//If click, do func
				if (_button->rect->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
					std::cout << "ClickED\n";
					if (_button->function != nullptr) _button->function();
				}
			}
		}
		manager.frozenClick = true;
	}
	else {
		manager.frozenClick = false;
	}
}

void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding)
{
	sf::Text* tempText = new sf::Text;
	tempText->setString(_string);
	tempText->setCharacterSize(_fontSize);
	tempText->setFillColor(_tColour);
	tempText->setStyle(_style);
	tempText->setPosition(_x, _y);
	tempText->setFont(manager.font);

	sf::RectangleShape* buttonRect = new sf::RectangleShape;
	buttonRect->setPosition(tempText->getGlobalBounds().left - _padding, tempText->getGlobalBounds().top - _padding);
	buttonRect->setSize(sf::Vector2f(tempText->getGlobalBounds().width + (2 * _padding), tempText->getGlobalBounds().height + (2 * _padding)));
	buttonRect->setFillColor(_bgColour);

	CButton* button = new CButton(buttonRect, tempText, function);
	manager.Buttons.push_back(button);
}

void Draw()
{
	manager.window->clear();

	for (sf::Drawable* item : manager.ToDrawList) {
		manager.window->draw(*item);
	}

	for (CButton* button : manager.Buttons) {
		manager.window->draw(*button->rect);
		button->text->setFont(manager.font);
		manager.window->draw(*button->text);
	}

	manager.window->display();
}
