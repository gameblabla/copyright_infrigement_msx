// -*- coding: utf-8-unix -*-
/*
 * Copyright (c) 2021-2023 Daishi Mori (mori0091)
 *
 * This software is released under the MIT License.\n
 * See https://github.com/mori0091/libmsx/blob/main/LICENSE
 *
 * GitHub libmsx project\n
 * https://github.com/mori0091/libmsx
 */
/**
 * \file graphics.c
 * \brief `MSX2` VDP command usecase.
 */

#include <msx.h>
#include <screen.h>
#include <stdint.h>
#include <text.h>
#include <resources.h>
#include <stdlib.h>
#include <bmem.h>
#include <ay_3_8910.h>

#include "vdp_functions.h"
#include "graphics_msx2/files_msx.h"
#include "sprites.h"
#include "./pcmenc.h"

uint8_t game_mode = 0, flag = 0;
uint8_t status, time = 0, wait = 0;
uint8_t clicked, pressed;
uint8_t msx_ver = 0;
uint16_t global_score = 0;

uint16_t yd = 0;
uint16_t game_start = 0;

const unsigned char status_level1[] =
{ 2, 3, 0, 2, 3, 0, 2, 3, 0, 4 };

#define WAIT_VSYNC \
	await_vsync(); await_vsync(); await_vsync(); \
	
// This is done in order to avoid more overhead
#define ownlocate(x,y) \
  CSRX = x; \
  CSRY = y; \

uint8_t frame_picture = 0;
uint8_t msx_graphics = 0;

extern const unsigned char text_minigame_1[204];
extern const unsigned char results_minigame_1[5][37];
extern const unsigned char gameover_mj1[217];
extern const unsigned char gameover_mj1_alternate[217];
extern const unsigned char gameover_mj1_alternate2[217];

const unsigned char BAKURA_OG_SR5_msx_palette[32] = {
0x00, 0x00, 0x77, 0x07, 0x55, 0x05, 0x45, 0x04, 0x25, 0x03, 0x75, 0x06, 0x64, 0x05, 0x46, 0x05, 0x33, 0x03, 0x21, 0x02, 0x22, 0x02, 0x24, 0x02, 0x52, 0x04, 0x31, 0x03, 0x62, 0x05, 0x02, 0x00 };

const char fuck_frame_msx_palette[32] = {
	0x00, 0x00, 
	0x01, 0x00, 
	0x02, 0x01, 
	0x11, 0x01, 
	0x12, 0x02, 
	0x33, 0x02, 
	0x13, 0x03, 
	0x23, 0x03, 
	0x24, 0x04, 
	0x34, 0x04, 
	0x45, 0x04, 
	0x65, 0x05, 
	0x45, 0x06, 
	0x77, 0x07, 
	0x00, 0x00, 
	0x00, 0x00
};

const char title_pal[32] = {
0x00, 0x00, 0x11, 0x01, 0x12, 0x02, 0x14, 0x02, 
0x11, 0x03, 0x16, 0x03, 0x12, 0x04, 0x31, 0x04, 
0x16, 0x04, 0x72, 0x03, 0x25, 0x05, 0x11, 0x06, 
0x34, 0x06, 0x36, 0x06, 0x77, 0x07, 0x00, 0x00 
};


uint8_t old_pressed;

/*

static const char* MSX_PLATFORM_NAME[] = {
  [0] = "MSX",
  [1] = "MSX2",
  [2] = "MSX2+",
  [3] = "MSXtR",
};

*/
volatile unsigned char *bank_switch = (volatile unsigned char *) 0xC000;
extern void showpicture_screenmode5();

void sprite_text(unsigned char id);
static inline void picture_msx1();
static inline void picture_msx2();
void add_score(unsigned short sc);
void decrease_score();
void place_score();
void ingame_fucking();
void switch_gamemode(unsigned char mode);

static inline void picture_msx2()
{
	*bank_switch = 6 + (frame_picture << 1);
	showpicture_screenmode5();
}

extern const unsigned char ohmy_pcm[];
extern const unsigned char whatdo_pcm[];
extern const unsigned char soinno_pcm[];
extern const unsigned char games_pcm[];
extern const unsigned char victim_pcm[];

extern const unsigned char rikuto_pcm[];
extern const unsigned char chance_pcm[];
extern const unsigned char worth_pcm[];

extern const unsigned char scared_pcm[];
extern const unsigned char challenge_pcm[];
extern const unsigned char them_pcm[];
extern const unsigned char loss_pcm[];

#define ARRAY_SIZEOF(a)    (sizeof(a) / sizeof(a[0]))

// sound data stream for PSG channel A
const uint8_t se_channel_1[] = {
  0xa0, 0x02, 0xd0, 0x10, 0x00, 0x00, 0xd6, // T180 L32 S0 M4096 O5C
  0x60, 0x03, 0x90, 0x00, 0xca,             // T180 L32 S0       O5C+
  0x60, 0x05, 0x90, 0x00, 0xbe,             // T180 L16 S0       O5D
  0xff,                                     // end of stream
};

