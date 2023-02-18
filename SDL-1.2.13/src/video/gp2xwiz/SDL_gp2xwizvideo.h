/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* GP2X Wiz video driver implementation.
   Copied from SDL-1.2.13/src/video/fbcon/SDL_fbvideo.h and modified it.
   2010.1.23 ikari
*/

#ifndef _SDL_gp2xwizvideo_h
#define _SDL_gp2xwizvideo_h

#include <sys/types.h>
#include <termios.h>
#include <linux/fb.h>

#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "../SDL_sysvideo.h"
#if SDL_INPUT_TSLIB
#include "tslib.h"
#endif

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this


/* This is the structure we use to keep track of video memory */
typedef struct vidmem_bucket {
	struct vidmem_bucket *prev;
	int used;
	unsigned char *base;
	unsigned int size;
	struct vidmem_bucket *next;
} vidmem_bucket;

/* Private display data */
struct SDL_PrivateVideoData {
	int current_vt;
	int saved_vt;
	int keyboard_fd;
	int saved_kbd_mode;
	struct termios saved_kbd_termios;

	int mouse_fd;
#if SDL_INPUT_TSLIB
	struct tsdev *ts_dev;
#endif

	SDL_Rect *SDL_modelist[5][3];

	SDL_mutex *hw_lock;

	int mem_fd;
	unsigned int *bkregs32[2];

	unsigned char *fb1[4];
	unsigned int fb1_length;
	unsigned int current_fb;

	unsigned char* hw_surface;
	unsigned char* hw_surface_start;
	vidmem_bucket surfaces;
	int hwsurface_memleft;

	unsigned int inited;
	unsigned int upper_surface;
};
/* Old variable names */
#define current_vt		(this->hidden->current_vt)
#define saved_vt		(this->hidden->saved_vt)
#define keyboard_fd		(this->hidden->keyboard_fd)
#define saved_kbd_mode		(this->hidden->saved_kbd_mode)
#define saved_kbd_termios	(this->hidden->saved_kbd_termios)
#define mouse_fd		(this->hidden->mouse_fd)
#if SDL_INPUT_TSLIB
#define ts_dev			(this->hidden->ts_dev)
#endif
#define SDL_modelist		(this->hidden->SDL_modelist)
#define hw_lock			(this->hidden->hw_lock)
#define mem_fd			(this->hidden->mem_fd)
#define bkregs32		(this->hidden->bkregs32)
#define fb1			(this->hidden->fb1)
#define fb1_length		(this->hidden->fb1_length)
#define current_fb		(this->hidden->current_fb)
#define hw_surface		(this->hidden->hw_surface)
#define hw_surface_start	(this->hidden->hw_surface_start)
#define surfaces		(this->hidden->surfaces)
#define hwsurface_memleft	(this->hidden->hwsurface_memleft)
#define inited			(this->hidden->inited)
#define upper_surface		(this->hidden->upper_surface)

#endif /* _SDL_fbvideo_h */
