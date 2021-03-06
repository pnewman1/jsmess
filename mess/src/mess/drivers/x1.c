/************************************************************************************************

    Sharp X1 (c) 1983 Sharp Corporation

    driver by Angelo Salese & Barry Rodewald,
    special thanks to Dirk Best for various wd17xx fixes

    TODO:
    - Rewrite keyboard input hook-up and decap/dump the keyboard MCU if possible;
    - Fix the 0xe80/0xe83 kanji ROM readback;
    - x1turbo keyboard inputs are currently broken, use x1turbo40 for now;
    - Hook-up remaining .tap image formats;
    - Implement APSS tape commands;
    - Sort out / redump the BIOS gfx roms;
    - X1Turbo: Implement SIO.
    - X1Twin: Hook-up the PC Engine part (actually needs his own driver?);
    - Implement SASI HDD interface;
    - clean-ups!
    - There are various unclear video things, these are:
        - Implement the remaining scrn regs;
        - Implement the new features of the x1turboz, namely the 4096 color feature amongst other things
        - (anything else?)
    - Driver Configuration switches:
        - OPN for X1
        - EMM, and hook-up for X1 too
        - RAM size for EMM
        - specific x1turboz features?

    per-game/program specific TODO:
    - CZ8FB02 / CZ8FB03: doesn't load at all, they are 2hd floppies apparently;
    - Chack'n Pop: game is too fast, presumably missing wait states;
    - Dragon Buster: it crashed to me once with a obj flag hang;
    - The Goonies (x1 only): goes offsync with the PCG beam positions;
    - Graphtol: sets up x1turboz paletteram, graphic garbage due of it;
    - Gyajiko2: hangs when it's supposed to load the character selection screen, FDC bug?
    - Hydlide 3: can't get the user disk to work properly, could be a bad dump;
    - Lupin the 3rd: don't know neither how to "data load" nor how to "create a character" ... does the game hangs?
    - Might & Magic: uses 0xe80-3 kanji ports, should be a good test case for that;
    - "newtype": trips a z80dma assert, worked around for now;
    - Saziri: doesn't re-initialize the tilemap attribute vram when you start a play, making it to have missing colors if you don't start a play in time;
    - Shilver Ghost: changes the vertical visible area during scrolling, and that doesn't work too well with current mc6845 core.
    - Suikoden: shows a JP message error (DFJustin: "Problem with the disk device !! Please set a floppy disk properly and press the return key. Retrying.")
    - Super Billiards (X1 Pack 14): has a slight PCG timing bug, that happens randomly;
    - Trivia-Q: dunno what to do on the selection screen, missing inputs?
    - Turbo Alpha: has z80dma / fdc bugs, doesn't show the presentation properly and then hangs;
    - Will 2: doesn't load, fdc issue presumably (note: it's a x1turbo game ONLY);
    - X1F Demo ("New X1 Demo"): needs partial updates, but they doesn't cope well with this system;
    - Ys 2: crashes after the disclaimer screen;
    - Ys 3: missing user disk, to hack it (and play with x1turboz features): bp 81ca,pc += 2
    - Ys 3: never uploads a valid 4096 palette, probably related to the fact that we don't have an user disk

    Notes:
    - An interesting feature of the Sharp X-1 is the extended i/o bank. When the ppi port c bit 5
      does a 1->0 transition, any write to the i/o space accesses 2 or 3 banks gradients of the bitmap RAM
      with a single write (generally used for layer clearances and bitmap-style sprites).
      Any i/o read disables this extended bitmap ram.
    - I/O port $700 bit 7 of X1 Turbo is a sound (dip-)switch / jumper setting. I don't know yet what is for,
      but King's Knight needs it to be active otherwise it refuses to boot.
    - ROM format is:
      0x00 ROM id (must be 0x01)
      0x01 - 0x0e ROM header
      0xff16 - 0xff17 start-up vector
      In theory, you can convert your tape / floppy games into ROM format easily, provided that you know what's the pinout of the
      cartridge slot and it doesn't exceed 64k (0x10000) of size.
    - Gruppe: shows a random bitmap graphic then returns "program load error" ... it wants that the floppy has write protection enabled (btanb)
    - Maidum: you need to load BOTH disk without write protection disabled, otherwise it refuses to run. (btanb)
    - Marvelous: needs write protection disabled (btanb)
    - Chack'n Pop: to load this game, do a files command on the "Jodan Dos" prompt then move the cursor up at the "Chack'n Pop" file.
      Substitute bin with load and press enter. Finally, do a run once that it loaded correctly.
    - Faeries Residence: to load this game, put a basic v2.0 in drive 0, then execute a NEWON command. Load game disks into drive 0 and 1 then
      type run"START" (case sensitive)
    - POPLEMON: same as above, but you need to type run"POP"

=================================================================================================

    X1 (CZ-800C) - November, 1982
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (4KB) + chargen (2KB)
     * RAM: Main memory (64KB) + VRAM (4KB) + RAM for PCG (6KB) + GRAM (48KB, Option)
     * Text Mode: 80x25 or 40x25
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, Cassette port (2700 baud)

    X1C (CZ-801C) - October, 1983
     * same but only 48KB GRAM

    X1D (CZ-802C) - October, 1983
     * same as X1C but with a 3" floppy drive (notice: 3" not 3" 1/2!!)

    X1Cs (CZ-803C) - June, 1984
     * two expansion I/O ports

    X1Ck (CZ-804C) - June, 1984
     * same as X1Cs
     * ROM: IPL (4KB) + chargen (2KB) + Kanji 1st level

    X1F Model 10 (CZ-811C) - July, 1985
     * Re-designed
     * ROM: IPL (4KB) + chargen (2KB)

    X1F Model 20 (CZ-812C) - July, 1985
     * Re-designed (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1G Model 10 (CZ-820C) - July, 1986
     * Re-designed again
     * ROM: IPL (4KB) + chargen (2KB)

    X1G Model 30 (CZ-822C) - July, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available

    X1twin (CZ-830C) - December, 1986
     * Re-designed again (same as Model 10)
     * ROM: IPL (4KB) + chargen (2KB) + Kanji
     * Built Tape drive plus a 5" floppy drive was available
     * It contains a PC-Engine

    =============  X1 Turbo series  =============

    X1turbo Model 30 (CZ-852C) - October, 1984
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji (128KB)
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors
     * Sound: PSG 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for 5" disks, two expansion I/O ports

    X1turbo Model 20 (CZ-851C) - October, 1984
     * same as Model 30, but only 1 Floppy drive is included

    X1turbo Model 10 (CZ-850C) - October, 1984
     * same as Model 30, but Floppy drive is optional and GRAM is 48KB (it can
        be expanded to 96KB however)

    X1turbo Model 40 (CZ-862C) - July, 1985
     * same as Model 30, but uses tv screen (you could watch television with this)

    X1turboII (CZ-856C) - November, 1985
     * same as Model 30, but restyled, cheaper and sold with utility software

    X1turboIII (CZ-870C) - November, 1986
     * with two High Density Floppy driver

    X1turboZ (CZ-880C) - December, 1986
     * CPU: z80A @ 4MHz, 80C49 x 2 (one for key scan, the other for TV & Cas Ctrl)
     * ROM: IPL (32KB) + chargen (8KB) + Kanji 1st & 2nd level
     * RAM: Main memory (64KB) + VRAM (6KB) + RAM for PCG (6KB) + GRAM (96KB)
     * Text Mode: 80xCh or 40xCh with Ch = 10, 12, 20, 25 (same for Japanese display)
     * Graphic Mode: 640x200 or 320x200, 8 colors [in compatibility mode],
        640x400, 8 colors (out of 4096); 320x400, 64 colors (out of 4096);
        320x200, 4096 colors [in multimode],
     * Sound: PSG 8 octave + FM 8 octave
     * I/O Ports: Centronic ports, 2 Joystick ports, built-in Cassette interface,
        2 Floppy drive for HD 5" disks, two expansion I/O ports

    X1turboZII (CZ-881C) - December, 1987
     * same as turboZ, but added 64KB expansion RAM

    X1turboZIII (CZ-888C) - December, 1988
     * same as turboZII, but no more built-in cassette drive

    Please refer to http://www2s.biglobe.ne.jp/~ITTO/x1/x1menu.html for
    more info

    BASIC has to be loaded from external media (tape or disk), the
    computer only has an Initial Program Loader (IPL)

=================================================================================================

    x1turbo specs (courtesy of Yasuhiro Ogawa):

    upper board: Z80A-CPU
                 Z80A-DMA
                 Z80A-SIO(O)
                 Z80A-CTC
                 uPD8255AC
                 LH5357(28pin mask ROM. for IPL?)
                 YM2149F
                 16.000MHz(X1)

    lower board: IX0526CE(HN61364) (28pin mask ROM. for ANK font?)
                 MB83256x4 (Kanji ROMs)
                 HD46505SP (VDP)
                 M80C49-277 (MCU)
                 uPD8255AC
                 uPD1990 (RTC) + battery
                 6.000MHz(X2)
                 42.9545MHz(X3)

    FDD I/O board: MB8877A (FDC)
                   MB4107 (VFO)

    RAM banks:
    upper board: MB8265A-15 x8 (main memory)
    lower board: MB8416A-12 x3 (VRAM)
                 MB8416A-15 x3 (PCG RAM)
                 MB81416-10 x12 (GRAM)

************************************************************************************************/

#include "emu.h"
#include "includes/x1.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
//#include "machine/z80sio.h"
#include "machine/z80dart.h"
#include "machine/i8255.h"
#include "machine/z80dma.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "sound/2151intf.h"
#include "sound/wave.h"

#include "machine/wd17xx.h"
#include "imagedev/cassette.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "formats/x1_tap.h"
#include "imagedev/cartslot.h"

#define MAIN_CLOCK XTAL_16MHz
#define VDP_CLOCK  XTAL_42_9545MHz
#define MCU_CLOCK  XTAL_6MHz






//static DEVICE_START(x1_daisy){}

/*************************************
 *
 *  Video Functions
 *
 *************************************/

static VIDEO_START( x1 )
{
//  x1_state *state = machine.driver_data<x1_state>();
//  state->m_avram = auto_alloc_array(machine, UINT8, 0x800);
//  state->m_tvram = auto_alloc_array(machine, UINT8, 0x800);
//  state->m_kvram = auto_alloc_array(machine, UINT8, 0x800);
//  state->m_gfx_bitmap_ram = auto_alloc_array(machine, UINT8, 0xc000*2);
//  state->m_pal_4096 = auto_alloc_array(machine, UINT8, 0x1000*3);
}

static void x1_draw_pixel(running_machine &machine, bitmap_t *bitmap,int y,int x,UINT16	pen,UINT8 width,UINT8 height)
{
	if((x)>machine.primary_screen->visible_area().max_x || (y)>machine.primary_screen->visible_area().max_y)
		return;

	if(width && height)
	{
		*BITMAP_ADDR32(bitmap, y+0, x+0) = machine.pens[pen];
		*BITMAP_ADDR32(bitmap, y+0, x+1) = machine.pens[pen];
		*BITMAP_ADDR32(bitmap, y+1, x+0) = machine.pens[pen];
		*BITMAP_ADDR32(bitmap, y+1, x+1) = machine.pens[pen];
	}
	else if(width)
	{
		*BITMAP_ADDR32(bitmap, y, x+0) = machine.pens[pen];
		*BITMAP_ADDR32(bitmap, y, x+1) = machine.pens[pen];
	}
	else if(height)
	{
		*BITMAP_ADDR32(bitmap, y+0, x) = machine.pens[pen];
		*BITMAP_ADDR32(bitmap, y+1, x) = machine.pens[pen];
	}
	else
		*BITMAP_ADDR32(bitmap, y, x) = machine.pens[pen];
}

#define mc6845_h_char_total 	(state->m_crtc_vreg[0])
#define mc6845_h_display		(state->m_crtc_vreg[1])
#define mc6845_h_sync_pos		(state->m_crtc_vreg[2])
#define mc6845_sync_width		(state->m_crtc_vreg[3])
#define mc6845_v_char_total		(state->m_crtc_vreg[4])
#define mc6845_v_total_adj		(state->m_crtc_vreg[5])
#define mc6845_v_display		(state->m_crtc_vreg[6])
#define mc6845_v_sync_pos		(state->m_crtc_vreg[7])
#define mc6845_mode_ctrl		(state->m_crtc_vreg[8])
#define mc6845_tile_height		(state->m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start	(state->m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end 	(state->m_crtc_vreg[0x0b])
#define mc6845_start_addr		(((state->m_crtc_vreg[0x0c]<<8) & 0x3f00) | (state->m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr  	(((state->m_crtc_vreg[0x0e]<<8) & 0x3f00) | (state->m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr	(((state->m_crtc_vreg[0x10]<<8) & 0x3f00) | (state->m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr  	(((state->m_crtc_vreg[0x12]<<8) & 0x3f00) | (state->m_crtc_vreg[0x13] & 0xff))

