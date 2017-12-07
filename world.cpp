#include "world.h"
#include <fstream>

using namespace lab309;

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
	int cellWidth = this->window->getWidth()/this->navmesh.getColums();
	int cellHeight = this->window->getHeight()/this->navmesh.getLines();
	Vector<int> v = (Vector<int>)pos;
	
	v[_X] = (v[_X] - v[_X] % cellWidth)/cellWidth;
	v[_Y] = (this->window->getHeight() - (v[_Y] - v[_Y] % cellHeight) - cellHeight)/cellHeight;
	
	return v;
}

Vector<float> World::mapToPixel (const Vector<int> &navPos) const {
	Vector<float> pixel(2);
	int gridWidth = this->window->getWidth()/this->navmesh.getColums();
	int gridHeight = this->window->getHeight()/this->navmesh.getLines();
	
	pixel[_X] = pos[_X]*gridWidth;
	pixel[_Y] = (this->navmesh.getLines() - pos[_Y] - 1) * gridHeight;
	
	return pixel;
}

Vector<int> World::add (int id, const Vector<float> &pos) {
	Vector<int> navPos = this->mapToNavmesh(pos);
	if (navPos[_X] != -1 && navPos[_X] < this->navmesh.getColums() && navPos[_Y] < this->navmesh.getLines()) {
		this->navmesh[navPos] |= id;
	}
	return navPos;
}

std::list<Vector<float>> World::getFromMesh (int content) const {
	Vector<int> pos(2);
	std::list<Vector<float>> l;
	for (pos[_X] = 0; pos[_X] < this->navmesh.getColums(); pos[_X]++) {
		for (pos[_Y] = 0; pos[_Y] < this->navmesh.getLines(); pos[_Y]++) {
			if ((this->navmesh[pos] & content) == content) {
				l.push_front(mapToPixel(pos));
			}
		}	
	}
	return l;
}

bool World::readFromFile (const char *filePath) {
	fsteam input;
	size_t meshWidth, meshHeight;
	char c;
	
	input.open(filePath, fstream::in);
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
					this->navmesh[i][j] = WALL_ID;
				break;
				case PACMAN_CELL:
					this->navmesh[i][j] = PACMAN_ID|PATHWAY_ID;
				break;
				case GHOST_CELL:
					this->navmesh[i][j] = GHOST_ID|PATHWAY_ID;
				break;
				case PILL_CELL:
					this->navmesh[i][j] = PILL_ID|PATHWAY_ID;
				break;
				case SUPERPILL_CELL:
					this->navmesh[i][j] = SUPERPILL_ID|PATHWAY_ID;
				break;
				default:
					this->navmesh[i][j] = 0;
				break;
			} 
		}
	}	
	
	return true;
}
