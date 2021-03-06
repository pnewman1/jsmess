#include "emu.h"
#include "video/konicdev.h"
#include "includes/xmen.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void xmen_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	xmen_state *state = machine.driver_data<xmen_state>();

	/* (color & 0x02) is flip y handled internally by the 052109 */
	if (layer == 0)
		*color = state->m_layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = state->m_layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void xmen_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	xmen_state *state = machine.driver_data<xmen_state>();
	int pri = (*color & 0x00e0) >> 4;	/* ??????? */

	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->m_sprite_colorbase + (*color & 0x001f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( xmen6p )
{
	xmen_state *state = machine.driver_data<xmen_state>();

	k053247_get_ram(state->m_k053246, &state->m_k053247_ram);

	state->m_screen_left  = auto_bitmap_alloc(machine, 64 * 8, 32 * 8, BITMAP_FORMAT_INDEXED16);
	state->m_screen_right = auto_bitmap_alloc(machine, 64 * 8, 32 * 8, BITMAP_FORMAT_INDEXED16);

	state->save_item(NAME(*state->m_screen_left));
	state->save_item(NAME(*state->m_screen_right));
}


/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( xmen )
{
	xmen_state *state = screen->machine().driver_data<xmen_state>();
	int layer[3], bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI4);
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI1);
	state->m_layer_colorbase[0] = k053251_get_palette_index(state->m_k053251, K053251_CI3);
	state->m_layer_colorbase[1] = k053251_get_palette_index(state->m_k053251, K053251_CI0);
	state->m_layer_colorbase[2] = k053251_get_palette_index(state->m_k053251, K053251_CI2);

	k052109_tilemap_update(state->m_k052109);

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI0);
	layer[2] = 2;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI2);

	konami_sortlayers3(layer, state->m_layerpri);

	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase + 1);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);
	return 0;
}


SCREEN_UPDATE( xmen6p )
{
	xmen_state *state = screen->machine().driver_data<xmen_state>();
	int x, y;

	if (screen == state->m_lscreen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(state->m_screen_left, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}
	else if (screen == state->m_rscreen)
		for(y = 0; y < 32 * 8; y++)
		{
			UINT16* line_dest = BITMAP_ADDR16(bitmap, y, 0);
			UINT16* line_src = BITMAP_ADDR16(state->m_screen_right, y, 0);

			for (x = 12 * 8; x < 52 * 8; x++)
				line_dest[x] = line_src[x];
		}

	return 0;
}

/* my lefts and rights are mixed up in several places.. */
SCREEN_EOF( xmen6p )
{
	xmen_state *state = machine.driver_data<xmen_state>();
	int layer[3], bg_colorbase;
	bitmap_t * renderbitmap;
	rectangle cliprect;
	int offset;

	state->m_current_frame ^= 0x01;

//  const rectangle *visarea = machine.primary_screen->visible_area();
//  cliprect.min_x = visarea->min_x;
//  cliprect.max_x = visarea->max_x;
//  cliprect.min_y = visarea->min_y;
//  cliprect.max_y = visarea->max_y;

	cliprect.min_x = 0;
	cliprect.max_x = 64 * 8 - 1;
	cliprect.min_y = 2 * 8;
	cliprect.max_y = 30 * 8 - 1;


	if (state->m_current_frame & 0x01)
	{

		/* copy the desired spritelist to the chip */
		memcpy(state->m_k053247_ram, state->m_xmen6p_spriteramright, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered
           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(state->m_k052109, offset, state->m_xmen6p_tilemapright[offset] & 0x00ff);
		}


		renderbitmap = state->m_screen_right;
	}
	else
	{
		/* copy the desired spritelist to the chip */
		memcpy(state->m_k053247_ram, state->m_xmen6p_spriteramleft, 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
           everything gets marked as dirty and the desired tilemap is rendered

           this is not very efficient!
           */
		for (offset = 0; offset < (0xc000 / 2); offset++)
		{
//          K052109_lsb_w
			k052109_w(state->m_k052109, offset, state->m_xmen6p_tilemapleft[offset] & 0x00ff);
		}


		renderbitmap = state->m_screen_left;
	}


	bg_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI4);
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI1);
	state->m_layer_colorbase[0] = k053251_get_palette_index(state->m_k053251, K053251_CI3);
	state->m_layer_colorbase[1] = k053251_get_palette_index(state->m_k053251, K053251_CI0);
	state->m_layer_colorbase[2] = k053251_get_palette_index(state->m_k053251, K053251_CI2);

	k052109_tilemap_update(state->m_k052109);

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI0);
	layer[2] = 2;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI2);

	konami_sortlayers3(layer, state->m_layerpri);

	bitmap_fill(machine.priority_bitmap, &cliprect, 0);
	/* note the '+1' in the background color!!! */
	bitmap_fill(renderbitmap, &cliprect, 16 * bg_colorbase + 1);
	k052109_tilemap_draw(state->m_k052109, renderbitmap, &cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->m_k052109, renderbitmap, &cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->m_k052109, renderbitmap, &cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	k053247_sprites_draw(state->m_k053246, renderbitmap, &cliprect);
}
