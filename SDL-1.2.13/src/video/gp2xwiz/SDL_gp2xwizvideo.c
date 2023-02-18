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

/* SDL GP2X Wiz video driver implementation.
   Copied from SDL-1.2.13/src/video/fbcon/SDL_fbvideo.c and modified it.
   2010.1.23 ikari
*/

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "SDL_gp2xwizvideo.h"
#include "SDL_gp2xwizmouse_c.h"
#include "SDL_gp2xwizevents_c.h"
#include "SDL_gp2xwizyuv.h"
#if SDL_VIDEO_OPENGL
#include "SDL_gp2xwizgles_c.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "warm.h"
#include "pollux_set.h"
#include "polluxregs.h"
#include "wiz_rotateblit.h"

#define Wiz_FlipFB(back) (MLCADDRESS1=this->fb1_realaddr[back])
#define Wiz_FlipFB_TV(back) (MLCADDRESS1_TV=this->fb1_realaddr[back])

#define WIZ_FB_LENGTH 0x400000
#define HWSURFACE_START 0x3000000
#define HWSURFACE_LENGTH 0x1000000

#define WIZ_ROTATESCREEN_ON 5 /* 320x240 */
#define WIZ_ROTATESCREEN_OFF 6 /* 240x320 */


// ghcstop,  SDL_sysideo.h:  #define _THIS        SDL_VideoDevice *_this

/* ghcstop_041124 add
 *
 * keyboard, mouse input을 없애는 define, 1이면 enable, 0이면 disable
 *
 * mplayer의 20041123이전의 버전에서는 이부분을 0으로 하고 사용할 것
 * 그 이후버전은 이부분을 1로 하고 SDL_fbevents.c의 #define FB_VT_OPEN 1 // ghcstop add를
 * - testbitmap.c 잘된다...--;, 20041124) 
 */
#define FB_INPUT_DEVICE_ENABLE 1 

/* HYUN_DEBUG */
//extern int SDL_videofd; // Dark add 20050510

/* Initialization/Query functions */
static int GP2XWIZ_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **GP2XWIZ_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *GP2XWIZ_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int GP2XWIZ_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void _GP2XWIZ_VideoQuit(_THIS);
static void GP2XWIZ_VideoQuit(_THIS);

/* Hardware surface functions */
static int GP2XWIZ_InitHWSurfaces(_THIS, SDL_Surface *screen, unsigned char *base, int size);
static void GP2XWIZ_FreeHWSurfaces(_THIS);
static int GP2XWIZ_AllocHWSurface(_THIS, SDL_Surface *surface);
static int GP2XWIZ_LockHWSurface(_THIS, SDL_Surface *surface);
static void GP2XWIZ_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void GP2XWIZ_FreeHWSurface(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipSWSurface(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipSWSurface_TV(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipSWSurface_Both(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipHWSurface(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipHWSurface_TV(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipHWSurface_Both(_THIS, SDL_Surface *surface);
static int GP2XWIZ_FlipSWSurface_Rotation(_THIS, SDL_Surface *surface);


static int GP2XWIZ_Available(void)
{	
	return 1;
}

static void GP2XWIZ_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
#if SDL_VIDEO_OPENGL
	SDL_free(device->gl_data);
#endif
	SDL_free(device);
}

static SDL_VideoDevice *GP2XWIZ_CreateDevice(int devindex)
{
	SDL_VideoDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *this->hidden));
#if SDL_VIDEO_OPENGL
		this->gl_data = (struct SDL_PrivateGLData *)
				SDL_malloc((sizeof *this->gl_data));
#endif
	}
	if ( (this == NULL) || (this->hidden == NULL) 
#if SDL_VIDEO_OPENGL
		|| (this->gl_data==NULL)
#endif
	) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));
#if SDL_VIDEO_OPENGL
	SDL_memset(this->gl_data, 0, (sizeof *this->gl_data));
#endif
	mem_fd = -1;
	mouse_fd = -1;
	keyboard_fd = -1;
	bkregs32[0]=NULL;
	bkregs32[1]=NULL;

	/* Set the function pointers */
	this->VideoInit = GP2XWIZ_VideoInit;
	this->ListModes = GP2XWIZ_ListModes;
	this->SetVideoMode = GP2XWIZ_SetVideoMode;
	this->SetColors = GP2XWIZ_SetColors;
	this->UpdateRects = NULL;
	this->CreateYUVOverlay = GP2XWIZ_CreateYUVOverlay;
	this->VideoQuit = GP2XWIZ_VideoQuit;
	this->AllocHWSurface = GP2XWIZ_AllocHWSurface;
	this->CheckHWBlit = NULL;
	this->FillHWRect = NULL;
	this->SetHWColorKey = NULL;
	this->SetHWAlpha = NULL;
	this->LockHWSurface = GP2XWIZ_LockHWSurface;
	this->UnlockHWSurface = GP2XWIZ_UnlockHWSurface;
	this->FlipHWSurface = GP2XWIZ_FlipHWSurface;
	this->FreeHWSurface = GP2XWIZ_FreeHWSurface;
	this->SetCaption = NULL;
	this->SetIcon = NULL;
	this->IconifyWindow = NULL;
	this->GrabInput = NULL;
	this->GetWMInfo = NULL;
	this->InitOSKeymap = GP2XWIZ_InitOSKeymap;
	this->PumpEvents = GP2XWIZ_PumpEvents;
#if SDL_VIDEO_OPENGL
	this->GL_LoadLibrary = GP2XWIZ_GL_LoadLibrary;
	this->GL_GetProcAddress = GP2XWIZ_GL_GetProcAddress;
	this->GL_GetAttribute = GP2XWIZ_GL_GetAttribute;
	this->GL_MakeCurrent = GP2XWIZ_GL_MakeCurrent;
	this->GL_SwapBuffers = GP2XWIZ_GL_SwapBuffers;
#endif

	this->free = GP2XWIZ_DeleteDevice;

	return this;
}

VideoBootStrap GP2XWIZ_bootstrap = {
	"gp2xwiz", "GP2X Wiz SDL video driver",
	GP2XWIZ_Available, GP2XWIZ_CreateDevice
};

static int GP2XWIZ_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	int i;
	struct fb_fix_screeninfo info;

	/* Open screen framebuffer */
	fb_fd=open("/dev/fb0",O_RDWR);
	if(fb_fd<0) 
	{
		SDL_SetError("Couldn't open /dev/fb0");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}

	if(ioctl(fb_fd,FBIOGET_FSCREENINFO,&info)<0) 
	{
		SDL_SetError("Couldn't read framebuffer information");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}

	/* Open /dev/mem */
	mem_fd=open("/dev/mem",O_RDWR);
	if(mem_fd<0)
	{
		SDL_SetError("Couldn't open GP2X mem device");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}

#if !SDL_THREADS_DISABLED
	/* Create the hardware surface lock mutex */
	hw_lock = SDL_CreateMutex();
	if ( hw_lock == NULL ) {
		SDL_SetError("Unable to create lock mutex");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}
