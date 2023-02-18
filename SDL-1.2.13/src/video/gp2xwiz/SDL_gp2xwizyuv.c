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
#include "SDL_gp2xwizyuv.h"
#include "../SDL_yuvfuncs.h"

#include "polluxregs.h"


#define YUVMEM_WIDTH 4096
#define YUVMEM_HEIGHT 4096

#define YUVMEM_BLOCK_COUNT 4
#define YUVMEM_BLOCK_WIDTH 2048
#define YUVMEM_BLOCK_HEIGHT 2048

#define YUVMEM_PADDR_START 0x3000000

/* The functions used to manipulate software video overlays */
static struct private_yuvhwfuncs gp2xwiz_yuvfuncs = {
	GP2XWIZ_LockYUVOverlay,
	GP2XWIZ_UnlockYUVOverlay,
	GP2XWIZ_DisplayYUVOverlay,
	GP2XWIZ_FreeYUVOverlay
};

struct private_yuvhwdata {
	unsigned int LuPaddr;
	unsigned int CbPaddr;
	unsigned int CrPaddr;

	unsigned char *LuVaddr;
	unsigned char *CbVaddr;
	unsigned char *CrVaddr;

	unsigned int block_offset;

	/* These are just so we don't have to allocate them separately */
	Uint16 pitches[3];
	Uint8 *planes[3];
};

static unsigned int block_used[YUVMEM_BLOCK_COUNT]={0,};


SDL_Overlay *GP2XWIZ_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display)
{
	SDL_Overlay *overlay;
	struct private_yuvhwdata *hwdata;
	unsigned int LuOffset,CbOffset,CrOffset;
	int i=0,mod;

	/* YUV 4:2:0 Only */
	if((format!=SDL_YV12_OVERLAY)&&(format!=SDL_IYUV_OVERLAY))
		return(NULL);

	if((display->flags&WIZ_UPPERSURFACE)==WIZ_UPPERSURFACE)
		return(NULL);

	/* Create the overlay structure */
	overlay = (SDL_Overlay *)SDL_malloc(sizeof *overlay);
	if ( overlay == NULL ) {
		SDL_OutOfMemory();
		return(NULL);
	}
	SDL_memset(overlay, 0, (sizeof *overlay));

	/* Fill in the basic members */
	overlay->format = format;
	overlay->w = width;
	overlay->h = height;

	/* Set up the YUV surface function structure */
	overlay->hwfuncs = &gp2xwiz_yuvfuncs;

	/* Create the pixel data and lookup tables */
	hwdata = (struct private_yuvhwdata *)SDL_malloc(sizeof *hwdata);
	overlay->hwdata = hwdata;
	if ( hwdata == NULL ) {
		SDL_OutOfMemory();
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}

	if(!hw_surface_start)
	{
		SDL_SetError("Couldn't access GP2X Wiz YUV 4:2:0 memory.");
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}

	for(i=0;i<YUVMEM_BLOCK_COUNT;i++)
	{
		if(block_used[i]==0) break;
	}

	if(i==YUVMEM_BLOCK_COUNT)
	{
		SDL_SetError("Not Enough GP2X Wiz YUV 4:2:0 memory.");
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}

	if(!this->rotate_screen)
	{
		LuOffset=((YUVMEM_BLOCK_HEIGHT*(i>>1))*YUVMEM_WIDTH)+(YUVMEM_BLOCK_WIDTH*(i&1));
		CbOffset=LuOffset+(height*YUVMEM_WIDTH);
		mod=(((width+1)&~1)>>1);
	}
	else
	{
		LuOffset=((YUVMEM_BLOCK_WIDTH*(i>>1))*YUVMEM_WIDTH)+(YUVMEM_BLOCK_HEIGHT*(i&1));
		CbOffset=(LuOffset+height+7) & ~7;
		mod=(((width+1)&~1)>>1)*YUVMEM_WIDTH;
	}

	CrOffset=CbOffset+mod;
	CrOffset=(CrOffset+7) & ~7; /* 8byte align */

	hwdata->LuPaddr=LuOffset+YUVMEM_PADDR_START;
	hwdata->CbPaddr=CbOffset+YUVMEM_PADDR_START;
	hwdata->CrPaddr=CrOffset+YUVMEM_PADDR_START;
	hwdata->LuVaddr=hw_surface_start+LuOffset;
	hwdata->CbVaddr=hw_surface_start+CbOffset;
	hwdata->CrVaddr=hw_surface_start+CrOffset;

	block_used[i]=1;
	hwdata->block_offset=i;
	overlay->hw_overlay = 1;

	/* Set up the plane pointers */
	hwdata->pitches[0]=hwdata->pitches[1]=hwdata->pitches[2]=YUVMEM_WIDTH;
	hwdata->planes[0]=hwdata->LuVaddr;
	if(format==SDL_YV12_OVERLAY)
	{
		hwdata->planes[1]=hwdata->CrVaddr;
		hwdata->planes[2]=hwdata->CbVaddr;
	}
	else
	{
		hwdata->planes[1]=hwdata->CbVaddr;
		hwdata->planes[2]=hwdata->CrVaddr;
	}

	overlay->pitches = hwdata->pitches;
	overlay->pixels = hwdata->planes;
	overlay->planes = 3;

	/* We're all done.. */
	return(overlay);
}