// sound data stream for PSG channel B
const uint8_t se_channel_2[] = {
  0xa0, 0x02, 0xd0, 0x10, 0x00, 0x01, 0xac, // T180 L32 S0 M4096 O4C
  0x60, 0x03, 0x90, 0x01, 0x94,             // T180 L32 S0       O4C+
  0x60, 0x05, 0x90, 0x01, 0x7d,             // T180 L16 S0       O4D
  0xff,                                     // end of stream
};

// the sound fragment structure that holds pointers to the sound data streams.
const struct sound_fragment se_A_part = {
  .streams = {
    [0] = se_channel_1,
    [1] = se_channel_2,
    [2] = 0,                    // not used
  },
};

// the list of "pointers" to the sound fragments.
const struct sound_fragment * se_fragments[] = {
  &se_A_part,
};

// the sound clip structure.
const struct sound_clip se = {
  .priority = 0,
  .num_fragments = ARRAY_SIZEOF(se_fragments),
  .fragments = se_fragments,
};


void pause()
{
	for(;;)
	{
		pressed = joypad_get_state(0) | joypad_get_state(1);
		clicked = (old_pressed ^ pressed) & pressed;
		old_pressed = pressed;
		
		if (clicked & VK_FIRE_0)
		{
			return;
		}
		await_vsync();
	}
}

void main(void)
{
	uint16_t x = 0;
	uint8_t exit = 0;
	
	sound_init();

	// Register the sound driver as VSYNC handler.
	set_vsync_handler(sound_player);

	sound_set_repeat(false);       // Turn on the auto-repeat of BGM.
	sound_start();                // Start the BGM.
	
	screen2();
	vdp_set_visible(true);

	yd = 0;
	game_start = 0;
	switch_gamemode(0);

	
	while(exit == 0)
	{
		pressed = joypad_get_state(0) | joypad_get_state(1);
		clicked = (old_pressed ^ pressed) & pressed;
		old_pressed = pressed;

		switch(game_mode)
		{
			case 0:
				titlescreen_func();
			break;
			case 1:
				ingame_fucking();
			break;
			case 2:
				results();
			break;
			case 3:
				story();
			break;
			case 4:
				details_beforemj1();
			break;
		}

		await_vsync();
	}

	return;
}

void titlescreen_func()
{
	if ((clicked & VK_FIRE_0) && game_start == 0)
	{
		cls();
		switch_gamemode(3);
	}
}

void sprite_text(unsigned char id)
{
	ownlocate(12, 22);
	switch(id)
	{
		/* Keep it up !!*/
		case 0:
			print("KEEP IT UP!");
		break;
		/* You win !!! */
		case 1:
			print("YOU WIN !  ");
		break;
		/* STOP !*/ 
		case 2:
			print("STOP       ");
		break;
		/* FUCK ! */
		case 3:
			print("FUCK HIM ! ");
		break;
		/* FINISH ! */
		case 4:
			print("FINISH     ");
		break;
	}
}


void add_score(unsigned short sc)
{
	global_score += sc << 3;
	ownlocate(0, 23);
	printu(global_score);
}

void decrease_score()
{
	if (global_score > 2) global_score-=3;
	ownlocate(0, 23);
	printu(global_score);
}

void ingame_fucking()
{
	picture_msx2();

	time+=4;

	if (time > 128)
	{
		time = 0;
		status++;
		if (status >= sizeof(status_level1)) game_mode = 2;
		sprite_text(status_level1[status]);
	}
		
	if (status_level1[status] != 4)
	{
		if (frame_picture == 4)
		{
			if ((clicked & VK_FIRE_0) && frame_picture == 4)
			{
				sound_effect(&se);
				frame_picture = 5;
			}
		}
		else if (frame_picture != 4)
		{
			if (frame_picture == 0 && !(clicked & VK_FIRE_0))
			{
					frame_picture++;
					if (frame_picture > 7) frame_picture = 0;
			}
			else if (frame_picture != 0)
			{
				frame_picture++;
				if (frame_picture > 7) frame_picture = 0;
			}
			
			
			switch(status_level1[status])
			{
				case 0:
				case 3:
					if (frame_picture != 0)
					{
						add_score(2);
					}
				break;
				case 2:
					decrease_score();
				break;
			}
			
		}	
	}
	// If Status level is 4, aka if game reached END
	else
	{
		if (time > 123)
		{
			switch_gamemode(2);
		}
	} 

}

extern const unsigned char text_introduction_firstline[5][110];

