#pragma once

#include "Lab309_ADT_Matrix.h"
#include "window.h"
#include "sprite.h"
#include <list>
#include <vector>

//IDS DE OBJETOS
#define PACMAN_ID		0x1
#define GHOST_ID		0x2
#define PATHWAY_ID		0x4
#define PILL_ID			0x8
#define SUPERPILL_ID	0x10
#define WALL_ID			0x20

//REPRESENTACAO DE MAPA EM ARQUIVO
#define EMPTY_CELL		'0'
#define WALL_CELL		'1'
#define PACMAN_CELL		'k'
#define GHOST_CELL		'y'
#define PILL_CELL		'.'
#define SUPERPILL_CELL	'*'

#define COLLISION_BOX_X_TOLERANCE 0.17
#define COLLISION_BOX_Y_TOLERANCE 0.17

namespace lab309 {
	
	class World {
		
		public: class Cell {
			public:
				int contents;
				unsigned long int trailTimeStamp;
				unsigned long int ghostTrailTimeStamp;
				
			public:
				Cell (void);
				Cell (int contents);

		};
		
		//INTERNAL METHODS
		public:
			Vector<int> mapToNavmesh (const Vector<float> &pos) const;
			Vector<float> mapToPixel (const Vector<int> &navPos) const;
		
		//ATRIBUTES
		private:
			Matrix<Cell> navmesh;
			const Window *window;
			
		//METHODS
		public:
			World (const Window &window, size_t navmeshWidth, size_t navmeshHeight);
			World (const Window &window);
			
			/*GETTERS*/
			int getCellWidth (void) const;
			int getCellHeight (void) const;
			Cell getCell (const Vector<int> &pos) const;
			
			size_t getWidth (void) const;	//retorna tamanho da malha do mundo
			size_t getHeight (void) const;
			
			/*METHODS*/
			void add (int id, const Vector<int> &pos);	//adiciona objeto a malha de navegacao retornando a posicao na malha em que foi adicionado
			std::list<Vector<float>> getFromMesh (int content) const;	//retorna uma lista com a posicao (em pixels) de todos as celulas da malha que contenham as caracteristicas em content
			
			void remove (int id, const Vector<int> &pos);
			
			void setTrail(unsigned long int timeStamp, const Vector<int> &cell);
			
			void setGhostTrail(unsigned long int timeStamp, const Vector<int> &cell);
			
			/*
			 * Le uma malha de um arquivo
			 * O arquivo deve conter:
			 * 	-Dois inteiros iniciais indicando a largura e altura do mapa
			 * 	-Caracteres representando seu conteudo de acordo com as macros no inicio desse arquivo
			 * Espacos e tabs sao ignorados durante a leitura
			 * Retorna true ao concluir ou false caso nao seja possivel ler o arquivo
			 */
			bool readFromFile (const char *filePath);
	
	};
	
	class Object : public Sprite {
		protected:
			World *world;
			int id;
			float speed;	//in pixels/s
			Vector<int> currentCell;
			Vector<float> moveDirection;
			
		public:
			/*CONSTRUCTORS*/
			Object (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight, World *world, int id, float speed, const Vector<float> &initialPos);
			
			/*SETTERS*/
			void setMoveDirection (const Vector<float> &moveDirection);
			
			/*GETTERS*/
			float getSpeed (void) const;
			Vector<int> getCurrentCell (void) const;
			Vector<float> getMoveDirection (void) const;
			
			/*METHODS*/
			void move (Sprite *wall, unsigned long int currentTime, double elapsedTime);	//current time in miliseconds, elapsed time in seconds
			void setSpeed (float speed);
			
	};
	
};
