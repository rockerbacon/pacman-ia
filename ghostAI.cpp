#include "ghostAI.h"

#include <vector>
#include <list>
#include "random.h"
#include "Lab309_ADT_Matrix.h"
#include <algorithm>

#define GHOST_VIEW_DISTANCE 0.25

#define IGNORE_PACMAN_TRAIL 10000 //tempo apos o qual qualquer trail do pacman deve ser ignorado
#define IGNORE_GHOST_TRAIL 3000 //tempo durante o qual o fantasma nao deve revisitar uma celula
#define FRESH_GHOST_TRAIL 500	//tempo durante o qual o fantasma deve ignorar trails do pacman em funcao dos trails dos fantasmas

#define HIDE_BEHIND_WALLS true

using namespace lab309;

class DirectedCell {
	public:
		int direction;
		unsigned int timeStamp;
		
		DirectedCell (int direction, unsigned int timeStamp) {
			this->direction = direction;
			this->timeStamp = timeStamp;
		}
		
		DirectedCell (void) = default;
					
		bool operator < (const DirectedCell &b) {
			return this->timeStamp < b.timeStamp;
		}	
};

typedef struct {
	Vector<int> pos;
	unsigned int dontExpand;
	unsigned int depth;
} SearchingCell;

Vector<int> bfs (World *world, const Vector<int> beginning, int targetId, unsigned int maxDepth, unsigned long int currentTime) {
	
	std::list<SearchingCell> cells;
	SearchingCell expanding;
	SearchingCell previous;
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
				if (world->getCell(expanding.pos+Object::directions[i]).contents & WALL_ID) {
					expanding.dontExpand |= 1<<i;
				}
			}
		}
		
		for (size_t i = 0; i < 4; i++) {
			SearchingCell add = {expanding.pos+Object::directions[i], expanding.dontExpand, expanding.depth+1};
			World::Cell c = world->getCell(add.pos);
			if ((c.contents & WALL_ID) == 0 && explorationMap[add.pos] == NULL && (expanding.dontExpand & 1<<i) == 0) {
				cells.push_back(add);
				explorationMap[add.pos] = new Vector<int>(expanding.pos);
			}	
		}
	}
	
	//std::cout << "search ended" << std::endl;	//debug
	if (found != Vector<int>({-1, -1})) {
		world->setTrail(currentTime, beginning);
		while (beginning != *explorationMap[found]) {
			world->setTrail(currentTime, found);
			found = *explorationMap[found];
		}
		for (size_t i = 0; i < explorationMap.getLines(); i++) {
			for (size_t j = 0; j < explorationMap.getColums(); j++) {
				delete(explorationMap[i][j]);
			}
		}
		return found-beginning;
	}
	for (size_t i = 0; i < explorationMap.getLines(); i++) {
		for (size_t j = 0; j < explorationMap.getColums(); j++) {
			delete(explorationMap[i][j]);
		}
	}
	return Vector<int>({0, 0});
}

Ghost::Ghost(SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos) : Object(texture, rectWidth, rectHeight, displayWidth, displayHeight, world, id, speed, initialPos) {
	this->previousCell = this->currentCell;
	this->viewDistance = (world->getWidth()+world->getHeight())/2*GHOST_VIEW_DISTANCE;
}

void Ghost::think (unsigned int currentTime) {
	size_t i;
	
	//ghost only thinks if it has reached it's previously desired cell or if it has been reset
	if (this->moveDirection == Vector<int>({0, 0}) || this->fitsCell()) {
		//std::cout << "needs to think" << std::endl;	//debug
		this->previousCell = this->currentCell;
		//search for pacman
		//std::cout << "searching" << std::endl;	//debug
		this->moveDirection = bfs(this->world, this->currentCell, PACMAN_ID, this->viewDistance, currentTime);
		//std::cout << "search ended" << std::endl;	//debug
		
		//in case pacman is not found use trail strategy
		if (this->moveDirection == Vector<int>({0, 0})) {
		
			//std::cout << "nothing found" << std::endl;	//debug
			
			std::list<DirectedCell> neighbours;
			std::vector<DirectedCell> ordered;
			float rnd;
			
			//search for nearby pacman trails
			for (int i = 0; i < 4; i++) {
				World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
				//std::cout << currentTime << std::endl;	//debug
				if (this->canMove(Object::directions[i]) && currentTime - c.trailTimeStamp < IGNORE_PACMAN_TRAIL && currentTime - c.ghostTrailTimeStamp > IGNORE_GHOST_TRAIL) {
					neighbours.push_back(DirectedCell(i, c.trailTimeStamp));
					//std::cout << "I smell a trail" << std::endl;	//debug	
				}
			}
			
			//if there are no pacman trails to the ghost will follow the least recently visited cell
			if (neighbours.size() == 0) {
				//std::cout << "no trail found " << std::endl;	//debug
				for (int i = 0; i < 4; i++) {
					World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
					if (this->canMove(Object::directions[i]) && currentTime - c.ghostTrailTimeStamp > IGNORE_GHOST_TRAIL) {
						neighbours.push_back(DirectedCell(i, c.trailTimeStamp));
						//std::cout << "I'm missing that spot" << std::endl;	//debug	
					}	
				}
			}
			
			//if all cells have been visited recently and there's no pacman trail the ghost will move to a random cell
			if (neighbours.size() == 0) {
				for (int i = 0; i < 4; i++) {
					//std::cout << "Just wandering" << std::endl;	//debug
					World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
					if (this->canMove(Object::directions[i])) {
						neighbours.push_back(DirectedCell(i, c.trailTimeStamp));
					}	
				}
			}
			
			ordered = std::vector<DirectedCell>();
			for (DirectedCell d : neighbours) {
				ordered.push_back(d);
			}
			std::sort(ordered.begin(), ordered.end());
			
			for (i = 0; i < ordered.size()-1; i++) {
				if (random() < chances[i]) {
					break;
				}
			}
			
			this->setMoveDirection(Object::directions[ordered[i].direction]);
			//std::cout << "moving to " << this->moveDirection[_X] << " " << this->moveDirection[_Y] << std::endl;	//debug
		} else {
			//std::cout << "I see pacman" << std::endl;	//debug
		}
		
	}
}

