#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "cab202_graphics.h"
#include "cab202_sprites.h"
#include "cab202_timers.h"
#include "doodlegame.h"
#include <ncurses.h>

int rand_between(int first, int last) 
{
	return first + rand() % (last - first);
}

//setup game borders and scores and time etc
void setup_game(DoodleGame * doodlegame)
{
	doodlegame->over = false;
	doodlegame->score = 0;
	doodlegame->level = 1;
	doodlegame->event_loop_delay = 10;
	doodlegame->life = 3;
	doodlegame->seconds = 0;
	doodlegame->minutes = 0;
	doodlegame->over = false;
	doodlegame->secondstimer = create_timer( 1000 );
}

// draw out the game borders, scores, time etc
void draw_game(DoodleGame * doodlegame)
{
	draw_line(0, screen_height() - 2, screen_width() - 1, screen_height() - 2, '-');
	draw_line(0, 1, screen_width() - 1, 1, '-');
	draw_formatted(0 , screen_height() - 1, "Level: %d , Score: %d", doodlegame->level, doodlegame->score);
	draw_formatted(screen_width() - 12 , 0, "Time: %d: %d", doodlegame -> minutes, doodlegame -> seconds );
	draw_formatted(0, 0, "Lives: %d", doodlegame -> life);
	draw_formatted(screen_width()-5, screen_height()-1, "%s", doodlegame->speed);
}
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

//setup character
void setup_player(DoodleGame * doodlegame)
{
	     Sprite cross;
     unsigned char bm[] = 
     {
        0b00011000,
        0b00011000,
        0b00011000,
        0xFF,
        0xFF,
        0b00011000,
        0b00011000,
        0b00011000
     };

     init_sprite(&cross,LCD_X/2,LCD_Y-2,8,8,bm);

     draw_sprite(&cross);
	doodlegame -> player -> dy = 1.;

	doodlegame->player_timer = create_timer(PLAYER_TIMER);
}

//------------------------------------------------------------------------------------//

//set the gravity of the character for it to fall down
bool movement_player(DoodleGame * doodlegame)
{


	if (!doodlegame -> player -> is_visible || !timer_expired(doodlegame -> player_timer))
	{
	return false;
	}

	int old_y; int old_x; int new_y; int new_x;

	old_y = (int) round(doodlegame -> player -> y);
	old_x = (int) round(doodlegame -> player -> x);
	doodlegame -> player -> y += doodlegame -> player -> dy;
	new_y = (int) round(doodlegame -> player -> y);
	new_x = (int) round(doodlegame -> player -> x);


	return (old_x != new_x || old_y != new_y);
}

//------------------------------------------------------------------------------------//

//set the collision detection to score and set user controls
bool update_player(DoodleGame * doodlegame, int key_code)
{

	for (int i = 0; i < N_SAFE; i++)

	{	//collision detection to detect '=' and add score++

		if(get_screen_char(doodlegame->player->x, doodlegame->player->y+3) == '=')
		{
			if (doodlegame->is_falling)
			{
				doodlegame->score++;
			}
			sprite_back(doodlegame->player);
			doodlegame->is_falling = false;
		}

		else if (get_screen_char(doodlegame->player->x, doodlegame->player->y+4) == ' ')
		{
			doodlegame->is_falling = true;
		}	//player dies when in contact with forbidden blocks or the border
		else if (get_screen_char(doodlegame->player->x, doodlegame->player->y+3) == 'x' || 
				get_screen_char(doodlegame->player->y, doodlegame->player->y+3) == '-')
		{
			doodlegame -> life--;
			doodlegame -> player -> y = screen_height() - 7;
			doodlegame -> player -> x = 0;
			doodlegame -> initsafeblock -> y = screen_height()-4;
			doodlegame -> initsafeblock -> x = 0;
			if (doodlegame-> life == 0)
			{
				doodlegame-> over = true;
			}

		}
	}

	int old_x, old_y, new_y, new_x;

	old_x = (int) round(doodlegame -> player -> x);
	old_y = (int) round(doodlegame -> player -> y);
	
	//controls for the game
	switch (key_code)
	{
		case KEY_LEFT:
			if (doodlegame->player->x > 0) 
			{
				doodlegame->player->x -= 1;
			}
			break;
		/*case KEY_DOWN:
			if ((doodlegame->player->y + doodlegame->player->height) < (screen_height() - 2)) 
			{
				doodlegame->player->y += 1;
			}
			break;*/
		case KEY_RIGHT:
			if ((doodlegame->player->x + doodlegame->player->width) < screen_width()) 
			{
				doodlegame->player->x += 1;
			}
			break;
		/*case KEY_UP:
			if (doodlegame->player->y > 2) 
			{
				doodlegame->player->y -= 1;
			}
			break;*/
		default:
			return false;
	
	}
	new_x = (int) round(doodlegame->player->x);
	new_y = (int) round(doodlegame->player->y);

	return (new_x != old_x) || (new_y != old_y);



}

