#include "world.h"

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

Vector<int> World::mapToNavmesh (const Vector<float> &pos) {
	if (pos[_X] < 0 || pos[_Y] < 0) {
		return {-1, -1};
	}
	int i, j;
	int cellWidth = this->window->getWidth()/this->navmesh.getColums();
	int cellHeight = this->window->getHeight()/this->navmesh.getLines();
	Vector<int> v = (Vector<int>)pos;
	
	v[COORDINATE_X] = (v[_X] - v[_X] % cellWidth)/cellWidth;
	v[COORDINATE_Y] = (this->window->getHeight() - (v[_Y] - v[_Y] % cellHeight) - cellHeight)/cellHeight;
	
	return v;
}

void World::add (int id, const Vector<float> &pos) {
	Vector<int> navPos = this->mapToNavmesh(pos);
	if (navPos[_X] != -1 && navPos[_X] < this->navmesh.getColums() && navPos[_Y] < this->navmesh.getLines()) {
		this->navmesh[navPos] |= id;
	}
}