#endif

	/* mmap GP2X Wiz hardware register */
	memregs32=(volatile unsigned int*)mmap(0,0x20000,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,0xC0000000);
	if(memregs32==(volatile unsigned int*)0xFFFFFFFF)
	{
		SDL_SetError("Couldn't mmap GP2X Wiz hardware register");
		close(mem_fd);
		mem_fd=-1;
		return(-1);
	}
	memregs16=(volatile unsigned short*)memregs32;

	fb1[0]=(unsigned char*)mmap(0,WIZ_FB_LENGTH,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,info.smem_start);
	if(fb1[0]==(unsigned char*)0xFFFFFFFF)
	{
		SDL_SetError("Couldn't mmap framebuffer area.");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}
	this->fb1_realaddr[0]=info.smem_start;

	hw_surface_start=(unsigned char*)mmap(0,HWSURFACE_LENGTH,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,HWSURFACE_START);
	if(hw_surface_start==(unsigned char*)0xFFFFFFFF)
	{
		SDL_SetError("Couldn't mmap 16MiB 3D/YUV area.");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}
	hw_surface=hw_surface_start;

	/* Determine TV-out mode */
	this->tvout_mode=0;
	if(DPCCTRL0 & 0x8000) 
		this->tvout_mode |= MLC_PRI_ENABLED;
	if(DPCCTRL0_TV & 0x8000) 
		this->tvout_mode |= MLC_SEC_ENABLED;

	/* Backup previous register values */
	bkregs32[0]=(unsigned int*)malloc(sizeof(unsigned int)*22);
	if(!bkregs32[0])
	{
		SDL_SetError("Not enough memory.");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}

	bkregs32[0][0]=MLCADDRESS1;
	bkregs32[0][1]=MLCCONTROL1;
	bkregs32[0][2]=MLCLEFTRIGHT1;
	bkregs32[0][3]=MLCTOPBOTTOM1;
	bkregs32[0][4]=MLCLEFTRIGHT1_0;
	bkregs32[0][5]=MLCTOPBOTTOM1_0;
	bkregs32[0][6]=MLCLEFTRIGHT1_1;
	bkregs32[0][7]=MLCTOPBOTTOM1_1;
	bkregs32[0][8]=MLCBGCOLOR;
	bkregs32[0][9]=MLCHSTRIDE1;
	bkregs32[0][10]=MLCVSTRIDE1;
	bkregs32[0][11]=MLCADDRESS2;
	bkregs32[0][12]=MLCADDRESSCB;
	bkregs32[0][13]=MLCADDRESSCR;
	bkregs32[0][14]=MLCCONTROL2;
	bkregs32[0][15]=MLCLEFTRIGHT2;
	bkregs32[0][16]=MLCTOPBOTTOM2;
	bkregs32[0][17]=MLCVSTRIDE2;
	bkregs32[0][18]=MLCVSTRIDECB;
	bkregs32[0][19]=MLCVSTRIDECR;
	bkregs32[0][20]=MLCHSCALE;
	bkregs32[0][21]=MLCVSCALE;

	bkregs32[1]=(unsigned int*)malloc(sizeof(unsigned int)*22);
	if(!bkregs32[1])
	{
		SDL_SetError("Not enough memory.");
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}

	bkregs32[1][0]=MLCADDRESS1_TV;
	bkregs32[1][1]=MLCCONTROL1_TV;
	bkregs32[1][2]=MLCLEFTRIGHT1_TV;
	bkregs32[1][3]=MLCTOPBOTTOM1_TV;
	bkregs32[1][4]=MLCLEFTRIGHT1_0_TV;
	bkregs32[1][5]=MLCTOPBOTTOM1_0_TV;
	bkregs32[1][6]=MLCLEFTRIGHT1_1_TV;
	bkregs32[1][7]=MLCTOPBOTTOM1_1_TV;
	bkregs32[1][8]=MLCBGCOLOR_TV;
	bkregs32[1][9]=MLCHSTRIDE1_TV;
	bkregs32[1][10]=MLCVSTRIDE1_TV;
	bkregs32[1][11]=MLCADDRESS2_TV;
	bkregs32[1][12]=MLCADDRESSCB_TV;
	bkregs32[1][13]=MLCADDRESSCR_TV;
	bkregs32[1][14]=MLCCONTROL2_TV;
	bkregs32[1][15]=MLCLEFTRIGHT2_TV;
	bkregs32[1][16]=MLCTOPBOTTOM2_TV;
	bkregs32[1][17]=MLCVSTRIDE2_TV;
	bkregs32[1][18]=MLCVSTRIDECB_TV;
	bkregs32[1][19]=MLCVSTRIDECR_TV;
	bkregs32[1][20]=MLCHSCALE_TV;
	bkregs32[1][21]=MLCVSCALE_TV;

	/* Fill initial values */
	fb1[1]=NULL;
	fb1[2]=NULL;
	fb1[3]=NULL;

	vformat->BitsPerPixel=16;
	vformat->BytesPerPixel=2;
	vformat->Rmask=0xf800;
	vformat->Gmask=0x07e0;
	vformat->Bmask=0x001f;
	vformat->Amask=0;

	/* Fill in our hardware acceleration capabilities */
	this->info.current_w = 240;
	this->info.current_h = 320;
	this->info.wm_available = 0;
	this->info.hw_available = 0;

	for(i=0;i<5;i++)
	{
		SDL_modelist[i][0]=SDL_malloc(sizeof(SDL_Rect));
		SDL_modelist[i][0]->x=0; SDL_modelist[i][0]->y=0;
		SDL_modelist[i][0]->w=240,SDL_modelist[i][0]->h=320;

		SDL_modelist[i][1]=SDL_malloc(sizeof(SDL_Rect));
		SDL_modelist[i][1]->x=0; SDL_modelist[i][1]->y=0;
		SDL_modelist[i][1]->w=320,SDL_modelist[i][1]->h=240;

		SDL_modelist[i][2]=NULL;
	}

	/* Enable mouse and keyboard support */
	if ( GP2XWIZ_OpenKeyboard(this) < 0 ) {
		GP2XWIZ_VideoQuit(this);
		return(-1);
	}
	#if 0 // ghcstop delete, no keyboard, no mouse in s3c2410
	// ghcstop_041123: SDL_fbevents.c, 아직은 마우스 살리지 말고나중에 살려야겠군...
	if ( FB_OpenMouse(this) < 0 ) {
		const char *sdl_nomouse;

		sdl_nomouse = getenv("SDL_NOMOUSE");
		if ( ! sdl_nomouse ) {
			SDL_SetError("Unable to open mouse");
			FB_VideoQuit(this);
			return(-1);
		}
	}
	#else
	GP2XWIZ_OpenMouse(this);
       #endif

	/* We're done! */
	return(0);
}

