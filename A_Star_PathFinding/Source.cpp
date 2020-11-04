////////////////////////////////////////////////////////////
//========================================================//
// Bachelor of Software Engineering                       //
// Media Design School                                    //
// Auckland                                               //
// New Zealand                                            //
//--------------------------------------------------------//
// (c) 2020 Media Design School                           //
//========================================================//
//   File Name  :   Source.cpp                   
//--------------------------------------------------------//
//  Description : This program demonstrates an A* path finding algorithm.
//                                                                                                           
//--------------------------------------------------------//
//    Author    : Keane Carotenuto & Nerys Thamm BSE20021                           
//--------------------------------------------------------//
//    E-mail    : 
//		KeaneCarotenuto@gmail.com
//		NerysThamm@gmail.com
//========================================================//
////////////////////////////////////////////////////////////


//Includes
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


//Functions
void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding);

int FixedUpdate();
void AStar();
void ProcessTile(CTile* _tile, std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd, std::vector<CTile*>& toRemove);
void RemoveTileFromVector(CTile* _tile, std::vector<CTile*>& _vector);
bool DoesTileExistInVector(CTile* _tile, std::vector<CTile*>& _vector);
std::vector<CTile*> GetNeighbours(CTile* _tile);
void CreateLine(CTile* _tile, CTile* tempTile);
float FindDistanceToTile(CTile * start, CTile * end);
void CheckButtonsPressed();
void Draw();

CManager manager;

//The Clickable Button's Functions
void StartStopSearch() {
	manager.startFinding = !manager.startFinding;
	manager.Buttons[0]->text->setString(manager.startFinding ? "PAUSE" : "PLAY");
	manager.Buttons[0]->rect->setFillColor(manager.startFinding ? sf::Color::Red : sf::Color::Color(0, 150, 0));
}

//Resets everything excluding walls
void Reset() {
	//Creates new tiles if needed, otherwise simply sets to default
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
			manager.tiles[x][y]->c = std::numeric_limits<float>::max();
			manager.tiles[x][y]->previous = nullptr;
		}
	}

	//Defines important values
	manager.start = manager.tiles[10][20];
	manager.end = manager.tiles[30][20];

	manager.start->c = 0;

	manager.start->SetType(TileType::Start);
	manager.end->SetType(TileType::End);

	//Clears stacks
	manager.searchStack.clear();
	manager.tempStack.clear();
	manager.doneStack.clear();
	manager.searchStack.push_back(manager.start);

	//Removes lines
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

void AllowDiagonal() {
	manager.allowDiagonal = !manager.allowDiagonal;
	manager.Buttons[3]->rect->setFillColor(manager.allowDiagonal ? sf::Color::Color(0, 150, 0) : sf::Color::Red);
}

void Speed() {
	manager.slowed += 5;
	manager.slowed = std::floor(manager.slowed / 5) * 5;
	if (manager.slowed > 60) {
		manager.slowed = 1;
	}
	manager.Buttons[4]->text->setString(std::to_string(60.0f / (float)manager.slowed).substr(0,4) +" steps/s");
}

