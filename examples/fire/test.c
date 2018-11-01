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
static sprite_t *graph[50]; // sprites (0..40 background, 41..45 fire, 46 stick, 47 light)
static display_context_t disp = 0; // screen
char tStr[32]; // text

// INTERNAL FUNCTIONS
int error=-1;

// FPS
int16_t fps_tick=0;
int16_t fps_sec=0;

// TEST VARIABLES
int mouse_x=160;
int mouse_y=120;
int num;

// FRAMEBUFFER
uint16_t buffer_texture[768]={0}; // 24*32
int buffer_width=24;
int buffer_height=32;
int buffer_x=0;
int angle=0;
int radius=4;
int waves=25000;
int speed=4000;
int delay=0;

// FIRE
typedef struct
{
    int x;
    int y;
    int alpha;	
} fire;

fire fire_obj[100];

int max_fire=0;
int sel_fire=0;
int fire_anim=0;
int fire_tick=0;
float sphere_size=1.0;

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

    int i=0;
    int i1=0;
    char sprite_path[32];
	
    // BASIC RDP CONFIG
    uint64_t RDP_CONFIG = ATOMIC_PRIM | ALPHA_DITHER_SEL_NO_DITHER | RGB_DITHER_SEL_NO_DITHER; // VERY IMPORTANT, this example needs atomic prim or crashes in real hardware
	
    // LOAD SPRITES
    for(i=1;i<48;i++)
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

        // SET CLIPPING (we don't clear buffer, the background covers the full screen)
        rdp_attach_display(disp);
        rdp_sync(SYNC_PIPE);
        rdp_set_clipping(0,0,320,240); 
		
        // SCAN CONTROLS		
        update_controls();

        // RDP COPY MODE
        rdp_texture_copy(RDP_CONFIG);
		
        // DRAW BACKGROUND
        num=1;
        for(i=0;i<240;i+=32) // Y
        {		
            for(i1=0;i1<320;i1+=64) // X
            {
                rdp_load_texture(graph[num]);
                rdp_draw_sprite(i1,i,0);
                num++;
            }
        }		
		
        // JOYSTICK CONTROL
        if (joystick_x<-4 || joystick_x>4) { mouse_x+=joystick_x/5; }
        if (joystick_y<-4 || joystick_y>4) { mouse_y-=joystick_y/5; }
		
        if (mouse_x<32) { mouse_x=32; }
        if (mouse_x>288) { mouse_x=288; }
        if (mouse_y<48) { mouse_y=48; }
        if (mouse_y>208) { mouse_y=208; }

        // FRAMEBUFFER S DEFORMATION
        rdp_buffer_copy(disp,buffer_texture,mouse_x-12,mouse_y-40,buffer_width,buffer_height,1); // do texture from framebuffer, but don't load on TMEM yet
		
        angle=delay;
		
        for(i=0;i<buffer_height;i++) // Y
        {				
            buffer_x=radius+get_distx(angle,radius); // S effect
								
            if (buffer_x<4) // left movement
            {	
                if (buffer_x<0)
                    buffer_x=0;			
				
                buffer_x=4-buffer_x;
				
                // soft at the beginning			
                if (i<buffer_x)
                    buffer_x=i;
				
                memmove(&buffer_texture[i*buffer_width],&buffer_texture[(i*buffer_width)+buffer_x],(buffer_width-buffer_x)*2); // move memory blocks *2 (int16)
            }
            else // right mov
            {	
                // 7 pixel block max
                if (buffer_x>7)
                    buffer_x=7;
			
                buffer_x=buffer_x-4;
							
                if (i<buffer_x)
                    buffer_x=i;
				
                memmove(&buffer_texture[(i*buffer_width)+buffer_x],&buffer_texture[i*buffer_width],(buffer_width-buffer_x)*2);
            }
			
            angle+=waves;
        }	
	
        delay+=speed;
	
        // 360000 IS A FULL ROTATION
        angle = angle % 360000;
        delay = delay % 360000;	
		
        // DRAW FRAMEBUFFER AREA ONCE PROCESSED
        data_cache_hit_writeback(buffer_texture, (buffer_width * buffer_height) << 1 ); // CPU cache to RDRAM
        rdp_load_texbuf(buffer_texture,buffer_width-1,buffer_height-1); // custom function to load 16bit textures generated from arrays
        rdp_draw_sprite(mouse_x-12,mouse_y-40,0);		

        // DRAW STICK
        rdp_load_texture(graph[46]);
        rdp_cp_sprite(mouse_x,mouse_y,0,0,0,0); // Stick have custom center align embedded on 46.sprite, we use this function instead rdp_draw_sprite

        // DRAW FIRE
        rdp_texture_cycle(0,1,RDP_CONFIG); // enables alpha on 1cycle mode, keeps atomic_prim
        rdp_set_prim_color(255,255,255,164); // alpha set to 164, no RGB changes
        rdp_load_texture(graph[41+fire_anim]);
        rdp_cp_sprite(mouse_x,mouse_y,0,0,0,0); // fire follows the mouse, have embedded center align

        // PRESS A BUTTON OR HOLD Z		
        if (a_button==1 || z_button>0)
        {		
            sel_fire++;

            if (max_fire<99) // 99 sprites max
                max_fire++;
            
            if (sel_fire>99) // Fire variables will be recycled
                sel_fire=1;
			
            fire_obj[sel_fire].x=mouse_x+randx(-2,2);
            fire_obj[sel_fire].y=mouse_y+randx(-2,2);
            fire_obj[sel_fire].alpha=129;
			
            if (z_button>0) // Z does blur effect
                fire_obj[sel_fire].alpha=128;				
        }
		
        // START BUTTON RESETS FIRE
        if (start_button==1)
        {			
            max_fire=0;
            sel_fire=0;			
        }
		
        // FIRE (without reload TMEM)
        if (max_fire>0)
        {
            for(i=1;i<max_fire+1;i++)
            {
                if (fire_obj[i].alpha>8) // only if visible
                {
                    if (fire_obj[i].alpha<129) // fire will vanish (Z button)
                    {
                        fire_obj[i].alpha-=8;
                        rdp_set_prim_color(255,255,255,fire_obj[i].alpha);
                    }
                    else
                        rdp_set_prim_color(255,255,255,128); // fire stays on screen (A button)

                    // DRAW
                    rdp_cp_sprite(fire_obj[i].x,fire_obj[i].y,0,0,0,0);
                }
            }				
        }

        // ANIMATION TIED TO FRAMERATE		
        if (fire_tick==0)
        {			
            fire_tick=4; // delay	
            fire_anim++;		
            if (fire_anim>4)
                fire_anim=0;			
        }			
        else
            fire_tick--;		

        // DRAW CIRCLE LIGHT
        rdp_set_prim_color(255,255,255,randx(16,48)); // alpha is random between 16 and 48		
        rdp_texture_cycle(0,1,RDP_CONFIG | SAMPLE_TYPE); // light is smoother with filter (1cycle,alpha,config+filter enabled)
        rdp_load_texture(graph[47]);
        sphere_size=(randx(70,100)/100.0)+1.0; // the size of the lighting is variable too
        rdp_cp_sprite_scaled(mouse_x,mouse_y-16,sphere_size,sphere_size,0,16,16,0); // CP sprite with input center x16 y16 (which is the center of the 32x32 sphere)	
		
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

        if (error>0)
        {
            sprintf(tStr,"FILE: %d\n",error-1);
            graphics_draw_text(disp,40,200,tStr);
        }

        // FRAME READY
        display_show(disp);
    }
}