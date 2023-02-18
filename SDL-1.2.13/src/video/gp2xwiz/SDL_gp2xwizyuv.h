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

/* SDL GP2X Wiz YUV 4:2:0 Layer support(Experimental)
   2010.5.27 ikari */

#include "SDL_video.h"
#include "SDL_gp2xwizvideo.h"

extern SDL_Overlay *GP2XWIZ_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display);

extern int GP2XWIZ_LockYUVOverlay(_THIS, SDL_Overlay *overlay);

extern void GP2XWIZ_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay);

extern int GP2XWIZ_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *src, SDL_Rect *dst);

extern void GP2XWIZ_FreeYUVOverlay(_THIS, SDL_Overlay *overlay);