int GP2XWIZ_LockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	return 0;
}

void GP2XWIZ_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	return;
}

int GP2XWIZ_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *src, SDL_Rect *dst)
{
	FB_VMEMINFO vmem;
	FB_VIDEO_CONF vconf;
	struct private_yuvhwdata *hwdata;
	int x=src->x,y=src->y;
	int mod;

	hwdata=overlay->hwdata;
	vmem.LuStride=vmem.CbStride=vmem.CrStride=YUVMEM_WIDTH;

	if(!this->rotate_screen)
	{
		mod=x&15;
		if(mod)
		{
			if(mod<8) x-=mod;
			else x+=(16-mod);
		}

		vmem.LuOffset=hwdata->LuPaddr+y*YUVMEM_WIDTH+x;
		x>>=1; y>>=1;
		vmem.CbOffset=hwdata->CbPaddr+y*YUVMEM_WIDTH+x;
		vmem.CrOffset=hwdata->CrPaddr+y*YUVMEM_WIDTH+x;

		vconf.SrcWidth=src->w;
		vconf.SrcHeight=src->h;
		vconf.DstWidth=dst->w;
		vconf.DstHeight=dst->h;
		vconf.Left=dst->x;
		vconf.Right=dst->x+dst->w;
		vconf.Top=dst->y;
		vconf.Bottom=dst->y+dst->h;
	}
	else
	{
		mod=y&15;
		if(mod)
		{
			if(mod<8) y-=mod;
			else y+=(16-mod);
		}

		unsigned int w=overlay->w,ww=src->w;
		vmem.LuOffset=hwdata->LuPaddr+(w-x-ww)*YUVMEM_WIDTH+y;
		x>>=1; y>>=1; w=(((w+1)&~1)>>1); ww=(((ww+1)&~1)>>1);
		vmem.CbOffset=hwdata->CbPaddr+(w-x-ww)*YUVMEM_WIDTH+y;
		vmem.CrOffset=hwdata->CrPaddr+(w-x-ww)*YUVMEM_WIDTH+y;

		vconf.SrcWidth=src->h;
		vconf.SrcHeight=src->w;
		vconf.DstWidth=dst->h;
		vconf.DstHeight=dst->w;
		vconf.Left=dst->y;
		vconf.Right=dst->y+dst->h;
		vconf.Bottom=320-dst->x;
		vconf.Top=vconf.Bottom-dst->w;
	}

	vconf.VideoDev=PRI_MLC;
	SDL_FBVideoUpdate(&vconf);

	if(this->tvout_mode & MLC_PRI_ENABLED)
	{
		vmem.VideoDev=PRI_MLC;

		if(!(MLCCONTROL2 & 0x20))
			SDL_FBVideoStart(&vmem);
		else
			SDL_FBVideoMemoryUpdate(&vmem);
	}
	if(this->tvout_mode & MLC_SEC_ENABLED)
	{
		vmem.VideoDev=SEC_MLC;

		if(!(MLCCONTROL2_TV & 0x20))
			SDL_FBVideoStart(&vmem);
		else
			SDL_FBVideoMemoryUpdate(&vmem);
	}

	return(0);
}

void GP2XWIZ_FreeYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata;

	hwdata = overlay->hwdata;
	if ( hwdata ) {
		block_used[hwdata->block_offset]=0;
		SDL_free(hwdata);
	}
}

