
#define P_PSG_REGS 0xA0   // PSG register write port
#define P_PSG_DATA 0xA1   // PSG value write port
#define P_PSG_STAT 0xA2   // PSG value read port
#define PSG_PORT_REG		P_PSG_REGS     //< Used to select a specific register by writing its number (0 to 15)
#define PSG_PORT_WRITE		P_PSG_DATA     //< Used to write to any register once it has been selected by the Address Port.	
#define PSG_PORT_READ		P_PSG_STAT     //< Used to read any register once it has been selected by the Address Port.

static inline void PCM_Reset()
{
	__asm
        xor     a
        ld      bc, #0xFFA1
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        inc     a
        out     (c), b
        out     (P_PSG_REGS), a
        ld      b, #0xBF
        out     (c), b
	__endasm;		
}

void PCM_Play_8K(unsigned short addr)
{
	addr; // HL
	
	PCM_Reset(); // don't change HL

__asm
		di
		ld		e, (hl)
		inc		hl
		ld		d, (hl)
		inc		hl
//-------------------------------------
// Plays one sample
// IN   HL - Encoded sample start address
//      DE - Sample length (#pcm samples)
//-------------------------------------
	PCM_8K_Play:
        ld      c, #PSG_PORT_WRITE
        push    hl
        ex      de, hl
        inc     h
        exx
        pop     hl
        ld      bc, #0x0010
        ld      de, #0x0000
        exx
        
	PCM_8K_Loop:
        exx                         // 5     -> 5

		// Calculate channel A volume
        ld      a, b                // 5
        sub     c                   // 5
        jr      nc, PCM_8K_WaitA    // 8/13
        ld      a, (hl)             // 8
        ld      b, a                // 5
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      b, a                // 5
        exx                         // 5     -> 61
	PCM_8K_DoneA:
        
		// Calculate channel B volume
        ld      a, d                // 5
        sub     c                   // 5
        jr      nc, PCM_8K_WaitB    // 8/13
        ld      a, (hl)             // 8
        ld      d, a                // 5
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      d, a                // 5
        exx                         // 5     -> 61
	PCM_8K_DoneB:
        
		// Calculate channel C volume
        ld      a, e                // 5
        sub     c                   // 5
        jr      nc, PCM_8K_WaitC    // 8/13
        ld      a, (hl)             // 8
        ld      e, a                // 5        
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      e, a                // 5     -> 56
	PCM_8K_DoneC:

        // Output channels
        ld      a, #8               // 8
        out     (PSG_PORT_REG), a   // 12
        inc     a                   // 5
        out     (c), b              // 15
        out     (PSG_PORT_REG), a   // 12
        inc     a                   // 5
        out     (c), d              // 15
        out     (PSG_PORT_REG), a   // 12
        out     (c), e              // 15   -> 99
        
        // Wait 147 t-cycles
        call    Wait147             // 147
        
        // Decrement length and return if zero
        dec     l                   // 5
        jr      nz, PCM_8K_Loop     // 8/13   -> 18   Total: 447
        dec     h                   // 5
        jp      nz, PCM_8K_Loop     // 11
		ei
        ret
        
PCM_8K_WaitA:
        ld      b, a                // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_8K_DoneA        // 13   -> 51 including branch
        
PCM_8K_WaitB:
        ld      d, a                // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_8K_DoneB        // 13   -> 51 including branch
								    
PCM_8K_WaitC:
        ld      e, a                // 5
        exx                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_8K_DoneC        // 11   -> 51 including branch


Wait147:
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        inc     hl                  // 7
        dec     hl                  // 7
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        ret                         // 11    -> 147 including branch
	__endasm;
}        


void PCM_Play_11K(unsigned short addr)
{
	addr; // HL

	PCM_Reset(); // don't change HL

	__asm
		di
		ld		e, (hl)
		inc		hl
		ld		d, (hl)
		inc		hl
//-------------------------------------
// Plays one sample
// IN   HL - Encoded sample start address
//      DE - Sample length (#pcm samples)
//-------------------------------------
	PCM_11K_Play:
        ld      c, #PSG_PORT_WRITE
        push    hl
        ex      de, hl
        inc     h
        exx
        pop     hl
        ld      bc, #0x0010
        ld      de, #0x0000
        exx

	PCM_11K_Loop:
        exx                         // 5     -> 5

		// Calculate channel A volume
        ld      a, b                // 5
        sub     c                   // 5
        jr      nc, PCM_11K_WaitA   // 8/13
        ld      a, (hl)             // 8
        ld      b, a                // 5
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      b, a                // 5
        exx                         // 5     -> 61
	PCM_11K_DoneA:

		// Calculate channel B volume
        ld      a, d                // 5
        sub     c                   // 5
        jr      nc, PCM_11K_WaitB   // 8/13
        ld      a, (hl)             // 8
        ld      d, a                // 5
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      d, a                // 5
        exx                         // 5     -> 61
	PCM_11K_DoneB:

		// Calculate channel C volume
        ld      a, e                // 5
        sub     c                   // 5
        jr      nc, PCM_11K_WaitC   // 8/13
        ld      a, (hl)             // 8
        ld      e, a                // 5        
        and     #0x0F               // 8
        inc     hl                  // 7
        exx                         // 5
        ld      e, a                // 5     -> 56
	PCM_11K_DoneC:

        // Output channels
        ld      a, #8               // 8
        out     (PSG_PORT_REG), a   // 12
        inc     a                   // 5
        out     (c), b              // 15
        out     (PSG_PORT_REG), a   // 12
        inc     a                   // 5
        out     (c), d              // 15
        out     (PSG_PORT_REG), a   // 12
        out     (c), e              // 15   -> 99

        // Wait 25 t-cycles
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5

        // Decrement length and return if zero
        dec     l                   // 5
        jr      nz, PCM_11K_Loop    // 8/13   -> 18   Total: 325
        dec     h                   // 5
        jp      nz, PCM_11K_Loop    // 11
		ei
        ret

	PCM_11K_WaitA:
        ld      b, a                // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_11K_DoneA       // 13   -> 51 including branch

	PCM_11K_WaitB:
        ld      d, a                // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_11K_DoneB       // 13   -> 51 including branch

	PCM_11K_WaitC:
        ld      e, a                // 5
        exx                         // 5
        nop                         // 5
        nop                         // 5
        nop                         // 5
        jr      PCM_11K_DoneC       // 11   -> 51 including branch
	__endasm;
}