//------------------------------------------------------------------------------------//

//draw out the character
void draw_player(DoodleGame * doodlegame)
{
	sprite_draw(doodlegame -> player);
}

//------------------------------------------------------------------------------------//

//clears the memory of the sprite created
void cleanup_player(DoodleGame * doodlegame) 
{
	sprite_destroy(doodlegame -> player);
	doodlegame -> player = NULL;
}
//------------------------------------------------------------------------------------//

//setup for the blocks
void setup_blocks_safe(DoodleGame * doodlegame, int rock_x[], int rock_y[])
{

	static char safeblock[14] =
	"======="
	"=======";

	static char forbid[14] =
	"xxxxxxx"
	"xxxxxxx";

	for (int i = 0; i < 7; i++)
	{
		if (i == 3 || i == 5 )
		{
			doodlegame -> safe_blocks[i] = sprite_create(rock_x[i], rock_y[i], 7, 2, forbid);

		}
		else 
		{
		doodlegame -> safe_blocks[i] = sprite_create(rock_x[i], rock_y[i], 7, 2, safeblock);
		}
		doodlegame -> safe_blocks[i] -> dy -= 1.;
	}
	doodlegame -> block_timer = create_timer(BLOCK_TIMER);
}
//------------------------------------------------------------------------------------//
void draw_safe_blocks(DoodleGame * doodlegame)
{
	for (int i = 0; i < N_SAFE; i++)
	{
		sprite_draw(doodlegame -> safe_blocks[i]);
	}
}
//------------------------------------------------------------------------------------//
bool move_safe_blocks(sprite_id safe_blocks)
{
	if (!safe_blocks -> is_visible)
	{
		return false;
	}

	int old_y = (int) round(safe_blocks -> y);
	safe_blocks -> y += safe_blocks -> dy;
	int new_y = (int) round(safe_blocks -> y);
	int old_x = (int) round(safe_blocks -> x);
	safe_blocks -> x += safe_blocks -> dx;
	int new_x = (int) round(safe_blocks -> x);

	if (new_y <= 2)
	{
		safe_blocks -> y = screen_height() - 4;
		safe_blocks -> x = rand_between(0, screen_width() - 7);
		return true;
	}
	return (old_y != new_y || old_x != new_x);

}

bool update_safe_blocks(DoodleGame * doodlegame)
{
		if (!timer_expired(doodlegame->block_timer)) {
		return false;
	}

	bool changed = false;

	for (int i = 0; i < N_SAFE; i++) {
		sprite_id safe_blocks = doodlegame->safe_blocks[i];
		changed = move_safe_blocks(safe_blocks) || changed;
	}

	return changed;
}



//------------------------------------------------------------------------------------//


//------------------------------------------------------------------------------------//

void setup_time(DoodleGame * doodlegame)
{
	if (timer_expired( doodlegame -> secondstimer) ) 
	{
		doodlegame -> seconds++;
		if (doodlegame -> seconds >= 60) 
		{
			doodlegame -> minutes++;
			doodlegame -> seconds = 0;
		}
	}
}


//------------------------------------------------------------------------------------//

