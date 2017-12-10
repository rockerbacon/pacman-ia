#include "sprite.h"
#include "world.h"
#include <iostream>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 640

#define PACMAN_SPEED WINDOW_WIDTH*WINDOW_HEIGHT*0.0003
#define GHOST_SPEED PACMAN_SPEED

#define GHOST_VIEWDISTANCE 10

using namespace lab309;

bool handleInput (Object *pacman) {
	SDL_Event event;
	
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			return false;
		} else if (event.type == SDL_KEYUP) {
			switch (event.key.keysym.sym) {
				case SDLK_RIGHT:
					pacman->setMoveDirection({1.0f, 0.0f});
				break;
				case SDLK_LEFT:
					pacman->setMoveDirection({-1.0f, 0.0f});
				break;
				case SDLK_UP:
					pacman->setMoveDirection({0.0f, -1.0f});
				break;
				case SDLK_DOWN:
					pacman->setMoveDirection({0.0f, 1.0f});
				break;
			}
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				//nothing here
			}
		}
	}
	
	return true;
}

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
			*superpill;
	Object	*pacman;
	std::list<Object*> ghosts;
			
	Window *window;
	World *world;
			
	bool running;
	std::list<Vector<float>> draw;
	std::vector<Vector<float>> wallsPos;
	
	if (argc < 2) {
		std::cout << "Por favor especifique o mapa a ser carregado" << std::endl;
		return 1;
	}
	
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	
	//load window and world
	window = new Window("Pacman", WINDOW_WIDTH, WINDOW_HEIGHT, LIMIT_30FPS);
	world = new World(*window);
	if (!world->readFromFile(args[1])) {
		std::cout << "Nao foi possivel ler a partir do arquivo " << args[1] << std::endl;
	}
	//load walls
	draw = world->getFromMesh(WALL_ID);
	for (Vector<float> i : draw) {
		wallsPos.push_back(i);
	}
	
	//load textures
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
	
	//load agents
	draw = world->getFromMesh(PACMAN_ID);
	pacman = new Object(texture_pacman, texture_pacman->w, texture_pacman->h, world->getCellWidth(), world->getCellHeight(), world, PACMAN_ID, PACMAN_SPEED, 0, draw.front());
	draw = world->getFromMesh(GHOST_ID);
	for (Vector<float> g : draw) {
		ghosts.push_front(new Object(texture_ghost, texture_ghost->w, texture_ghost->h, world->getCellWidth(), world->getCellHeight(), world, GHOST_ID, GHOST_SPEED, 0, g));
	}
		
	running = true;
	window->update();
	while (running) {
		running = handleInput(pacman);
		
		pacman->move(wall, window->getTimeDelta());
		
		//draw background
		background->blitTo(*window);
		//draw walls
		for (Vector<float> i : wallsPos) {
			wall->setPos(i);
			wall->blitTo(*window);
		}
		//std::cout << wallsPos.size() << std::endl;	//debug
		
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
		
		//draw pacman
		pacman->blitTo(*window);	
		
		//draw ghosts
		for (Object *i : ghosts) {
			i->blitTo(*window);
		}
		
		window->update();
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
	delete(wall);
	delete(pill);
	delete(superpill);
	delete(pacman);
	for (Object *i : ghosts) {
		delete(i);
	}
	
	SDL_Quit();
	return 0;
}