//Start
int main() {
	//Create Windows
	sf::RenderWindow window(sf::VideoMode(800, 800), "A* Pathfinding - By Keane Carotenuto & Nerys Thamm");
	sf::RenderWindow controlWindow(sf::VideoMode(200, 200), "Controls");
	controlWindow.setPosition(sf::Vector2i(window.getPosition().x +  window.getSize().x, window.getPosition().y));

	manager.window = &window;
	manager.controlWindow = &controlWindow;

	if (!manager.font.loadFromFile("Fonts/Roboto.ttf")) std::cout << "Failed to load Roboto\n";

	//Create Buttons
	CreateButton(&StartStopSearch, "PLAY", 25, sf::Color::White, sf::Text::Style::Bold, 0, 0, sf::Color::Color(0,150,0), 5);
	CreateButton(&Reset, "Reset", 25, sf::Color::White, sf::Text::Style::Bold, 0, 40, sf::Color::Red, 5);
	CreateButton(&ClearWalls, "CLEAR WALLS", 25, sf::Color::White, sf::Text::Style::Bold, 0, 80, sf::Color::Red, 5);
	CreateButton(&AllowDiagonal, "DIAGONAL", 25, sf::Color::White, sf::Text::Style::Bold, 0, 120, sf::Color::Red, 5);
	CreateButton(&Speed, "60.0 steps/s", 25, sf::Color::White, sf::Text::Style::Bold, 0, 160, sf::Color::Red, 5);
	
	Reset();

	float stepTime = 0;
	bool drawn = false;
	
	//Game loop
	while (window.isOpen() == true)
	{
		stepTime += manager.clock.getElapsedTime().asSeconds();
		manager.clock.restart();

		while (stepTime >= manager.step)
		{
			//Main Loop of Game Runs at fixed rate
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

		//Checks for Clickable buttons being pressed
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

//Fixed update runs ata  fixed time step rate
int FixedUpdate()
{
	manager.currentStep ++;
	manager.ToDrawList.clear();
	
	//The algorithm is called
	AStar();

	//Checks for mouse clicks
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
	{
		for (int x = 0; x < 40; x++) {
			for (int y = 0; y < 40; y++) {
				if (manager.tiles[x][y]->type == TileType::Start || manager.tiles[x][y]->type == TileType::End) continue;

				sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);

				//Creates walls, or shifts start point
				if (manager.tiles[x][y]->sprite->getGlobalBounds().contains(mousePos)) {
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && manager.tiles[x][y]->type == TileType::Empty) {
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && manager.startFinding == false) {
							//reset start tile
							manager.start->SetType(TileType::Empty);
							manager.start->c = std::numeric_limits<float>::max();

							//assign new start tile
							manager.tiles[x][y]->SetType(TileType::Start);
							manager.start = manager.tiles[x][y];
							manager.start->c = 0;

							manager.searchStack.clear();
							manager.searchStack.push_back(manager.start);
						}
						else {
							manager.tiles[x][y]->SetType(TileType::Wall);
						}
						
					}
					//Deletes walls or shifts end point
					if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && manager.startFinding == false && manager.tiles[x][y]->type == TileType::Empty) {
							manager.end->SetType(TileType::Empty);
							manager.end = manager.tiles[x][y];
							manager.end->SetType(TileType::End);
						}
						else if (manager.tiles[x][y]->type == TileType::Wall) {
							manager.tiles[x][y]->SetType(TileType::Empty);
						}

						
					}
				}
			}
		}
	}

	//Pushes all items to draw list
	for (int x = 0; x < 40; x++) {
		for (int y = 0; y < 40; y++) {
			manager.ToDrawList.push_back(manager.tiles[x][y]);
		}
	}

	//Pauses the game and draws the final route
	if (manager.foundRoute && !manager.drawnRoute) {
		StartStopSearch();
		manager.drawnRoute = true;

		CTile* currentTile = manager.end;

		//Draws lines between selected tiles from end to start
		while (currentTile->previous != nullptr) {

			for (int i = 0; i <= 100; i++) {
				sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
				lines->operator[](0).position = sf::Vector2f(currentTile->sprite->getPosition().x + 10 + rand() % 2 * (rand() % 2 == 0 ? -1: 1), currentTile->sprite->getPosition().y + 10 + rand() % 2 * (rand() % 2 == 0 ? -1 : 1));
				lines->operator[](0).color = sf::Color::Green;
				lines->operator[](1).position = sf::Vector2f(currentTile->previous->sprite->getPosition().x + 10 + rand() % 2 * (rand() % 2 == 0 ? -1 : 1), currentTile->previous->sprite->getPosition().y + 10 + rand() % 2 * (rand() % 2 == 0 ? -1 : 1));
				lines->operator[](1).color = sf::Color::Blue;

				manager.lines.push_back(lines);
			}

			

			currentTile = currentTile->previous;
			
		}


	}

	//Pushes lines to be drawn
	for (sf::VertexArray* _line : manager.lines) {
		manager.ToDrawList.push_back(_line);
	}

	return 1;
}


void AStar() {
	//if finding, and step is multiple of 6
	if (!manager.foundRoute && manager.startFinding && manager.currentStep % manager.slowed == 0) {

		std::vector<CTile*> toAdd;
		std::vector<CTile*> toRemove;

		//If nore more tiles to search, re-checks done tiles for empty spaces
		if (manager.searchStack.empty()) {
			for (CTile* _tile : manager.doneStack) {
				std::vector<CTile*> neighbours = GetNeighbours(_tile);

				if (!neighbours.empty()) {
					toAdd.push_back(_tile);
				}
				
			}
		}

		//Processes a tile and its neighbours
		for (CTile* _tile : manager.searchStack) {
			if (_tile == manager.searchStack[0]) {
				std::vector<CTile*> neighbours = GetNeighbours(_tile);
				ProcessTile(_tile, neighbours, toAdd, toRemove);
				break;
			}
		}

		//If found end
		done:

		for (CTile* _tile : toAdd) {
			manager.searchStack.push_back(_tile);
		}
		for (CTile* _tile : toRemove) {
			RemoveTileFromVector(_tile, manager.searchStack);
		}

		std::sort(manager.searchStack.begin(), manager.searchStack.end(), CTile::IsSmaller);
	}
}

