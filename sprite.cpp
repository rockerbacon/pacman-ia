#include "sprite.h"
#include <iostream>

/*CONSTRUCTORS*/
lab309::Sprite::Sprite (SDL_Surface *texture, int rectWidth, int rectHeight, int displayWidth, int displayHeight) {
	this->texture = texture;
	this->rect = {0, 0, rectWidth, rectHeight};
	this->displayRect = {0, 0, displayWidth, displayHeight};
	this->pos = {0, 0};
}

/*GETTERS*/
lab309::Vector<float> lab309::Sprite::getSpritePos (void) const {
	return { (float)this->rect.x/this->rect.w, (float)this->rect.y/this->rect.h };
}

lab309::Vector<float> lab309::Sprite::getPos (void) const {
	return this->pos;
}

int lab309::Sprite::getXPos (void) const {
	return this->pos[_X];
}

int lab309::Sprite::getYPos (void) const {
	return this->pos[_Y];
}

lab309::Vector<float> lab309::Sprite::getCenter (void) const {
	return { this->pos[_X]+this->displayRect.w/2.0f, this->pos[_Y]+this->displayRect.h/2.0f };
}

int lab309::Sprite::getDisplayWidth (void) const {
	return this->displayRect.w;
}

int lab309::Sprite::getDisplayHeight (void) const {
	return this->displayRect.h;
}

/*SETTERS*/
void lab309::Sprite::setSpritePos (const lab309::Vector<float> &pos) {
	this->rect.x = (int)(pos[_X]*this->rect.w+0.5f);
	this->rect.y = (int)(pos[_Y]*this->rect.h+0.5f);
}

void lab309::Sprite::setPos (const lab309::Vector<float> &pos) {
	this->pos = pos;
}

/*METHODS*/
void lab309::Sprite::blitTo (const lab309::Window &window) {
	this->displayRect.x = this->pos[_X] + 0.5f;
	this->displayRect.y = this->pos[_Y] + 0.5f;
	SDL_BlitScaled(this->texture, &this->rect, window.surface, &this->displayRect);
}

void lab309::Sprite::translate (const Vector<float> &offset) {
	this->pos = this->pos+offset;
}

int lab309::collision (const lab309::Sprite &a, const lab309::Sprite &b) {
	int colx, coly;
	int	bxmax = b.pos[_X]+b.displayRect.w,
		bymax = b.pos[_Y]+b.displayRect.h,
		axmax = a.pos[_X]+a.displayRect.w,
		aymax = a.pos[_Y]+a.displayRect.h;
		
	colx = bxmax > a.pos[_X] && b.pos[_X] < axmax;
	coly = bymax > a.pos[_Y] && b.pos[_Y] < aymax;
	return colx && coly;
}