static SDL_Rect **GP2XWIZ_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	if(format->BitsPerPixel==15)
		return SDL_modelist[4];
	else
		return(SDL_modelist[((format->BitsPerPixel+7)/8)-1]);
}

/* Various screen update functions available */
static void GP2XWIZ_DirectUpdate(_THIS, int numrects, SDL_Rect *rects);

static SDL_Surface *GP2XWIZ_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	Uint32 Rmask,Gmask,Bmask,Amask=0;
	Uint32 mode,bytesperpixel,hwsurface_length;
	char* env=NULL;

#if FB_INPUT_DEVICE_ENABLE // ghcstop_041123 alive
	/* Set the terminal into graphics mode */
	if ( GP2XWIZ_EnterGraphicsMode(this) < 0 ) {
		return(NULL);
	}
#endif

	#if 0 // ghcstop_041123 delete
	/* Restore the original palette */
	FB_RestorePalette(this);
	#endif

#if SDL_VIDEO_OPENGL
	/* If it's OpenGL ES mode, set OpenGL ES and return */
	if((flags & SDL_OPENGL)==SDL_OPENGL)
	{
		/* Clear screen */
		if(fb1[0]) 
		{
			memset((void*)fb1[0],0x00,0x25800);
			Wiz_FlipFB(0);
			Wiz_DirtyLayer1();
			munmap((void*)fb1[0],WIZ_FB_LENGTH);
			fb1[0]=NULL;
		}

		if(hw_surface_start)
		{
			munmap((void*)hw_surface_start,HWSURFACE_LENGTH);
			hw_surface_start=NULL;
		}
		hw_surface=NULL;

		/* Set screenmode to landscape -> portrait */
		if(SDL_SetLcdChange(((flags & WIZ_TEARING)==WIZ_TEARING)?
					WIZ_ROTATESCREEN_ON:WIZ_ROTATESCREEN_OFF,0)<0)
		{
			SDL_SetError("Couldn't set screen mode");
			GP2XWIZ_VideoQuit(this);
			return(NULL);
		}

		/* Init OpenGL ES lite */
		if(GP2XWIZ_GL_CreateContext(this)<0)
		{
			GP2XWIZ_VideoQuit(this);
			return(NULL);
		}

		/* Alloc dummy surface for fooling the app */
		Rmask=((2<<(this->gl_config.red_size-1))-1)<<(this->gl_config.blue_size+this->gl_config.green_size);
		Gmask=((2<<(this->gl_config.green_size-1))-1)<<this->gl_config.blue_size;
		Bmask=(2<<(this->gl_config.blue_size-1))-1;
		Amask=(this->gl_config.alpha_size>0)?(2<<(((this->gl_config.alpha_size-1))-1)<<
			(this->gl_config.red_size+this->gl_config.blue_size+this->gl_config.green_size)):0;
		if ( ! SDL_ReallocFormat(current, bpp,
	                                  Rmask, Gmask, Bmask, Amask) ) {
			SDL_SetError("Couldn't alloc Video surface");
			GP2XWIZ_VideoQuit(this);
			return(NULL);
		}

		current->w = width;
		current->h = height;
		current->flags = (SDL_FULLSCREEN|SDL_OPENGL) | (flags & SDL_OPENGLBLIT)
				| (flags & WIZ_240X320) | (flags & WIZ_TEARING) | (WIZ_MMUHACKOFF|WIZ_RAMTWEAKOFF);
		current->pitch = SDL_CalculatePitch(current);

		return(current);
	}
#endif

#ifdef GP2XWIZ_MPLAYER
	env=SDL_getenv("WIZ_ARGB");
