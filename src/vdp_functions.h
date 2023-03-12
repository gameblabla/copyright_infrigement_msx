#ifndef VDP_FUNCTIONS_H
#define VDP_FUNCTIONS_H

#include <msx.h>
#include <screen.h>
#include <stdint.h>
#include <text.h>
#include <resources.h>
#include <stdlib.h>
#include <bmem.h>

#define PATTERNS        (0x00000) // pattern generator table
#define IMAGE           (0x01800) // pattern name table
#define COLORS          (0x02000) // color table

typedef uint16_t cv_vmemp; // 14-Bit Video memory address type
extern void cv_set_write_vram_address(cv_vmemp address);
extern void cv_memtovmemcpy_fast(const void *src, size_t n);
extern void cv_memtovmemcpy_slow(const void *src, size_t n);

extern const cv_vmemp color_address;
extern const cv_vmemp pattern_addres;
extern volatile uint8_t cv_vint_indicator;

inline void vmem_write_unsafe(vmemptr_t loc, const uint8_t * p, size_t len) __naked __sdcccall(1) 
{
	VDP_SET_VMEM_WRITE_POINTER(loc);
	while (len--) {
		VDP_SET_VMEM_VALUE(*p++);
	}
}

extern void VDP_WriteVRAM_128K(const uint8_t* src, uint16_t destLow, uint8_t destHigh, uint16_t count) __sdcccall(0);
extern void VDP_WriteVRAM_16K(const uint8_t* src, uint16_t dest, uint16_t count) __sdcccall(0);


#define PUT_PALETTE_MSX(b) \
	vdp_write_palette(b##_msx_palette);
	
#define PUSH_IMAGE_SCREEN_MSX2(b) \
    bmem_set_bank(b##_BANK); \
    VDP_WriteVRAM_128K(b[0], 0, 0, SIZE_##b##_PART_1); \
    bmem_set_bank(b##_BANK + 1); \
    VDP_WriteVRAM_128K(b[1], 0x0000+SIZE_##b##_PART_1, 0, SIZE_##b##_PART_2);
    
#define PUSH_IMAGE_SCREEN_MSX2_WITH_PAL(b) \
    bmem_set_bank(b##_BANK); \
    VDP_WriteVRAM_128K(b[0], 0, 0, SIZE_##b##_PART_1); \
    bmem_set_bank(b##_BANK + 1); \
    VDP_WriteVRAM_128K(b[1], 0x0000+SIZE_##b##_PART_1, 0, SIZE_##b##_PART_2); \
	vdp_write_palette(b##_msx_palette); \




#endif
