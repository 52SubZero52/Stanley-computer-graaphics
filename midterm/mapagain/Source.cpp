#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "SDL2-2.0.8\\include\\SDL_mixer.h"
#include "SDL2-2.0.8\\include\\SDL.h"
#include "SDL2-2.0.8\\include\\SDL_image.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2_image-2.0.3\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2_mixer.lib")

#pragma comment(linker,"/subsystem:console")

#include <iostream>
#include <fstream>
#include "Table_File_core.h"


struct Vec2D
{
	float x;
	float y;
};

struct Box_Size
{
	float w;
	float h;
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

namespace Body
{
	struct Body
	{
		Vec2D force;
		Vec2D pos;
		Box_Size size;
		Vec2D vel;
		float mass;
	};

	void init(Body *b)
	{
		*b = {};
		b->mass = 1.0;
	}

	void add_Force(Body *b, float fx, float fy)
	{
		b->force.x += fx;
		b->force.y += fy;
	}

	void update(Body *b, float dt)
	{
		Vec2D a;
		a.x = b->force.x / b->mass;
		a.y = b->force.y / b->mass;
		b->vel.x += a.x * dt;
		b->vel.y += a.y * dt;
		b->pos.x += b->vel.x * dt;
		b->pos.y += b->vel.y * dt;
	}
}

struct RGB
{
	unsigned char r, g, b;
};


namespace Particle_Emitter
{
	struct Particle_Emitter
	{
		Vec2D *pos;
		Vec2D *force;
		Vec2D *vel;
		float *life;
		int *state;
		int n_particles;

		float emitter_mass;
		float particle_mass;
		Vec2D emitter_pos;
		Vec2D emitter_force;
		Vec2D emitter_vel;

		RGB particle_color;
		float particle_size;

	};

	void init(Particle_Emitter *p, int n_particles)
	{
		p->n_particles = n_particles;

		//allocations
		p->pos = new Vec2D[p->n_particles];
		p->force = new Vec2D[p->n_particles];
		p->vel = new Vec2D[p->n_particles];
		p->life = new float[p->n_particles];
		p->state = new int[p->n_particles];

		memset(p->pos, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->force, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->vel, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->life, 0, sizeof(float)*p->n_particles);
		memset(p->state, 0, sizeof(int)*p->n_particles);

		p->emitter_force = {};
		p->emitter_vel = {};
		p->emitter_pos = { 50,50 };//TODO
		p->particle_color = { 165,75,0 };
		p->particle_size = 4.0;
		p->emitter_mass = 1.0;
		p->particle_mass = 0.1;

	}

	void spawn_Many(Particle_Emitter *p, Vec2D influence_min, Vec2D influence_max, int how_many, float min_life, float max_life)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0)
			{
				if (--how_many < 0) break;//TODO

				p->state[i] = 1;

				p->pos[i] = p->emitter_pos;
				p->vel[i] = p->emitter_vel;
				p->life[i] = min_life + (max_life - min_life)*rand() / RAND_MAX;
				p->force[i] = {};

				p->force[i].x += influence_min.x + (influence_max.x - influence_min.x)*rand() / RAND_MAX;
				p->force[i].y += influence_min.y + (influence_max.y - influence_min.y)*rand() / RAND_MAX;
			}
		}
	}

	void clear_Forces_from_Particles(Particle_Emitter *p)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] != 0) p->force[i] = {};
		}
	}

	void add_Force_to_Particles(Particle_Emitter *p, Vec2D f)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] != 0)
			{
				p->force[i].x += f.x;
				p->force[i].y += f.y;
			}
		}
	}

	void update(Particle_Emitter *p, float time_elapsed)
	{
		//implicit euler, mass=1.0
		Vec2D accel;
		accel.x = p->emitter_force.x / p->emitter_mass;
		accel.y = p->emitter_force.y / p->emitter_mass;

		p->emitter_vel.x += accel.x*time_elapsed;
		p->emitter_vel.y += accel.y*time_elapsed;
		p->emitter_pos.x += p->emitter_vel.x*time_elapsed;
		p->emitter_pos.y += p->emitter_vel.y*time_elapsed;

		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0) continue;

			p->life[i] -= time_elapsed;
			if (p->life[i] <= 0.0)
			{
				p->state[i] = 0;
				continue;
			}

			Vec2D accel;
			accel.x = p->force[i].x / p->particle_mass;
			accel.y = p->force[i].y / p->particle_mass;
			p->vel[i].x += accel.x*time_elapsed;
			p->vel[i].y += accel.y*time_elapsed;
			p->pos[i].x += p->vel[i].x*time_elapsed;
			p->pos[i].y += p->vel[i].y*time_elapsed;
		}
	}

	void draw(Particle_Emitter *p, SDL_Renderer *renderer)
	{
		SDL_SetRenderDrawColor(renderer, p->particle_color.r, p->particle_color.g, p->particle_color.b, 255);
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0) continue;

			SDL_Rect rect = { p->pos[i].x,p->pos[i].y,p->particle_size, p->particle_size };
			SDL_RenderFillRect(renderer, &rect);
		}
	}

}




