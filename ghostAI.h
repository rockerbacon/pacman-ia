#pragma once

#include "world.h"

namespace lab309 {

	const float chances[4] = {0.5f, 0.7f, 0.85f, 1.1f};
	
	class Ghost : public Object {
		protected:
			Vector<int> previousCell;
			size_t viewDistance;
		public:
			Ghost (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos);
			
			//sets the direction the ghost should move
			void think (unsigned int currentTime);
			void flee (const Vector<int> &pacmanPos, unsigned int currentTime);
			
	};

};
