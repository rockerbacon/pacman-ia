#pragma once

#include "world.h"

#define CELL_DETECTION_X_TOLERANCE 0.17
#define CELL_DETECTION_Y_TOLERANCE 0.17

#define GHOST_VIEW_DISTANCE 5

#define IGNORE_PACMAN_TRAIL 3000 //tempo apos o qual qualquer trail do pacman deve ser ignorado
#define IGNORE_GHOST_TRAIL 3000 //tempo durante o qual o fantasma nao deve revisitar uma celula

#define HIDE_BEHIND_WALLS true

namespace lab309 {

	const float chances[4] = {1.05f, 0.85f, 0.7f, 0.5f};
	
	class Ghost : public Object {
		protected:
			Vector<int> previousCell;
		public:
			Ghost (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos);
			
			//sets the direction the ghost should move
			void think (unsigned long int currentTime);
			void move (unsigned long int currentTime, double elapsedTime);
			void flee (const Vector<int> &pacmanPos, unsigned long int currentTime);
			
	};

};
