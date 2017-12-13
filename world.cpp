#include "world.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <SDL2/SDL.h>

using namespace lab309;

#define COLLISION_BOX_X_TOLERANCE 0.55
#define COLLISION_BOX_Y_TOLERANCE 0.55

#define QUEUEDMOVE_WINDOW 400

const Vector<int> Object::directions[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

/*WORLD*/
World::Cell::Cell (void) {
	this->contents = 0;
	this->trailTimeStamp = 0;
	this->ghostTrailTimeStamp = 0;
}

World::Cell::Cell (int contents) {
	this->contents = contents;
	this->trailTimeStamp = 0;
	this->ghostTrailTimeStamp = 0;
}

World::World (const Window &window, size_t navmeshWidth, size_t navmeshHeight) {
	this->window = &window;
	this->navmesh = Matrix<Cell>(navmeshHeight, navmeshWidth);
	for (size_t i = 0; i < navmeshHeight; i++) {
		for (size_t j = 0; j < navmeshWidth; j++) {
			this->navmesh[i][j] = Cell();
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

size_t World::getWidth (void) const {
	return this->navmesh.getColums();
}

size_t World::getHeight (void) const {
	return this->navmesh.getLines();
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

void World::setTrail (unsigned long int timeStamp, const Vector<int> &cell) {
	this->navmesh[cell].trailTimeStamp = timeStamp;
}

void World::setGhostTrail (unsigned long int timeStamp, const Vector<int> &cell) {
	if (cell[_X] >= 0 && cell[_Y] >= 0 && cell[_X] < this->getWidth() && cell[_Y] < this->getHeight()) {
		this->navmesh[cell].ghostTrailTimeStamp = timeStamp;
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
	
	this->navmesh = Matrix<Cell>(meshWidth, meshHeight);
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
Object::Object (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos) : Sprite(texture, rectWidth, rectHeight, displayWidth, displayHeight) {
	this->world = world;
	this->id = id;
	this->speed = speed;
	this->setPos(initialPos);
	this->currentCell = world->mapToNavmesh(initialPos);
	this->moveDirection = {0, 0};
	this->queuedMove = false;
}

void Object::setMoveDirection (const Vector<int> &moveDirection) {
	if (this->moveDirection == -1*moveDirection || this->fitsCell() && this->canMove(moveDirection)) {
		this->moveDirection = moveDirection;
		this->queuedMove = false;
	} else {
		this->moveDirectionQueue = moveDirection;
		this->queuedMove = true;
		this->queuedMoveTimeStamp = SDL_GetTicks();
	}
}

bool Object::canMove (const Vector<int> &moveDirection) const {
	World::Cell c = this->world->getCell(this->currentCell+moveDirection);
	return (c.contents & WALL_ID) == 0 && (c.contents & this->id) == 0;
}

float Object::getSpeed (void) const {
	return this->speed;
}

Vector<int> Object::getCurrentCell (void) const {
	return this->currentCell;
}

bool Object::fitsCell (void) const {
	Vector<float> corners[4];
	Vector<float> center = this->getCenter();
	float xOffset = (float)this->getDisplayWidth()*(1.0-COLLISION_BOX_X_TOLERANCE);
	float yOffset = (float)this->getDisplayHeight()*(1.0-COLLISION_BOX_Y_TOLERANCE);
	bool fits;
	size_t i;
	
	corners[0] = center+Vector<float>({xOffset, yOffset});
	corners[1] = center+Vector<float>({-xOffset, yOffset});
	corners[2] = center+Vector<float>({xOffset, -yOffset});
	corners[3] = center+Vector<float>({-xOffset, -yOffset});
	
	fits = true;
	i = 0;
	while(fits && i < 3) {
		if (this->world->mapToNavmesh(corners[i]) != this->world->mapToNavmesh(corners[i+1])) {
			fits = false;
		}
		i++;
	}
	return fits;
}

Vector<int> Object::getMoveDirection (void) const {
	return this->moveDirection;
}

void Object::move (Sprite *wall, unsigned long int currentTime, double elapsedTime) {
	bool fits = this->fitsCell();
	
	//if (this->id & PACMAN_ID) std::cout << SDL_GetTicks() - this->queuedMoveTimeStamp << std::endl;	//debug
	if (fits && this->queuedMove && currentTime - this->queuedMoveTimeStamp < QUEUEDMOVE_WINDOW) {
		this->setMoveDirection(this->moveDirectionQueue);
	}
	
	if (!fits || this->canMove(this->moveDirection)) {
		Vector<float> oldPosition = this->getPos();
		Vector<int> oldCell = this->currentCell;
		Vector<int> nextCell = this->currentCell+this->moveDirection;
		
		this->translate(Vector<float>(this->speed*elapsedTime*this->moveDirection));
		this->currentCell = this->world->mapToNavmesh(this->getCenter());
		
		if (oldCell != this->currentCell) {
			//safeguard for excessive movement that may happen if the elapsed time is too big
			wall->setPos(this->world->mapToPixel(nextCell));
			if ((this->world->getCell(nextCell).contents & WALL_ID) && collision((Sprite)(*this), *wall)) {
				this->setPos(this->world->mapToPixel(oldCell));
				this->currentCell = oldCell;
			} else {
			
				this->world->remove(this->id, oldCell);
				this->world->add(this->id, this->currentCell);
				if (this->id & PACMAN_ID) {
					this->world->setTrail(currentTime, oldCell);
				} else if (this->id & GHOST_ID) {
					this->world->setGhostTrail(currentTime, oldCell);
				}
				
			}	
		}	
		
	}
}

void Object::setSpeed (float speed) {
	this->speed = speed;
}
