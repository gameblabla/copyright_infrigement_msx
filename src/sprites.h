#ifndef SPRITES_FUNCTIONS_H
#define SPRITES_FUNCTIONS_H

#include <msx.h>
#include <screen.h>
#include <stdint.h>
#include <text.h>
#include <resources.h>
#include <stdlib.h>
#include <bmem.h>

#define SPRITE_COLORS   (SPRITES - 512) // sprite color table
#define SPRITES         (0x07600) // sprite attribute table
#define SPRITE_PATTERNS (0x07800) // sprite pattern generator table


#define UPDATE_SPRITE(x) \
	vmem_write(SPRITES + (x << 2), &spr[x], sizeof(spr[x]));

#define SET_SPRITE_VISIBLE(n, x) \
	vmem_set_write_address(SPRITES + (n << 2)); \
	vmem_set(x); 


extern void Sprites_All_Invisible() __naked __sdcccall(1) ;

#endif
