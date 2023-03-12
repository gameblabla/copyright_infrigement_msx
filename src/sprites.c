#include "sprites.h"

#define PATTERNS        (0x00000) // pattern generator table
#define IMAGE           (0x01800) // pattern name table
#define COLORS          (0x02000) // color table
#define SPRITES         (0x07600) // sprite attribute table


void Sprites_All_Invisible() __sdcccall(1) 
{
	uint8_t i;
	for(i=0;i<32;i++)
	{
		vmem_write(SPRITES + (i << 2), 0x00, 1); // write 0x80 to byte 4 (index 1) in the sprite attribute table
	}
}
