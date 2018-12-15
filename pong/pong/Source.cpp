#include <iostream>
#include <assert.h>
using namespace std;

//include SDL header
#include "SDL2-2.0.8\include\SDL.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")

#pragma comment(linker,"/subsystem:console")

struct Pad_Pos
{
	float x, y;

};
struct Pad_Size
{
	float w, h;
};
struct Pad_Speed
{
	float sx, sy;
};





struct Box_Pos
{
	float x, y;
};
struct Box_Size
{
	float w, h;
};
struct Box_Speed
{
	float sx, sy;
};


namespace Collision
{
	enum { NO_COLLISION = 0, TOP_OF_1, RIGHT_OF_1, BOTTOM_OF_1, LEFT_OF_1 };

	int minkowski(float x0, float y0, float w0, float h0, float x1, float y1, float w1, float h1)
	{
		float w = 0.5 * (w0 + w1);
		float h = 0.5 * (h0 + h1);
		float dx = x0 - x1 + 0.5*(w0 - w1);
		float dy = y0 - y1 + 0.5*(h0 - h1);

		if (dx*dx <= w * w && dy*dy <= h * h)
		{
			float wy = w * dy;
			float hx = h * dx;

			if (wy > hx)
			{
				return (wy + hx > 0) ? BOTTOM_OF_1 : LEFT_OF_1;
			}
			else
			{
				return (wy + hx > 0) ? RIGHT_OF_1 : TOP_OF_1;
			}
		}
		return NO_COLLISION;
	}
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	int screen_width = 800;
	int screen_height = 600;

	SDL_Window *window = SDL_CreateWindow(
		"Pong",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_ACCELERATED);

	int n_boxes = 1;
	Box_Pos *box_pos = new Box_Pos[n_boxes];
	Box_Size *box_size = new Box_Size[n_boxes];
	Box_Speed *box_speed = new Box_Speed[n_boxes];

	//Pads

	int n_pads = 1;
	Pad_Pos *pad_pos = new Pad_Pos[n_pads];
	Pad_Size *pad_size = new Pad_Size[n_pads];
	Pad_Speed *pad_speed = new Pad_Speed[n_pads];

	Pad_Pos *pad2_pos = new Pad_Pos[n_pads];
	Pad_Size *pad2_size = new Pad_Size[n_pads];
	Pad_Speed *pad2_speed = new Pad_Speed[n_pads];

	Pad_Pos *pad3_pos = new Pad_Pos[n_pads];
	Pad_Size *pad3_size = new Pad_Size[n_pads];
	Pad_Speed *pad3_speed = new Pad_Speed[n_pads];


	float speed_coefficient = 1;

	for (int i = 0; i < n_boxes; i++)
	{
		box_pos[i].x = screen_width / 2;
		box_pos[i].y = screen_height / 2;

		box_size[i].w = 4 + rand() % 9;
		box_size[i].h = box_size[i].w;

		box_speed[i].sx = speed_coefficient * (0.2);
		box_speed[i].sy = speed_coefficient * (0.2);
	}

	for (int i = 0; i < n_pads; i++)
	{
		pad_pos[i].x = screen_width / 10;
		pad_pos[i].y = box_pos[i].y;


		pad_size[i].w = 20;
		pad_size[i].h = 80;


		pad_speed[i].sy = box_speed[i].sy;
	}

	for (int i = 0; i < n_pads; i++)
	{
		pad2_pos[i].x = 700;
		pad2_pos[i].y = 500;


		pad2_size[i].w = 20;
		pad2_size[i].h = 80;


		pad2_speed[i].sy = box_speed[i].sy;
	}
	

	for (int i = 0; i < n_pads; i++)
	{
		pad3_pos[i].x = screen_width/2;
		pad3_pos[i].y = -9;


		pad3_size[i].w = 20;
		pad3_size[i].h = screen_height-12;


	}

	
	bool done = false;
	while (!done)
	{
		//consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				done = true;
			}
		}

		/*
		GAME CODE
		*/




		//UPDATE
		for (int i = 0; i < n_boxes; i++)
		{
			box_pos[i].x += box_speed[i].sx;
			box_pos[i].y += box_speed[i].sy;

			if (box_pos[i].x < 0 || box_pos[i].x + box_size[i].w > screen_width)
			{
				box_speed[i].sx *= -1.0;
			}
			if (box_pos[i].y < 0 || box_pos[i].y + box_size[i].h > screen_height)
			{
				box_speed[i].sy *= -1.0;
			}

		}

		for (int i = 0; i < n_pads; i++)
		{
			pad_pos[i].y += pad_speed[i].sy;


			if (pad_pos[i].y < 0 || pad_pos[i].y + pad_size[i].h > screen_height)
			{
				pad_speed[i].sy *= -1.0;
			}

		}

		for (int i = 0; i < n_pads; i++)
		{
			pad2_pos[i].y += pad2_speed[i].sy;


			if (pad2_pos[i].y < 0 || pad2_pos[i].y + pad2_size[i].h > screen_height)
			{
				pad2_speed[i].sy *= -1.0;
			}

		}




		//RENDER

		//set color to white
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		//clear screen with white
		SDL_RenderClear(renderer);

		//set color to red
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);


		for (int i = 0; i < n_pads; i++)
		{
			SDL_Rect rect;
			rect.x = pad_pos[i].x;
			rect.y = pad_pos[i].y;
			rect.w = pad_size[i].w;
			rect.h = pad_size[i].h;


			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			//draw filled rectangle in the backbuffer
			SDL_RenderFillRect(renderer, &rect);
		}

		for (int i = 0; i < n_pads; i++)
		{
			SDL_Rect rect;
			rect.x = pad2_pos[i].x;
			rect.y = pad2_pos[i].y;
			rect.w = pad2_size[i].w;
			rect.h = pad2_size[i].h;



			//draw filled rectangle in the backbuffer
			SDL_RenderFillRect(renderer, &rect);
		}

		for (int i = 0; i < n_pads; i++)
		{
			SDL_Rect rect;
			rect.x = pad3_pos[i].x;
			rect.y = pad3_pos[i].y;
			rect.w = pad3_size[i].w;
			rect.h = pad3_size[i].h;



			//draw filled rectangle in the backbuffer
			SDL_RenderFillRect(renderer, &rect);
		}

		for (int i = 0; i < n_boxes; i++)
		{
			SDL_Rect rect;
			rect.x = box_pos[i].x;
			rect.y = box_pos[i].y;
			rect.w = box_size[i].w;
			rect.h = box_size[i].h;

			//draw filled rectangle in the backbuffer
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

			SDL_RenderFillRect(renderer, &rect);
		}
		//flip buffer
		SDL_RenderPresent(renderer);

	}



	return 0;
}
