#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include <stdlib.h>
#include <math.h>

#include "system.c"
#include "controls.c"

// SYSTEM
static sprite_t *graph[19]; // sprites
static display_context_t disp = 0; // screen
char tStr[32]; // text

// INTERNAL FUNCTIONS
uint8_t restore_type=1;
int error=-1;

// FPS
int16_t fps_tick=0;
int16_t fps_sec=0;

// 4bit (15 colors used)
uint16_t palette_0[16] = { 0,19,29791,48609,21141,59193,22851,2115,31303,45961,24577,32769,10573,56585,47105,0 };
uint16_t palette_copy[16] = { 0,19,29791,48609,21141,59193,22851,2115,31303,45961,24577,32769,10573,56585,47105,0 };

// VARIABLES
uint16_t color;
uint16_t color_buf=0;
int color_sel=0;

int mouse_x;
int mouse_y;

uint8_t r;
uint8_t g;
uint8_t b;
uint8_t color_text;

int backup_num;
int backup_color;

int start_rotate=0;
int rotate_direction=0;
int rotate_counter=0;

// PROGRAM
int main(void)
{
    // INTERRUPTS
    init_interrupts();

    // VIDEO
    display_init( RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
   
    // SYSTEM INIT
    dfs_init(DFS_DEFAULT_LOCATION);
    rdp_init();
    controller_init();
    timer_init();
	
    // FPS
    new_timer(TIMER_TICKS(1000000), TF_CONTINUOUS, update_counter);
	
    // INIT RAND
    srand(timer_ticks() & 0x7FFFFFFF);	
	
    // BASIC RDP CONFIG
    uint64_t RDP_CONFIG = ATOMIC_PRIM | ALPHA_DITHER_SEL_NO_DITHER | RGB_DITHER_SEL_NO_DITHER;	
	
    int i=0;
    char sprite_path[32];
		
    // LOAD SPRITES
    for(i=0;i<19;i++)
    {
        sprintf(sprite_path,"/%d.sprite",i);
        graph[i] = load_sprite(sprite_path);
		
        if (graph[i]==0)
        {	
            error=i+1;
            break;
        }
    }
	
    // LOOP
    while(1) 
    {	
        // WAIT BUFFER
        while( !(disp = display_lock()) );
        
        // SET CLIPPING
        rdp_attach_display(disp);
        rdp_sync(SYNC_PIPE);
        rdp_set_clipping(0,0,320,240);
		
        // CLEAN BUFFER: optional, if the full screen is covered by a scroll restore_type can be set to 0
        if (restore_type==1)
        {	
            rdp_enable_primitive_fill();
            rdp_set_fill_color(48,0,72,255);
            rdp_draw_filled_rectangle(0,0,319,239);
        }

        // SCAN CONTROLS		
        update_controls();	 
		
        // RDP TEXTURE MODE
        rdp_texture_copy(RDP_CONFIG);

        // DRAW COLOR TABLE		
        for(i=0;i<16;i++)
        {	
            rdp_load_texture(graph[i]);	// load	to TMEM	
            rdp_draw_sprite(115,30+(i*8),0); // draw
        }			

        // Cheap: RDP seems to work in parallel with CPU 
        // We force the last texture of the table to be drawn with the CPU to ensure the framebuffer we are going to read is updated
        graphics_draw_sprite(disp,115,158,graph[16]);

        // MOUSE USES JOYSTICK
        if (joystick_x<-4 || joystick_x>4) { mouse_x+=joystick_x/5; }
        if (joystick_y<-4 || joystick_y>4) { mouse_y-=joystick_y/5; }
		
        if (mouse_x<0) { mouse_x=0; }
        if (mouse_x>304) { mouse_x=304; }
        if (mouse_y<0) { mouse_y=0; }
        if (mouse_y>224) { mouse_y=224; }
		
        // GET PIXEL
        if (mouse_x>115)
        {	
            color=get_pixel(disp,mouse_x,mouse_y);

            // Extract
            uint8_t r1 = (color & 0xF800) >> 11; // 63488
            uint8_t g1 = (color & 0x7C0) >> 6; // 1984
            uint8_t b1 = (color & 0x3E) >> 1; // 62

            // Expand to 8bit
            r = r1 << 3;
            g = g1 << 3;
            b = b1 << 3;			
			
            if (mouse_x<297 && mouse_y>30 && mouse_y<166) 
            {	
                if (a_button>0)
                    color_buf=get_pixel(disp,mouse_x,mouse_y);	
            }
            else
                r = g = b = 0;
			
        }
		
        // SMALL BOX WITH THE COLOR SELECTED
        graphics_draw_box(disp, 40, 50, 16, 16, color_buf);		
		
        // CONTROL PALETTE ENTRY
        if (a_button==1)
        {
            // 0..7
            if (mouse_y>174 && mouse_y<174+24)
            {	
                for (i=1;i<8;i++)
                {	
                    if (mouse_x>49+(i*32) && mouse_x<49+24+(i*32))
                    {	
                        if (palette_0[i]!=color_buf)
                        {	
                            backup_color=palette_0[i];
                            palette_0[i]=color_buf;
                            backup_num=i;					
                        }	
                    }	
                }
            }		
			
            // 8..15
            if (mouse_y>206 && mouse_y<206+24)
            {	
                for (i=0;i<8;i++)
                {	
                    if (mouse_x>49+(i*32) && mouse_x<49+24+(i*32))
                    {	
                        if (palette_0[i+8]!=color_buf)
                        {	
                            backup_color=palette_0[i+8];
                            palette_0[i+8]=color_buf;
                            backup_num=i+8;
                        }	
                    }	
                }
            }			
			
        }
					
        // STOP PALETTE ROTATION			
        if (r_button==1)			
        {
            if (start_rotate==2)
            {	
                start_rotate=0;
                start_button=1;
            }	
        }
		
        // RESET CUSTOM PALETTE
        if (start_button==1)			
        {
            for(i=0;i<16;i++)
            {
                palette_0[i]=palette_copy[i];
            }
        }
		
        // REVERSE LAST COLOR EDITED
        if (z_button==1)
            palette_0[backup_num]=backup_color;
		
        // START PALETTE ROTATION
        if (l_button==1)
        {	
            if (start_rotate==0)
                start_rotate=1;
        }	
		
        if (start_rotate==1)
        {
            rotate_counter=0;
            rotate_direction=0;
            start_rotate=2;
        }
		
        if (start_rotate==2)
        {
			
            if (rotate_direction==0)
            {	
                rotate_counter+=4;
                if (rotate_counter>164) 
                    rotate_direction=1;
            }
            else
            {
                rotate_counter-=4;
                if (rotate_counter<4) 
                {	
                    rotate_direction=0;
                    rotate_counter=0;
                }	
            }		
			
            palette_0[1]=graphics_make_color(0,0,255-rotate_counter,255); // blue
            palette_0[10]=graphics_make_color(192-rotate_counter,0,0,255); // red
            palette_0[11]=graphics_make_color(255-rotate_counter,0,0,255); // red 1
            palette_0[14]=graphics_make_color(255-rotate_counter,0,0,255); // red 2
        }	
					
        // PALETTE SCREEN BOX
        for(i=0;i<8;i++)
        {	
            if (i!=0)
                graphics_draw_box(disp, 49+(i*32), 174, 24, 24, palette_0[i]);
			
            if (i!=7)
                graphics_draw_box(disp, 49+(i*32), 206, 24, 24, palette_0[i+8]);
        }					
		
        // cache must be invalidated for palette updates
        data_cache_hit_writeback_invalidate( palette_0, 16*2 ); // 16 colors, int16 = 2 bytes
		
        // DRAW ALUCARD SPRITE
        rdp_texture_cycle(0,0,RDP_CONFIG | EN_TLUT); // 1cycle because is going to be 2x size, enable tlut for palettes		
        rdp_load_palette(0,15,palette_0); // upload starting on palette 0, 15 colors upload, point to the palette struct
        rdp_load_texture(graph[18]);
        rdp_draw_sprite_scaled(45,70,2.0,2.0,0); // scaled 2X
		
        // THEN DRAW MOUSE
        rdp_texture_copy(RDP_CONFIG); // we disable tlut & we don't need 1cycle here
        rdp_load_texture(graph[17]);
        rdp_draw_sprite(mouse_x, mouse_y, 0 );	
		
        // RDP IS DONE
        rdp_detach_display();
		
        // FRAMERATE
        if(framerate_refresh==1)
        {
            fps_sec=fps_tick;
            fps_tick=0;
            framerate_refresh=0;
        }	
        fps_tick++;
		
        // TEXT
        sprintf(tStr,"FPS: %d\n",fps_sec);
        graphics_draw_text(disp,40,10,tStr);		

        sprintf(tStr, "R: %d\n",r);
        graphics_draw_text( disp, 40, 20, tStr );

        sprintf(tStr, "G: %d\n",g);
        graphics_draw_text( disp, 40, 30, tStr );

        sprintf(tStr, "B: %d\n",b);
        graphics_draw_text( disp, 40, 40, tStr );			
		
        if (error>0)
        {
            sprintf(tStr,"FILE: %d\n",error-1);
            graphics_draw_text(disp,40,200,tStr);
        }
		
        // FRAME READY
        display_show(disp);
    }
}