/* adjust tile index when we are under double height condition */
static UINT8 check_prev_height(running_machine &machine,int x,int y,int x_size)
{
	x1_state *state = machine.driver_data<x1_state>();
	UINT8 prev_tile = state->m_tvram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 cur_tile = state->m_tvram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 prev_attr = state->m_avram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	UINT8 cur_attr = state->m_avram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if(prev_tile == cur_tile && prev_attr == cur_attr)
		return 8;

	return 0;
}

/* Exoa II - Warroid: if double height isn't enabled on the first tile of the line then double height is disabled on everything else. */
static UINT8 check_line_valid_height(running_machine &machine,int y,int x_size,int height)
{
	x1_state *state = machine.driver_data<x1_state>();
	UINT8 line_attr = state->m_avram[(0+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if((line_attr & 0x40) == 0)
		return 0;

	return height;
}

static void draw_fgtilemap(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	/*
        attribute table:
        x--- ---- double width
        -x-- ---- double height
        --x- ---- PCG select
        ---x ---- color blinking
        ---- x--- reverse color
        ---- -xxx color pen

        x--- ---- select Kanji ROM
        -x-- ---- Kanji side (0=left, 1=right)
        --x- ---- Underline
        ---x ---- Kanji ROM select (0=level 1, 1=level 2) (TODO: implement this)
        ---- xxxx Kanji upper 4 bits
    */

	x1_state *state = machine.driver_data<x1_state>();
	int y,x,res_x,res_y;
	UINT32 tile_offset;
	UINT8 x_size,y_size;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	if(x_size == 0 || y_size == 0)
		return; //don't bother if screen is off

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	for (y=0;y<y_size;y++)
	{
		for (x=0;x<x_size;x++)
		{
			int tile = state->m_tvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff];
			int color = state->m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x1f;
			int width = (state->m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x80)>>7;
			int height = (state->m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x40)>>6;
			int pcg_bank = (state->m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x20)>>5;
			UINT8 *gfx_data = machine.region(pcg_bank ? "pcg" : "cgrom")->base();
			int	knj_enable = 0;
			int knj_side = 0;
			int knj_bank = 0;
			int knj_uline = 0;
			if(state->m_is_turbo)
			{
				knj_enable = (state->m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x80)>>7;
				knj_side = (state->m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x40)>>6;
				knj_uline = (state->m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x20)>>5;
				//knj_lv2 = (state->m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x10)>>4;
				knj_bank = (state->m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x0f)>>0;
				if(knj_enable)
				{
					gfx_data = machine.region("kanji")->base();
					tile = ((tile + (knj_bank << 8)) << 1) + (knj_side & 1);
				}
			}

			{
				int pen[3],pen_mask,pcg_pen,xi,yi,dy;

				pen_mask = color & 7;

				dy = 0;

				height = check_line_valid_height(machine,y,x_size,height);

				if(height && y)
					dy = check_prev_height(machine,x,y,x_size);

				/* guess: assume that Kanji VRAM doesn't double the vertical size */
				if(knj_enable) { height = 0; }

				for(yi=0;yi<mc6845_tile_height;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						if(knj_enable) //kanji select
						{
							tile_offset  = tile * 16;
							tile_offset += (yi+dy*(state->m_scrn_reg.v400_mode+1)) >> (height+state->m_scrn_reg.v400_mode);
							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 4)>>2;

							if(yi == mc6845_tile_height-1 && knj_uline) //underlined attribute
							{
								pen[0] = (pen_mask & 1)>>0;
								pen[1] = (pen_mask & 2)>>1;
								pen[2] = (pen_mask & 4)>>2;
							}

							if((yi >= 16 && state->m_scrn_reg.v400_mode == 0) || (yi >= 32 && state->m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else if(pcg_bank) // PCG
						{
							tile_offset  = tile * 8;
							tile_offset += (yi+dy*(state->m_scrn_reg.v400_mode+1)) >> (height+state->m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x1000]>>(7-xi) & (pen_mask & 4)>>2;

							if((yi >= 8 && state->m_scrn_reg.v400_mode == 0) || (yi >= 16 && state->m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else
						{
							tile_offset  = tile * (8*(state->m_scrn_reg.ank_sel+1));
							tile_offset += (yi+dy*(state->m_scrn_reg.v400_mode+1)) >> (height+state->m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+state->m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+state->m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+state->m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 4)>>2;

							if(state->m_scrn_reg.ank_sel)
							{
								if((yi >= 16 && state->m_scrn_reg.v400_mode == 0) || (yi >= 32 && state->m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
							else
							{
								if((yi >=  8 && state->m_scrn_reg.v400_mode == 0) || (yi >= 16 && state->m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
						}

						pcg_pen = pen[2]<<2|pen[1]<<1|pen[0]<<0;

						if(color & 0x10 &&	machine.primary_screen->frame_number() & 0x10) //reverse flickering
							pcg_pen^=7;

						if(pcg_pen == 0 && (!(color & 8)))
							continue;

						if(color & 8) //revert the used color pen
							pcg_pen^=7;

						if((state->m_scrn_reg.blackclip & 8) && (color == (state->m_scrn_reg.blackclip & 7)))
							pcg_pen = 0; // clip the pen to black

						res_x = x*8+xi*(width+1);
						res_y = y*(mc6845_tile_height)+yi;

						if(res_y < cliprect->min_y || res_y > cliprect->max_y) // partial update, TODO: optimize
							continue;

						x1_draw_pixel(machine,bitmap,res_y,res_x,pcg_pen,width,0);
					}
				}
			}

			if(width) //skip next char if we are under double width condition
				x++;
		}
	}
}

/*
 * Priority Mixer Calculation (pri)
 *
 * If pri is 0xff then the bitmap entirely covers the tilemap, if it's 0x00 then
 * the tilemap priority is entirely above the bitmap. Any other value mixes the
 * bitmap and the tilemap priorities based on the pen value, bit 0 = entry 0 <-> bit 7 = entry 7
 * of the bitmap.
 *
 */
static int priority_mixer_pri(running_machine &machine,int color)
{
	int pri_i,pri_mask_calc;

	pri_i = 0;
	pri_mask_calc = 1;

	while(pri_i < 7)
	{
		if((color & 7) == pri_i)
			break;

		pri_i++;
		pri_mask_calc<<=1;
	}

	return pri_mask_calc;
}

static void draw_gfxbitmap(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect, int plane,int pri)
{
	x1_state *state = machine.driver_data<x1_state>();
	int xi,yi,x,y;
	int pen_r,pen_g,pen_b,color;
	int pri_mask_val;
	UINT8 x_size,y_size;
	int gfx_offset;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	if(x_size == 0 || y_size == 0)
		return; //don't bother if screen is off

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	//popmessage("%04x %02x",mc6845_start_addr,mc6845_tile_height);

	for (y=0;y<y_size;y++)
	{
		for(x=0;x<x_size;x++)
		{
			for(yi=0;yi<(mc6845_tile_height);yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					gfx_offset = ((x+(y*x_size)) + mc6845_start_addr) & 0x7ff;
					gfx_offset+= ((yi >> state->m_scrn_reg.v400_mode) * 0x800) & 0x3fff;
					pen_b = (state->m_gfx_bitmap_ram[gfx_offset+0x0000+plane*0xc000]>>(7-xi)) & 1;
					pen_r = (state->m_gfx_bitmap_ram[gfx_offset+0x4000+plane*0xc000]>>(7-xi)) & 1;
					pen_g = (state->m_gfx_bitmap_ram[gfx_offset+0x8000+plane*0xc000]>>(7-xi)) & 1;

					color =  (pen_g<<2 | pen_r<<1 | pen_b<<0) | 8;

					pri_mask_val = priority_mixer_pri(machine,color);
					if(pri_mask_val & pri) continue;

					if((color == 8 && state->m_scrn_reg.blackclip & 0x10) || (color == 9 && state->m_scrn_reg.blackclip & 0x20)) // bitmap color clip to black conditions
						color = 0;

					if(y*(mc6845_tile_height)+yi < cliprect->min_y || y*(mc6845_tile_height)+yi > cliprect->max_y) // partial update TODO: optimize
						continue;

					x1_draw_pixel(machine,bitmap,y*(mc6845_tile_height)+yi,x*8+xi,color,0,0);
				}
			}
		}
	}
}

static SCREEN_UPDATE( x1 )
{
	x1_state *state = screen->machine().driver_data<x1_state>();

	bitmap_fill(bitmap, cliprect, MAKE_ARGB(0xff,0x00,0x00,0x00));

	draw_gfxbitmap(screen->machine(),bitmap,cliprect,state->m_scrn_reg.disp_bank,state->m_scrn_reg.pri);
	draw_fgtilemap(screen->machine(),bitmap,cliprect);
	draw_gfxbitmap(screen->machine(),bitmap,cliprect,state->m_scrn_reg.disp_bank,state->m_scrn_reg.pri^0xff);

	return 0;
}

static SCREEN_EOF( x1 )
{
//  x1_state *state = screen->machine().driver_data<x1_state>();

//  state->m_old_vpos = -1;
}

/*************************************
 *
 *  Keyboard MCU simulation
 *
 *************************************/


static UINT16 check_keyboard_press(running_machine &machine)
{
	static const char *const portnames[3] = { "key1","key2","key3" };
	int i,port_i,scancode;
	UINT8 keymod = input_port_read(machine,"key_modifiers") & 0x1f;
	UINT32 pad = input_port_read(machine,"tenkey");
	UINT32 f_key = input_port_read(machine, "f_keys");
	scancode = 0;

	for(port_i=0;port_i<3;port_i++)
	{
		for(i=0;i<32;i++)
		{
			if((input_port_read(machine,portnames[port_i])>>i) & 1)
			{
				//key_flag = 1;
				if(keymod & 0x02)  // shift not pressed
				{
					if(scancode >= 0x41 && scancode < 0x5a)
						scancode += 0x20;  // lowercase
				}
				else
				{
					if(scancode >= 0x31 && scancode < 0x3a)
						scancode -= 0x10;
					if(scancode == 0x30)
					{
						scancode = 0x3d;
					}
				}
				if((keymod & 0x10) == 0) // graph on
					scancode |= 0x80;

				return scancode;
			}
			scancode++;
		}
	}

	// check numpad
	scancode = 0x30;
	for(i=0;i<10;i++)
	{
		if((pad >> i) & 0x01)
		{
			return scancode | 0x100;
		}
		scancode++;
	}

	// check function keys
	scancode = 0x71;
	for(i=0;i<5;i++)
	{
		if((f_key >> i) & 0x01)
		{
			return (scancode + ((keymod & 0x02) ? 0 : 5)) | 0x100;
		}
		scancode++;
	}

	return 0;
}

static UINT8 check_keyboard_shift(running_machine &machine)
{
	UINT8 val = 0xe0;
	/*
    all of those are active low
    x--- ---- TEN: Numpad, Function key, special input key
    -x-- ---- KIN: Valid key
    --x- ---- REP: Key repeat
    ---x ---- GRAPH key ON
    ---- x--- CAPS lock ON
    ---- -x-- KANA lock ON
    ---- --x- SHIFT ON
    ---- ---x CTRL ON
    */

	val |= input_port_read(machine,"key_modifiers") & 0x1f;

	if(check_keyboard_press(machine) != 0)
		val &= ~0x40;

	if(check_keyboard_press(machine) & 0x100) //function keys
		val &= ~0x80;

	return val;
}

static UINT8 get_game_key(running_machine &machine, int port)
{
	// key status returned by sub CPU function 0xE3.
	// in order from bit 7 to 0:
	// port 0: Q,W,E,A,D,Z,X,C
	// port 1: numpad 7,4,1,8,2,9,6,3
	// port 2: ESC,1,[-],[+],[*],TAB,SPC,RET ([] = numpad)
	// bits are active high
	UINT8 ret = 0;
	UINT32 key1 = input_port_read(machine,"key1");
	UINT32 key2 = input_port_read(machine,"key2");
	UINT32 key3 = input_port_read(machine,"key3");
	UINT32 pad = input_port_read(machine,"tenkey");

	switch(port)
	{
		case 0:
			if(key3 & 0x00020000) ret |= 0x80;  // Q
			if(key3 & 0x00800000) ret |= 0x40;  // W
			if(key3 & 0x00000020) ret |= 0x20;  // E
			if(key3 & 0x00000002) ret |= 0x10;  // A
			if(key3 & 0x00000010) ret |= 0x08;  // D
			if(key3 & 0x04000000) ret |= 0x04;  // Z
			if(key3 & 0x01000000) ret |= 0x02;  // X
			if(key3 & 0x00000008) ret |= 0x01;  // C
			break;
		case 1:
			if(pad & 0x00000080) ret |= 0x80;  // Tenkey 7
			if(pad & 0x00000010) ret |= 0x40;  // Tenkey 4
			if(pad & 0x00000002) ret |= 0x20;  // Tenkey 1
			if(pad & 0x00000100) ret |= 0x10;  // Tenkey 8
			if(pad & 0x00000004) ret |= 0x08;  // Tenkey 2
			if(pad & 0x00000200) ret |= 0x04;  // Tenkey 9
			if(pad & 0x00000040) ret |= 0x02;  // Tenkey 6
			if(pad & 0x00000008) ret |= 0x01;  // Tenkey 3
			break;
		case 2:
			if(key1 & 0x08000000) ret |= 0x80;  // ESC
			if(key2 & 0x00020000) ret |= 0x40;  // 1
			if(pad & 0x00000400) ret |= 0x20;  // Tenkey -
			if(pad & 0x00000800) ret |= 0x10;  // Tenkey +
			if(pad & 0x00001000) ret |= 0x08;  // Tenkey *
			if(key1 & 0x00000200) ret |= 0x04;  // TAB
			if(key2 & 0x00000001) ret |= 0x02;  // SPC
			if(key1 & 0x00002000) ret |= 0x01;  // RET
			break;
	}

	return ret;
}

static READ8_HANDLER( sub_io_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 ret,bus_res;

	/* Looks like that the HW retains the latest data putted on the bus here, behaviour confirmed by Rally-X */
	if(state->m_sub_obf)
	{
		bus_res = state->m_sub_val[state->m_key_i];
		/* FIXME: likely to be different here. */
		state->m_key_i++;
		if(state->m_key_i >= 2) { state->m_key_i = 0; }

		return bus_res;
	}

#if 0
	if(key_flag == 1)
	{
		key_flag = 0;
		return 0x82; //TODO: this is for shift/ctrl/kana lock etc.
	}
#endif

	state->m_sub_cmd_length--;
	state->m_sub_obf = (state->m_sub_cmd_length) ? 0x00 : 0x20;

	ret = state->m_sub_val[state->m_sub_val_ptr];

	state->m_sub_val_ptr++;
	if(state->m_sub_cmd_length <= 0)
		state->m_sub_val_ptr = 0;

	return ret;
}

static void cmt_command( running_machine &machine, UINT8 cmd )
{
	x1_state *state = machine.driver_data<x1_state>();
	// CMT deck control command (E9 xx)
	// E9 00 - Eject
	// E9 01 - Stop
	// E9 02 - Play
	// E9 03 - Fast Forward
	// E9 04 - Rewind
	// E9 05 - APSS Fast Forward
	// E9 06 - APSS Rewind
	// E9 0A - Record
	/*
    APSS is a Sharp invention and stands for Automatic Program Search System, it scans the tape for silent parts that are bigger than 4 seconds.
    It's basically used for audio tapes in order to jump over the next/previous "track".
    */
	state->m_cmt_current_cmd = cmd;

	if(machine.device<cassette_image_device>(CASSETTE_TAG)->get_image() == NULL) //avoid a crash if a disk game tries to access this
		return;

	switch(cmd)
	{
		case 0x01:  // Stop
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			state->m_cmt_test = 1;
			popmessage("CMT: Stop");
			break;
		case 0x02:  // Play
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Play");
			break;
		case 0x03:  // Fast Forward
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Fast Forward");
			break;
		case 0x04:  // Rewind
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Rewind");
			break;
		case 0x05:  // APSS Fast Forward
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Fast Forward");
			break;
		case 0x06:  // APSS Rewind
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
			popmessage("CMT: APSS Rewind");
			break;
		case 0x0a:  // Record
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
			machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_RECORD,CASSETTE_MASK_UISTATE);
			popmessage("CMT: Record");
			break;
		default:
			logerror("Unimplemented or invalid CMT command (0x%02x)\n",cmd);
	}
	logerror("CMT: Command 0xe9-0x%02x received.\n",cmd);
}

static TIMER_DEVICE_CALLBACK( cmt_wind_timer )
{
	x1_state *state = timer.machine().driver_data<x1_state>();
	cassette_image_device* cmt = timer.machine().device<cassette_image_device>(CASSETTE_TAG);

	if(cmt->get_image() == NULL) //avoid a crash if a disk game tries to access this
		return;

	switch(state->m_cmt_current_cmd)
	{
		case 0x03:
		case 0x05:  // Fast Forwarding tape
			cmt->seek(1,SEEK_CUR);
			if(cmt->get_position() >= cmt->get_length())  // at end?
				cmt_command(timer.machine(),0x01);  // Stop tape
			break;
		case 0x04:
		case 0x06:  // Rewinding tape
			cmt->seek(-1,SEEK_CUR);
			if(cmt->get_position() <= 0) // at beginning?
				cmt_command(timer.machine(),0x01);  // Stop tape
			break;
	}
}

static WRITE8_HANDLER( sub_io_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	/* sub-routine at $10e sends to these sub-routines when a keyboard input is triggered:
     $17a -> floppy
     $094 -> ROM
     $0c0 -> timer
     $052 -> cmt
     $0f5 -> reload sub-routine? */

	if(state->m_sub_cmd == 0xe4)
	{
		state->m_key_irq_vector = data;
		logerror("Key vector set to 0x%02x\n",data);
		data = 0;
	}

	if(state->m_sub_cmd == 0xe9)
	{
		cmt_command(space->machine(),data);
		data = 0;
	}

	if((data & 0xf0) == 0xd0) //reads here tv recording timer data. (Timer set (0xd0) / Timer readout (0xd8))
	{
		/*
            xx-- ---- mode
            --xx xxxx interval
        */
		state->m_sub_val[0] = 0;
		/*
            xxxx xxxx command code:
            00 timer disabled
            01 TV command
            10 interrupt
            11 Cassette deck
        */
		state->m_sub_val[1] = 0;
		/*
            ---x xxxx minute
        */
		state->m_sub_val[2] = 0;
		/*
            ---- xxxx hour
        */
		state->m_sub_val[3] = 0;
		/*
            xxxx ---- month
            ---- -xxx day of the week
        */
		state->m_sub_val[4] = 0;
		/*
            --xx xxxx day
        */
		state->m_sub_val[5] = 0;
		state->m_sub_cmd_length = 6;
	}

	switch(data)
	{
		case 0xe3: //game key obtaining
			state->m_sub_cmd_length = 3;
			state->m_sub_val[0] = get_game_key(space->machine(),0);
			state->m_sub_val[1] = get_game_key(space->machine(),1);
			state->m_sub_val[2] = get_game_key(space->machine(),2);
			break;
		case 0xe4: //irq vector setting
			break;
		//case 0xe5: //timer irq clear
		//  break;
		case 0xe6: //key data readout
			state->m_sub_val[0] = check_keyboard_shift(space->machine()) & 0xff;
			state->m_sub_val[1] = check_keyboard_press(space->machine()) & 0xff;
			state->m_sub_cmd_length = 2;
			break;
//      case 0xe7: // TV Control
//          break;
		case 0xe8: // TV Control read-out
			state->m_sub_val[0] = state->m_sub_cmd;
			state->m_sub_cmd_length = 1;
			break;
		case 0xe9: // CMT Control
			break;
		case 0xea:  // CMT Control status
			state->m_sub_val[0] = state->m_cmt_current_cmd;
			state->m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEA received, returning 0x%02x.\n",state->m_sub_val[0]);
			break;
		case 0xeb:  // CMT Tape status
		            // bit 0 = tape end (0=end of tape)
					// bit 1 = tape inserted
					// bit 2 = record status (1=OK, 0=write protect)
			state->m_sub_val[0] = 0x05;
			if(space->machine().device<cassette_image_device>(CASSETTE_TAG)->get_image() != NULL)
				state->m_sub_val[0] |= 0x02;
			state->m_sub_cmd_length = 1;
			logerror("CMT: Command 0xEB received, returning 0x%02x.\n",state->m_sub_val[0]);
			break;
//      case 0xec: //set date
//          break;
		case 0xed: //get date
			state->m_sub_val[0] = state->m_rtc.day;
			state->m_sub_val[1] = (state->m_rtc.month<<4) | (state->m_rtc.wday & 0xf);
			state->m_sub_val[2] = state->m_rtc.year;
			state->m_sub_cmd_length = 3;
			break;
//      case 0xee: //set time
//          break;
		case 0xef: //get time
			state->m_sub_val[0] = state->m_rtc.hour;
			state->m_sub_val[1] = state->m_rtc.min;
			state->m_sub_val[2] = state->m_rtc.sec;
			state->m_sub_cmd_length = 3;
			break;
	}

	state->m_sub_cmd = data;

	state->m_sub_obf = (state->m_sub_cmd_length) ? 0x00 : 0x20;

	if(data != 0xe6)
		logerror("SUB: Command byte 0x%02x\n",data);
}

/*************************************
 *
 *  ROM Image / Banking Handling
 *
 *************************************/


static READ8_HANDLER( x1_rom_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *rom = space->machine().region("cart_img")->base();

//  printf("%06x\n",state->m_rom_index[0]<<16|state->m_rom_index[1]<<8|state->m_rom_index[2]<<0);

	return rom[state->m_rom_index[0]<<16|state->m_rom_index[1]<<8|state->m_rom_index[2]<<0];
}

static WRITE8_HANDLER( x1_rom_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_rom_index[offset] = data;
}

static WRITE8_HANDLER( rom_bank_0_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_ram_bank = 0x10;
}

static WRITE8_HANDLER( rom_bank_1_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_ram_bank = 0x00;
}

/*************************************
 *
 *  MB8877A FDC (wd17XX compatible)
 *
 *************************************/

//static UINT8 fdc_irq_flag;
//static UINT8 fdc_drq_flag;
//static UINT8 fdc_side;
//static UINT8 fdc_drive;

static READ8_HANDLER( x1_fdc_r )
{
	device_t* dev = space->machine().device("fdc");
	//UINT8 ret = 0;

	switch(offset+0xff8)
	{
		case 0x0ff8:
			return wd17xx_status_r(dev,offset);
		case 0x0ff9:
			return wd17xx_track_r(dev,offset);
		case 0x0ffa:
			return wd17xx_sector_r(dev,offset);
		case 0x0ffb:
			return wd17xx_data_r(dev,offset);
		case 0x0ffc:
			printf("FDC: read FM type\n");
			return 0xff;
		case 0x0ffd:
			printf("FDC: read MFM type\n");
			return 0xff;
		case 0x0ffe:
			printf("FDC: read 1.6M type\n");
			return 0xff;
		case 0x0fff:
			printf("FDC: switching between 500k/1M\n");
			return 0xff;
	}

	return 0x00;
}

static WRITE8_HANDLER( x1_fdc_w )
{
	device_t* dev = space->machine().device("fdc");

	switch(offset+0xff8)
	{
		case 0x0ff8:
			wd17xx_command_w(dev,offset,data);
			break;
		case 0x0ff9:
			wd17xx_track_w(dev,offset,data);
			break;
		case 0x0ffa:
			wd17xx_sector_w(dev,offset,data);
			break;
		case 0x0ffb:
			wd17xx_data_w(dev,offset,data);
			break;
		case 0x0ffc:
			wd17xx_set_drive(dev,data & 3);
			floppy_mon_w(floppy_get_device(space->machine(), data & 3), !BIT(data, 7));
			floppy_drive_set_ready_state(floppy_get_device(space->machine(), data & 3), data & 0x80,0);
			wd17xx_set_side(dev,(data & 0x10)>>4);
			break;
		case 0x0ffd:
		case 0x0ffe:
		case 0x0fff:
			logerror("FDC: undefined write to %04x = %02x\n",offset+0xff8,data);
			break;
	}
}

static const wd17xx_interface x1_mb8877a_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static WRITE_LINE_DEVICE_HANDLER( fdc_drq_w )
{
	z80dma_rdy_w(device->machine().device("dma"), state ^ 1);
}

static const wd17xx_interface x1turbo_mb8877a_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

/*************************************
 *
 *  Programmable Character Generator
 *
 *************************************/

static UINT16 check_pcg_addr(running_machine &machine)
{
	x1_state *state = machine.driver_data<x1_state>();
	if(state->m_avram[0x7ff] & 0x20) return 0x7ff;
	if(state->m_avram[0x3ff] & 0x20) return 0x3ff;
	if(state->m_avram[0x5ff] & 0x20) return 0x5ff;
	if(state->m_avram[0x1ff] & 0x20) return 0x1ff;

	return 0x3ff;
}

static UINT16 check_chr_addr(running_machine &machine)
{
	x1_state *state = machine.driver_data<x1_state>();
	if(!(state->m_avram[0x7ff] & 0x20)) return 0x7ff;
	if(!(state->m_avram[0x3ff] & 0x20)) return 0x3ff;
	if(!(state->m_avram[0x5ff] & 0x20)) return 0x5ff;
	if(!(state->m_avram[0x1ff] & 0x20)) return 0x1ff;

	return 0x3ff;
}

static UINT16 get_pcg_addr(running_machine &machine,UINT16 width,UINT8 y_char_size)
{
	x1_state *state = machine.driver_data<x1_state>();
	int hbeam = machine.primary_screen->hpos() >> 3;
	int vbeam = machine.primary_screen->vpos() / y_char_size;
	UINT16 pcg_offset = ((hbeam + vbeam*width) + mc6845_start_addr) & 0x7ff;

	//printf("%08x %d %d %d %d\n",(hbeam+vbeam*width),hbeam,vbeam,machine.primary_screen->vpos() & 7,width);

	return pcg_offset;
}

static READ8_HANDLER( x1_pcg_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	int addr;
	int pcg_offset;
	UINT8 res;
	UINT8 *gfx_data;

	addr = (offset & 0x300) >> 8;

	if(addr == 0 && state->m_scrn_reg.pcg_mode) // Kanji ROM read, X1Turbo only
	{
		gfx_data = space->machine().region("kanji")->base();
		pcg_offset = (state->m_tvram[check_chr_addr(space->machine())]+(state->m_kvram[check_chr_addr(space->machine())]<<8)) & 0xfff;
		pcg_offset*=0x20;
		pcg_offset+=(offset & 0x0f);
		pcg_offset+=(state->m_kvram[check_chr_addr(space->machine())] & 0x40) >> 2; //left-right check

		res = gfx_data[pcg_offset];
	}
	else
	{
		UINT8 y_char_size;

		/* addr == 0 reads from the ANK rom */
		gfx_data = space->machine().region((addr == 0) ? "cgrom" : "pcg")->base();
		y_char_size = (mc6845_tile_height > 8) ? 8 : mc6845_tile_height;
		if(y_char_size == 0) { y_char_size = 1; }
		pcg_offset = state->m_tvram[get_pcg_addr(space->machine(),mc6845_h_display,y_char_size)]*8;
		pcg_offset+= space->machine().primary_screen->vpos() & (y_char_size-1);
		if(addr) { pcg_offset+= ((addr-1)*0x800); }
		res = gfx_data[pcg_offset];
	}

	return res;
}

static WRITE8_HANDLER( x1_pcg_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	int addr,pcg_offset;
	UINT8 *PCG_RAM = space->machine().region("pcg")->base();

	addr = (offset & 0x300) >> 8;

	if(addr == 0)
	{
		/* NOP */
		logerror("Warning: write to the ANK area! %04x %02x\n",offset,data);
	}
	else
	{
		if(state->m_scrn_reg.pcg_mode) // Hi-Speed Mode, X1Turbo only
		{
			pcg_offset = state->m_tvram[check_pcg_addr(space->machine())]*8;
			pcg_offset+= (offset & 0xe) >> 1;
			pcg_offset+=((addr-1)*0x800);
			PCG_RAM[pcg_offset] = data;

			pcg_offset &= 0x7ff;

    		gfx_element_mark_dirty(space->machine().gfx[1], pcg_offset >> 3);
		}
		else // Compatible Mode
		{
			UINT8 y_char_size;

			/* TODO: Brain Breaker doesn't work with this arrangement in high resolution mode, check out why */
			y_char_size = mc6845_tile_height > 8 ? mc6845_tile_height-8 : mc6845_tile_height;
			if(y_char_size == 0) { y_char_size = 1; }
			pcg_offset = state->m_tvram[get_pcg_addr(space->machine(),mc6845_h_display,y_char_size)]*8;
			pcg_offset+= space->machine().primary_screen->vpos() & (y_char_size-1);
			pcg_offset+= ((addr-1)*0x800);

			PCG_RAM[pcg_offset] = data;

			pcg_offset &= 0x7ff;

			gfx_element_mark_dirty(space->machine().gfx[1], pcg_offset >> 3);
		}
	}
}

/*************************************
 *
 *  Other Video-related functions
 *
 *************************************/

/* for bitmap mode */
static void set_current_palette(running_machine &machine)
{
	x1_state *state = machine.driver_data<x1_state>();
	UINT8 addr,r,g,b;

	for(addr=0;addr<8;addr++)
	{
		r = ((state->m_x_r)>>(addr)) & 1;
		g = ((state->m_x_g)>>(addr)) & 1;
		b = ((state->m_x_b)>>(addr)) & 1;

		palette_set_color_rgb(machine, addr|8, pal1bit(r), pal1bit(g), pal1bit(b));
	}
}

static WRITE8_HANDLER( x1turboz_4096_palette_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT32 pal_entry;
	UINT8 r,g,b;

	pal_entry = ((offset & 0xff) << 4) | ((data & 0xf0) >> 4);

	state->m_pal_4096[pal_entry+((offset & 0x300)<<4)] = data & 0xf;

	r = state->m_pal_4096[pal_entry+(1<<12)];
	g = state->m_pal_4096[pal_entry+(2<<12)];
	b = state->m_pal_4096[pal_entry+(0<<12)];

	palette_set_color_rgb(space->machine(), pal_entry+16, pal3bit(r), pal3bit(g), pal3bit(b));
}

/* Note: docs claims that reading the palette ports makes the value to change somehow in X1 mode ...
         In 4096 color mode, it's used for reading the value back. */
static WRITE8_HANDLER( x1_pal_r_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if(state->m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(state->m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else //compatible mode
	{
		state->m_x_r = data;
		set_current_palette(space->machine());
		//if(state->m_old_vpos != space->machine().primary_screen->vpos())
		//{
		//  space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());
		//  state->m_old_vpos = space->machine().primary_screen->vpos();
		//}
	}
}

static WRITE8_HANDLER( x1_pal_g_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if(state->m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(state->m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else
	{
		state->m_x_g = data;
		set_current_palette(space->machine());
		//if(state->m_old_vpos != space->machine().primary_screen->vpos())
		//{
			space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());
		//  state->m_old_vpos = space->machine().primary_screen->vpos();
		//}
	}
}

static WRITE8_HANDLER( x1_pal_b_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if(state->m_turbo_reg.pal & 0x80) //AEN bit, Turbo Z
	{
		if(state->m_turbo_reg.gfx_pal & 0x80) //APEN bit
			x1turboz_4096_palette_w(space,offset & 0x3ff,data);
	}
	else
	{
		state->m_x_b = data;
		set_current_palette(space->machine());
		//if(state->m_old_vpos != space->machine().primary_screen->vpos())
		//{
		//  space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());
		//  state->m_old_vpos = space->machine().primary_screen->vpos();
		//}
	}
}

static WRITE8_HANDLER( x1_ex_gfxram_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 ex_mask;

	if     (offset >= 0x0000 && offset <= 0x3fff)	{ ex_mask = 7; }
	else if(offset >= 0x4000 && offset <= 0x7fff)	{ ex_mask = 6; }
	else if(offset >= 0x8000 && offset <= 0xbfff)	{ ex_mask = 5; }
	else                                        	{ ex_mask = 3; }

	if(ex_mask & 1) { state->m_gfx_bitmap_ram[(offset & 0x3fff)+0x0000+(state->m_scrn_reg.gfx_bank*0xc000)] = data; }
	if(ex_mask & 2) { state->m_gfx_bitmap_ram[(offset & 0x3fff)+0x4000+(state->m_scrn_reg.gfx_bank*0xc000)] = data; }
	if(ex_mask & 4) { state->m_gfx_bitmap_ram[(offset & 0x3fff)+0x8000+(state->m_scrn_reg.gfx_bank*0xc000)] = data; }
}

/*
    SCRN flags

    d0(01) = 0:low resolution (15KHz) 1: high resolution (24KHz)
    d1(02) = 0:1 raster / pixel       1:2 raster / pixel
    d2(04) = 0:8 rasters / CHR        1:16 rasters / CHR
    d3(08) = 0:bank 0                 0:bank 1    <- display
    d4(10) = 0:bank 0                 0:bank 1    <- access
    d5(20) = 0:compatibility          1:high speed  <- define PCG mode
    d6(40) = 0:8-raster graphics      1:16-raster graphics
    d7(80) = 0:don't display          1:display  <- underline (when 1, graphics are not displayed)
*/
static WRITE8_HANDLER( x1_scrn_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_scrn_reg.pcg_mode = (data & 0x20)>>5;
	state->m_scrn_reg.gfx_bank = (data & 0x10)>>4;
	state->m_scrn_reg.disp_bank = (data & 0x08)>>3;
	state->m_scrn_reg.ank_sel = (data & 0x04)>>2;
	state->m_scrn_reg.v400_mode = ((data & 0x03) == 3) ? 1 : 0;

	if(data & 0x80)
		printf("SCRN = %02x\n",data & 0x80);
	if((data & 0x03) == 1)
		printf("SCRN sets true 400 lines mode\n");
}

static WRITE8_HANDLER( x1_pri_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_scrn_reg.pri = data;
//  printf("PRI = %02x\n",data);
}

static WRITE8_HANDLER( x1_6845_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	if(offset == 0)
	{
		state->m_crtc_index = data;
		space->machine().device<mc6845_device>("crtc")->address_w(*space, offset, data);
	}
	else
	{
		state->m_crtc_vreg[state->m_crtc_index] = data;
		space->machine().device<mc6845_device>("crtc")->register_w(*space, offset, data);
	}
}

static READ8_HANDLER( x1_blackclip_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	/*  TODO: this returns only on x1turboz */
	return state->m_scrn_reg.blackclip;
}

static WRITE8_HANDLER( x1_blackclip_w )
{
	/*
    -x-- ---- replace blanking duration with black
    --x- ---- replace bitmap palette 1 with black
    ---x ---- replace bitmap palette 0 with black
    ---- x--- enable text blackclip
    ---- -xxx palette color number for text black
    */
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_scrn_reg.blackclip = data;
	if(data & 0x40)
		printf("Blackclip data access %02x\n",data);
}

static READ8_HANDLER( x1turbo_pal_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	return state->m_turbo_reg.pal;
}

static READ8_HANDLER( x1turbo_txpal_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	return state->m_turbo_reg.txt_pal[offset];
}

static READ8_HANDLER( x1turbo_txdisp_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	return state->m_turbo_reg.txt_disp;
}

static READ8_HANDLER( x1turbo_gfxpal_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	return state->m_turbo_reg.gfx_pal;
}

static WRITE8_HANDLER( x1turbo_pal_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	printf("TURBO PAL %02x\n",data);
	state->m_turbo_reg.pal = data;
}

static WRITE8_HANDLER( x1turbo_txpal_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	int r,g,b;

	printf("TURBO TEXT PAL %02x %02x\n",data,offset);
	state->m_turbo_reg.txt_pal[offset] = data;

	if(state->m_turbo_reg.pal & 0x80)
	{
		r = (data & 0x0c) >> 2;
		g = (data & 0x30) >> 4;
		b = (data & 0x03) >> 0;

		palette_set_color_rgb(space->machine(), offset, pal2bit(r), pal2bit(g), pal2bit(b));
	}
}

static WRITE8_HANDLER( x1turbo_txdisp_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	printf("TURBO TEXT DISPLAY %02x\n",data);
	state->m_turbo_reg.txt_disp = data;
}

static WRITE8_HANDLER( x1turbo_gfxpal_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	printf("TURBO GFX PAL %02x\n",data);
	state->m_turbo_reg.gfx_pal = data;
}


/*
 *  FIXME: bit-wise this doesn't make any sense, I guess that it uses the lv 2 kanji roms
 *         Test cases for this port so far are Hyper Olympics '84 disk version and Might & Magic.
 */
static UINT16 jis_convert(int kanji_addr)
{
	if(kanji_addr >= 0x0e00 && kanji_addr <= 0x0e9f) { kanji_addr -= 0x0e00; kanji_addr &= 0x0ff; return ((0x0e0) + (kanji_addr >> 3)) << 4; } // numbers
	if(kanji_addr >= 0x0f00 && kanji_addr <= 0x109f) { kanji_addr -= 0x0f00; kanji_addr &= 0x1ff; return ((0x4c0) + (kanji_addr >> 3)) << 4; } // lower case chars
	if(kanji_addr >= 0x1100 && kanji_addr <= 0x129f) { kanji_addr -= 0x1100; kanji_addr &= 0x1ff; return ((0x2c0) + (kanji_addr >> 3)) << 4; } // upper case chars
	if(kanji_addr >= 0x0100 && kanji_addr <= 0x01ff) { kanji_addr -= 0x0100; kanji_addr &= 0x0ff; return ((0x040) + (kanji_addr >> 3)) << 4; } // grammar symbols
	if(kanji_addr >= 0x0500 && kanji_addr <= 0x06ff) { kanji_addr -= 0x0500; kanji_addr &= 0x1ff; return ((0x240) + (kanji_addr >> 3)) << 4; } // math symbols
	if(kanji_addr >= 0x0300 && kanji_addr <= 0x04ff) { kanji_addr -= 0x0300; kanji_addr &= 0x1ff; return ((0x440) + (kanji_addr >> 3)) << 4; } // parentesis

	if(kanji_addr != 0x0720 && kanji_addr != 0x0730)
		printf("%08x\n",kanji_addr);

	return 0x0000;
}

static READ8_HANDLER( x1_kanji_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *kanji_rom = space->machine().region("kanji")->base();
	UINT8 res;

	res = kanji_rom[jis_convert(state->m_kanji_addr & 0xfff0)+(offset*0x10)+(state->m_kanji_addr & 0xf)];

	if(offset == 1)
		state->m_kanji_addr_latch++;

	return res;
}

static WRITE8_HANDLER( x1_kanji_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
//  if(offset < 2)

	switch(offset)
	{
		case 0: state->m_kanji_addr_latch = (data & 0xff)|(state->m_kanji_addr_latch&0xff00); break;
		case 1: state->m_kanji_addr_latch = (data<<8)|(state->m_kanji_addr_latch&0x00ff);
			//if(state->m_kanji_addr_latch != 0x720 && state->m_kanji_addr_latch != 0x730)
			//  printf("%08x\n",state->m_kanji_addr_latch);
			break;
		case 2:
		{
			/* 0 -> selects Expanded EEPROM */
			/* 1 -> selects Kanji ROM */
			/* 0 -> 1 -> latches Kanji ROM data */

			if(((state->m_kanji_eksel & 1) == 0) && ((data & 1) == 1))
			{
				state->m_kanji_addr = (state->m_kanji_addr_latch);
				//state->m_kanji_addr &= 0x3fff; //<- temp kludge until the rom is redumped.
				//printf("%08x\n",state->m_kanji_addr);
				//state->m_kanji_addr+= state->m_kanji_count;
			}
			state->m_kanji_eksel = data & 1;
		}
		break;
	}
}

static READ8_HANDLER( x1_emm_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *emm_ram = space->machine().region("emm")->base();
	UINT8 res;

	if(offset & ~3)
	{
		printf("Warning: read EMM BASIC area [%02x]\n",offset & 0xff);
		return 0xff;
	}

	if(offset != 3)
		printf("Warning: read EMM address [%02x]\n",offset);

	res = 0xff;

	if(offset == 3)
	{
		res = emm_ram[state->m_emm_addr];
		state->m_emm_addr++;
	}

	return res;
}

static WRITE8_HANDLER( x1_emm_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *emm_ram = space->machine().region("emm")->base();

	if(offset & ~3)
	{
		printf("Warning: write EMM BASIC area [%02x] %02x\n",offset & 0xff,data);
		return;
	}

	switch(offset)
	{
		case 0: state->m_emm_addr = (state->m_emm_addr & 0xffff00) | (data & 0xff); break;
		case 1: state->m_emm_addr = (state->m_emm_addr & 0xff00ff) | (data << 8);   break;
		case 2: state->m_emm_addr = (state->m_emm_addr & 0x00ffff) | (data << 16);  break; //TODO: this has a max size limit, check exactly how much
		case 3:
			emm_ram[state->m_emm_addr] = data;
			state->m_emm_addr++;
			break;
	}
}

/*
    CZ-141SF, CZ-127MF, X1turboZII, X1turboZ3 boards
*/
static READ8_HANDLER( x1turbo_bank_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();

//  printf("BANK access read\n");
	return state->m_ex_bank & 0x3f;
}

static WRITE8_HANDLER( x1turbo_bank_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	//UINT8 *RAM = space->machine().region("maincpu")->base();
	/*
    --x- ---- BML5: latch bit (doesn't have any real function)
    ---x ---- BMCS: select bank RAM, active low
    ---- xxxx BMNO: Bank memory ID
    */

	state->m_ex_bank = data & 0x3f;
//  printf("BANK access write %02x\n",data);
}

/* TODO: waitstate penalties */
static READ8_HANDLER( x1_mem_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *wram = space->machine().region("wram")->base();

	if((offset & 0x8000) == 0 && (state->m_ram_bank == 0))
	{
		UINT8 *ipl = space->machine().region("ipl")->base();
		return ipl[offset]; //ROM
	}

	return wram[offset]; //RAM
}

static WRITE8_HANDLER( x1_mem_w )
{
	//x1_state *state = space->machine().driver_data<x1_state>();
	UINT8 *wram = space->machine().region("wram")->base();

	wram[offset] = data; //RAM
}

static READ8_HANDLER( x1turbo_mem_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if((state->m_ex_bank & 0x10) == 0)
	{
		UINT8 *wram = space->machine().region("wram")->base();

		return wram[offset+((state->m_ex_bank & 0xf)*0x10000)];
	}

	return x1_mem_r(space,offset);
}

static WRITE8_HANDLER( x1turbo_mem_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if((state->m_ex_bank & 0x10) == 0)
	{
		UINT8 *wram = space->machine().region("wram")->base();

		wram[offset+((state->m_ex_bank & 0xf)*0x10000)] = data; //RAM
	}
	else
		x1_mem_w(space,offset,data);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static READ8_HANDLER( x1_io_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_io_bank_mode = 0; //any read disables the extended mode.

	if(offset == 0x0e03)                    		{ return x1_rom_r(space, 0); }
	// TODO: user could install ym2151 on plain X1 too
	//0x700, 0x701
	//if(offset >= 0x0704 && offset <= 0x0707)      { return z80ctc_r(space->machine().device("ctc"), offset-0x0704); }
	else if(offset >= 0x0ff8 && offset <= 0x0fff)	{ return x1_fdc_r(space, offset-0xff8); }
	else if(offset >= 0x1400 && offset <= 0x17ff)	{ return x1_pcg_r(space, offset-0x1400); }
	else if(offset >= 0x1900 && offset <= 0x19ff)	{ return sub_io_r(space, 0); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)	{ return space->machine().device<i8255_device>("ppi8255_0")->read(*space, (offset-0x1a00) & 3); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)	{ return ay8910_r(space->machine().device("ay"), 0); }
//  else if(offset >= 0x1f80 && offset <= 0x1f8f)   { return z80dma_r(space->machine().device("dma"), 0); }
//  else if(offset >= 0x1f90 && offset <= 0x1f91)   { return z80sio_c_r(space->machine().device("sio"), (offset-0x1f90) & 1); }
//  else if(offset >= 0x1f92 && offset <= 0x1f93)   { return z80sio_d_r(space->machine().device("sio"), (offset-0x1f92) & 1); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)	{ return z80ctc_r(space->machine().device("ctc"), offset-0x1fa0); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)	{ return z80ctc_r(space->machine().device("ctc"), offset-0x1fa8); }
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { return x1_scrn_r(space,offset-0x1fd0); }
//  else if(offset == 0x1fe0)                       { return x1_blackclip_r(space,0); }
	else if(offset >= 0x2000 && offset <= 0x2fff)	{ return state->m_avram[offset & 0x7ff]; }
	else if(offset >= 0x3000 && offset <= 0x3fff)	{ return state->m_tvram[offset & 0x7ff]; } // Ys checks if it's a x1/x1turbo machine by checking if this area is a mirror
	else if(offset >= 0x4000 && offset <= 0xffff)	{ return state->m_gfx_bitmap_ram[offset-0x4000+(state->m_scrn_reg.gfx_bank*0xc000)]; }
	else
	{
		logerror("(PC=%06x) Read i/o address %04x\n",cpu_get_pc(&space->device()),offset);
	}
	return 0xff;
}

static WRITE8_HANDLER( x1_io_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	if(state->m_io_bank_mode == 1)                  	{ x1_ex_gfxram_w(space, offset, data); }
	// TODO: user could install ym2151 on plain X1 too
	//0x700, 0x701
//  else if(offset >= 0x0704 && offset <= 0x0707)   { z80ctc_w(space->machine().device("ctc"), offset-0x0704,data); }
//  else if(offset >= 0x0c00 && offset <= 0x0cff)   { x1_rs232c_w(space->machine(), 0, data); }
	else if(offset >= 0x0e00 && offset <= 0x0e02)	{ x1_rom_w(space, offset-0xe00,data); }
//  else if(offset >= 0x0e80 && offset <= 0x0e82)   { x1_kanji_w(space->machine(), offset-0xe80,data); }
	else if(offset >= 0x0ff8 && offset <= 0x0fff)	{ x1_fdc_w(space, offset-0xff8,data); }
	else if(offset >= 0x1000 && offset <= 0x10ff)	{ x1_pal_b_w(space, 0,data); }
	else if(offset >= 0x1100 && offset <= 0x11ff)	{ x1_pal_r_w(space, 0,data); }
	else if(offset >= 0x1200 && offset <= 0x12ff)	{ x1_pal_g_w(space, 0,data); }
	else if(offset >= 0x1300 && offset <= 0x13ff)	{ x1_pri_w(space, 0,data); }
	else if(offset >= 0x1400 && offset <= 0x17ff)	{ x1_pcg_w(space, offset-0x1400,data); }
	else if(offset == 0x1800 || offset == 0x1801)	{ x1_6845_w(space, offset-0x1800, data); }
	else if(offset >= 0x1900 && offset <= 0x19ff)	{ sub_io_w(space, 0,data); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)	{ space->machine().device<i8255_device>("ppi8255_0")->write(*space, (offset-0x1a00) & 3,data); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)	{ ay8910_data_w(space->machine().device("ay"), 0,data); }
	else if(offset >= 0x1c00 && offset <= 0x1cff)	{ ay8910_address_w(space->machine().device("ay"), 0,data); }
	else if(offset >= 0x1d00 && offset <= 0x1dff)	{ rom_bank_1_w(space,0,data); }
	else if(offset >= 0x1e00 && offset <= 0x1eff)	{ rom_bank_0_w(space,0,data); }
//  else if(offset >= 0x1f80 && offset <= 0x1f8f)   { z80dma_w(space->machine().device("dma"), 0,data); }
//  else if(offset >= 0x1f90 && offset <= 0x1f91)   { z80sio_c_w(space->machine().device("sio"), (offset-0x1f90) & 1,data); }
//  else if(offset >= 0x1f92 && offset <= 0x1f93)   { z80sio_d_w(space->machine().device("sio"), (offset-0x1f92) & 1,data); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)	{ z80ctc_w(space->machine().device("ctc"), offset-0x1fa0,data); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)	{ z80ctc_w(space->machine().device("ctc"), offset-0x1fa8,data); }
//  else if(offset == 0x1fb0)                       { x1turbo_pal_w(space,0,data); }
//  else if(offset >= 0x1fb9 && offset <= 0x1fbf)   { x1turbo_txpal_w(space,offset-0x1fb9,data); }
//  else if(offset == 0x1fc0)                       { x1turbo_txdisp_w(space,0,data); }
//  else if(offset == 0x1fc5)                       { x1turbo_gfxpal_w(space,0,data); }
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { x1_scrn_w(space,0,data); }
//  else if(offset == 0x1fe0)                       { x1_blackclip_w(space,0,data); }
	else if(offset >= 0x2000 && offset <= 0x2fff)	{ state->m_avram[offset & 0x7ff] = data; }
	else if(offset >= 0x3000 && offset <= 0x3fff)	{ state->m_tvram[offset & 0x7ff] = data; }
	else if(offset >= 0x4000 && offset <= 0xffff)	{ state->m_gfx_bitmap_ram[offset-0x4000+(state->m_scrn_reg.gfx_bank*0xc000)] = data; }
	else
	{
		logerror("(PC=%06x) Write %02x at i/o address %04x\n",cpu_get_pc(&space->device()),data,offset);
	}
}

/* TODO: I should actually simplify this, by just overwriting X1 Turbo specifics here, and call plain X1 functions otherwise */
static READ8_HANDLER( x1turbo_io_r )
{
	x1_state *state = space->machine().driver_data<x1_state>();
	state->m_io_bank_mode = 0; //any read disables the extended mode.

	// a * at the end states devices used on plain X1 too
	if(offset == 0x0700)							{ return (ym2151_r(space->machine().device("ym"), offset-0x0700) & 0x7f) | (input_port_read(space->machine(), "SOUND_SW") & 0x80); }
	else if(offset == 0x0701)		                { return ym2151_r(space->machine().device("ym"), offset-0x0700); }
	//0x704 is FM sound detection port on X1 turboZ
	else if(offset >= 0x0704 && offset <= 0x0707)   { return z80ctc_r(space->machine().device("ctc"), offset-0x0704); }
	else if(offset == 0x0801)						{ printf("Color image board read\n"); return 0xff; } // *
	else if(offset == 0x0803)						{ printf("Color image board 2 read\n"); return 0xff; } // *
	else if(offset >= 0x0a00 && offset <= 0x0a07)	{ printf("Stereoscopic board read %04x\n",offset); return 0xff; } // *
	else if(offset == 0x0b00)						{ return x1turbo_bank_r(space,0); }
	else if(offset >= 0x0c00 && offset <= 0x0cff)   { printf("RS-232C read %04x\n",offset); return 0; } // *
	else if(offset >= 0x0d00 && offset <= 0x0dff)	{ return x1_emm_r(space,offset & 0xff); } // *
	else if(offset == 0x0e03)                   	{ return x1_rom_r(space, 0); }
	else if(offset >= 0x0e80 && offset <= 0x0e81)	{ return x1_kanji_r(space, offset-0xe80); }
	else if(offset >= 0x0fd0 && offset <= 0x0fd3)	{ /* printf("SASI HDD read %04x\n",offset); */ return 0xff; } // *
	else if(offset >= 0x0fe8 && offset <= 0x0fef)	{ printf("8-inch FD read %04x\n",offset); return 0xff; } // *
	else if(offset >= 0x0ff8 && offset <= 0x0fff)	{ return x1_fdc_r(space, offset-0xff8); }
	else if(offset >= 0x1400 && offset <= 0x17ff)	{ return x1_pcg_r(space, offset-0x1400); }
	else if(offset >= 0x1900 && offset <= 0x19ff)	{ return sub_io_r(space, 0); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)	{ return space->machine().device<i8255_device>("ppi8255_0")->read(*space, (offset-0x1a00) & 3); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)	{ return ay8910_r(space->machine().device("ay"), 0); }
	else if(offset >= 0x1f80 && offset <= 0x1f8f)	{ return z80dma_r(space->machine().device("dma"), 0); }
	else if(offset >= 0x1f90 && offset <= 0x1f93)	{ return z80dart_ba_cd_r(space->machine().device("sio"), (offset-0x1f90) & 3); }
	else if(offset >= 0x1f98 && offset <= 0x1f9f)	{ printf("Extended SIO/CTC read %04x\n",offset); return 0xff; }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)	{ return z80ctc_r(space->machine().device("ctc"), offset-0x1fa0); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)	{ return z80ctc_r(space->machine().device("ctc"), offset-0x1fa8); }
	else if(offset == 0x1fb0)						{ return x1turbo_pal_r(space,0); } // Z only!
	else if(offset >= 0x1fb8 && offset <= 0x1fbf)	{ return x1turbo_txpal_r(space,offset-0x1fb8); } //Z only!
	else if(offset == 0x1fc0)						{ return x1turbo_txdisp_r(space,0); } // Z only!
	else if(offset == 0x1fc5)						{ return x1turbo_gfxpal_r(space,0); } // Z only!
//  else if(offset >= 0x1fd0 && offset <= 0x1fdf)   { return x1_scrn_r(space,offset-0x1fd0); } //Z only
	else if(offset == 0x1fe0)						{ return x1_blackclip_r(space,0); }
	else if(offset == 0x1ff0)						{ return input_port_read(space->machine(), "X1TURBO_DSW"); }
	else if(offset >= 0x2000 && offset <= 0x2fff)	{ return state->m_avram[offset & 0x7ff]; }
	else if(offset >= 0x3000 && offset <= 0x37ff)	{ return state->m_tvram[offset & 0x7ff]; }
	else if(offset >= 0x3800 && offset <= 0x3fff)	{ return state->m_kvram[offset & 0x7ff]; }
	else if(offset >= 0x4000 && offset <= 0xffff)	{ return state->m_gfx_bitmap_ram[offset-0x4000+(state->m_scrn_reg.gfx_bank*0xc000)]; }
	else
	{
		logerror("(PC=%06x) Read i/o address %04x\n",cpu_get_pc(&space->device()),offset);
	}
	return 0xff;
}

static WRITE8_HANDLER( x1turbo_io_w )
{
	x1_state *state = space->machine().driver_data<x1_state>();

	// a * at the end states devices used on plain X1 too
	if(state->m_io_bank_mode == 1)                    { x1_ex_gfxram_w(space, offset, data); }
	else if(offset == 0x0700 || offset == 0x0701)	{ ym2151_w(space->machine().device("ym"), offset-0x0700,data); }
	//0x704 is FM sound detection port on X1 turboZ
	else if(offset >= 0x0704 && offset <= 0x0707)	{ z80ctc_w(space->machine().device("ctc"), offset-0x0704,data); }
	else if(offset == 0x0800)						{ printf("Color image board write %02x\n",data); } // *
	else if(offset == 0x0802)						{ printf("Color image board 2 write %02x\n",data); } // *
	else if(offset >= 0x0a00 && offset <= 0x0a07)	{ printf("Stereoscopic board write %04x %02x\n",offset,data); } // *
	else if(offset == 0x0b00)						{ x1turbo_bank_w(space,0,data); }
	else if(offset >= 0x0c00 && offset <= 0x0cff)   { printf("RS-232C write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0d00 && offset <= 0x0dff)	{ x1_emm_w(space,offset & 0xff,data); } // *
	else if(offset >= 0x0e00 && offset <= 0x0e02)	{ x1_rom_w(space, offset-0xe00,data); }
	else if(offset >= 0x0e80 && offset <= 0x0e83)	{ x1_kanji_w(space, offset-0xe80,data); }
	else if(offset >= 0x0fd0 && offset <= 0x0fd3)	{ printf("SASI HDD write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0fe8 && offset <= 0x0fef)	{ printf("8-inch FD write %04x %02x\n",offset,data); } // *
	else if(offset >= 0x0ff8 && offset <= 0x0fff)	{ x1_fdc_w(space, offset-0xff8,data); }
	else if(offset >= 0x1000 && offset <= 0x10ff)	{ x1_pal_b_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1100 && offset <= 0x11ff)	{ x1_pal_r_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1200 && offset <= 0x12ff)	{ x1_pal_g_w(space, offset & 0x3ff,data); }
	else if(offset >= 0x1300 && offset <= 0x13ff)	{ x1_pri_w(space, 0,data); }
	else if(offset >= 0x1400 && offset <= 0x17ff)	{ x1_pcg_w(space, offset-0x1400,data); }
	else if(offset == 0x1800 || offset == 0x1801)	{ x1_6845_w(space, offset-0x1800, data); }
	else if(offset >= 0x1900 && offset <= 0x19ff)	{ sub_io_w(space, 0,data); }
	else if(offset >= 0x1a00 && offset <= 0x1aff)	{ space->machine().device<i8255_device>("ppi8255_0")->write(*space, (offset-0x1a00) & 3,data); }
	else if(offset >= 0x1b00 && offset <= 0x1bff)	{ ay8910_data_w(space->machine().device("ay"), 0,data); }
	else if(offset >= 0x1c00 && offset <= 0x1cff)	{ ay8910_address_w(space->machine().device("ay"), 0,data); }
	else if(offset >= 0x1d00 && offset <= 0x1dff)	{ rom_bank_1_w(space,0,data); }
	else if(offset >= 0x1e00 && offset <= 0x1eff)	{ rom_bank_0_w(space,0,data); }
	else if(offset >= 0x1f80 && offset <= 0x1f8f)	{ z80dma_w(space->machine().device("dma"), 0,data); }
	else if(offset >= 0x1f90 && offset <= 0x1f93)	{ z80dart_ba_cd_w(space->machine().device("sio"), (offset-0x1f90) & 3,data); }
	else if(offset >= 0x1f98 && offset <= 0x1f9f)	{ printf("Extended SIO/CTC write %04x %02x\n",offset,data); }
	else if(offset >= 0x1fa0 && offset <= 0x1fa3)	{ z80ctc_w(space->machine().device("ctc"), offset-0x1fa0,data); }
	else if(offset >= 0x1fa8 && offset <= 0x1fab)	{ z80ctc_w(space->machine().device("ctc"), offset-0x1fa8,data); }
	else if(offset == 0x1fb0)						{ x1turbo_pal_w(space,0,data); } // Z only!
	else if(offset >= 0x1fb8 && offset <= 0x1fbf)	{ x1turbo_txpal_w(space,offset-0x1fb8,data); } //Z only!
	else if(offset == 0x1fc0)						{ x1turbo_txdisp_w(space,0,data); } //Z only!
	else if(offset == 0x1fc1)						{ printf("Z image capturing access %02x\n",data); } // Z only!
	else if(offset == 0x1fc2)						{ printf("Z mosaic effect access %02x\n",data); } // Z only!
	else if(offset == 0x1fc3)						{ printf("Z Chroma key access %02x\n",data); } // Z only!
	else if(offset == 0x1fc4)						{ printf("Z Extra scroll config access %02x\n",data); } // Z only!
	else if(offset == 0x1fc5)						{ x1turbo_gfxpal_w(space,0,data); } // Z only!
	else if(offset >= 0x1fd0 && offset <= 0x1fdf)	{ x1_scrn_w(space,0,data); }
	else if(offset == 0x1fe0)						{ x1_blackclip_w(space,0,data); }
	else if(offset >= 0x2000 && offset <= 0x2fff)	{ state->m_avram[offset & 0x7ff] = data; }
	else if(offset >= 0x3000 && offset <= 0x37ff)	{ state->m_tvram[offset & 0x7ff] = data; }
	else if(offset >= 0x3800 && offset <= 0x3fff)	{ state->m_kvram[offset & 0x7ff] = data; }
	else if(offset >= 0x4000 && offset <= 0xffff)	{ state->m_gfx_bitmap_ram[offset-0x4000+(state->m_scrn_reg.gfx_bank*0xc000)] = data; }
	else
	{
		logerror("(PC=%06x) Write %02x at i/o address %04x\n",cpu_get_pc(&space->device()),data,offset);
	}
}

static ADDRESS_MAP_START( x1_mem, AS_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1_mem_r,x1_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1turbo_mem, AS_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1turbo_mem_r,x1turbo_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1_io , AS_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1_io_r, x1_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( x1turbo_io , AS_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(x1turbo_io_r, x1turbo_io_w)
ADDRESS_MAP_END

/*************************************
 *
 *  PPI8255
 *
 *************************************/

static READ8_DEVICE_HANDLER( x1_porta_r )
{
	printf("PPI Port A read\n");
	return 0xff;
}

/* this port is system related */
static READ8_DEVICE_HANDLER( x1_portb_r )
{
	x1_state *state = device->machine().driver_data<x1_state>();
	//printf("PPI Port B read\n");
	/*
    x--- ---- "v disp"
    -x-- ---- "sub cpu ibf"
    --x- ---- "sub cpu obf"
    ---x ---- ROM/RAM flag (0=ROM, 1=RAM)
    ---- x--- "busy" <- allow printer data output
    ---- -x-- "v sync"
    ---- --x- "cmt read"
    ---- ---x "cmt test" (active low) <- actually this is "Sub CPU detected BREAK"
    */
	UINT8 res = 0;
	int vblank_line = mc6845_v_display * mc6845_tile_height;
	int vsync_line = mc6845_v_sync_pos * mc6845_tile_height;
	state->m_vdisp = (device->machine().primary_screen->vpos() < vblank_line) ? 0x80 : 0x00;
	state->m_vsync = (device->machine().primary_screen->vpos() < vsync_line) ? 0x00 : 0x04;

//  popmessage("%d",vsync_line);
//  popmessage("%d",vblank_line);

	res = state->m_ram_bank | state->m_sub_obf | state->m_vsync | state->m_vdisp;

	if((device->machine().device<cassette_image_device>(CASSETTE_TAG))->input() > 0.03)
		res |= 0x02;

//  if(cassette_get_state(device->machine().device<cassette_image_device>(CASSETTE_TAG)) & CASSETTE_MOTOR_DISABLED)
//      res &= ~0x02;  // is zero if not playing

	// CMT test bit is set low when the CMT Stop command is issued, and becomes
	// high again when this bit is read.
	res |= 0x01;
	if(state->m_cmt_test != 0)
	{
		state->m_cmt_test = 0;
		res &= ~0x01;
	}

	return res;
}

/* I/O system port */
static READ8_DEVICE_HANDLER( x1_portc_r )
{
	x1_state *state = device->machine().driver_data<x1_state>();
	//printf("PPI Port C read\n");
	/*
    x--- ---- Printer port output
    -x-- ---- 320 mode (r/w), divider for the pixel clock
    --x- ---- i/o mode (r/w)
    ---x ---- smooth scroll enabled (?)
    ---- ---x cassette output data
    */
	return (state->m_io_sys & 0x9f) | state->m_hres_320 | ~state->m_io_switch;
}

static WRITE8_DEVICE_HANDLER( x1_porta_w )
{
	//printf("PPI Port A write %02x\n",data);
}

static WRITE8_DEVICE_HANDLER( x1_portb_w )
{
	//printf("PPI Port B write %02x\n",data);
}

static WRITE8_DEVICE_HANDLER( x1_portc_w )
{
	x1_state *state = device->machine().driver_data<x1_state>();
	state->m_hres_320 = data & 0x40;

	/* set up the pixel clock according to the above divider */
	device->machine().device<mc6845_device>("crtc")->set_clock(VDP_CLOCK/((state->m_hres_320) ? 48 : 24));

	if(((data & 0x20) == 0) && (state->m_io_switch & 0x20))
		state->m_io_bank_mode = 1;

	state->m_io_switch = data & 0x20;
	state->m_io_sys = data & 0xff;

	device->machine().device<cassette_image_device>(CASSETTE_TAG)->output((data & 0x01) ? +1.0 : -1.0);
}

static I8255A_INTERFACE( ppi8255_intf )
{
	DEVCB_HANDLER(x1_porta_r),						/* Port A read */
	DEVCB_HANDLER(x1_porta_w),						/* Port A write */
	DEVCB_HANDLER(x1_portb_r),						/* Port B read */
	DEVCB_HANDLER(x1_portb_w),						/* Port B write */
	DEVCB_HANDLER(x1_portc_r),						/* Port C read */
	DEVCB_HANDLER(x1_portc_w)						/* Port C write */
};

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }

static Z80DMA_INTERFACE( x1_dma )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_HALT),
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	DEVCB_MEMORY_HANDLER("maincpu", IO, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", IO, memory_write_byte)
};

/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_CHANGED( ipl_reset )
{
	//address_space *space = field.machine().device("maincpu")->memory().space(AS_PROGRAM);
	x1_state *state = field.machine().driver_data<x1_state>();

	cputag_set_input_line(field.machine(), "maincpu", INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	state->m_ram_bank = 0x00;
	if(state->m_is_turbo) { state->m_ex_bank = 0x10; }
	//anything else?
}

/* Apparently most games doesn't support this (not even the Konami ones!), one that does is...177 :o */
static INPUT_CHANGED( nmi_reset )
{
	cputag_set_input_line(field.machine(), "maincpu", INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( x1 )
	PORT_START("FP_SYS") //front panel buttons, hard-wired with the soft reset/NMI lines
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED(ipl_reset,0) PORT_NAME("IPL reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CHANGED(nmi_reset,0) PORT_NAME("NMI reset")

	PORT_START("SOUND_SW") //FIXME: this is X1Turbo specific
	PORT_DIPNAME( 0x80, 0x80, "OPM Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IOSYS") //TODO: implement front-panel DIP-SW here
	PORT_DIPNAME( 0x01, 0x01, "IOSYS" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sound Setting?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("key1") //0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED) //0x00 null
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-8") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-7") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-8") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)

	PORT_START("key2") //0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED) //0x21 !
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_UNUSED) //0x22 "
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_UNUSED) //0x23 #
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_UNUSED) //0x24 $
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_UNUSED) //0x25 %
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_UNUSED) //0x26 &
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED) //0x27 '
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED) //0x28 (
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED) //0x29 )
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2a *
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2b +
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2c ,
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2e .
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x2f /

	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3c <
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3d =
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3e >
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_UNUSED) //0x3f ?

	PORT_START("key3") //0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_")

	PORT_START("f_keys")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("tenkey")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	// TODO: add other numpad keys

	PORT_START("key_modifiers")
	PORT_BIT(0x00000001,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00000002,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x00000010,IP_ACTIVE_LOW,IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)

	#if 0
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey /") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey =")
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xEF\xBF\xA5")

	PORT_START("key3")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey ,")
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("EL") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLS") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DUP") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10)

	#endif
INPUT_PORTS_END

INPUT_PORTS_START( x1turbo )
	PORT_INCLUDE( x1 )

	PORT_START("X1TURBO_DSW")
	PORT_DIPNAME( 0x01, 0x01, "Interlace mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Default Auto-boot Device" ) // this selects what kind of device is loaded at start-up
	PORT_DIPSETTING(    0x00, "5/3-inch 2D" )
	PORT_DIPSETTING(    0x02, "5/3-inch 2DD" )
	PORT_DIPSETTING(    0x04, "5/3-inch 2HD" )
	PORT_DIPSETTING(    0x06, "5/3-inch 2DD (IBM)" )
	PORT_DIPSETTING(    0x08, "8-inch 2D256" )
	PORT_DIPSETTING(    0x0a, "8-inch 2D256 (IBM)" )
	PORT_DIPSETTING(    0x0c, "8-inch 1S128 (IBM)" )
	PORT_DIPSETTING(    0x0e, "SASI HDD" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) //this is a port conditional of some sort ...
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  GFX decoding
 *
 *************************************/

static const gfx_layout x1_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout x1_chars_8x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8*16
};

static const gfx_layout x1_pcg_8x8 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout x1_chars_16x16 =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

/* decoded for debugging purpose, this will be nuked in the end... */
static GFXDECODE_START( x1 )
	GFXDECODE_ENTRY( "cgrom",   0x00000, x1_chars_8x8,    0, 1 )
	GFXDECODE_ENTRY( "pcg",     0x00000, x1_pcg_8x8,      0, 1 )
	GFXDECODE_ENTRY( "font",    0x00000, x1_chars_8x16,   0, 1 )
	GFXDECODE_ENTRY( "kanji",   0x00000, x1_chars_16x16,  0, 1 )
GFXDECODE_END

/*************************************
 *
 *  CTC
 *
 *************************************/

static Z80CTC_INTERFACE( ctc_intf )
{
	0,					// timer disables
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),		// interrupt handler
	DEVCB_LINE(z80ctc_trg3_w),		// ZC/TO0 callback
	DEVCB_LINE(z80ctc_trg1_w),		// ZC/TO1 callback
	DEVCB_LINE(z80ctc_trg2_w),		// ZC/TO2 callback
};

#if 0
static const z80sio_interface sio_intf =
{
	0,					/* interrupt handler */
	0,					/* DTR changed handler */
	0,					/* RTS changed handler */
	0,					/* BREAK changed handler */
	0,					/* transmit handler */
	0					/* receive handler */
};
#endif


static Z80DART_INTERFACE( sio_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0)
};

static const z80_daisy_config x1_daisy[] =
{
    { "x1kb" },
	{ "ctc" },
	{ NULL }
};

static const z80_daisy_config x1turbo_daisy[] =
{
    { "x1kb" },
	{ "ctc" },
	{ "dma" },
	{ "sio" },
	{ NULL }
};

/*************************************
 *
 *  Sound HW Config
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("P1"),
	DEVCB_INPUT_PORT("P2"),
	DEVCB_NULL,
	DEVCB_NULL
};

// (ym-2151 handler here)

/*************************************
 *
 *  Cassette configuration
 *
 *************************************/

static const cassette_interface x1_cassette_interface =
{
	x1_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	"x1_cass",
	NULL
};


/*************************************
 *
 *  Machine Functions
 *
 *************************************/

#ifdef UNUSED_FUNCTION
static IRQ_CALLBACK(x1_irq_callback)
{
	x1_state *state = device->machine().driver_data<x1_state>();
    if(state->m_ctc_irq_flag != 0)
    {
        state->m_ctc_irq_flag = 0;
        if(state->m_key_irq_flag == 0)  // if no other devices are pulling the IRQ line high
            device_set_input_line(device, 0, CLEAR_LINE);
        return state->m_irq_vector;
    }
    if(state->m_key_irq_flag != 0)
    {
        state->m_key_irq_flag = 0;
        if(state->m_ctc_irq_flag == 0)  // if no other devices are pulling the IRQ line high
            device_set_input_line(device, 0, CLEAR_LINE);
        return state->m_key_irq_vector;
    }
    return state->m_irq_vector;
}
#endif

static TIMER_DEVICE_CALLBACK(keyboard_callback)
{
	x1_state *state = timer.machine().driver_data<x1_state>();
	address_space *space = timer.machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT32 key1 = input_port_read(timer.machine(),"key1");
	UINT32 key2 = input_port_read(timer.machine(),"key2");
	UINT32 key3 = input_port_read(timer.machine(),"key3");
	UINT32 key4 = input_port_read(timer.machine(),"tenkey");
	UINT32 f_key = input_port_read(timer.machine(), "f_keys");

	if(state->m_key_irq_vector)
	{
		//if(key1 == 0 && key2 == 0 && key3 == 0 && key4 == 0 && f_key == 0)
		//  return;

		if((key1 != state->m_old_key1) || (key2 != state->m_old_key2) || (key3 != state->m_old_key3) || (key4 != state->m_old_key4) || (f_key != state->m_old_fkey))
		{
			// generate keyboard IRQ
			sub_io_w(space,0,0xe6);
			state->m_irq_vector = state->m_key_irq_vector;
			state->m_key_irq_flag = 1;
			cputag_set_input_line(timer.machine(),"maincpu",0,ASSERT_LINE);
			state->m_old_key1 = key1;
			state->m_old_key2 = key2;
			state->m_old_key3 = key3;
			state->m_old_key4 = key4;
			state->m_old_fkey = f_key;
		}
	}
}

static TIMER_CALLBACK(x1_rtc_increment)
{
	x1_state *state = machine.driver_data<x1_state>();
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

	state->m_rtc.sec++;

	if((state->m_rtc.sec & 0x0f) >= 0x0a)				{ state->m_rtc.sec+=0x10; state->m_rtc.sec&=0xf0; }
	if((state->m_rtc.sec & 0xf0) >= 0x60)				{ state->m_rtc.min++; state->m_rtc.sec = 0; }
	if((state->m_rtc.min & 0x0f) >= 0x0a)				{ state->m_rtc.min+=0x10; state->m_rtc.min&=0xf0; }
	if((state->m_rtc.min & 0xf0) >= 0x60)				{ state->m_rtc.hour++; state->m_rtc.min = 0; }
	if((state->m_rtc.hour & 0x0f) >= 0x0a)				{ state->m_rtc.hour+=0x10; state->m_rtc.hour&=0xf0; }
	if((state->m_rtc.hour & 0xff) >= 0x24)				{ state->m_rtc.day++; state->m_rtc.wday++; state->m_rtc.hour = 0; }
	if((state->m_rtc.wday & 0x0f) >= 0x07)				{ state->m_rtc.wday = 0; }
	if((state->m_rtc.day & 0x0f) >= 0x0a)					{ state->m_rtc.day+=0x10; state->m_rtc.day&=0xf0; }
	/* FIXME: very crude leap year support (i.e. it treats the RTC to be with a 2000-2099 timeline), dunno how the real x1 supports this,
       maybe it just have a 1980-1999 timeline since year 0x00 shows as a XX on display */
	if(((state->m_rtc.year % 4) == 0) && state->m_rtc.month == 2)
	{
		if((state->m_rtc.day & 0xff) >= dpm[state->m_rtc.month-1]+1+1)
			{ state->m_rtc.month++; state->m_rtc.day = 0x01; }
	}
	else if((state->m_rtc.day & 0xff) >= dpm[state->m_rtc.month-1]+1){ state->m_rtc.month++; state->m_rtc.day = 0x01; }
	if(state->m_rtc.month > 12)							{ state->m_rtc.year++;  state->m_rtc.month = 0x01; }
	if((state->m_rtc.year & 0x0f) >= 0x0a)				{ state->m_rtc.year+=0x10; state->m_rtc.year&=0xf0; }
	if((state->m_rtc.year & 0xf0) >= 0xa0)				{ state->m_rtc.year = 0; } //roll over
}

static MACHINE_RESET( x1 )
{
	x1_state *state = machine.driver_data<x1_state>();
	//UINT8 *ROM = machine.region("maincpu")->base();
	UINT8 *PCG_RAM = machine.region("pcg")->base();
	int i;

	memset(state->m_gfx_bitmap_ram,0x00,0xc000*2);

	for(i=0;i<0x1800;i++)
	{
		PCG_RAM[i] = 0;
		gfx_element_mark_dirty(machine.gfx[1], i >> 3);
	}

	state->m_is_turbo = 0;

	state->m_io_bank_mode = 0;

	//device_set_irq_callback(machine.device("maincpu"), x1_irq_callback);

	state->m_cmt_current_cmd = 0;
	state->m_cmt_test = 0;
	machine.device<cassette_image_device>(CASSETTE_TAG)->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

	state->m_key_irq_flag = state->m_ctc_irq_flag = 0;
	state->m_sub_cmd = 0;
	state->m_key_irq_vector = 0;
	state->m_sub_cmd_length = 0;
	state->m_sub_val[0] = 0;
	state->m_sub_val[1] = 0;
	state->m_sub_val[2] = 0;
	state->m_sub_val[3] = 0;
	state->m_sub_val[4] = 0;
	state->m_sub_obf = (state->m_sub_cmd_length) ? 0x00 : 0x20;

	state->m_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	/* Reinitialize palette here if there's a soft reset for the Turbo PAL stuff*/
	for(i=0;i<0x10;i++)
		palette_set_color_rgb(machine, i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));

	state->m_ram_bank = 0;
//  state->m_old_vpos = -1;
}

static MACHINE_RESET( x1turbo )
{
	x1_state *state = machine.driver_data<x1_state>();
	MACHINE_RESET_CALL( x1 );
	state->m_is_turbo = 1;
	state->m_ex_bank = 0x10;

	state->m_scrn_reg.blackclip = 0;
}

static MACHINE_START( x1 )
{
	x1_state *state = machine.driver_data<x1_state>();

	/* set up RTC */
	{
		system_time systime;
		machine.base_datetime(systime);

		state->m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
		state->m_rtc.month = ((systime.local_time.month+1));
		state->m_rtc.wday = ((systime.local_time.weekday % 10) & 0xf);
		state->m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
		state->m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
		state->m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
		state->m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);

		state->m_rtc_timer = machine.scheduler().timer_alloc(FUNC(x1_rtc_increment));
	}
}

static PALETTE_INIT(x1)
{
	int i;

	for(i=0;i<(0x10+0x1000);i++)
		palette_set_color(machine, i,MAKE_RGB(0x00,0x00,0x00));
}

static FLOPPY_OPTIONS_START( x1 )
	FLOPPY_OPTION( img2d, "2d", "2D disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
FLOPPY_OPTIONS_END

static const floppy_interface x1_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD_40,
	FLOPPY_OPTIONS_NAME(x1),
	"floppy_5_25",
	NULL
};

static MACHINE_CONFIG_START( x1, x1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(x1_mem)
	MCFG_CPU_IO_MAP(x1_io)
	MCFG_CPU_CONFIG(x1_daisy)

	MCFG_Z80CTC_ADD( "ctc", MAIN_CLOCK/4 , ctc_intf )

	MCFG_DEVICE_ADD("x1kb", X1_KEYBOARD, 0)

	MCFG_I8255A_ADD( "ppi8255_0", ppi8255_intf )

	MCFG_MACHINE_START(x1)
	MCFG_MACHINE_RESET(x1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(x1)
	MCFG_SCREEN_EOF(x1)

	MCFG_MC6845_ADD("crtc", H46505, (VDP_CLOCK/48), mc6845_intf) //unknown divider
	MCFG_PALETTE_LENGTH(0x10+0x1000)
	MCFG_PALETTE_INIT(x1)

	MCFG_GFXDECODE(x1)

	MCFG_VIDEO_START(x1)

	MCFG_MB8877_ADD("fdc",x1_mb8877a_interface)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom")
	MCFG_CARTSLOT_NOT_MANDATORY

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	/* TODO:is the AY mono or stereo? Also volume balance isn't right. */
	MCFG_SOUND_ADD("ay", AY8910, MAIN_CLOCK/8)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  0.5)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.5)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_CASSETTE_ADD(CASSETTE_TAG,x1_cassette_interface)
	MCFG_SOFTWARE_LIST_ADD("cass_list","x1_cass")

	MCFG_FLOPPY_4_DRIVES_ADD(x1_floppy_interface)
	MCFG_SOFTWARE_LIST_ADD("flop_list","x1_flop")

	MCFG_TIMER_ADD_PERIODIC("keyboard_timer", keyboard_callback, attotime::from_hz(250))
	MCFG_TIMER_ADD_PERIODIC("cmt_wind_timer", cmt_wind_timer, attotime::from_hz(16))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( x1turbo, x1 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(x1turbo_mem)
	MCFG_CPU_IO_MAP(x1turbo_io)
	MCFG_CPU_CONFIG(x1turbo_daisy)

	MCFG_MACHINE_RESET(x1turbo)

//  MCFG_Z80SIO_ADD( "sio", MAIN_CLOCK/4 , sio_intf )
	MCFG_Z80SIO0_ADD("sio", MAIN_CLOCK/4 , sio_intf )
	MCFG_Z80DMA_ADD( "dma", MAIN_CLOCK/4 , x1_dma )

	MCFG_DEVICE_REMOVE("fdc")
	MCFG_MB8877_ADD("fdc",x1turbo_mb8877a_interface)

	MCFG_SOUND_ADD("ym", YM2151, MAIN_CLOCK/8) //option board
//  MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker",  0.50)
MACHINE_CONFIG_END

/*************************************
 *
 * ROM definitions
 *
 *************************************/

 ROM_START( x1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1", 0x0000, 0x1000, CRC(7b28d9de) SHA1(c4db9a6e99873808c8022afd1c50fef556a8b44d) )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000000, "emm", ROMREGION_ERASEFF )

	ROM_REGION(0x1800, "pcg", ROMREGION_ERASEFF)

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, BAD_DUMP CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x1800, "cgrom", 0)
	ROM_LOAD("fnt0808.x1",  0x00000, 0x00800, CRC(e3995a57) SHA1(1c1a0d8c9f4c446ccd7470516b215ddca5052fb2) )
	ROM_COPY("font",	0x1000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)

	ROM_REGION( 0x1000000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0xffffff, ROM_OPTIONAL | ROM_NOMIRROR)
ROM_END

ROM_START( x1twin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x1000, CRC(e70011d3) SHA1(d3395e9aeb5b8bbba7654dd471bcd8af228ee69a) )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000000, "emm", ROMREGION_ERASEFF )

	ROM_REGION(0x1800, "pcg", ROMREGION_ERASEFF)

	ROM_REGION(0x1000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank16.rom", 0x0000, 0x1000, CRC(8f9fb213) SHA1(4f06d20c997a79ee6af954b69498147789bf1847) )

	ROM_REGION(0x1800, "cgrom", 0)
	ROM_LOAD("ank8.rom", 0x00000, 0x00800, CRC(e3995a57) SHA1(1c1a0d8c9f4c446ccd7470516b215ddca5052fb2) )
	ROM_COPY("font",	 0x00000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF) // these comes from x1 turbo
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, BAD_DUMP CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, BAD_DUMP CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, BAD_DUMP CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, BAD_DUMP CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )

	ROM_REGION( 0x1000000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0xffffff, ROM_OPTIONAL | ROM_NOMIRROR)
ROM_END

ROM_START( x1turbo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.x1t", 0x0000, 0x8000, CRC(2e8b767c) SHA1(44620f57a25f0bcac2b57ca2b0f1ebad3bf305d3) )

	ROM_REGION( 0x10000*0x10, "wram", ROMREGION_ERASE00 )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000000, "emm", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000*2, "bank_ram", ROMREGION_ERASEFF )

	ROM_REGION(0x1800, "pcg", ROMREGION_ERASEFF)

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1", 0x00000, 0x00800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",	         0x01000, 0x00800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )

	ROM_REGION( 0x1000000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0xffffff, ROM_OPTIONAL | ROM_NOMIRROR)
ROM_END

ROM_START( x1turbo40 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x8000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.bin", 0x0000, 0x8000, CRC(112f80a2) SHA1(646cc3fb5d2d24ff4caa5167b0892a4196e9f843) )

	ROM_REGION( 0x10000*0x10, "wram", ROMREGION_ERASE00 )

	ROM_REGION(0x1000, "mcu", ROMREGION_ERASEFF) //MCU for the Keyboard, "sub cpu"
	ROM_LOAD( "80c48", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x1800, "pcg", ROMREGION_ERASEFF)

	ROM_REGION( 0x1000000, "emm", ROMREGION_ERASEFF )

	ROM_REGION(0x2000, "font", 0) //TODO: this contains 8x16 charset only, maybe it's possible that it derivates a 8x8 charset by skipping gfx lines?
	ROM_LOAD( "ank.fnt", 0x0000, 0x2000, CRC(19689fbd) SHA1(0d4e072cd6195a24a1a9b68f1d37500caa60e599) )

	ROM_REGION(0x4800, "cgrom", 0)
	ROM_LOAD("fnt0808_turbo.x1",0x00000, 0x0800, CRC(84a47530) SHA1(06c0995adc7a6609d4272417fe3570ca43bd0454) )
	ROM_COPY("font",	        0x01000, 0x0800, 0x1000 )

	ROM_REGION(0x20000, "kanji", ROMREGION_ERASEFF)

	ROM_REGION(0x20000, "raw_kanji", ROMREGION_ERASEFF)
	ROM_LOAD("kanji4.rom", 0x00000, 0x8000, CRC(3e39de89) SHA1(d3fd24892bb1948c4697dedf5ff065ff3eaf7562) )
	ROM_LOAD("kanji2.rom", 0x08000, 0x8000, CRC(e710628a) SHA1(103bbe459dc8da27a9400aa45b385255c18fcc75) )
	ROM_LOAD("kanji3.rom", 0x10000, 0x8000, CRC(8cae13ae) SHA1(273f3329c70b332f6a49a3a95e906bbfe3e9f0a1) )
	ROM_LOAD("kanji1.rom", 0x18000, 0x8000, CRC(5874f70b) SHA1(dad7ada1b70c45f1e9db11db273ef7b385ef4f17) )

	ROM_REGION( 0x1000000, "cart_img", ROMREGION_ERASE00 )
	ROM_CART_LOAD("cart", 0x0000, 0xffffff, ROM_OPTIONAL | ROM_NOMIRROR)
ROM_END


/* Convert the ROM interleaving into something usable by the write handlers */
static DRIVER_INIT( kanji )
{
	UINT32 i,j,k,l;
	UINT8 *kanji = machine.region("kanji")->base();
	UINT8 *raw_kanji = machine.region("raw_kanji")->base();

	k = 0;
	for(l=0;l<2;l++)
	{
		for(j=l*16;j<(l*16)+0x10000;j+=32)
		{
			for(i=0;i<16;i++)
			{
				kanji[j+i] = raw_kanji[k];
				kanji[j+i+0x10000] = raw_kanji[0x10000+k];
				k++;
			}
		}
	}
}


/*    YEAR  NAME       PARENT  COMPAT   MACHINE  INPUT       INIT   COMPANY   FULLNAME      FLAGS */
COMP( 1982, x1,        0,      0,       x1,      x1,         0,    "Sharp",  "X1 (CZ-800C)",         0 )
COMP( 1986, x1twin,    x1,     0,       x1, 	 x1,         kanji,"Sharp",  "X1 Twin (CZ-830C)",    GAME_NOT_WORKING )
COMP( 1984, x1turbo,   x1,     0,       x1turbo, x1turbo,    kanji,"Sharp",  "X1 Turbo (CZ-850C)",   GAME_NOT_WORKING ) //model 10
COMP( 1985, x1turbo40, x1,     0,       x1turbo, x1turbo,    kanji,"Sharp",  "X1 Turbo (CZ-862C)",   0 ) //model 40
// x1turboz  /* 1986 Sharp X1 TurboZ  */