void ProcessTile(CTile* _tile, std::vector<CTile*>& neighbours, std::vector<CTile*>& toAdd, std::vector<CTile*>& toRemove)
{
	//Checks all neighbours and adds them to the search list
	for (CTile* _neighbour : neighbours) {
		if (!DoesTileExistInVector(_neighbour, manager.searchStack) && !DoesTileExistInVector(_neighbour, manager.doneStack) && !DoesTileExistInVector(_neighbour, toAdd)) {
			toAdd.push_back(_neighbour);
			if (_neighbour->type != TileType::Start && _neighbour->type != TileType::End) _neighbour->SetType(TileType::EmptySearching);
		}

		//Checks if neighbour is the goal tile
		if (_neighbour == manager.end) {
			manager.foundRoute = true;
		}
	}
	
	//Sets the style of a tile, and moves done tiles to done stack
	if (_tile->type != TileType::Start && _tile->type != TileType::End) _tile->SetType(TileType::EmptySearched);
	if (!DoesTileExistInVector(_tile, manager.doneStack)) manager.doneStack.push_back(_tile);
	toRemove.push_back(_tile);
}

//Removes a tile for a vector
void RemoveTileFromVector(CTile* _tile, std::vector<CTile*>& _vector)
{
	std::vector<CTile*>::iterator pos = std::find(_vector.begin(), _vector.end(), _tile);
	if (pos != _vector.end()) {
		_vector.erase(pos);
	}
}

//Checks if tile exists in vector
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

//Finds all neighbours and calcualtes thier values
std::vector<CTile*> GetNeighbours(CTile* _tile) {
	std::vector<CTile*> neigh;

	//If diagonal or not, do in specific pattern (kinda long)
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

		//Discards bad neighbours
		if (tempx < 0 || tempx > 39 || tempy < 0 || tempy > 39) continue;

		CTile* tempN = manager.tiles[tempx][tempy];

		if (DoesTileExistInVector(tempN, manager.doneStack)) continue;
		if (tempN->type == TileType::Wall) continue;

		//Calculates values
		float tempC = _tile->c + FindDistanceToTile(_tile, tempN);
		if (tempN->c > tempC) {
			tempN->c = tempC;
			tempN->previous = _tile;
			CreateLine(_tile, tempN);
		}
		tempN->d = FindDistanceToTile(tempN, manager.end);
		tempN->f = tempN->c + tempN->d;
		

		//Adds to list
		neigh.push_back(tempN);
	}

	return neigh;
}

//Creates a line between two tiles
void CreateLine(CTile* _tile, CTile* _tile2)
{
	sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
	lines->operator[](0).position = sf::Vector2f(_tile->sprite->getPosition().x + 10, _tile->sprite->getPosition().y + 10);
	lines->operator[](0).color = sf::Color::Color(0,0,255, 100);
	lines->operator[](1).position = sf::Vector2f(_tile2->sprite->getPosition().x + 10, _tile2->sprite->getPosition().y + 10);
	lines->operator[](1).color = sf::Color::Color(255, 0, 0, 100);
	manager.lines.push_back(lines);
}

//Finds dist between two tiles depending on Diag mode or not
float FindDistanceToTile(CTile * start, CTile * end)
{
	float dist = 0;
	if (manager.allowDiagonal && end != manager.end) {
		dist = std::sqrt((start->arrayPos.x - end->arrayPos.x) * (start->arrayPos.x - end->arrayPos.x) + (start->arrayPos.y - end->arrayPos.y) * (start->arrayPos.y - end->arrayPos.y));
	}
	else {
		dist = std::abs(start->arrayPos.x - end->arrayPos.x) + std::abs(start->arrayPos.y - end->arrayPos.y);
	}
	
	return dist;
}


//Checks if a button has been pressed then executes function tied to button
void CheckButtonsPressed()
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
		
		if (!manager.frozenClick) {
			for (CButton* _button : manager.Buttons)
			{

				//If click, do func
				if (_button->rect->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.controlWindow))) {
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

//Creates a clickable button with a function
void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding)
{
	sf::Text* tempText = new sf::Text;
	tempText->setString(_string);
	tempText->setCharacterSize(_fontSize);
	tempText->setFillColor(_tColour);
	tempText->setStyle(_style);
	tempText->setFont(manager.font);

	tempText->setPosition(100 - (tempText->getGlobalBounds().width) / 2, _y);
	
	
	sf::RectangleShape* buttonRect = new sf::RectangleShape;
	buttonRect->setPosition(0, tempText->getGlobalBounds().top - _padding);
	buttonRect->setSize(sf::Vector2f(200, 30));
	buttonRect->setFillColor(_bgColour);

	CButton* button = new CButton(buttonRect, tempText, function);
	manager.Buttons.push_back(button);

	//manager.controlWindow->setSize(sf::Vector2u( (float)200, (float)buttonRect->getGlobalBounds().top + (float)buttonRect->getGlobalBounds().height));
}

//Draws everything that needs to be drawn to screen
void Draw()
{
	manager.window->clear();

	for (sf::Drawable* item : manager.ToDrawList) {
		manager.window->draw(*item);
	}

	manager.window->display();


	manager.controlWindow->clear();

	for (CButton* button : manager.Buttons) {
		manager.controlWindow->draw(*button->rect);
		button->text->setFont(manager.font);
		manager.controlWindow->draw(*button->text);
	}

	manager.controlWindow->display();
	
}
