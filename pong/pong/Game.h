#pragma once
#ifndef GAME_H
#define GAME_H

#include <SDL.h>

#include "Player.h"
#include "Ball.h"
#include "Score.h"

class Game
{
public:
	Game();
	~Game();

	SDL_Renderer* getRenderer()
	{
		return this->_renderer;
	}

	void draw();
	void update();

private:
	SDL_Window * _window;
	SDL_Renderer* _renderer;

	void gameLoop();

	bool _quitFlag;

	Player player1, player2;
	Ball ball;
	Score score1, score2;
};