int manhattan (const Vector<int> &a, const Vector<int> &b) {
	return abs(a[_X]-b[_X]) + abs(a[_Y]-b[_Y]);
}

void Ghost::flee (const Vector<int> &pacmanPos, unsigned int currentTime) {
	size_t i;
	
	//flees if has reached a new cell or has been reset
	if (this->moveDirection == Vector<float>({0.0f, 0.0f}) || this->fitsCell()) {
		//flee based on trails if pacman is out of range
		if (manhattan(pacmanPos, this->currentCell) > this->viewDistance) {
			//std::cout << "flee to where?" << std::endl;	//debug
			std::list<DirectedCell> neighbours;
			std::vector<DirectedCell> ordered;
			float rnd;
			
			//search for nearby old pacman trails
			for (int i = 0; i < 4; i++) {
				World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
				if (this->canMove(Object::directions[i]) && currentTime - c.trailTimeStamp > IGNORE_PACMAN_TRAIL) {
					neighbours.push_back(DirectedCell(i, c.trailTimeStamp));
					//std::cout << "something here" << std::endl;	//debug	
				}
			}
			
			//if there are no pacman trails search for nearby recent ghost trails
			if (neighbours.size() == 0) {
				//std::cout << "no trail found " << std::endl;	//debug
				for (int i = 0; i < 4; i++) {
					World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
					if (this->canMove(Object::directions[i]) && currentTime - c.ghostTrailTimeStamp < IGNORE_GHOST_TRAIL) {
						neighbours.push_back(DirectedCell(i, c.ghostTrailTimeStamp));
					}	
				}
			}
			
			//cells have not been visited recently and there's no pacman trail move to a random cell
			if (neighbours.size() == 0) {
				for (int i = 0; i < 4; i++) {
					World::Cell c = this->world->getCell(this->currentCell+Object::directions[i]);
					if (this->canMove(Object::directions[i])) {
						neighbours.push_back(DirectedCell(i, c.trailTimeStamp));
					}	
				}
			}
			
			ordered = std::vector<DirectedCell>();
			for (DirectedCell i : neighbours) {
				ordered.push_back(i);
			}
			std::sort(ordered.rbegin(), ordered.rend());
			
			for (i = 0; i < ordered.size()-1; i++) {
				if (random() < chances[i]) {
					break;
				}
			}
			this->setMoveDirection(Object::directions[ordered[i].direction]);
			
		} else {
			//if pacman is nearby flee using a bfs to the farthest location
			//std::cout << "needs to flee" << std::endl;	//debug
			std::list<SearchingCell> cells;
			SearchingCell expanding;
			Vector<int> beginning;
			Matrix<Vector<int>*> explorationMap(world->getWidth(), world->getHeight());
			Vector<int> found;
	
			//std::cout << "starting map" << std::endl;	//debug
			for (size_t i = 0; i < explorationMap.getLines(); i++) {
				for (size_t j = 0; j < explorationMap.getColums(); j++) {
					explorationMap[i][j] = NULL;
				}
			}
	
			//std::cout << "map started" << std::endl;	//debug
			beginning = this->currentCell;
			cells.push_back({beginning, 0, 0});
			explorationMap[beginning] = new Vector<int>(beginning);
			found = beginning;
			while (cells.size() > 0) {
				//std::cout << "expanding cell" << std::endl;	//debug
				expanding = cells.front();
				cells.pop_front();
		
				if (manhattan(pacmanPos, found) < manhattan(pacmanPos, expanding.pos)) {
					found = expanding.pos;
				}
		
				if (expanding.depth == this->viewDistance) {
					//std::cout << "max search depth reached" << std::endl;	//debug
					break;
				}
		
				for (size_t i = 0; i < 4; i++) {
					SearchingCell add = {expanding.pos+Object::directions[i], expanding.dontExpand, expanding.depth+1};
					World::Cell c = world->getCell(add.pos);
					if ((c.contents & WALL_ID) == 0 && explorationMap[add.pos] == NULL) {
						cells.push_back(add);
						explorationMap[add.pos] = new Vector<int>(expanding.pos);
					}	
				}
			}
	
			//std::cout << "search ended" << std::endl;	//debug
			while (beginning != *explorationMap[found]) {
				//std::cout << found[_X] << " " << found[_Y] << std::endl;	//debug
				found = *explorationMap[found];
			}
			//std::cout << "cell found" << std::endl;	//debug
			for (size_t i = 0; i < explorationMap.getLines(); i++) {
				for (size_t j = 0; j < explorationMap.getColums(); j++) {
					delete(explorationMap[i][j]);
				}
			}
			//std::cout << "direction found" << std::endl;	//debug
			this->setMoveDirection(found-beginning);
		}	
	}	
}
