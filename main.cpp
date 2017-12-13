#include "sprite.h"
#include "world.h"
#include <iostream>
#include <SDL2/SDL.h>
#include "ghostAI.h"
#include "random.h"
#include <math.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define PILL_SCORE 300
#define SUPERPILL_SCORE 500
#define GHOST_KILL_SCORE 200

#define SUPERPILL_DURATION 10000

#define SUPERSPEED_MULT 1.4f

#define PACMAN_SPEED 2.4 //in cells per second
#define GHOST_SPEED PACMAN_SPEED*0.85

using namespace lab309;

bool handleInput (Object *pacman) {
	SDL_Event event;
	
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			return false;
		} else if (event.type == SDL_KEYUP) {
			switch (event.key.keysym.sym) {
				case SDLK_RIGHT:
					pacman->setMoveDirection({1, 0});
				break;
				case SDLK_LEFT:
					pacman->setMoveDirection({-1, 0});
				break;
				case SDLK_UP:
					pacman->setMoveDirection({0, -1});
				break;
				case SDLK_DOWN:
					pacman->setMoveDirection({0, 1});
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
				*texture_ghost,
				*texture_superpacman;
				
	Sprite	*background,
			*wall,
			*pill,
			*superpill,
			*superpacman;
	Object	*pacman;
	Vector<float> pacmanResetPos;
	std::list<Ghost*> ghosts;
	double dt;
			
	Window *window;
	World *world;
	
	unsigned int score = 0;
	unsigned int lives = 3;
	bool super = false;
	unsigned int superTimeStamp;
	unsigned int killStreak;
	
	float	pacmanSpeed,
			ghostSpeed;
	
	int endGame = 0;
			
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
	
	pacmanSpeed = (world->getCellHeight()+world->getCellWidth())/2.0*PACMAN_SPEED;
	ghostSpeed = (world->getCellHeight()+world->getCellWidth())/2.0*GHOST_SPEED;
	
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
	texture_superpacman = window->loadTexture("img/superpacman.png");
	
	background = new Sprite(texture_background, texture_background->w, texture_background->h, WINDOW_WIDTH, WINDOW_HEIGHT);
	wall = new Sprite(texture_wall, texture_wall->w, texture_wall->h, world->getCellWidth(), world->getCellHeight());
	pill = new Sprite(texture_pill, texture_pill->w, texture_pill->h, world->getCellWidth()*0.8, world->getCellHeight()*0.8);
	superpill = new Sprite(texture_superpill, texture_superpill->w, texture_superpill->h, world->getCellWidth(), world->getCellHeight());
	superpacman = new Sprite(texture_superpacman, texture_superpacman->w, texture_superpacman->h, world->getCellWidth()*0.82, world->getCellHeight()*0.82);
	
	//load agents
	draw = world->getFromMesh(PACMAN_ID);
	pacman = new Object(texture_pacman, texture_pacman->w, texture_pacman->h, world->getCellWidth()*0.8, world->getCellHeight()*0.8, world, PACMAN_ID, pacmanSpeed, draw.front());
	pacmanResetPos = pacman->getPos();
	draw = world->getFromMesh(GHOST_ID);
	for (Vector<float> g : draw) {
		ghosts.push_front(new Ghost(texture_ghost, texture_ghost->w, texture_ghost->h, world->getCellWidth()*0.8, world->getCellHeight()*0.8, world, GHOST_ID, ghostSpeed, g));
	}
		
	running = true;
	window->update();
	dt = 0.0;
	while (running) {
		//std::cout << "handle input" << std::endl;	//debug
		timeSeed();
		running = handleInput(pacman);
		endGame = 0;
		
		pacman->move(wall, SDL_GetTicks(), dt);
		//std::cout << "move pacman" << std::endl;	//debug
		
		//wear super effect off
		if (super && SDL_GetTicks() - superTimeStamp > SUPERPILL_DURATION) {
			super = false;
			for (Ghost *i : ghosts) {
				i->setSpeed(ghostSpeed);
				i->setMoveDirection({0, 0});
			}
		}
		
		for (std::list<Ghost*>::iterator i = ghosts.begin(); i != ghosts.end(); i++) {
			
			//std::cout << "ghost think" << std::endl;	//debug
			if (!super) {
				(*i)->think(SDL_GetTicks());
			} else {
				(*i)->flee(world->mapToNavmesh(pacman->getCenter()), SDL_GetTicks());
			}	
			//std::cout << "ghost move" << std::endl;	//debug
			(*i)->move(wall, SDL_GetTicks(), dt);
			
			if (collision(**i, *pacman)) {
				if (!super) {
					lives--;
					if (lives > 0) {
						world->remove(PACMAN_ID, world->mapToNavmesh(pacman->getCenter()));
						world->setTrail(0, world->mapToNavmesh(pacman->getCenter()));
						delete(pacman);
						pacman = new Object(texture_pacman, texture_pacman->w, texture_pacman->h, world->getCellWidth()*0.8, world->getCellHeight()*0.8, world, PACMAN_ID, pacmanSpeed, pacmanResetPos);
						std::cout << "Lives: " << lives << std::endl;
					} else {
						std::cout << "GAME OVER" << std::endl;
						goto END;
					}
				} else {
					std::list<Ghost*>::iterator prev = i;
					prev--;
					delete(*i);
					ghosts.erase(i);
					i = prev;
					killStreak++;
					score += pow(GHOST_KILL_SCORE, killStreak);
					std::cout << "Score: " << score << std::endl;
				}	
			}
			//std::cout << "ghost collision" << std::endl;	//debug
		}
		
		//draw background
		background->blitTo(*window);
		//draw walls
		for (Vector<float> i : wallsPos) {
			wall->setPos(i);
			wall->blitTo(*window);
		}
		//std::cout << "background drawn" << std::endl;	//debug
		//std::cout << wallsPos.size() << std::endl;	//debug
		
		//draw pills
		draw = world->getFromMesh(PILL_ID);
		if (draw.size() == 0) {
			endGame++;
		}
		for (Vector<float> i : draw) {
			pill->setPos(i);
			if (collision(*pill, *pacman)) {
				score += PILL_SCORE;
				world->remove(PILL_ID, world->mapToNavmesh(i));
				std::cout << "Score: " << score << std::endl;
			} else {
				pill->blitTo(*window);
			}	
		}
		//std::cout << "pills drawn" << std::endl;	//debug
		
		//draw superpills
		draw = world->getFromMesh(SUPERPILL_ID);
		if (draw.size() == 0) {
			endGame++;
		}
		for (Vector<float> i : draw) {
			superpill->setPos(i);
			if (!super && collision(*superpill, *pacman)) {
				score += SUPERPILL_SCORE;
				world->remove(SUPERPILL_ID, world->mapToNavmesh(i));
				super = true;
				superTimeStamp = SDL_GetTicks();
				killStreak = 0;
				for (Ghost *i : ghosts) {
					i->setSpeed(i->getSpeed()/SUPERSPEED_MULT);
					i->setMoveDirection({0, 0});
				}
				std::cout << "Score: " << score << std::endl;
			} else {
				superpill->blitTo(*window);
			}
		}
		//std::cout << "superpills drawn" << std::endl;	//debug
		
		//draw pacman
		if (!super) {
			pacman->blitTo(*window);
		} else {
			superpacman->setPos(pacman->getPos());
			superpacman->blitTo(*window);
		}
		//std::cout << "pacman drawn" << std::endl;	//debug
		
		//draw ghosts
		for (Object *i : ghosts) {
			i->blitTo(*window);
		}
		//std::cout << "ghosts drawn" << std::endl;	//debug
		
		if (endGame == 2) {
			running = false;
			std::cout << "VICTORY" << std::endl;
		} else if (ghosts.empty()) {
			running = false;
			std::cout << "VICTORY" << std::endl;
		}
		
		//std::cout << window->getTimeDelta() << std::endl;	//debug
		dt = window->getTimeDelta();
		window->update();
	}
	
	END:
	
	//clear memory
	delete(window);
	delete(world);
	
	SDL_FreeSurface(texture_background);
	SDL_FreeSurface(texture_wall);
	SDL_FreeSurface(texture_pill);
	SDL_FreeSurface(texture_superpill);
	SDL_FreeSurface(texture_pacman);
	SDL_FreeSurface(texture_ghost);
	SDL_FreeSurface(texture_superpacman);
	
	delete(background);
	delete(wall);
	delete(pill);
	delete(superpill);
	delete(pacman);
	delete(superpacman);
	for (Object *i : ghosts) {
		delete(i);
	}
	
	SDL_Quit();
	return 0;
}
