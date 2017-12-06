#pragma once

#include "Lab309_ADT_Matrix.h"
#include "window.h"
#include "sprite.h"


//IDS DE OBJETOS
#define PACMAN_ID		0x1
#define GHOST_ID		0x2
#define PATHWAY_ID		0x4
#define PILL_ID			0x8
#define SUPERPILL_ID	0x10
#define WALL_ID			0x20

namespace lab309 {
	
	class World {
		
		public class Cell {
			private:
				int contents;
				
			public:
				
		};
		
		//INTERNAL METHODS
		private:
			Vector<int> mapToNavmesh (const Vector<float> &pos);
		
		//ATRIBUTES
		private:
			Matrix<Cell> navmesh;
			Window *window;
			
		//METHODS
		public:
			World (const Window &window, size_t navmeshWidth, size_t navmeshHeight);
			
			void add (int id, const Vector<float> &pos);	//adiciona objeto a malha de navegacao
	
	};
	
};