void switch_gamemode(unsigned char mode)
{
	game_mode = mode;
	status = 0;
	time = 0;
	flag = 0;
	switch(mode)
	{
		/* Titlescreen */
		case 0:
			vdp_set_visible(false);
			screen5();
			
			vdp_set_screen_lines(VDP_SCREEN_LINES_212);
			vmem_set_write_address(0);
			
			color(14, 1, 1);
			vdp_write_palette(title_pal);
			bmem_set_bank(22);
			VDP_WriteVRAM_128K(titltescreen_msx2_sc5_part1_bin, 0, 0, sizeof(titltescreen_msx2_sc5_part1_bin));
			/* Titlescreen only takes the upper half, so clear the rest manually */
			vdp_cmd_execute_HMMV(0, 104, 256, 212-104, 1);
			/* The VDP command is done asynchronously, wait a bit */
			WAIT_VSYNC;
			ownlocate(7, 22);
			print("PRESS FIRE TO START");
		break;
		
		/* In-game (gameplay) */
		case 1:
			frame_picture = 4;
			screen5();
			vdp_set_screen_lines(VDP_SCREEN_LINES_212);
			PUT_PALETTE_MSX(fuck_frame);
			color(13, 1, 0);
			status = 0;
			time = 0;
			global_score = 0;
			sprite_text(status_level1[status]);
			ownlocate(0, 22);
			print("Score:");
			add_score(0);
		break;

		/* Introduction */
		case 3:
			vdp_set_visible(false);
			color(7, 0, 0);
			vdp_write_palette(BAKURA_OG_SR5_msx_palette);
			*bank_switch = 3;
			showpicture_screenmode5();
			
			vdp_cmd_execute_HMMV(0, 160, 256, 212-160, 0);
			/* The VDP command is done asynchronously, wait a bit */
			WAIT_VSYNC;
			
			ownlocate(0, 20);
			print(text_introduction_firstline[0]);
			vdp_set_visible(true);
			
			bmem_set_bank(23);
			PCM_Play_11K((unsigned short)ohmy_pcm);
			
			WAIT_VSYNC;
			
			bmem_set_bank(24);
			PCM_Play_8K((unsigned short)whatdo_pcm);
			
			bmem_set_bank(25);
			PCM_Play_8K((unsigned short)soinno_pcm);
			
			yd = 0;
		break;

		// No 2
		case 4:

		break;
	}
	
	vdp_set_visible(true);
}

void details_beforemj1()
{
	screen5();
	cls();
	ownlocate(0, 2);
	print(text_minigame_1);
	pause();
	switch_gamemode(1);
}

void results()
{
			screen5();
			ownlocate(0, 2);
			print(results_minigame_1[0]);
			ownlocate(0, 3);
			printu(global_score);
			ownlocate(0, 5);
			if (global_score == 666)
			{
				print(results_minigame_1[2]);
				pause();
				cls();
				ownlocate(0, 2);
				print(gameover_mj1_alternate2);
				pause();
			}
			else if (global_score < 300)
			{
				print(results_minigame_1[1]);
				pause();
				cls();
				ownlocate(0, 2);
				print(gameover_mj1);
				pause();
			}
			else if (global_score < 1100)
			{
				print(results_minigame_1[2]);
				pause();
				cls();
				ownlocate(0, 2);
				print(gameover_mj1_alternate);
				pause();
			}
			else
			{
				print(results_minigame_1[3]);
				pause();

				vdp_set_visible(false);
				color(7, 0, 0);
				vdp_write_palette(BAKURA_OG_SR5_msx_palette);
				*bank_switch = 3;
				showpicture_screenmode5();
				vdp_cmd_execute_HMMV(0, 160, 256, 212-160, 0);
				
				WAIT_VSYNC;
				
				vdp_set_visible(true);
				ownlocate(0, 20);
				print("Congrats ! Haha!\nYou're a Alpha male !\nI should spare you for now.\nHope you 'enjoyed' my game.\nSee ya.");
				pause();
			}
	switch_gamemode(0);
}


void story()
{
	if ((clicked & VK_FIRE_0))
	{
		if (yd > 3)
		{
			yd = 0;
			while(yd < 202)
			{
				vdp_cmd_execute_HMMV(0, yd, 256, 2, 0);
				yd += 2;
				await_vsync();
				if (yd > 200)
				{
					break;
				}
			}
			switch_gamemode(4);

		}
		else
		{
			vdp_cmd_execute_HMMV(0, 160, 256, 212-160, 0);
			/* The VDP command is done asynchronously, wait a bit */
			WAIT_VSYNC;
			
			yd++;
			ownlocate(0, 20);
			print(text_introduction_firstline[yd]);
			await_vsync();
			switch(yd)
			{
				case 1:
			
					bmem_set_bank(26);
					PCM_Play_8K((unsigned short)victim_pcm);
					
					bmem_set_bank(27);
					PCM_Play_8K((unsigned short)games_pcm);
				break;
				case 2:
			
					bmem_set_bank(28);
					PCM_Play_8K((unsigned short)rikuto_pcm);
					
					bmem_set_bank(29);
					PCM_Play_8K((unsigned short)chance_pcm);
					
					bmem_set_bank(30);
					PCM_Play_8K((unsigned short)worth_pcm);
				break;
				case 3:
			
					bmem_set_bank(2);
					PCM_Play_8K((unsigned short)scared_pcm);
				break;
				case 4:
					bmem_set_bank(1);
					PCM_Play_8K((unsigned short)challenge_pcm);
					bmem_set_bank(5);
					PCM_Play_8K((unsigned short)them_pcm);
					/*bmem_set_bank(6);
					PCM_Play_8K((unsigned short)loss_pcm);*/
				break;
				
			}
		}
	}
}