#endif
	switch(bpp)
	{
		case 8: /* 256 color */
			Rmask=255;
			Gmask=255;
			Bmask=255;
			mode=0x443AD020;
			fb1_length=0x12C00; /* 320*240 */
			break;
		case 15: /* RGB555 - Unusual */
			Rmask=0x7c00;
			Gmask=0x03e0;
			Bmask=0x001f;
			mode=0x4342D020;
			fb1_length=0x25800; /* 320*240*2 */
			break;
		case 16: /* RGB565 or ARGB4444 */
#ifdef GP2XWIZ_MPLAYER
			if(env && (strcmp(env,"1")==0))
			{
				Rmask=0x0F00;
				Gmask=0x00F0;
				Bmask=0x000F;
				Amask=0xF000;
				mode=0x2211D024;
			}
			else
#endif
			{
				Rmask=0xf800;
				Gmask=0x07e0;
				Bmask=0x001f;
				mode=0x4432D020;
			}

			fb1_length=0x25800; /* 320*240*2 */
			break;
		case 24: /* RGB888 - It's too slow */
			Rmask=0xff0000;
			Gmask=0x00ff00;
			Bmask=0x0000ff;
			mode=0x4653D020;
			fb1_length=0x38400; /* 320*240*3 */
			break;
		case 32: /* XRGB8888 or ARGB8888 */
#ifdef GP2XWIZ_MPLAYER
			if(env && (strcmp(env,"1")==0))
			{
				Rmask=0x00ff0000;
				Gmask=0x0000ff00;
				Bmask=0x000000ff;
				Amask=0xff000000;
				mode=0x0653D024;
			}
			else
#endif
			{
				Rmask=0x00ff0000;
				Gmask=0x0000ff00;
				Bmask=0x000000ff;
				mode=0x4653D020;
			}
			fb1_length=0x4B000; /* 320*240*4 */
			break;
		default:
			SDL_SetError("Not supported bpp");
			GP2XWIZ_VideoQuit(this);
			return(NULL);
	}

	if ( ! SDL_ReallocFormat(current, bpp,
	                                  Rmask, Gmask, Bmask, Amask) ) {
		SDL_SetError("Couldn't set SDL videosurface");
		GP2XWIZ_VideoQuit(this);
		return(NULL);
	}
	bytesperpixel=current->format->BytesPerPixel;

	if(this->rotate_screen)
	{
		this->shadow_pitch=320*bytesperpixel;
		this->FlipHWSurface = GP2XWIZ_FlipSWSurface_Rotation;
	}
	else if((flags&SDL_HWSURFACE)==SDL_SWSURFACE)
		this->FlipHWSurface = GP2XWIZ_FlipSWSurface;

	env=SDL_getenv("WIZ_MMUHACKOFF");
	if(env && (strcmp(env,"1")==0))
		flags |= WIZ_MMUHACKOFF;
	env=SDL_getenv("WIZ_RAMTWEAKOFF");
	if(env && (strcmp(env,"1")==0))
		flags |= WIZ_RAMTWEAKOFF;

	if(!inited)
	{
		this->mmuhack_type=0;
		if((flags&WIZ_MMUHACKOFF)!=WIZ_MMUHACKOFF)
		{
			if(access("warm_2.6.24.ko",F_OK)==0)
				this->mmuhack_type=1;
			else if(access("mmuhack.ko",F_OK)==0)
				this->mmuhack_type=2;
			else
				flags|=WIZ_MMUHACKOFF;
		}
	}

	/* No mmuhack, no Upper surface */
	if(((flags & WIZ_UPPERSURFACE)==WIZ_UPPERSURFACE)&&!this->mmuhack_type)
		flags &= ~WIZ_UPPERSURFACE;

	/* Enable mmuhack when .ko file exists on same directory */
	if(!inited)
	{
		inited = 2;
		if(this->mmuhack_type==1)
		{
			if(warm_init()==0)
			{
				warm_change_cb_upper(WCB_C_BIT|WCB_B_BIT,1);
				inited=1;
			}
			else
				this->mmuhack_type=0;
		}
		else
		{
			if(this->mmuhack_type==2)
			{
				int mmufd=open("/dev/mmuhack",O_RDWR);
				if(mmufd<0)
				{
					system("/sbin/insmod mmuhack.ko");
					mmufd=open("/dev/mmuhack",O_RDWR);
				}

				if(mmufd<0)
					this->mmuhack_type=0;
				else
				{
					close(mmufd);				
					inited=1;
				}
			}
		}

	}

	if(bpp==8)
		memset((void*)fb1[0],0x00,fb1_length*2);
	else
		memset((void*)fb1[0],0x00,fb1_length);

	/* Set up the new mode framebuffer */
	current->flags = (SDL_FULLSCREEN|SDL_HWSURFACE) | (flags & SDL_DOUBLEBUF) |
			(flags & WIZ_240X320) | (flags & WIZ_TEARING) | (flags & WIZ_MMUHACKOFF) |
			(flags & WIZ_RAMTWEAKOFF);
	current->pixels = (void*)fb1[0];
	current_fb=0;

	hw_surface=fb1[0]+fb1_length;
	hwsurface_length=WIZ_FB_LENGTH-fb1_length;
	upper_surface=0;
	/* Set up double buffer */
	if((flags & SDL_DOUBLEBUF)==SDL_DOUBLEBUF)
	{
		this->fb1_realaddr[1]=this->fb1_realaddr[0]+fb1_length;
		fb1[1]=fb1[0]+fb1_length;
		if(bpp>8) memset((void*)fb1[1],0x00,fb1_length);

		this->fb1_realaddr[2]=this->fb1_realaddr[1]+fb1_length;
		fb1[2]=fb1[1]+fb1_length;
		memset((void*)fb1[2],0x00,fb1_length);

		this->fb1_realaddr[3]=this->fb1_realaddr[2]+fb1_length;
		fb1[3]=fb1[2]+fb1_length;
		memset((void*)fb1[3],0x00,fb1_length);

		if((flags & SDL_HWSURFACE)==SDL_HWSURFACE)
			current->pixels = (void*)fb1[1];

		current->flags |= (flags & WIZ_VSYNCOFF);
		hw_surface+=(fb1_length*3);
		hwsurface_length-=(fb1_length*3);
	}

	/* Set up "Upper" surface */
	GP2XWIZ_FreeHWSurfaces(this);
	if((flags & WIZ_UPPERSURFACE)==WIZ_UPPERSURFACE)
	{
		unsigned int shadow_length=0;

		/* Alloc shadow surface on "Upper Surface" */
		current->flags |= WIZ_UPPERSURFACE;
		upper_surface=1;
		hw_surface=hw_surface_start;
		
		if(this->rotate_screen)
			shadow_length=0x12C00*bytesperpixel;
		else if((flags&SDL_HWSURFACE)==SDL_SWSURFACE)
		{
			this->shadow_pitch=((width*bytesperpixel)+3)&~3; /* 4byte align */
			shadow_length=this->shadow_pitch*height; 
		}

		if(shadow_length>0)
		{
			this->shadow_pixels=(void*)hw_surface;
			hw_surface+=shadow_length;
			memset(this->shadow_pixels,0x00,shadow_length);
		}

		GP2XWIZ_InitHWSurfaces(this, current, hw_surface, HWSURFACE_LENGTH-shadow_length);
	}
	else
		GP2XWIZ_InitHWSurfaces(this, current, hw_surface, hwsurface_length);

#ifndef GP2XWIZ_MPLAYER
	/* Set screen mode */
	if(SDL_SetLcdChange(((flags & WIZ_TEARING)==WIZ_TEARING)?
			WIZ_ROTATESCREEN_ON:WIZ_ROTATESCREEN_OFF,0)<0)
	{
		SDL_SetError("Couldn't set GP2X Wiz screen mode");
		GP2XWIZ_VideoQuit(this);
		return(NULL);
	}
#endif

	if(this->tvout_mode & MLC_PRI_ENABLED)
	{
#ifndef GP2XWIZ_MPLAYER
		if(!(this->tvout_mode & MLC_SEC_ENABLED))
		{
			/* set screen refresh rate to 120Hz */
			if(!inited) pollux_set(memregs16, "lcd_timings=397,1,37,277,341,0,17,337;dpc_clkdiv0=9");
		}
#endif

		MLCLEFTRIGHT1_0=0;
		MLCTOPBOTTOM1_0=0;
		MLCLEFTRIGHT1_1=0;
		MLCTOPBOTTOM1_1=0;
		Wiz_DirtyLayer1();

		/* Framebuffer layer settings */
		MLCBGCOLOR=0x000000;
		Wiz_DirtyMLC();
		if((flags & WIZ_TEARING)!=WIZ_TEARING)
		{
			current->w = 240;
			current->h = 320;
			MLCLEFTRIGHT1=239;
			MLCTOPBOTTOM1=319;
		}
		else
		{
			current->w = 320;
			current->h = 240;
			MLCLEFTRIGHT1=319;
			MLCTOPBOTTOM1=239;
		}

		Wiz_DirtyLayer1();
		current->pitch = SDL_CalculatePitch(current);

		MLCCONTROL1=mode;
		Wiz_DirtyLayer1();

		if(bpp==8)
		{
			int i;
			current->flags|=SDL_HWPALETTE;
			for(i=0;i<256;i++)
				MLCPALETTE1=i<<24;
		}
		Wiz_DirtyLayer1();

		MLCHSTRIDE1=bytesperpixel;
		MLCVSTRIDE1=current->pitch;
		Wiz_DirtyLayer1();

		Wiz_FlipFB(0);
		Wiz_DirtyLayer1();
	}
	if(this->tvout_mode & MLC_SEC_ENABLED)
	{
		MLCLEFTRIGHT1_0_TV=0;
		MLCTOPBOTTOM1_0_TV=0;
		MLCLEFTRIGHT1_1_TV=0;
		MLCTOPBOTTOM1_1_TV=0;
		Wiz_DirtyLayer1_TV();

		/* Framebuffer layer settings */
		MLCBGCOLOR_TV=0x000000;
		Wiz_DirtyMLC_TV();

		current->w = 320;
		current->h = 240;
		MLCLEFTRIGHT1_TV=319;
		MLCTOPBOTTOM1_TV=239;

		Wiz_DirtyLayer1_TV();
		current->pitch = SDL_CalculatePitch(current);

		MLCCONTROL1_TV=mode;
		Wiz_DirtyLayer1_TV();

		if(bpp==8)
		{
			int i;
			current->flags|=SDL_HWPALETTE;
			for(i=0;i<256;i++)
				MLCPALETTE1_TV=i<<24;
		}
		Wiz_DirtyLayer1_TV();

		MLCHSTRIDE1_TV=bytesperpixel;
		MLCVSTRIDE1_TV=current->pitch;
		Wiz_DirtyLayer1_TV();

		Wiz_FlipFB_TV(0);
		Wiz_DirtyLayer1_TV();

		if((flags & SDL_HWSURFACE)==SDL_HWSURFACE)
		{
			if(this->tvout_mode & MLC_PRI_ENABLED)
				this->FlipHWSurface = GP2XWIZ_FlipHWSurface_Both;
			else
				this->FlipHWSurface = GP2XWIZ_FlipHWSurface_TV;
		}
		else
		{
			if(this->tvout_mode & MLC_PRI_ENABLED)
				this->FlipHWSurface = GP2XWIZ_FlipSWSurface_Both;
			else
				this->FlipHWSurface = GP2XWIZ_FlipSWSurface_TV;
		}
	}

	if((flags & WIZ_RAMTWEAKOFF)!=WIZ_RAMTWEAKOFF)
		pollux_set(memregs16, "ram_timings=2,9,4,1,1,1,1");

	usleep(100000);

	/* Set the update rectangle function */
	this->UpdateRects = GP2XWIZ_DirectUpdate;

	/* We're done */
	return(current);
}

