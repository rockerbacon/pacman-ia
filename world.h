#pragma once

#include "Lab309_ADT_Matrix.h"
#include "window.h"
#include "sprite.h"
#include <list>

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

namespace lab309 {
	
	class World {
		
		public class Cell {
			private:
				int contents;
				
			public:
				Cell (void) : contents(0) {}
				Cell (int contents) : contents(contents) {}
		};
		
		//INTERNAL METHODS
		private:
			Vector<int> mapToNavmesh (const Vector<float> &pos) const;
			Vector<float> mapToPixel (const Vector<int> &navPos) const;
		
		//ATRIBUTES
		private:
			Matrix<Cell> navmesh;
			Window *window;
			
		//METHODS
		public:
			World (const Window &window, size_t navmeshWidth, size_t navmeshHeight);
			World (const Window &window);
			
			Vector<int> add (int id, const Vector<float> &pos);	//adiciona objeto a malha de navegacao retornando a posicao na malha em que foi adicionado
			
			std::list<Vector<float>> getFromMesh (int content) const;	//retorna uma lista com a posicao (em pixels) de todos as celulas da malha que contenham as caracteristicas em content
			
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
	
};
