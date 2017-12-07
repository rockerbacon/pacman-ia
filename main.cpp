#include "sprite.h"
#include "world.h"
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 640

using namespace lab309;

int main (int argc, char **args) {
	SDL_Surface	*texture_background,
				*texture_wall,
				*texture_pill,
				*texture_superpill,
				*texture_pacman,
				*texture_ghost;
				
	Sprite	*background,
			*wall,
			*pill,
			*superpill,
			*pacman,
			*ghost;
			
	Window *window;
	World *world;
			
	bool running;
	std::list<Vector<float>> draw;
	std::list<Vector<float>> pacmans;
	std::list<Vector<float>> ghosts;
	std::vector<Vector<float>> walls;
	
	if (argc < 2) {
		std::cout << "Por favor especifique o mapa a ser carregado" << std::endl;
		return 1;
	}
	
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(SDL_INIT_PNG);
	
	//load window and world
	window = new Window("Pacman", WINDOW_WIDTH, WINDOW_HEIGHT, LIMIT_30FPS);
	world = new World(*window);
	if (!world.readFromFile(argc[1])) {
		std::cout << "Nao foi possivel ler a partir do arquivo " << argc[1] << std::endl;
	}
	draw = world->getFromMesh(WALL_ID);
	for (Vector<float> i : draw) {
		walls.push_front(i);
	}
	pacmans = world->getFromMesh(PACMAN_ID);
	ghosts = world->getFromMesh(GHOST_ID);
	
	//lead textures
	texture_background = window->loadTexture("img/background.png");
	texture_wall = window->loadTexture("img/wall.png");
	texture_pill = window->loadTexture("img/pill.png");
	texture_superpill = window->loadTexture("img/superpill.png");
	texture_pacman = window->loadTexture("img/pacman.png");
	texture_ghost = window->loadTexture("img/ghost.png");
	
	background = new Sprite(texture_background, texture_background->w, texture_background->h, WINDOW_WIDTH, WINDOW_HEIGHT);
	wall = new Sprite(texture_wall, texture_wall->w, texture_wall->h, world->getCellWidth(), world->getCellHeight());
	pill = new Sprite(texture_pill, texture_pill->w, texture_pill->h, world->getCellWidth(), world->getCellHeight());
	superpill = new Sprite(texture_superpill, texture_superpill->w, texture_superpill->h, world->getCellWidth(), world->getCellHeight());
	pacman = new Sprite(texture_pacman, texture_pacman->w, texture_pacman->h, world->getCellWidth(), world->getCellHeight());
	ghost = new Sprite(texture_ghost, texture_ghost->w, texture_ghost->h, world->getCellWidth(), world->getCellHeight());
		
	while (running) {
	
		//draw background
		background->blitTo(*window);
		//draw walls
		for (Vector<float> i : walls) {
			wall->setPos(i);
			wall->blitTo(*window);
		}
		
		//draw pills
		draw = world->getFromMesh(PILL_ID);
		for (Vector<float> i : draw) {
			pill->setPos(i);
			pill->blitTo(*window);
		}
		
		//draw superpills
		draw = world->getFromMesh(SUPERPILL_ID);
		for (Vector<float> i : draw) {
			superpill->setPos(i);
			superpill->blitTo(*window);
		}
		
		//draw pacmans
		for (Vector<float> i : pacmans) {
			pacman->setpos(i);
			pacman->blitTo(*window);
		}	
		
		//draw ghosts
		for (Vector<float> i : ghosts) {
			ghost->setpos(i);
			ghost->blitTo(*window);
		}
	}
	
	//clear memory
	delete(window);
	delete(world);
	
	SDL_FreeSurface(texture_background);
	SDL_FreeSurface(texture_wall);
	SDL_FreeSurface(texture_pill);
	SDL_FreeSurface(texture_superpill);
	SDL_FreeSurface(texture_pacman);
	SDL_FreeSurface(texture_ghost);
	
	delete(background);
	delete(wall)
	delete(pill);
	delete(superpill);
	delete(pacman);
	delete(ghost);
	
	SDL_Quit();
	return 0;
}