static int GP2XWIZ_InitHWSurfaces(_THIS, SDL_Surface *screen, unsigned char *base, int size)
{
	vidmem_bucket *bucket;

	hwsurface_memleft = size;
	if ( hwsurface_memleft > 0 ) {
		bucket = (vidmem_bucket *)SDL_malloc(sizeof(*bucket));
		if ( bucket == NULL ) {
			SDL_OutOfMemory();
			return(-1);
		}
		bucket->prev = &surfaces;
		bucket->used = 0;
		bucket->base = base;
		bucket->size = size;
		bucket->next = NULL;
	} else {
		bucket = NULL;
	}

	surfaces.prev = NULL;
	surfaces.used = 1;
	surfaces.next = bucket;

	if(upper_surface)
	{
		surfaces.base = NULL;
		surfaces.size = 0;
	}
	else
	{
		surfaces.base = screen->pixels;
		surfaces.size = (unsigned int)((long)base - (long)surfaces.base);
		screen->hwdata = (struct private_hwdata *)&surfaces;
	}

	return(0);
}

static void GP2XWIZ_FreeHWSurfaces(_THIS)
{
	vidmem_bucket *bucket, *freeable;

	bucket = surfaces.next;
	while ( bucket ) {
		freeable = bucket;
		bucket = bucket->next;
		SDL_free(freeable);
	}
	surfaces.next = NULL;
}

static int GP2XWIZ_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	vidmem_bucket *bucket;
	int size;
	int extra;

	size = surface->h * surface->pitch;
#ifdef GP2XWIZ_DEBUG
	fprintf(stderr, "Allocating bucket of %d bytes\n", size);
#endif

	/* Quick check for available mem */
	if ( size > hwsurface_memleft ) {
		SDL_SetError("Not enough video memory");
		return(-1);
	}

	/* Search for an empty bucket big enough */
	for ( bucket=&surfaces; bucket; bucket=bucket->next ) {
		if ( ! bucket->used && (size <= bucket->size) ) {
			break;
		}
	}
	if ( bucket == NULL ) {
		SDL_SetError("Video memory too fragmented");
		return(-1);
	}

	/* Create a new bucket for left-over memory */
	extra = (bucket->size - size);
	if ( extra ) {
		vidmem_bucket *newbucket;

#ifdef GP2XWIZ_DEBUG
	fprintf(stderr, "Adding new free bucket of %d bytes\n", extra);
#endif
		newbucket = (vidmem_bucket *)SDL_malloc(sizeof(*newbucket));
		if ( newbucket == NULL ) {
			SDL_OutOfMemory();
			return(-1);
		}
		newbucket->prev = bucket;
		newbucket->used = 0;
		newbucket->base = bucket->base+size;
		newbucket->size = extra;
		newbucket->next = bucket->next;
		if ( bucket->next ) {
			bucket->next->prev = newbucket;
		}
		bucket->next = newbucket;
	}

	/* Set the current bucket values and return it! */
	bucket->used = 1;
	bucket->size = size;
#ifdef GP2XWIZ_DEBUG
	fprintf(stderr, "Allocated %d bytes at %p\n", bucket->size, bucket->base);
#endif
	hwsurface_memleft -= size;

	if(upper_surface)
		surface->flags|=WIZ_UPPERSURFACE;
	else
		surface->flags|=SDL_HWSURFACE;

	surface->pixels = (void*)bucket->base;
	surface->hwdata = (struct private_hwdata *)bucket;
	memset(surface->pixels,0,size);
	return(0);
}

static void GP2XWIZ_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	vidmem_bucket *bucket, *freeable;

	/* Look for the bucket in the current list */
	for ( bucket=&surfaces; bucket; bucket=bucket->next ) {
		if ( bucket == (vidmem_bucket *)surface->hwdata ) {
			break;
		}
	}
	if ( bucket && bucket->used ) {
		/* Add the memory back to the total */
#ifdef GP2XWIZ_DEBUG
	fprintf(stderr,"Freeing bucket of %d bytes\n", bucket->size);
#endif
		hwsurface_memleft += bucket->size;

		/* Can we merge the space with surrounding buckets? */
		bucket->used = 0;
		if ( bucket->next && ! bucket->next->used ) {
#ifdef GP2XWIZ_DEBUG
	fprintf(stderr,"Merging with next bucket, for %d total bytes\n", bucket->size+bucket->next->size);
#endif
			freeable = bucket->next;
			bucket->size += bucket->next->size;
			bucket->next = bucket->next->next;
			if ( bucket->next ) {
				bucket->next->prev = bucket;
			}
			SDL_free(freeable);
		}
		if ( bucket->prev && ! bucket->prev->used ) {
#ifdef GP2XWIZ_DEBUG
	fprintf(stderr,"Merging with previous bucket, for %d total bytes\n", bucket->prev->size+bucket->size);
#endif
			freeable = bucket;
			bucket->prev->size += bucket->size;
			bucket->prev->next = bucket->next;
			if ( bucket->next ) {
				bucket->next->prev = bucket->prev;
			}
			SDL_free(freeable);
		}
	}
	surface->pixels = NULL;
	surface->hwdata = NULL;
}

static int GP2XWIZ_LockHWSurface(_THIS, SDL_Surface *surface)
{
	if ( surface == this->screen ) {
		SDL_mutexP(hw_lock);
	}
	return 0;
}
static void GP2XWIZ_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	if ( surface == this->screen ) {
		SDL_mutexV(hw_lock);
	}
}

