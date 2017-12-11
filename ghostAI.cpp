#include "ghostAI.h"

#include <vector>
#include <list>
#include "random.h"
#include "Lab309_ADT_Matrix.h"
#include <algorithm>

using namespace lab309;

class DirectedCell {
	public:
		int direction;
		unsigned long int timeStamp;
					
		bool operator < (const DirectedCell &b) {
			return this->timeStamp < b.timeStamp;
		}	
};

typedef struct {
	Vector<int> pos;
	unsigned int dontExpand;
	unsigned int depth;
} SearchingCell;

Vector<float> bfs (const World *world, const Vector<int> beginning, int targetId, unsigned int maxDepth, unsigned long int currentTime) {
	
	std::list<SearchingCell> cells;
	SearchingCell expanding;
	SearchingCell previous;
	Vector<int> directions[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
	Matrix<Vector<int>*> explorationMap(world->getWidth(), world->getHeight());
	Vector<int> found = {-1, -1};
	
	//std::cout << "starting map" << std::endl;	//debug
	for (size_t i = 0; i < explorationMap.getLines(); i++) {
		for (size_t j = 0; j < explorationMap.getColums(); j++) {
			explorationMap[i][j] = NULL;
		}
	}
	
	//std::cout << "map started iput" << std::endl;	//debug
	cells.push_back({beginning, 0, 0});
	explorationMap[beginning] = new Vector<int>(beginning);
	while (cells.size() > 0) {
		//std::cout << "expanding cell" << std::endl;	//debug
		expanding = cells.front();
		cells.pop_front();
		
		if (expanding.depth == maxDepth) {
			//std::cout << "max search depth reached" << std::endl;	//debug
			break;
		}
		if ((world->getCell(expanding.pos).contents & targetId) == targetId) {
			//std::cout << "found target" << std::endl;	//debug
			found = expanding.pos;
			break;
		}
		
		//set the dontExpand flags so that the ghost can't see through walls
		if (HIDE_BEHIND_WALLS) {
			for (size_t i = 0; i < 4; i++) {
				if (world->getCell(expanding.pos+directions[i]).contents & WALL_ID) {
					expanding.dontExpand |= 1<<i;
				}
			}
		}
		
		for (size_t i = 0; i < 4; i++) {
			SearchingCell add = {expanding.pos+directions[i], expanding.dontExpand, expanding.depth+1};
			World::Cell c = world->getCell(add.pos);
			if ((c.contents & WALL_ID) == 0 && explorationMap[add.pos] == NULL && (expanding.dontExpand & 1<<i) == 0 && currentTime-c.ghostTrailTimeStamp > IGNORE_GHOST_TRAIL) {
				cells.push_back(add);
				explorationMap[add.pos] = new Vector<int>(expanding.pos);
			}	
		}
	}
	
	//std::cout << "search ended" << std::endl;	//debug
	if (found != Vector<int>({-1, -1})) {
		Vector<int> direction;
		while (beginning != *explorationMap[found]) {
			found = *explorationMap[found];
		}
		for (size_t i = 0; i < explorationMap.getLines(); i++) {
			for (size_t j = 0; j < explorationMap.getColums(); j++) {
				delete(explorationMap[i][j]);
			}
		}
		direction = found-beginning;
		return Vector<float>({(float)direction[_X], (float)direction[_Y]});
	}
	for (size_t i = 0; i < explorationMap.getLines(); i++) {
		for (size_t j = 0; j < explorationMap.getColums(); j++) {
			delete(explorationMap[i][j]);
		}
	}
	return Vector<float>({0.0f, 0.0f});
}

Ghost::Ghost(SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos) : Object(texture, rectWidth, rectHeight, displayWidth, displayHeight, world, id, speed, initialPos) {
	this->previousCell = this->currentCell;
}

void Ghost::think (unsigned long int currentTime) {
	Vector<float> corners[4];
	bool cornersEqual;
	float xOffset = (float)this->getDisplayWidth()*CELL_DETECTION_X_TOLERANCE;
	float yOffset = (float)this->getDisplayHeight()*CELL_DETECTION_Y_TOLERANCE;
	size_t i;
	
	corners[0] = this->getCenter()+Vector<float>({xOffset, yOffset});
	corners[1] = this->getCenter()+Vector<float>({-xOffset, yOffset});
	corners[2] = this->getCenter()+Vector<float>({xOffset, -yOffset});
	corners[3] = this->getCenter()+Vector<float>({-xOffset, -yOffset});
	
	//std::cout << "corners equal" << std::endl;	//debug
	cornersEqual = true;
	i = 0;
	while (cornersEqual && i < 3) {
		if (this->world->mapToNavmesh(corners[i]) != this->world->mapToNavmesh(corners[i+1])) {
			//std::cout<<"corners not equal" << std::endl;	//debug
			cornersEqual = false;
		}
		i++;
	}
	//cornersEqual = true;	//debug
	
	//ghost only thinks if it has reached it's previously desired cell or if it has never thought before
	if (this->moveDirection == Vector<float>({0.0f, 0.0f}) || cornersEqual && this->currentCell != this->previousCell) {
		//std::cout << "needs to think" << std::endl;	//debug
		this->previousCell = this->currentCell;
		//search for pacman
		//std::cout << "searching" << std::endl;	//debug
		this->moveDirection = bfs(this->world, this->currentCell, PACMAN_ID, GHOST_VIEW_DISTANCE, currentTime);
		
		//use move direction based on trail in case pacman is not found
		if (this->moveDirection == Vector<float>({0, 0})) {
		
			//std::cout << "nothing found" << std::endl;	//debug
			
			Vector<int> directions[4] = {Vector<int>({1, 0}), Vector<int>({0, 1}), Vector<int>({-1, 0}), Vector<int>({0, -1})};
			std::list<DirectedCell> neighbours;
			std::vector<DirectedCell> ordered;
			float rnd;
			
			//generate list ordered by the least recent pacman trail
			for (int i = 0; i < 4; i++) {
				World::Cell c = this->world->getCell(this->currentCell+directions[i]);
				if ((c.contents & WALL_ID) == 0 && currentTime - c.trailTimeStamp < IGNORE_PACMAN_TRAIL && currentTime - c.ghostTrailTimeStamp > IGNORE_GHOST_TRAIL) {
					neighbours.push_back({i, c.trailTimeStamp});
					//std::cout << "something here" << std::endl;	//debug	
				}
			}
			
			//if there's no pacman trail to follow the ghost will go for the least recently visited place
			if (neighbours.size() == 0) {
				//std::cout << "no trail found " << std::endl;	//debug
				for (int i = 0; i < 4; i++) {
					World::Cell c = this->world->getCell(this->currentCell+directions[i]);
					if ((c.contents & WALL_ID) == 0 && currentTime - c.ghostTrailTimeStamp > IGNORE_GHOST_TRAIL) {
						neighbours.push_back({i, c.ghostTrailTimeStamp});
					} else {
						//std::cout << "wall at " << directions[i][_X] << " " << directions[i][_Y] << std::endl;	//debug
					}	
				}
			}
			
			//if all cells have been visited recently and there's no pacman trail move to a random cell
			if (neighbours.size() == 0) {
				for (int i = 0; i < 4; i++) {
					World::Cell c = this->world->getCell(this->currentCell+directions[i]);
					if ((c.contents & WALL_ID) == 0) {
						neighbours.push_back({i, c.ghostTrailTimeStamp});
					}	
				}
			}
			ordered = std::vector<DirectedCell>(neighbours.size());
			for (DirectedCell i : neighbours) {
				ordered.push_back(i);
			}
			std::sort(ordered.begin(), ordered.end());
			
			for (int i = ordered.size()-1; i > -1; i--) {
				//std::cout << rnd << std::endl;	//debug
				rnd = random();
				if (rnd < chances[i]) {
					this->moveDirection = Vector<float>(directions[ordered[i].direction]);
					//std::cout << "moving to " << this->moveDirection[_X] << " " << this->moveDirection[_Y] << std::endl;	//debug
					break;
				}
			}
			this->moveDirection = Vector<float>(directions[(ordered.end()-1)->direction]);
		}
		
	}
}

void Ghost::move (unsigned long int currentTime, double elapsedTime) {
	this->translate(elapsedTime*this->speed*this->moveDirection);
	this->currentCell = this->world->mapToNavmesh(this->getCenter());
	if (this->currentCell != this->previousCell) {
		Vector<int> directions[4] = {Vector<int>({1, 0}), Vector<int>({0, 1}), Vector<int>({-1, 0}), Vector<int>({0, -1})};
		this->world->remove(this->id, this->previousCell);
		this->world->setGhostTrail(currentTime, this->currentCell);
	}
}
