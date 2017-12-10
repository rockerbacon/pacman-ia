#include "world.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace lab309;

/*WORLD*/
World::Cell::Cell (void) {
	this->contents = 0;
	this->trail = 0;
}

World::Cell::Cell (int contents) {
	this->contents = contents;
	this->trail = 0;
}

World::World (const Window &window, size_t navmeshWidth, size_t navmeshHeight) {
	this->window = &window;
	this->navmesh = Matrix<Cell>(navmeshHeight, navmeshWidth);
	for (size_t i = 0; i < navmeshHeight; i++) {
		for (size_t j = 0; j < navmeshWidth; j++) {
			this->navmesh[i][j] = 0;
		}
	}
}

World::World (const Window &window) {
	this->window = &window;
}

Vector<int> World::mapToNavmesh (const Vector<float> &pos) const {
	if (pos[_X] < 0 || pos[_Y] < 0) {
		return {-1, -1};
	}
	int i, j;
	int cellWidth = this->getCellWidth();
	int cellHeight = this->getCellHeight();
	Vector<int> v = (Vector<int>)pos;
	
	v[_X] = (v[_X] - v[_X] % this->getCellWidth())/this->getCellWidth();
	v[_Y] = (v[_Y] - v[_Y] % this->getCellHeight())/this->getCellHeight();
	
	return v;
}

Vector<float> World::mapToPixel (const Vector<int> &pos) const {
	Vector<float> pixel(2);
	
	pixel[_X] = pos[_X]*this->getCellWidth();
	pixel[_Y] = pos[_Y]*this->getCellHeight();
	
	return pixel;
}

int World::getCellWidth (void) const {
	return this->window->getWidth()/this->navmesh.getColums();
}

int World::getCellHeight (void) const {
	return this->window->getHeight()/this->navmesh.getLines();
}

World::Cell World::getCell (const Vector<int> &pos) const {
	if (pos[_X] >= 0 && pos[_X] < this->navmesh.getColums() && pos[_Y] >= 0 && pos[_Y] < this->navmesh.getLines()) {
		return this->navmesh[pos];
	}
	return Cell(WALL_ID);
}

void World::add (int id, const Vector<int> &pos) {
	if (pos[_X] >= 0 && pos[_X] < this->navmesh.getColums() && pos[_Y] >= 0 && pos[_Y] < this->navmesh.getLines()) {
		this->navmesh[pos].contents |= id;
	}
}

std::list<Vector<float>> World::getFromMesh (int content) const {
	Vector<int> pos(2);
	std::list<Vector<float>> l;
	for (pos[_Y] = 0; pos[_Y] < this->navmesh.getLines(); pos[_Y]++) {
		for (pos[_X] = 0; pos[_X] < this->navmesh.getColums(); pos[_X]++) {
			if ((this->navmesh[pos].contents & content) == content) {
				l.push_front(this->mapToPixel(pos));
			}
		}	
	}
	return l;
}

void World::remove (int id, const Vector<int> &pos) {
	if (pos[_X] >= 0 && pos[_X] < this->navmesh.getColums() && pos[_Y] >= 0 && pos[_Y] < this->navmesh.getLines()) {
		//std::cout << this->navmesh[pos].contents << std::endl;	//debug
		this->navmesh[pos].contents &= ~id;
		//std::cout << this->navmesh[pos].contents << std::endl;	//debug
	}	
}

bool World::readFromFile (const char *filePath) {
	std::fstream input;
	size_t meshWidth, meshHeight;
	char c;
	
	input.open(filePath, std::fstream::in);
	if (input.fail()) {
		return false;
	}
	
	input >> meshWidth;
	input >> meshHeight;
	
	this->navmesh = Matrix<Cell>(meshHeight, meshWidth);
	for (size_t i = 0; i < this->navmesh.getLines(); i++) {
		for (size_t j = 0; j < this->navmesh.getColums(); j++) {
			//ignorar espacos, tabs e fins de linha
			do {
				input >> c;
			} while (c==' '||c=='\n'||c=='\t');
			//preencher celula da malha
			switch(c) {
				case WALL_CELL:
					this->navmesh[j][i].contents = WALL_ID;
				break;
				case PACMAN_CELL:
					this->navmesh[j][i].contents = PACMAN_ID|PATHWAY_ID;
				break;
				case GHOST_CELL:
					this->navmesh[j][i].contents = GHOST_ID|PATHWAY_ID;
				break;
				case PILL_CELL:
					this->navmesh[j][i].contents = PILL_ID|PATHWAY_ID;
				break;
				case SUPERPILL_CELL:
					this->navmesh[j][i].contents = SUPERPILL_ID|PATHWAY_ID;
				break;
				default:
					this->navmesh[j][i].contents = 0;
				break;
			} 
		}
	}	
	
	return true;
}

/*OBJECT*/
Object::Object (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, int viewDistance, const Vector<float> &initialPos) : Sprite(texture, rectWidth, rectHeight, displayWidth, displayHeight) {
	this->world = world;
	this->id = id;
	this->speed = speed;
	this->viewDistance = viewDistance;
	this->setPos(initialPos);
	this->currentCell = world->mapToNavmesh(initialPos);
	this->moveDirection = {0.0f, 0.0f};
}

void Object::setMoveDirection (const Vector<float> &moveDirection) {
	if (fabs(moveDirection[_X]) > fabs(moveDirection[_Y])) {
		this->moveDirection[_X] = moveDirection[_X]/fabs(moveDirection[_X]);
		this->moveDirection[_Y] = 0.0f;
	} else {
		this->moveDirection[_X] = 0.0f;
		this->moveDirection[_Y] = moveDirection[_Y]/fabs(moveDirection[_Y]);
	}
}

float Object::getSpeed (void) const {
	return this->speed;
}

int Object::getViewDistance (void) const {
	return this->viewDistance;
}

Vector<int> Object::getCurrentCell (void) const {
	return this->currentCell;
}

Vector<float> Object::getMoveDirection (void) const {
	return this->moveDirection;
}

void Object::move (Sprite *wall, double elapsedTime) {
	Vector<float> oldPosition = this->getPos();
	Vector<int> oldCell = this->currentCell;
	Vector<int> nextCell = this->currentCell+this->moveDirection;
	
	//std::cout << this->speed*elapsedTime << std::endl;	//debug
	this->translate(this->speed*elapsedTime*this->moveDirection);
	this->currentCell = this->world->mapToNavmesh(this->getCenter());
	
	//std::cout << this->currentCell[_X] << " " << this->currentCell[_Y] << std::endl;	//debug
	
	if (oldCell != this->currentCell) {
		this->world->remove(this->id, oldCell);
		this->world->add(this->id, this->currentCell);
	}
	

	if (this->world->getCell(nextCell).contents & WALL_ID) {
		wall->setPos(this->world->mapToPixel(nextCell));
		if (collision((Sprite)(*this), *wall)) {
			//fallback because it's not possible to move inside a wall
			std::cout << "wall" << std::endl;	//debug
			this->setPos(oldPosition);
			this->currentCell = oldCell;
		}
	}
}