/* 240x320 mode */
static int GP2XWIZ_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0 & 0x400));
				
		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();

		DPCCTRL0 |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();
	}

	current_fb++;
	if(current_fb>3) current_fb=0;
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static int GP2XWIZ_FlipHWSurface_TV(_THIS, SDL_Surface *surface)
{
	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0_TV & 0x400));
				
		/* Page flip */
		Wiz_FlipFB_TV(current_fb);
		Wiz_DirtyLayer1_TV();

		DPCCTRL0_TV |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB_TV(current_fb);
		Wiz_DirtyLayer1_TV();
	}

	current_fb++;
	if(current_fb>3) current_fb=0;
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static int GP2XWIZ_FlipHWSurface_Both(_THIS, SDL_Surface *surface)
{
	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0_TV & 0x400));

		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();
		Wiz_FlipFB_TV(current_fb);
		Wiz_DirtyLayer1_TV();

		DPCCTRL0_TV |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();
		Wiz_FlipFB_TV(current_fb);
		Wiz_DirtyLayer1_TV();
	}

	current_fb++;
	if(current_fb>3) current_fb=0;
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static int GP2XWIZ_FlipSWSurface(_THIS, SDL_Surface *surface)
{
	unsigned char *srcpixel,*dstpixel;
	unsigned int h=surface->h;
	unsigned int dstskip=surface->pitch;

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
	{
		if(SDL_LockSurface(SDL_ShadowSurface)<0)
			return(-1);
	}

	/* Copy shadow surface to video surface */
	current_fb++;
	if(current_fb>3) current_fb=0;
	srcpixel=(unsigned char*)SDL_ShadowSurface->pixels;
	dstpixel=fb1[current_fb];

	if(surface->offset>0)
	{
		unsigned int w=surface->w*surface->format->BytesPerPixel;
		unsigned int srcskip=SDL_ShadowSurface->pitch;

		dstpixel+=surface->offset;
		if(!this->mmuhack_type)
		{
			while(h--)
			{
				gp2x_memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
		else
		{
			while(h--)
			{
				memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
	}
	else
	{
		if(!this->mmuhack_type)
			gp2x_memcpy(dstpixel,srcpixel,h*dstskip);
		else
			memcpy(dstpixel,srcpixel,h*dstskip);
	}

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
		SDL_UnlockSurface(SDL_ShadowSurface);

	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0 & 0x400));

		/* Page flip */
		Wiz_FlipFB(current_fb);	
		Wiz_DirtyLayer1();

		DPCCTRL0 |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB(current_fb);	
		Wiz_DirtyLayer1();
	}
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static int GP2XWIZ_FlipSWSurface_TV(_THIS, SDL_Surface *surface)
{
	unsigned char *srcpixel,*dstpixel;
	unsigned int h=surface->h;
	unsigned int dstskip=surface->pitch;

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
	{
		if(SDL_LockSurface(SDL_ShadowSurface)<0)
			return(-1);
	}

	/* Copy shadow surface to video surface */
	current_fb++;
	if(current_fb>3) current_fb=0;
	srcpixel=(unsigned char*)SDL_ShadowSurface->pixels;
	dstpixel=fb1[current_fb];

	if(surface->offset>0)
	{
		unsigned int w=surface->w*surface->format->BytesPerPixel;
		unsigned int srcskip=SDL_ShadowSurface->pitch;

		dstpixel+=surface->offset;
		if(!this->mmuhack_type)
		{
			while(h--)
			{
				gp2x_memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
		else
		{
			while(h--)
			{
				memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
	}
	else
	{
		if(!this->mmuhack_type)
			gp2x_memcpy(dstpixel,srcpixel,h*dstskip);
		else
			memcpy(dstpixel,srcpixel,h*dstskip);
	}

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
		SDL_UnlockSurface(SDL_ShadowSurface);

	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0_TV & 0x400));

		/* Page flip */
		Wiz_FlipFB_TV(current_fb);	
		Wiz_DirtyLayer1_TV();

		DPCCTRL0_TV |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB_TV(current_fb);	
		Wiz_DirtyLayer1_TV();
	}
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static int GP2XWIZ_FlipSWSurface_Both(_THIS, SDL_Surface *surface)
{
	unsigned char *srcpixel,*dstpixel;
	unsigned int h=surface->h;
	unsigned int dstskip=surface->pitch;

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
	{
		if(SDL_LockSurface(SDL_ShadowSurface)<0)
			return(-1);
	}

	/* Copy shadow surface to video surface */
	current_fb++;
	if(current_fb>3) current_fb=0;
	srcpixel=(unsigned char*)SDL_ShadowSurface->pixels;
	dstpixel=fb1[current_fb];

	if(surface->offset>0)
	{
		unsigned int w=surface->w*surface->format->BytesPerPixel;
		unsigned int srcskip=SDL_ShadowSurface->pitch;

		dstpixel+=surface->offset;
		if(!this->mmuhack_type)
		{
			while(h--)
			{
				gp2x_memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
		else
		{
			while(h--)
			{
				memcpy(dstpixel,srcpixel,w);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
	}
	else
	{
		if(!this->mmuhack_type)
			gp2x_memcpy(dstpixel,srcpixel,h*dstskip);
		else
			memcpy(dstpixel,srcpixel,h*dstskip);
	}

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
		SDL_UnlockSurface(SDL_ShadowSurface);

	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0_TV & 0x400));

		/* Page flip */
		Wiz_FlipFB(current_fb);	
		Wiz_DirtyLayer1();
		Wiz_FlipFB_TV(current_fb);	
		Wiz_DirtyLayer1_TV();

		DPCCTRL0_TV |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB(current_fb);	
		Wiz_DirtyLayer1();
		Wiz_FlipFB_TV(current_fb);	
		Wiz_DirtyLayer1_TV();
	}
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

/* 320x240 mode */
static int GP2XWIZ_FlipSWSurface_Rotation(_THIS, SDL_Surface *surface)
{
	unsigned char* dstpixel;
	unsigned int h=surface->h;
	unsigned int bpp=surface->format->BytesPerPixel;

	if(SDL_MUSTLOCK(SDL_ShadowSurface))
	{
		if(SDL_LockSurface(SDL_ShadowSurface)<0)
			return(-1);
	}

#ifdef GP2XWIZ_ROTATIONBUFFER
	if((surface->flags&WIZ_VSYNCOFF)==WIZ_VSYNCOFF)
	{
		SDL_Surface* rotation_buffer=this->rotation_buffer;
		unsigned int rotskip=rotation_buffer->pitch;

		current_fb++;
		if(current_fb>3) current_fb=0;
		if(surface->offset>0)
		{
			unsigned char* srcpixel;
			SDL_Rect rect;
			unsigned int dstskip=surface->pitch;
			unsigned int w,hh;

			rect.x=this->offset_y-this->width_odd;
			rect.y=this->offset_x;
			w=rect.w=surface->h;
			hh=rect.h=surface->w;

			srcpixel=(unsigned char*)this->shadow_pixels+rect.y*SDL_ShadowSurface->pitch+rect.x*bpp;
			dstpixel=(unsigned char*)rotation_buffer->pixels+(319-rect.x)*rotskip+rect.y*bpp;

			if(bpp==1)
				wiz_rotateblitsurface8(srcpixel,dstpixel,&rect);
			else
			{
				if(bpp==2)
					wiz_rotateblitsurface16((unsigned short*)srcpixel,
								(unsigned short*)dstpixel,&rect);
				else
					wiz_rotateblitsurface32((unsigned int*)srcpixel,
								(unsigned int*)dstpixel,&rect);
			}

			srcpixel=(unsigned char*)rotation_buffer->pixels+(320-rect.x-rect.w)*rotskip+rect.y*bpp;
			dstpixel=fb1[current_fb]+surface->offset;

			hh*=bpp;
			if(!this->mmuhack_type)
			{
				while(w--)
				{
					gp2x_memcpy(dstpixel,srcpixel,hh);
					srcpixel+=rotskip;
					dstpixel+=dstskip;
				}
			}
			else
			{
				while(w--)
				{
					memcpy(dstpixel,srcpixel,hh);
					srcpixel+=rotskip;
					dstpixel+=dstskip;
				}
			}
		}
		else
		{
			dstpixel=(unsigned char*)rotation_buffer->pixels+(h-1)*rotskip;

			if(bpp==1)
				rotateblitfast8(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
			else
			{
				if(bpp==2)
					rotateblitfast16_4x4(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
				else
					rotateblitfast32_4x1(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
			}

			if(!this->mmuhack_type)
				gp2x_memcpy(surface->pixels,rotation_buffer->pixels,320*240*bpp);
			else
				memcpy(surface->pixels,rotation_buffer->pixels,320*240*bpp);
		}

		goto flip_end;
	}
#endif

	/* Rotateblit shadow surface to video surface */
	current_fb++;
	if(current_fb>3) current_fb=0;
	if(surface->offset>0)
	{
		unsigned char* srcpixel;
		SDL_Rect rect;
		rect.x=this->offset_y-this->width_odd;
		rect.y=this->offset_x;
		rect.w=surface->h;
		rect.h=surface->w;

		srcpixel=(unsigned char*)this->shadow_pixels+rect.y*SDL_ShadowSurface->pitch+rect.x*bpp;
		dstpixel=fb1[current_fb]+surface->offset+(rect.w-1)*surface->pitch;

		if(bpp==1)
			wiz_rotateblitsurface8(srcpixel,dstpixel,&rect);
		else
		{
			if(bpp==2)
				wiz_rotateblitsurface16((unsigned short*)srcpixel,
							(unsigned short*)dstpixel,&rect);
			else
				wiz_rotateblitsurface32((unsigned int*)srcpixel,
							(unsigned int*)dstpixel,&rect);
		}
	}
	else
	{
		dstpixel=fb1[current_fb]+(h-1)*surface->pitch;
		if(bpp==1)
			rotateblitfast8(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
		else
		{ 
			if(bpp==2)
				rotateblitfast16_4x4(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
			else
				rotateblitfast32_4x1(SDL_ShadowSurface->pixels,dstpixel,h,surface->w);
		}
	}

flip_end:
	if(SDL_MUSTLOCK(SDL_ShadowSurface))
		SDL_UnlockSurface(SDL_ShadowSurface);

	/* Wait for VSYNC */
	if((surface->flags&WIZ_VSYNCOFF)!=WIZ_VSYNCOFF)
	{
		while(!(DPCCTRL0 & 0x400));

		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();

		DPCCTRL0 |= 0x400;
	}
	else
	{
		/* Page flip */
		Wiz_FlipFB(current_fb);
		Wiz_DirtyLayer1();
	}
	surface->pixels=(void*)fb1[current_fb];
	return(0);
}

static void GP2XWIZ_DirectUpdate(_THIS, int numrects, SDL_Rect *rects)
{
	/* The application is already updating the visible video memory */
	return;
}

/* Set hardware palette */
static int GP2XWIZ_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	int i;
	unsigned short color;

	if(this->tvout_mode & MLC_PRI_ENABLED)
	{
		for (i=firstcolor;i<ncolors;i++)
		{
			color=(colors[i].r>>3)<<11 | (colors[i].g>>2)<<5 | (colors[i].b>>3);
			MLCPALETTE1=i<<24 | color;
		}

		Wiz_DirtyLayer1();
	}
	if(this->tvout_mode & MLC_SEC_ENABLED)
	{
		for (i=firstcolor;i<ncolors;i++)
		{
			color=(colors[i].r>>3)<<11 | (colors[i].g>>2)<<5 | (colors[i].b>>3);
			MLCPALETTE1_TV=i<<24 | color;
		}

		Wiz_DirtyLayer1_TV();
	}

	return 0;
}


/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
static void GP2XWIZ_VideoQuit(_THIS)
{
	int i, j;

	if ( this->screen ) {
		/* Clear screen and tell SDL not to free the pixels */
// ghcstop fix to original: 041123
#if FB_INPUT_DEVICE_ENABLE
		if ( this->screen->pixels && GP2XWIZ_InGraphicsMode(this) ) {
#else			
		if ( this->screen->pixels  ) {
#endif
			
#if defined(__powerpc__) || defined(__ia64__)	/* SIGBUS when using SDL_memset() ?? */
			Uint8 *rowp = (Uint8 *)this->screen->pixels;
			int left = this->screen->pitch*this->screen->h;
			while ( left-- ) { *rowp++ = 0; }
#else
			// GP2X WIZ jhkang 081006
			printf("FB_VideoQuit() h=%d, pitch=%d\n", this->screen->h, this->screen->pitch);

			/*SDL_memset(this->screen->pixels,0,this->screen->h*this->screen->pitch); */
#endif
		/* This test fails when using the VGA16 shadow memory */
#if SDL_VIDEO_OPENGL
			if((this->screen->flags & SDL_OPENGL)!=SDL_OPENGL)
#endif
				this->screen->pixels = NULL;
		}
	}

	/* Clear the lock mutex */
	if ( hw_lock ) {
		SDL_DestroyMutex(hw_lock);
		hw_lock = NULL;
	}

	/* Clean up defined video modes */
	for(i=0;i<5;i++)
	{
		for(j=0;j<3;j++)
		{
			if(SDL_modelist[i][j])
			{
				SDL_free(SDL_modelist[i][j]);
				SDL_modelist[i][j]=NULL;
			}
		}
	}

#if SDL_VIDEO_OPENGL
	if((this->screen->flags & SDL_OPENGL)==SDL_OPENGL)
	{
		GP2XWIZ_GL_Shutdown(this);

		if(mem_fd>0)
		{
			/* clear screen and set screen mode to portrait -> landscape */
			if(memregs32)
			{
				fb1[0]=(unsigned char*)mmap(0,0x25800,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,this->fb1_realaddr[0]);
				if(fb1[0]!=(unsigned char*)0xFFFFFFFF)
				{
					memset((void*)fb1[0],0x00,0x25800);
					Wiz_FlipFB(0);
					Wiz_DirtyLayer1();
					munmap((void*)fb1[0],0x25800);
					fb1[0]=NULL;
				}

				SDL_SetLcdChange(WIZ_ROTATESCREEN_ON,0);
				munmap((void*)memregs32,0x20000);
				memregs32=NULL;
			}
			memregs16=NULL;
			close(mem_fd);
			mem_fd=-1;
		}

		if(fb_fd>0) 
		{
			close(fb_fd);
			fb_fd=-1;
		}

		inited=0;
	}
	else
#endif
	{
		/* Close hardware surface */
		GP2XWIZ_FreeHWSurfaces(this);
		hw_surface=NULL;

		/* Clear screen */
		if(fb1[0])
		{
			memset((void*)fb1[0],0x00,fb1_length);
			if(fb1[1])
			{
				memset((void*)fb1[1],0x00,fb1_length);
				fb1[1]=NULL;
			}
			if(fb1[2])
			{
				memset((void*)fb1[2],0x00,fb1_length);
				fb1[2]=NULL;
			}
			if(fb1[3])
			{
				memset((void*)fb1[3],0x00,fb1_length);
				fb1[3]=NULL;
			}	
		}

		if(memregs32)
		{
			/* Switch to system default framebuffer */
			if(this->tvout_mode & MLC_PRI_ENABLED)
			{
				Wiz_FlipFB(0);
				Wiz_DirtyLayer1();
			}
			if(this->tvout_mode & MLC_SEC_ENABLED)
			{
				Wiz_FlipFB_TV(0);
				Wiz_DirtyLayer1_TV();
			}
			current_fb=0;
		}

#ifndef GP2XWIZ_MPLAYER
		/* Set screen mode to portrait -> landscape */
		SDL_SetLcdChange(WIZ_ROTATESCREEN_ON,0);
#endif

		if(inited==1)
		{
			/* Finish mmuhack if it's enabled */
			if(this->mmuhack_type==1)
				warm_finish();
			else if(this->mmuhack_type==2)
				system("/sbin/rmmod mmuhack");
			this->mmuhack_type=0;
		}

		if(hw_surface_start)
		{
			munmap((void*)hw_surface_start,HWSURFACE_LENGTH);
			hw_surface_start=NULL;
		}

		if(fb1[0])
		{
			munmap((void*)fb1[0],WIZ_FB_LENGTH);
			fb1[0]=NULL;
		}

		if(memregs32)
		{
			/* Restore backuped hw register values */
			if(bkregs32[0])
			{	
				MLCADDRESS1=bkregs32[0][0];
				MLCCONTROL1=bkregs32[0][1];
				MLCLEFTRIGHT1=bkregs32[0][2];
				MLCTOPBOTTOM1=bkregs32[0][3];
				MLCLEFTRIGHT1_0=bkregs32[0][4];
				MLCTOPBOTTOM1_0=bkregs32[0][5];
				MLCLEFTRIGHT1_1=bkregs32[0][6];
				MLCTOPBOTTOM1_1=bkregs32[0][7];
				MLCBGCOLOR=bkregs32[0][8];
				MLCHSTRIDE1=bkregs32[0][9];
				MLCVSTRIDE1=bkregs32[0][10];
				MLCADDRESS2=bkregs32[0][11];
				MLCADDRESSCB=bkregs32[0][12];
				MLCADDRESSCR=bkregs32[0][13];
				MLCCONTROL2=bkregs32[0][14];
				MLCLEFTRIGHT2=bkregs32[0][15];
				MLCTOPBOTTOM2=bkregs32[0][16];
				MLCVSTRIDE2=bkregs32[0][17];
				MLCVSTRIDECB=bkregs32[0][18];
				MLCVSTRIDECR=bkregs32[0][19];
				MLCHSCALE=bkregs32[0][20];
				MLCVSCALE=bkregs32[0][21];

				Wiz_DirtyLayer2();
				Wiz_DirtyLayer1();
				Wiz_DirtyMLC();

				free(bkregs32[0]);
				bkregs32[0]=NULL;
			}

			if(bkregs32[1])
			{
				MLCADDRESS1_TV=bkregs32[1][0];
				MLCCONTROL1_TV=bkregs32[1][1];
				MLCLEFTRIGHT1_TV=bkregs32[1][2];
				MLCTOPBOTTOM1_TV=bkregs32[1][3];
				MLCLEFTRIGHT1_0_TV=bkregs32[1][4];
				MLCTOPBOTTOM1_0_TV=bkregs32[1][5];
				MLCLEFTRIGHT1_1_TV=bkregs32[1][6];
				MLCTOPBOTTOM1_1_TV=bkregs32[1][7];
				MLCBGCOLOR_TV=bkregs32[1][8];
				MLCHSTRIDE1_TV=bkregs32[1][9];
				MLCVSTRIDE1_TV=bkregs32[1][10];
				MLCADDRESS2_TV=bkregs32[1][11];
				MLCADDRESSCB_TV=bkregs32[1][12];
				MLCADDRESSCR_TV=bkregs32[1][13];
				MLCCONTROL2_TV=bkregs32[1][14];
				MLCLEFTRIGHT2_TV=bkregs32[1][15];
				MLCTOPBOTTOM2_TV=bkregs32[1][16];
				MLCVSTRIDE2_TV=bkregs32[1][17];
				MLCVSTRIDECB_TV=bkregs32[1][18];
				MLCVSTRIDECR_TV=bkregs32[1][19];
				MLCHSCALE_TV=bkregs32[1][20];
				MLCVSCALE_TV=bkregs32[1][21];

				Wiz_DirtyLayer2_TV();
				Wiz_DirtyLayer1_TV();
				Wiz_DirtyMLC_TV();

				free(bkregs32[1]);
				bkregs32[1]=NULL;
			}

			munmap((void*)memregs32,0x20000);
			memregs32=NULL;
		}

		memregs16=NULL;
		if(mem_fd>0) 
		{
			close(mem_fd);
			mem_fd=-1;
		}

		if(fb_fd>0) 
		{	
			close(fb_fd);
			fb_fd=-1;
		}

		inited=0;
	}

#if FB_INPUT_DEVICE_ENABLE	// ghcstop
	#if 0 // ghcstop_041123 delete, 이부분은 나중에 원복할 것
	FB_CloseMouse(this);
	#endif
//	dprintf("FB_closeKeyboard\n"); // ghcstop
	GP2XWIZ_CloseKeyboard(this);
#endif	
	GP2XWIZ_CloseMouse(this); //hyun
//printf("[4]FB_VideoQuit() h=%d, pitch=%d\n", this->screen->h, this->screen->pitch);
}

