
#include <libtcod_sdl2_render.h>

#include <SDL.h>

#include <libtcod_int.h>

/* Return the renderer drawing region */
static struct SDL_Rect GetDestinationRect(struct TCOD_Tileset *tileset,
                                          int console_x,
                                          int console_y) {
  struct SDL_Rect rect = {
      console_x * tileset->tile_width,
      console_y * tileset->tile_height,
      tileset->tile_width,
      tileset->tile_height
  };
  return rect;
}
/* Return the rect for tile_id from tileset */
static struct SDL_Rect GetTileRect(struct TCOD_Tileset *tileset,
                                   int tile_id) {
  struct SDL_Rect rect = {
      tile_id % tileset->columns * tileset->tile_width,
      tile_id / tileset->columns * tileset->tile_height,
      tileset->tile_width,
      tileset->tile_height
  };
  return rect;
}

static int IsTileDirty(struct TCOD_Console *console,
                       struct TCOD_Console *cache,
                       int console_i) {
  int ch = console->ch_array[console_i];
  TCOD_color_t bg = console->bg_array[console_i];
  TCOD_color_t fg = console->fg_array[console_i];
  if (cache) {
    int cache_ch = cache->ch_array[console_i];
    TCOD_color_t cache_bg = cache->bg_array[console_i];
    TCOD_color_t cache_fg = cache->fg_array[console_i];
    if (ch == cache_ch &&
        bg.r == cache_bg.r && bg.g == cache_bg.g && bg.b == cache_bg.b &&
        fg.r == cache_fg.r && fg.g == cache_fg.g && fg.b == cache_fg.b) {
      return 0;
    }
  }
  return 1;
}

static void RenderTile(struct SDL_Renderer *renderer,
                       struct TCOD_Tileset *tileset,
                       struct TCOD_Console *console,
                       struct TCOD_Console *cache,
                       struct SDL_Texture *tile_texture,
                       int console_x, int console_y) {
  int console_i = console_y*console->w+console_x;
  int ch = console->ch_array[console_i];
  int tile_id = TCOD_tileset_get_tile_for_charcode(tileset, ch);
  struct SDL_Rect dest_rect = GetDestinationRect(tileset,
                                                 console_x, console_y);
  struct SDL_Rect tex_rect = GetTileRect(tileset, tile_id);
  TCOD_color_t bg = console->bg_array[console_i];
  TCOD_color_t fg = console->fg_array[console_i];
  if (!IsTileDirty(console, cache, console_i)) { return; }
  /* fill in the background color */
  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
  SDL_RenderFillRect(renderer, &dest_rect);
  /* skip invisible glyphs */
  if (bg.r == fg.r && bg.g == fg.g && bg.b == fg.b) { return; }
  if (ch == 0 || ch == 0x20) { return; }
  /* draw the foreground glyph */
  SDL_SetTextureColorMod(tile_texture, fg.r, fg.g, fg.b);
  SDL_RenderCopy(renderer, tile_texture, &tex_rect, &dest_rect);
  return;
}

static struct TCOD_Console* TCOD_sdl2_verify_cache_(
    struct TCOD_Console *console,
    struct TCOD_Console **cache_console) {
  if (!cache_console) { return NULL; }
  if (!*cache_console) { return NULL; }
  if ((TCOD_console_get_width(console) !=
       TCOD_console_get_width(*cache_console)) ||
      (TCOD_console_get_height(console) !=
       TCOD_console_get_height(*cache_console))) {
    TCOD_console_delete(*cache_console);
    *cache_console = NULL;
  }
  return *cache_console;
}

static int TCOD_sdl2_update_cache_(struct TCOD_Console *console,
                                   struct TCOD_Console **cache_console) {
  if (!cache_console) { return 1; }
  if (!*cache_console) {
    *cache_console = TCOD_console_new(TCOD_console_get_width(console),
                                      TCOD_console_get_height(console));
    if (!*cache_console) { return -1; }
  }
  TCOD_console_blit(console, 0, 0, 0, 0, *cache_console, 0, 0, 1.0f, 1.0f);
  return 0;
}

int TCOD_sdl_render_console(struct SDL_Renderer *renderer,
                            struct TCOD_Tileset *tileset,
                            struct TCOD_Console *console,
                            struct TCOD_Console **cache_console) {
  int console_x, console_y; /* console coordinate */
  struct TCOD_Console *con = console;
  struct TCOD_Console *cache = TCOD_sdl2_verify_cache_(console, cache_console);
  struct SDL_Texture *texture = TCOD_tileset_get_sdl_texture_(tileset,
                                                              renderer);
  if (!texture) { return -1; }
  /* renderer will fill in all rects without blending */
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  /* texture will blend using the alpha channel */
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(texture, 255);
  for (console_y = 0; console_y < con->h; ++console_y) {
    for (console_x = 0; console_x < con->w; ++console_x) {
      RenderTile(renderer, tileset, con, cache, texture,
                 console_x, console_y);
    }
  }
  TCOD_sdl2_update_cache_(console, cache_console);
  return 0;
}