using namespace std;

namespace Tileset
{
	struct Tileset
	{
		int tile_w;
		int tile_h;
		int n_cols;
	};

	int get_Col(int tile_id, Tileset *t)
	{
		return t->tile_w * (tile_id % t->n_cols);
	}

	int get_Row(int tile_id, Tileset *t)
	{
		return t->tile_w * (tile_id / t->n_cols);
	}
}

namespace Game
{
	SDL_Renderer *renderer;
	int screen_width = 32 * 32;
	int screen_height = 32 * 16;
}

int main(int argc, char **argv)
{
	//SDL

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 2048);

	Mix_Music *music2 = Mix_LoadMUS("2 12 Boss Victory.mp3");
	Mix_Music *music = Mix_LoadMUS("3 29 Bowser Battle.mp3");
	//SDL_ShowCursor(SDL_DISABLE);

	SDL_Window *window = SDL_CreateWindow(
		"tilemap",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		Game::screen_width, Game::screen_height, SDL_WINDOW_SHOWN);

	Game::renderer = SDL_CreateRenderer(window,
		-1, SDL_RENDERER_ACCELERATED);

	//sky map

	SDL_Surface *surface = IMG_Load("sky.png");
	SDL_Texture *texture = SDL_CreateTextureFromSurface(Game::renderer, surface);
	SDL_FreeSurface(surface);
	
	Tileset::Tileset tileset_sky;
	tileset_sky.tile_w = 32;
	tileset_sky.tile_h = 32;
	tileset_sky.n_cols = 23;

	Table_File::Table_File sky;
	Table_File::read("dungeon_sky.csv", &sky);

	int **map = new int*[sky.nrows];
	for (int i = 0; i < sky.nrows; i++)
	{
		map[i] = new int[sky.ncols[i]];

		for (int j = 0; j < sky.ncols[i]; j++)
		{
			map[i][j] = atoi(sky.table[i][j]);
		}
	}

	int map_n_cols = sky.ncols[0];
	int map_n_rows = sky.nrows;
	
	//dungeon map

	SDL_Surface *dungeon_surface = IMG_Load("dungeon.png");
	SDL_Texture *dungeon_texture = SDL_CreateTextureFromSurface(Game::renderer, dungeon_surface);
	SDL_FreeSurface(dungeon_surface);

	Tileset::Tileset tileset_dungeon;
	tileset_dungeon.tile_w = 32;
	tileset_dungeon.tile_h = 32;
	tileset_dungeon.n_cols = 23;

	Table_File::Table_File dungeon;
	Table_File::read("dungeon.csv", &dungeon);

	int **dungeon_map = new int*[dungeon.nrows];
	for (int i = 0; i < dungeon.nrows; i++)
	{
		dungeon_map[i] = new int[dungeon.ncols[i]];

		for (int j = 0; j < dungeon.ncols[i]; j++)
		{
			dungeon_map[i][j] = atoi(dungeon.table[i][j]);
		}
	}

	int dungeon_map_n_cols = dungeon.ncols[0];
	int dungeon_map_n_rows = dungeon.nrows;
	//goku tex
	SDL_Surface *goku_surface = IMG_Load("goku2.png");
	SDL_Texture *goku_texture = SDL_CreateTextureFromSurface(Game::renderer, goku_surface);
	SDL_FreeSurface(goku_surface);
	//gob tex
	SDL_Surface *gob_surface = IMG_Load("gob.png");
	SDL_Texture *gob_texture = SDL_CreateTextureFromSurface(Game::renderer, gob_surface);
	SDL_FreeSurface(gob_surface);

	SDL_Surface *knight_surface = IMG_Load("knight.png");
	SDL_Texture *knight_texture = SDL_CreateTextureFromSurface(Game::renderer, knight_surface);
	SDL_FreeSurface(knight_surface);

	SDL_Surface *knight2_surface = IMG_Load("knight.png");
	SDL_Texture *knight2_texture = SDL_CreateTextureFromSurface(Game::renderer, knight2_surface);
	SDL_FreeSurface(knight2_surface);

	SDL_Surface *knight3_surface = IMG_Load("knight.png");
	SDL_Texture *knight3_texture = SDL_CreateTextureFromSurface(Game::renderer, knight3_surface);
	SDL_FreeSurface(knight3_surface);

	double dt = 1.0;
	
	//goku
	Body::Body goku;
	Body::init(&goku);
	goku.pos.x = 0;
	goku.pos.y = 0;
	goku.size.w = 96;
	goku.size.h = 96;
	goku.mass = 40;

	//gob
	Body::Body gob;
	Body::init(&gob);
	gob.pos.x = 820;
	gob.pos.y = 320;
	gob.size.w = 96;
	gob.size.h = 96;
	gob.mass = 10;
	
	//knight
	Body::Body knight;
	Body::init(&knight);
	knight.pos.x =100;
	knight.pos.y = 365;
	knight.size.w = 96;
	knight.size.h = 96;
	knight.mass = 20;

	//knight2
	Body::Body knight2;
	Body::init(&knight2);
	knight2.pos.x = 200;
	knight2.pos.y = 365;
	knight2.size.w = 96;
	knight2.size.h = 96;
	knight2.mass = 20;

	//knight3
	Body::Body knight3;
	Body::init(&knight3);
	knight3.pos.x = 300;
	knight3.pos.y = 365;
	knight3.size.w = 96;
	knight3.size.h = 96;
	knight3.mass = 20;


	bool has_landed = false;
	bool has_punched = false;
	bool been_punched = false;
	bool not_punched = false;
	int time_punch = 0;

	//floor
	Body::Body ground;
	Body::init(&ground);
	ground.pos.x = 0;
	ground.pos.y = 448;
	ground.size.w = Game::screen_width;
	ground.size.h = 64;

	float f_gravity = 0.007;
	float f_move = 0.01;
	float f_jump = f_move * 4;
	float floor_friction = 0.001;
	float wall_friction = 0.8;
	float max_horizontal_vel = 0.2;
	
	float f_move_l = -0.02;
	float f_move_up = -0.0065;
	float f_move_strong = 0.05;
	float f_move_knight = 0.0015;
	float f_move_dance_knight = -0.005;
	float f_gob_up = -0.0085;
	float f_knight_l = 8;


	//animation
	int first_frame_pos_x = 0;
	int first_frame_pos_y = 0;
	int frame_w = 64;
	int frame_h = 64;
	int n_frames = 4;
	int frame_duration = 250;
	unsigned int last_frame_change_time = 0;
	int current_frame = 0;
	
	int knight_frame_pos_x = 0;
	int knight_frame_pos_y = 0;
	int knight_frame_w = 68;
	int knight_frame_h = 68;
	int n_walk_frames = 8;

	int knight_dance_pos_x = 0;
	int knight_dance_pos_y = 68;
	int knight_dance_w = 68;
	int knight_dance_h = 68;
	int n_dance_frames = 8;
	//main loop


	Particle_Emitter::Particle_Emitter rocket;
	Particle_Emitter::init(&rocket, 200);
	rocket.emitter_pos.x = goku.pos.x;
	rocket.emitter_pos.y = goku.pos.y;

	Particle_Emitter::Particle_Emitter sparks_big;
	Particle_Emitter::init(&sparks_big, 50);
	sparks_big.particle_color = { 255,60,0 };
	sparks_big.particle_size = 6;
	sparks_big.particle_mass = 1.0;

	Particle_Emitter::Particle_Emitter sparks_small;
	Particle_Emitter::init(&sparks_small, 50);
	sparks_small.particle_color = { 120,180,0 };
	sparks_small.particle_size = 4;
	sparks_small.particle_mass = 0.25;

	Vec2D f_part_gravity = { 0.0,0.00005 };


			bool done = false;
			if (been_punched ==false)
			{
				Mix_PlayMusic(music, -1);
				
			}
			if (been_punched == true)
			{
				Mix_PlayMusic(music2, -1);
			}
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

				int current_time = SDL_GetTicks();

				int cmd_UP = 0;
				int cmd_LEFT = 0;
				int cmd_RIGHT = 0;
				int cmd_DOWN = 0;

				if (goku.pos.y) cmd_UP = 1;
				if (goku.pos.y) cmd_DOWN = 1;
				if (goku.pos.x) cmd_LEFT = 1;
				if (goku.pos.x) cmd_RIGHT = 1;

				rocket.emitter_force = {};
				sparks_big.emitter_force = {};
				sparks_small.emitter_force = {};

				//goku falls
				goku.force = {};
				gob.force = {};
				knight.force = {};
				knight2.force = {};
				knight3.force = {};
			

				Particle_Emitter::clear_Forces_from_Particles(&rocket);
				Particle_Emitter::clear_Forces_from_Particles(&sparks_big);
				Particle_Emitter::clear_Forces_from_Particles(&sparks_small);

				//apply gravity to emitter
				rocket.emitter_force.x += f_part_gravity.x;
				rocket.emitter_force.y += f_part_gravity.y;
				//apply gravity to particles
				Particle_Emitter::add_Force_to_Particles(&rocket, f_part_gravity);
				Particle_Emitter::add_Force_to_Particles(&sparks_big, f_part_gravity);
				Particle_Emitter::add_Force_to_Particles(&sparks_small, f_part_gravity);


				if (has_landed == true)
				{
					Body::add_Force(&goku, f_move, 0);

				}
				
				if (goku.pos.x > 765)
				{
					f_move = 0;
					goku.vel.x = 0;
				}

				if (current_time - time_punch > 5700)
				{
					
					Body::add_Force(&goku, f_move_l, f_move_up);
				}

				//goblin punched
				if (not_punched == false)
				{	

					Body::add_Force(&knight, f_move_knight, 0);
					Body::add_Force(&knight2, f_move_knight, 0);
					Body::add_Force(&knight3, f_move_knight, 0);
				}


				if (knight.pos.x > 500)
				{
					f_move_knight = 0;
					knight.vel.x = 0;
				}

				if (been_punched == true)
				{
					//cout << "move ur ass";
					Body::add_Force(&gob, f_move_strong, f_gob_up);
					knight.vel.x = -0.05;
			
					Body::add_Force(&knight, f_move_dance_knight, 0);

				}

				if (knight2.pos.x > 400)
				{
					f_move_knight = 0;
					knight2.vel.x = 0;
				}

				if (been_punched == true)
				{
					//cout << "move ur ass";
				
					knight2.vel.x = -0.05;

					Body::add_Force(&knight2, f_move_dance_knight, 0);

				}

				if (knight3.pos.x > 600)
				{
					f_move_knight = 0;
					knight3.vel.x = 0;
				}

				if (been_punched == true)
				{
					//cout << "move ur ass";

					knight3.vel.x = -0.05;

					Body::add_Force(&knight3, f_move_dance_knight, 0);

				}
			//updates
				Body::add_Force(&goku, 0.0, f_gravity);
				Body::update(&goku, dt);
				Body::update(&gob, dt);
				Body::update(&knight, dt);
				Body::update(&knight2, dt);
				Body::update(&knight3, dt);
				Particle_Emitter::update(&rocket, 1.0);
				Particle_Emitter::update(&sparks_big, 1.0);
				Particle_Emitter::update(&sparks_small, 1.0);

				SDL_Rect floor;
				floor.x = 0;
				floor.y = 448;
				floor.w = Game::screen_width;
				floor.h = 64;

				SDL_Rect goku_rect;
				goku_rect.x = goku.pos.x;
				goku_rect.y = goku.pos.y;
				goku_rect.w = 64 * 1.35;
				goku_rect.h = 64 * 1.35;


				int col = Collision::minkowski(goku_rect.x, goku_rect.y, goku_rect.w, goku_rect.h, floor.x, floor.y, floor.w, floor.h);

				if (col == Collision::BOTTOM_OF_1 || col == Collision::TOP_OF_1)
				{
					goku.vel.y *= -0.005;
					f_gravity = -0.0002;
					has_landed = true;
				}
				//goku moves

				if (current_time - last_frame_change_time >= frame_duration)
				{
					current_frame = (current_frame + 1) % n_frames;
					last_frame_change_time = current_time;
				}

			/*	if (current_time - last_frame_change_time >= frame_duration)
				{
					current_frame = (current_frame + 1) % n_walk_frames;
					last_frame_change_time = current_time;
				}
*/
				//DRAW
				SDL_SetRenderDrawColor(Game::renderer, 0, 0, 0, 255);
				SDL_RenderClear(Game::renderer);

				for (int i = 0; i < map_n_rows; i++)
				{
					for (int j = 0; j < map_n_cols; j++)
					{
						int tile_id = map[i][j];

						SDL_Rect src;
						src.x = Tileset::get_Col(tile_id, &tileset_sky);
						src.y = Tileset::get_Row(tile_id, &tileset_sky);
						src.w = 32;
						src.h = 32;

						SDL_Rect dest;
						dest.x = j * tileset_sky.tile_w;
						dest.y = i * tileset_sky.tile_h;
						dest.w = 32;
						dest.h = 32;

						SDL_RenderCopyEx(Game::renderer, texture, &src, &dest, 0, NULL, SDL_FLIP_NONE);

					}
				}

				for (int i = 0; i < dungeon_map_n_rows; i++)
				{
					for (int j = 0; j < dungeon_map_n_cols; j++)
					{
						int tile_id = dungeon_map[i][j];

						SDL_Rect src;
						src.x = Tileset::get_Col(tile_id, &tileset_dungeon);
						src.y = Tileset::get_Row(tile_id, &tileset_dungeon);
						src.w = 32;
						src.h = 32;

						SDL_Rect dest;
						dest.x = j * tileset_dungeon.tile_w;
						dest.y = i * tileset_dungeon.tile_h;
						dest.w = 32;
						dest.h = 32;

						SDL_RenderCopyEx(Game::renderer, dungeon_texture, &src, &dest, 0, NULL, SDL_FLIP_NONE);


					}
				}


				SDL_Rect goku_src;
				goku_src.x = 0;
				goku_src.y = 0;
				goku_src.w = 64;
				goku_src.h = 64;
				
				SDL_Rect goku_punch_src;
				goku_punch_src.x = 64*5;
				goku_punch_src.y = 0;
				goku_punch_src.w = 64;
				goku_punch_src.h = 64;
				
				SDL_Rect goku_fly_src;
				goku_fly_src.x = 64 * 2;
				goku_fly_src.y = 0;
				goku_fly_src.w = 64;
				goku_fly_src.h = 64;

				SDL_Rect goku_dest;
				goku_dest.x = goku.pos.x;
				goku_dest.y = goku.pos.y;
				goku_dest.w = 64*1.5;
				goku_dest.h = 64*1.5;
				
				//Gob render
				SDL_Rect gob_src;
				gob_src.x = 0;
				gob_src.y = 0;
				gob_src.w = 64;
				gob_src.h = 64;
				
				SDL_Rect gob_hit_src;
				gob_hit_src.x = 64;
				gob_hit_src.y = 0;
				gob_hit_src.w = 64;
				gob_hit_src.h = 64;

				SDL_Rect gob_dest;
				gob_dest.x = gob.pos.x;
				gob_dest.y = gob.pos.y;
				gob_dest.w = 64 * 2;
				gob_dest.h = 64 * 2;

				//knight
				SDL_Rect knight_src;
				knight_src.x = 0;
				knight_src.y = 0;
				knight_src.w = 64;
				knight_src.h = 64;

				SDL_Rect knight_dance_src;
				knight_dance_src.x = 0;
				knight_dance_src.y = 0;
				knight_dance_src.w = 64;
				knight_dance_src.h = 64;

				SDL_Rect knight_dest;
				knight_dest.x = knight.pos.x;
				knight_dest.y = knight.pos.y;
				knight_dest.w = 64 * 1.5;
				knight_dest.h = 64 * 1.5;

				//knight2
				SDL_Rect knight2_src;
				knight2_src.x = 0;
				knight2_src.y = 0;
				knight2_src.w = 64;
				knight2_src.h = 64;

				SDL_Rect knight2_dance_src;
				knight2_dance_src.x = 0;
				knight2_dance_src.y = 0;
				knight2_dance_src.w = 64;
				knight2_dance_src.h = 64;

				SDL_Rect knight2_dest;
				knight2_dest.x = knight2.pos.x;
				knight2_dest.y = knight2.pos.y;
				knight2_dest.w = 64 * 1.5;
				knight2_dest.h = 64 * 1.5;
				
				//knight3
				SDL_Rect knight3_src;
				knight3_src.x = 0;
				knight3_src.y = 0;
				knight3_src.w = 64;
				knight3_src.h = 64;

				SDL_Rect knight3_dance_src;
				knight3_dance_src.x = 0;
				knight3_dance_src.y = 0;
				knight3_dance_src.w = 64;
				knight3_dance_src.h = 64;

				SDL_Rect knight3_dest;
				knight3_dest.x = knight3.pos.x;
				knight3_dest.y = knight3.pos.y;
				knight3_dest.w = 64 * 1.5;
				knight3_dest.h = 64 * 1.5;

				
				
				if (has_landed == false)
				{
					SDL_RenderCopyEx(Game::renderer, goku_texture, &goku_src, &goku_dest, 0, NULL, SDL_FLIP_NONE);
					//SDL_RenderCopyEx(Game::renderer, knight_texture, &knight_src, &knight_dest, 0, NULL, SDL_FLIP_NONE);

				}

				if (has_landed == true)
				{
					
					SDL_Rect src;
					src.x = first_frame_pos_x + current_frame * frame_w;
					src.y = first_frame_pos_y;
					src.w = frame_w;
					src.h = frame_h;
					//SDL_RenderCopyEx(Game::renderer, goku_texture, &src, &goku_dest, 0, NULL, SDL_FLIP_NONE);
					
					if (has_punched == false)
					{
						if (goku.pos.x < 765)
						{
							SDL_RenderCopyEx(Game::renderer, goku_texture, &src, &goku_dest, 0, NULL, SDL_FLIP_NONE);
							//SDL_RenderCopyEx(Game::renderer, knight_texture, &src, &knight_dest, 0, NULL, SDL_FLIP_NONE);
						}
						if (goku.pos.x > 765)
						{
							SDL_RenderCopyEx(Game::renderer, goku_texture, &goku_punch_src, &goku_dest, 0, NULL, SDL_FLIP_NONE);
							been_punched = true;
						}
					}
					
					if (current_time - time_punch > 4500)
					{
						
						SDL_RenderCopyEx(Game::renderer, goku_texture, &goku_fly_src, &goku_dest, 0, NULL, SDL_FLIP_HORIZONTAL);
						has_punched = true;
					}

				}
				if (been_punched == false)
				{
					SDL_RenderCopyEx(Game::renderer, gob_texture, &gob_src, &gob_dest, 0, NULL, SDL_FLIP_NONE);
				}
				if (been_punched == true)
				{
					SDL_RenderCopyEx(Game::renderer, gob_texture, &gob_hit_src, &gob_dest, 0, NULL, SDL_FLIP_NONE);
				
				}
				
				SDL_Rect src;
				src.x = knight_frame_pos_x + current_frame * knight_frame_w;
				src.y = knight_frame_pos_y;
				src.w = knight_frame_w;
				src.h = knight_frame_h;


				if (been_punched == false)
				{
					if (knight.pos.x < 500)
					{
						SDL_RenderCopyEx(Game::renderer, knight_texture, &src, &knight_dest, 0, NULL, SDL_FLIP_NONE);
					}
					if (knight.pos.x > 500)
					{
						SDL_RenderCopyEx(Game::renderer, knight_texture, &src, &knight_dest, 0, NULL, SDL_FLIP_HORIZONTAL);
					}
			
				}

				
				
				if (been_punched == true)
				{
					
					SDL_Rect src;
					src.x = knight_dance_pos_x + current_frame * knight_dance_w;
					src.y = knight_dance_pos_y;
					src.w = knight_dance_w;
					src.h = knight_dance_h;

					SDL_RenderCopyEx(Game::renderer, knight_texture, &src, &knight_dest, 0, NULL, SDL_FLIP_HORIZONTAL);
				
				}

				//------------------------------------------------------------------------------------------------------------------------------
				
				SDL_Rect src2;
				src2.x = knight_frame_pos_x + current_frame * knight_frame_w;
				src2.y = knight_frame_pos_y;
				src2.w = knight_frame_w;
				src2.h = knight_frame_h;


				if (been_punched == false)
				{
					if (knight2.pos.x < 400)
					{
						SDL_RenderCopyEx(Game::renderer, knight2_texture, &src2, &knight2_dest, 0, NULL, SDL_FLIP_NONE);
					}
					if (knight2.pos.x > 400)
					{
						SDL_RenderCopyEx(Game::renderer, knight2_texture, &src2, &knight2_dest, 0, NULL, SDL_FLIP_HORIZONTAL);
					}

				}



				if (been_punched == true)
				{

					SDL_Rect src2;
					src2.x = knight_dance_pos_x + current_frame * knight_dance_w;
					src2.y = knight_dance_pos_y;
					src2.w = knight_dance_w;
					src2.h = knight_dance_h;

					SDL_RenderCopyEx(Game::renderer, knight2_texture, &src2, &knight2_dest, 0, NULL, SDL_FLIP_HORIZONTAL);

				}



				SDL_Rect src3;
				src3.x = knight_frame_pos_x + current_frame * knight_frame_w;
				src3.y = knight_frame_pos_y;
				src3.w = knight_frame_w;
				src3.h = knight_frame_h;


				if (been_punched == false)
				{
					if (knight3.pos.x < 600)
					{
						SDL_RenderCopyEx(Game::renderer, knight3_texture, &src3, &knight3_dest, 0, NULL, SDL_FLIP_NONE);
					}
					if (knight3.pos.x > 600)
					{
						SDL_RenderCopyEx(Game::renderer, knight3_texture, &src3, &knight3_dest, 0, NULL, SDL_FLIP_HORIZONTAL);
					}

				}



				if (been_punched == true)
				{

					SDL_Rect src3;
					src3.x = knight_dance_pos_x + current_frame * knight_dance_w;
					src3.y = knight_dance_pos_y;
					src3.w = knight_dance_w;
					src3.h = knight_dance_h;

					SDL_RenderCopyEx(Game::renderer, knight3_texture, &src3, &knight3_dest, 0, NULL, SDL_FLIP_HORIZONTAL);

				}

				SDL_SetRenderDrawColor(Game::renderer, 255, 0, 255, 255);
				SDL_Rect rect = { rocket.emitter_pos.x,rocket.emitter_pos.y,5,5 };
				SDL_RenderFillRect(Game::renderer, &rect);

				//draw rocket boost
				Particle_Emitter::draw(&rocket, Game::renderer);
				//draw sparks
				Particle_Emitter::draw(&sparks_big, Game::renderer);
				Particle_Emitter::draw(&sparks_small, Game::renderer);

				//SDL_RenderDrawRect(Game::renderer, &goku_rect);
				//SDL_RenderDrawRect(Game::renderer, &floor);
				
				SDL_RenderPresent(Game::renderer);
			}

			return 0;

		
	}
