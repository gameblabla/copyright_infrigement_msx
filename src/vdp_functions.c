#include "vdp_functions.h"

volatile uint8_t cv_vint_indicator;
const cv_vmemp color_address = 0x2000;
const cv_vmemp pattern_addres = 0x0000;

#define F_VDP_REG			0x80 // VDP register write port (bit 7=1 in second write)
#define F_VDP_VRAM			0x00 // VRAM address register (bit 7=0 in second write)
#define F_VDP_WRIT			0x40 // bit 6: read/write access (1=write)
#define F_VDP_READ			0x00 // bit 6: read/write access (0=read)
#define VDP_DI_ALL
#define VDP_EI_ALL
#define VDP_DI_DEF
#define VDP_EI_DEF
#define VDP_DI
#define VDP_EI
#define VDP_REG(_r)			(F_VDP_REG | (_r))
#define P_VDP_0			0x98			///< Primary MSX port for VDP port #0
#define P_VDP_DATA		P_VDP_0			///< VRAM data port (read/write)
__sfr __at(P_VDP_DATA)	g_VDP_DataPort; ///< VRAM data port (read/write)
// 99h
#define P_VDP_1			0x99			///< Primary MSX port for VDP port #1
#define P_VDP_REG		P_VDP_1			///< Register setup port (write) (bit 7=1 in second write)
__sfr __at(P_VDP_REG)	g_VDP_RegPort;	///< Register setup port (write) (bit 7=1 in second write)
#define P_VDP_ADDR		P_VDP_1			///< VRAM address port (write) (bit 7=0 in second write, bit 6: read/write access (0=read, 1=write))
__sfr __at(P_VDP_ADDR)	g_VDP_AddrPort;	///< VRAM address port (write) (bit 7=0 in second write, bit 6: read/write access (0=read, 1=write))
#define P_VDP_STAT		P_VDP_1			///< Status register port (read)
__sfr __at(P_VDP_STAT)	g_VDP_StatPort;	///< Status register port (read)


void VDP_WriteVRAM_128K(const uint8_t* src, uint16_t destLow, uint8_t destHigh, uint16_t count) __sdcccall(0)
{
	src;      // iy+5 iy+4
	destLow;  // iy+7 iy+6
	destHigh; // iy+8
	count;    // iy+10 iy+9
	__asm
		ld		iy, #2
		add		iy, sp
		// Setup address register 
		ld		a, 4(iy)
		add		a, a
		add		a, a
		ld		c, a
		ld		a, 3(iy)
		rlca
		rlca
		and		a, #0x03
		add		a, c
		VDP_DI //~~~~~~~~~~~~~~~~~~~~~~~~~~
		out		(P_VDP_ADDR), a			// RegPort = (page << 2) + (dest >> 14)
		ld		a, #VDP_REG(14)
		out		(P_VDP_REG), a			// RegPort = VDP_REG(14)
		ld		a, 2(iy)
		out		(P_VDP_ADDR), a			// RegPort = (dest & 0xFF)
		ld		a, 3(iy)
		and		a, #0x3f
		add		a, #F_VDP_WRIT
		VDP_EI_DEF //~~~~~~~~~~~~~~~~~~~~~~~~~~
		out		(P_VDP_ADDR), a			// RegPort = ((dest >> 8) & 0x3F) + F_VDP_WRIT
		// while(count--) DataPort = *src++;
		ld		l, 0(iy)				// source address
		ld		h, 1(iy)
		ld		c, #P_VDP_DATA			// data register
		// Handle count LSB
		ld		a, 5(iy)				// count LSB
		cp		a, #0
		jp		z, wrt_loop_init		// skip LSB
		ld		b, a					// send (count & 0x00FF) bytes
		otir
		// Handle count MSB
	wrt_loop_init:
		ld		a, 6(iy)				// count MSB
	wrt_loop_start:
		cp		a, #0
		jp		z, wrt_loop_end			// finished
		ld		b, #0					// send 256 bytes packages
		otir
		dec		a
		jp		wrt_loop_start

	wrt_loop_end:
		VDP_EI_ALL //~~~~~~~~~~~~~~~~~~~~~~~~~~
	__endasm;
}