void setup_initsafeblock(DoodleGame * doodlegame) 
{
	static char initsafeblock[] =
		"======="
		"=======";

	doodlegame->initsafeblock = sprite_create(0, screen_height() - 4, 7, 2, initsafeblock);
	doodlegame->initsafeblock->dy -= 1. ;
}
/*----------------------------------------------------------------------------*/
// Draw initial safe block on screen
void draw_initsafeblock(DoodleGame * doodlegame) 
{
	sprite_draw(doodlegame->initsafeblock);
}
/*----------------------------------------------------------------------------*/
// Move initial safe block at upward motion if visible on screen
bool move_initsafeblock(sprite_id initsafeblock) 
{
	if (!initsafeblock->is_visible) 
	{
		return false;
	}

	int old_y = (int) round(initsafeblock->y);
	initsafeblock->y += initsafeblock->dy;
	int new_y = (int) round(initsafeblock->y);

	return old_y != new_y;
}
/*----------------------------------------------------------------------------*/
// Update initial safe block position according to a set timer
bool update_initsafeblock(DoodleGame* doodlegame) 
{
	if (!timer_expired(doodlegame->block_timer)) 
	{
		return false;
	}

	bool changed = false;

	sprite_id initsafeblock = doodlegame->initsafeblock;
	changed = move_initsafeblock(initsafeblock) || changed;

	return changed;
}
/*----------------------------------------------------------------------------*/
//Clear memory of initial safe block
void cleanup_initsafeblock(DoodleGame* doodlegame) 
{
	sprite_destroy(doodlegame->initsafeblock);
	doodlegame->initsafeblock = NULL;

}


//------------------------------------------------------------------------------------//

void setup_level(DoodleGame * doodlegame)
{
	int key_code = get_char();
	if (key_code == 'l')
	{
		doodlegame->level++;
		doodlegame->block_timer =  create_timer(500/ doodlegame->level);

	}
	else if (key_code == '1')
	{
		doodlegame->level = 1;
		doodlegame->block_timer = create_timer(500*4);
		doodlegame-> speed = "SLOW";
	}
	else if (key_code == '2')
	{
		doodlegame->level = 2;
		doodlegame->block_timer = create_timer(500);
		doodlegame-> speed = "NORM";
	}
	else if (key_code == '3')
	{
		doodlegame->level = 3;
		doodlegame->block_timer = create_timer(500/4);
		doodlegame-> speed = "FAST";
	}
}

//------------------------------------------------------------------------------------//

void setup_all(DoodleGame * doodlegame)
{
	setup_game(doodlegame);
	setup_player(doodlegame);
	int safe_blocks_x[N_SAFE], safe_blocks_y[N_SAFE];
	for (int i = 0; i < N_SAFE; i++)
	{
		safe_blocks_x[i] = rand_between(0, screen_width() - 7);
		safe_blocks_y[i] = rand_between(2, screen_height() - 3);
	}
	setup_blocks_safe(doodlegame, safe_blocks_x, safe_blocks_y);
	setup_initsafeblock(doodlegame);
}

//------------------------------------------------------------------------------------//

void draw_all(DoodleGame * doodlegame)
{
	clear_screen();
	draw_game(doodlegame);
	draw_player(doodlegame);
	draw_safe_blocks(doodlegame);
	draw_initsafeblock(doodlegame);
	show_screen();
}

void cleanup_all(DoodleGame * doodlegame) {
	cleanup_player(doodlegame);
	cleanup_initsafeblock(doodlegame);

}

//------------------------------------------------------------------------------------//

void event_loop()
{
	DoodleGame doodlegame;

	setup_all(&doodlegame);
	draw_all(&doodlegame);

	while (!doodlegame.over)
	{	
		setup_time(&doodlegame);
		movement_player(&doodlegame);
		int key_code = get_char();


		if (key_code == 'q')
		{
			doodlegame.over = true;
		}
		else
		{
			bool show_player = update_player(&doodlegame, key_code);
			bool show_safe_blocks = update_safe_blocks(&doodlegame);
			bool show_initsafeblock = update_initsafeblock(&doodlegame);

			if (show_player || show_safe_blocks || show_initsafeblock)
			{
				draw_all(&doodlegame);
			}
		}
		setup_level(&doodlegame);

		if (doodlegame.life == 0)
		{
			if (key_code == 'r')
			{
				doodlegame.over = false;
			}
		}
	}
	if (doodlegame.over)
	{
		clear_screen();
		draw_formatted(10,30, "press 'R' to restart");
		show_screen();
		timer_pause(2000);
		//restart game
		if (get_char() == 'r')
		{
			doodlegame.over = false;
			setup_all(&doodlegame);
		}
	}

	cleanup_all(&doodlegame);
}

//------------------------------------------------------------------------------------//

int main()
{
	setup_screen();
	auto_save_screen = true;
	event_loop();
	cleanup_screen();


	return 0;
}