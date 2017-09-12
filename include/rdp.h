/**
 * @file rdp.h
 * @brief Hardware Display Interface
 * @ingroup rdp
 */
#ifndef __LIBDRAGON_RDP_H
#define __LIBDRAGON_RDP_H

#include "display.h"
#include "graphics.h"

/**
 * @addtogroup rdp
 * @{
 */
 

/**
 * @brief RDP sync operations
 */
typedef enum
{
    /** @brief Wait for any operation to complete before causing a DP interrupt */
    SYNC_FULL,
    /** @brief Sync the RDP pipeline */
    SYNC_PIPE,
    /** @brief Block until all texture load operations are complete */
    SYNC_LOAD,
    /** @brief Block until all tile operations are complete */
    SYNC_TILE
} sync_t;

/**
 * @brief Caching strategy for loaded textures
 */
typedef enum
{
    /** @brief Textures are assumed to be pre-flushed */
    FLUSH_STRATEGY_NONE,
    /** @brief Cache will be flushed on all incoming textures */
    FLUSH_STRATEGY_AUTOMATIC
} flush_t;

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

void rdp_init( void );
void rdp_attach_display( display_context_t disp );
void rdp_detach_display( void );
void rdp_sync( sync_t sync );
void rdp_set_clipping( uint32_t tx, uint32_t ty, uint32_t bx, uint32_t by );
void rdp_set_default_clipping( void );
void rdp_enable_primitive_fill( void );
void rdp_enable_blend_fill( void );
void rdp_enable_texture_copy( void );
void rdp_load_texture(sprite_t *sprite );
void rdp_draw_textured_rectangle( int tx, int ty, int bx, int by, int flags );
void rdp_draw_textured_rectangle_scaled( int tx, int ty, int bx, int by, double x_scale, double y_scale, int flags );
void rdp_draw_sprite( int x, int y, int flags );
void rdp_draw_sprite_scaled( int x, int y, float x_scale, float y_scale, int flags );
void rdp_set_primitive_color( uint32_t color );
void rdp_set_blend_color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _alpha );
void rdp_draw_filled_rectangle( int tx, int ty, int bx, int by );
void rdp_draw_filled_triangle( float x1, float y1, float x2, float y2, float x3, float y3 );
void rdp_set_texture_flush( flush_t flush );
void rdp_close( void );

// RDP new
void rdp_send( void );
void rdp_command( uint32_t data );
void rdp_cp_sprite( int x, int y, int flags, int cp_x, int cp_y, int line );
void rdp_cp_sprite_scaled( int x, int y, float x_scale, float y_scale, int flags, int cp_x, int cp_y, int line );
void rdp_enable_filter( int type );
void rdp_enable_alpha( int type );
void rdp_enable_tlut( int type );
void rdp_enable_1primitive( int type );
void rdp_texture_1cycle( void );
void rdp_additive_blending( void );
void rdp_intensify( void );
void rdp_color( void );
void rdp_rgba_scale( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _alpha );
void rdp_load_tlut( uint8_t _pal_bp, uint8_t _pal_num, uint16_t *_palette );
void rdp_triangle_setup( int type );

// FRAMEBUFFER new
uint32_t get_pixel( display_context_t disp, int x, int y );
void rdp_buffer_copy(display_context_t disp, uint16_t *buffer_texture, uint16_t x_buf, uint16_t y_buf, uint16_t width, uint16_t height);
void rdp_buffer_screen(display_context_t disp, uint16_t *buffer_texture, int texture_mode);
void rdp_load_texbuf(uint16_t *buffer_texture, int sh, int th);

#ifdef __cplusplus
}
#endif

#endif
