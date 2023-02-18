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

/* The high-level video driver subsystem */

//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>

#include "SDL.h"
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"
#include "SDL_cursor_c.h"
#include "../events/SDL_sysevents.h"
#include "../events/SDL_events_c.h"
/*#include <linux/i2c-dev.h>*/

#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.1.23 */
#include "gp2xwiz/wiz_rotateblit.h"
#include "gp2xwiz/polluxregs.h"
#endif

/* Available video drivers */
static VideoBootStrap *bootstrap[] = {
#if SDL_VIDEO_DRIVER_QUARTZ
	&QZ_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_X11
	&X11_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DGA
	&DGA_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_NANOX
	&NX_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_IPOD
	&iPod_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_QTOPIA
	&Qtopia_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_WSCONS
	&WSCONS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.1.23 */
	&GP2XWIZ_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_FBCON
	&FBCON_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DIRECTFB
	&DirectFB_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_PS2GS
	&PS2GS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_GGI
	&GGI_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_VGL
	&VGL_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_SVGALIB
	&SVGALIB_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_GAPI
	&GAPI_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_WINDIB
	&WINDIB_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DDRAW
	&DIRECTX_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_BWINDOW
	&BWINDOW_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_TOOLBOX
	&TOOLBOX_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DRAWSPROCKET
	&DSp_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_PHOTON
	&ph_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_EPOC
	&EPOC_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_XBIOS
	&XBIOS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_GEM
	&GEM_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_PICOGUI
	&PG_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DC
	&DC_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_NDS
	&NDS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_RISCOS
	&RISCOS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_OS2FS
	&OS2FSLib_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_AALIB
	&AALIB_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DUMMY
	&DUMMY_bootstrap,
#endif
	NULL
};

SDL_VideoDevice *current_video = NULL;

/* Various local functions */
int SDL_VideoInit(const char *driver_name, Uint32 flags);
void SDL_VideoQuit(void);
void SDL_GL_UpdateRectsLock(SDL_VideoDevice* this, int numrects, SDL_Rect* rects);

static SDL_GrabMode SDL_WM_GrabInputOff(void);
#if SDL_VIDEO_OPENGL
static int lock_count = 0;
#endif

/* modified by ikari 2010.1.23 */
#if SDL_VIDEO_DRIVER_GP2XWIZ
static int Wiz_RotateBlitShadowToScreen(SDL_Surface* src,SDL_Surface* dst,SDL_Rect* rect);
static int Wiz_BlitShadowToScreen(SDL_Surface* src,SDL_Surface* dst,SDL_Rect* rect);
static int (*SDL_BlitShadowToScreen)(SDL_Surface*,SDL_Surface*,SDL_Rect*)=NULL;
static unsigned int shadow_malloc=0;
#endif

/*
 * Initialize the video and event subsystems -- determine native pixel format
 */
int SDL_VideoInit (const char *driver_name, Uint32 flags)
{
	SDL_VideoDevice *video;
	int index;
	int i;
	SDL_PixelFormat vformat;
	Uint32 video_flags;

	/* Toggle the event thread flags, based on OS requirements */
#if defined(MUST_THREAD_EVENTS)
	flags |= SDL_INIT_EVENTTHREAD;
#elif defined(CANT_THREAD_EVENTS)
	if ( (flags & SDL_INIT_EVENTTHREAD) == SDL_INIT_EVENTTHREAD ) {
		SDL_SetError("OS doesn't support threaded events");
		return(-1);
	}
#endif

	/* Check to make sure we don't overwrite 'current_video' */
	if ( current_video != NULL ) {
		SDL_VideoQuit();
	}

	/* Select the proper video driver */
	index = 0;
	video = NULL;
	if ( driver_name != NULL ) {
#if 0	/* This will be replaced with a better driver selection API */
		if ( SDL_strrchr(driver_name, ':') != NULL ) {
			index = atoi(SDL_strrchr(driver_name, ':')+1);
		}
#endif
		for ( i=0; bootstrap[i]; ++i ) {
			if ( SDL_strcasecmp(bootstrap[i]->name, driver_name) == 0) {
				if ( bootstrap[i]->available() ) {
					video = bootstrap[i]->create(index);
					break;
				}
			}
		}
	} else {
		for ( i=0; bootstrap[i]; ++i ) {
			if ( bootstrap[i]->available() ) {
				video = bootstrap[i]->create(index);
				if ( video != NULL ) {
					break;
				}
			}
		}
	}
	if ( video == NULL ) {
		SDL_SetError("No available video device");
		return(-1);
	}
	current_video = video;
	current_video->name = bootstrap[i]->name;

	/* Do some basic variable initialization */
	video->screen = NULL;
	video->shadow = NULL;
	video->visible = NULL;
	video->physpal = NULL;
	video->gammacols = NULL;
	video->gamma = NULL;
	video->wm_title = NULL;
	video->wm_icon  = NULL;
	video->offset_x = 0;
	video->offset_y = 0;
	/* modified by ikari 2010.2.15 */
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.12 */
	video->_fb_fd=-1;
	video->fb1_realaddr[0]=0;
	video->fb1_realaddr[1]=0;
	video->fb1_realaddr[2]=0;
	video->fb1_realaddr[3]=0;
	video->_memregs32=NULL;
	video->_memregs16=NULL;
	video->shadow_pixels=NULL;
	video->shadow_pitch=0;
	video->rotate_screen=0;
	video->mmuhack_type=0;
	video->width_odd=0;
	video->tvout_mode=0;
#ifdef GP2XWIZ_ROTATIONBUFFER
	video->rotation_buffer=NULL;
#endif
#endif
	SDL_memset(&video->info, 0, (sizeof video->info));
	
	video->displayformatalphapixel = NULL;

	/* Set some very sane GL defaults */
	video->gl_config.driver_loaded = 0;
	video->gl_config.dll_handle = NULL;
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.12 */
	video->gl_config.red_size = 5;
	video->gl_config.green_size = 6;
	video->gl_config.blue_size = 5;
	video->gl_config.alpha_size = 0;
	video->gl_config.buffer_size = 0;
	video->gl_config.depth_size = 16;
	video->gl_config.swap_control = 1;
#else
	video->gl_config.red_size = 3;
	video->gl_config.green_size = 3;
	video->gl_config.blue_size = 2;
	video->gl_config.alpha_size = 0;
	video->gl_config.buffer_size = 0;
	video->gl_config.depth_size = 16;
	video->gl_config.stencil_size = 0;
	video->gl_config.double_buffer = 1;
	video->gl_config.accum_red_size = 0;
	video->gl_config.accum_green_size = 0;
	video->gl_config.accum_blue_size = 0;
	video->gl_config.accum_alpha_size = 0;
	video->gl_config.stereo = 0;
	video->gl_config.multisamplebuffers = 0;
	video->gl_config.multisamplesamples = 0;
	video->gl_config.accelerated = -1; /* not known, don't set */
	video->gl_config.swap_control = -1; /* not known, don't set */
#endif

	/* Initialize the video subsystem */
	SDL_memset(&vformat, 0, sizeof(vformat));
	if ( video->VideoInit(video, &vformat) < 0 ) {
		SDL_VideoQuit();
		return(-1);
	}

	/* Create a zero sized video surface of the appropriate format */
	video_flags = SDL_SWSURFACE;
	SDL_VideoSurface = SDL_CreateRGBSurface(video_flags, 0, 0,
				vformat.BitsPerPixel,
				vformat.Rmask, vformat.Gmask, vformat.Bmask, 0);
	if ( SDL_VideoSurface == NULL ) {
		SDL_VideoQuit();
		return(-1);
	}
	SDL_PublicSurface = NULL;	/* Until SDL_SetVideoMode() */

#if 0 /* Don't change the current palette - may be used by other programs.
       * The application can't do anything with the display surface until
       * a video mode has been set anyway. :)
       */
	/* If we have a palettized surface, create a default palette */
	if ( SDL_VideoSurface->format->palette ) {
		SDL_PixelFormat *vf = SDL_VideoSurface->format;
		SDL_DitherColors(vf->palette->colors, vf->BitsPerPixel);
		video->SetColors(video,
				 0, vf->palette->ncolors, vf->palette->colors);
	}
#endif
	video->info.vfmt = SDL_VideoSurface->format;

	/* Start the event loop */
	if ( SDL_StartEventLoop(flags) < 0 ) {
		SDL_VideoQuit();
		return(-1);
	}
	SDL_CursorInit(flags & SDL_INIT_EVENTTHREAD);

	/* We're ready to go! */
	return(0);
}

/* HYUN_DEBUG */
/* ********************************************************************************** */
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.18 */
extern int SDL_videofd;
#endif

static int lcd_mode=0;

/* modified by ikari 2010.2.24 */
int SDL_GetVideofd(void)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(this)
        return fb_fd;
    return -1;
#else
    return SDL_videofd;
#endif
}

#include <sys/ioctl.h>
#if 0
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include "pollux_fb_ioctl.h"

#define RESUME_OPER		0xEF
#define LASTER_OPER		0x6F

//#define usleep(x) microsleep(x)
//#define msleep(x) usleep(x*1000)

#define I2C_DEV   "/dev/i2c-0"
#define SLAVE_EEPROM			(0xA0 >> 1)
#define PIO_BASE     0xC0000000
#define IO_SIZE      0x10000

#define PIO_I2CCON_ADDR    	0xC0000D00
#define PIO_I2CSTAT_ADDR    0xC0000D02
#define PIO_I2CADD_ADDR    	0xC0000D04
#define PIO_I2CDD_ADDR    	0xC0000D06

#define PIO_GPIOH_ADDR			0xC000106E
#define PIO_GPIOH_ALN_ADDR		0xC000102E

#define PIO_I2CCON_OFFSET     ( PIO_I2CCON_ADDR  - PIO_BASE )
#define PIO_I2CSTAT_OFFSET    ( PIO_I2CSTAT_ADDR - PIO_BASE )
#define PIO_I2CADD_OFFSET     ( PIO_I2CADD_ADDR  - PIO_BASE )
#define PIO_I2CDD_OFFSET      ( PIO_I2CDD_ADDR   - PIO_BASE )

#define PIO_GPIOH_OFFSET	  (	PIO_GPIOH_ADDR - PIO_BASE )
#define PIO_GPIOH_ALN_OFFSET  (	PIO_GPIOH_ALN_ADDR - PIO_BASE )

#define BASEADDR 0x50
#define DRMSIZE 0xC0
#define TOTALSIZE (BASEADDR + DRMSIZE)

#define U8 unsigned char
#define U16 unsigned short
#define U32 unsigned long

static const unsigned char adr[8] = {0xBA, 0xF1, 0xF0, 0xE3, 0xBA, 0xF8, 0xF0, 0xF8};

typedef struct tagI2CBUS{
	int mfd;
	U16* I2CCON;
	U16* I2CSTAT;
	U16* I2CADD;
	U16* I2CDD;
	void* base;
}I2CBUS;

static I2CBUS i2c;

static inline int SDL_InitEEPROM()
{
	char dev[9];
	int i;

	for (i=0;i<8;i++)
		dev[i] = adr[i] ^ 0x95;
	dev[8] = 0;

	i2c.mfd = open( dev, O_RDWR|O_SYNC );
	if( i2c.mfd < 0 )
	{
		i2c.base = NULL;
		return 0;
	}

	i2c.base = (U16 *) mmap( 0, IO_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, i2c.mfd, PIO_BASE );

	if( i2c.base != NULL) {
		i2c.I2CCON  =	(U16 *) (i2c.base + PIO_I2CCON_OFFSET);
		i2c.I2CSTAT =	(U16 *) (i2c.base + PIO_I2CSTAT_OFFSET);
		i2c.I2CADD  =	(U16 *) (i2c.base + PIO_I2CADD_OFFSET);
		i2c.I2CDD   =	(U16 *) (i2c.base + PIO_I2CDD_OFFSET);
	} else {
		close(i2c.mfd);
		return 0;
	}

	*(i2c.I2CCON)  = RESUME_OPER;
	*(i2c.I2CADD)  = 0xA0;
	*(i2c.I2CSTAT) = 0x10;
	usleep(100);

	return 1;
}
/* ================================================================================ */
static inline void SDL_CloseEEPROM(void)
{
	if( i2c.base == NULL ) return ;

	munmap( i2c.base, IO_SIZE );
	close(i2c.mfd);
}

int SDL_RdEEPROM(unsigned int rAddr,unsigned int size,unsigned char * buff) 
{
	struct i2c_rdwr_ioctl_data    i2c_rctl;
	struct i2c_msg                i2c_msgs;
	int i2c_rfd,err,i;
	unsigned char addr[2];
	unsigned char tempbuff;
	
	addr[0]=(unsigned char)(rAddr >> 8); 		//MSB
	addr[1]=(unsigned char)(rAddr & 0xff);		//LSB	
	
	if((i2c_rfd = open( I2C_DEV, O_RDWR ))< 0 )
	{
		printf("I2c driver open error\n");
		return -1;
	}

	i2c_rctl.msgs=&i2c_msgs;
	i2c_rctl.nmsgs = 1;
    	i2c_rctl.msgs[0].addr  = SLAVE_EEPROM;
    	i2c_rctl.msgs[0].flags = 0;
    	i2c_rctl.msgs[0].len   = 2;
    	i2c_rctl.msgs[0].buf   = addr;
	err=ioctl( i2c_rfd, I2C_RDWR, (struct i2c_rdwr_ioctl_data *) &i2c_rctl);
	if(err < 0) {
		printf("EEPROM addr set error\n");
		close(i2c_rfd);
		return -1;
	}


	//for(i=0;i<size;i++)
	{	
		i2c_rctl.msgs=&i2c_msgs;
		i2c_rctl.nmsgs = 1;
		i2c_rctl.msgs[0].addr  = SLAVE_EEPROM;
		i2c_rctl.msgs[0].flags = I2C_M_RD;
		i2c_rctl.msgs[0].len   = 1;
		i2c_rctl.msgs[0].buf   = buff;		//&tempbuff;
		
		err=ioctl( i2c_rfd, I2C_RDWR, (struct i2c_rdwr_ioctl_data *) &i2c_rctl);
		if(err < 0) {
			printf("EEPROM addr set error\n");
			close(i2c_rfd);
			return -1;
		}	
		
		//buff++;						//=tempbuff;
	}
	//printf("RdEEPROM() buff=%s\n", buff);
	close(i2c_rfd);
	return 0;	
}

#if 1
void SDL_GetDRMNumber(unsigned char * buf)
{
	const unsigned char addr[3] = {0xB7, 0x86, 0x9F};
	unsigned char val[3] = {0x0,};
	short drmaddr;
	int i;

	SDL_InitEEPROM();

	for(i=0;i<3;i++)
		//val[i] = RdEEPROM(addr[i] + BASEADDR);
		//RdEEPROM(addr[i] + BASEADDR, 1, val[i]);
		SDL_RdEEPROM(addr[i] + BASEADDR, 1, val+i);

	if( ((val[0] & 0xfe) == (unsigned char)(val[1] << 1)) &&
	    ((val[1] & 0xfe) == (unsigned char)(val[2] << 1)) ) {
		short testaddr = val[0] | (((short)val[1]) << 1) | (((short)val[2]) << 2);
		drmaddr = val[0] | (((short)val[2]) << 2);
		if((drmaddr >= BASEADDR) && (drmaddr < TOTALSIZE) && (drmaddr == testaddr)) {
			for(i=0; i < 16; i++) {
				//buf[i + 0x10] = RdEEPROM(drmaddr + i) ^ val[i % 3];
				//RdEEPROM(drmaddr + i, 1, buf[i + 0x10]);
				 SDL_RdEEPROM(drmaddr + i,1,buf+(i+0x10));	
				buf[i + 0x10]  = buf[i + 0x10]  ^ val[i % 3];

				buf[i << 1] = (buf[i + 0x10] >> 4);
				buf[(i << 1) + 1] = (buf[i + 0x10] & 0x0f);
			}

			for(i=0; i< 32; i++) {
				if(buf[i] < 10) {
					buf[i] = '0' + buf[i];
				} else {
					buf[i] = 'A' - 10 + buf[i];
				}
			}

			SDL_CloseEEPROM();

			return;
		}
	}

	for(i=0;i<32;i++)
		//buf[i] = RdEEPROM(i);
		SDL_RdEEPROM(i, 1, buf+i);
	
	//RdEEPROM(0, 32, buf);
	//printf("getcode(buf=%s)\n", buf);
	SDL_CloseEEPROM();
}
#endif

void SDL_GetSerialNumber(unsigned char *buf)
{
	int i;

	SDL_InitEEPROM();

	for(i=0;i<24;i++)
		//buf[i] = RdEEPROM(i + 0x20);
		SDL_RdEEPROM(i + 0x20, 1, buf+i);

	//RdEEPROM(0x20, 24, buf);
	//printf("getserial(buf=%s)\n", buf);
	SDL_CloseEEPROM();
}
#endif

int SDL_SetLcdChange(unsigned int subCmd, unsigned int value)
{
    unsigned int send[2];
    send[0] = subCmd;
    send[1] = value;

    /* modified by ikari 2010.2.24 */
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(fb_fd>=0)
        ioctl(fb_fd, FBIO_LCD_CHANGE_CONTROL, &send);
#else
    ioctl(SDL_videofd, FBIO_LCD_CHANGE_CONTROL, &send);
#endif
	return 0;
}

int SDL_TvConfig(FB_TVCONF * tv_cfg)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(fb_fd>=0)
        ioctl(fb_fd, FBIO_SET_TVCFG_MODE, tv_cfg);
#else
    ioctl(SDL_videofd, FBIO_SET_TVCFG_MODE, tv_cfg);
#endif
    return 0;
}

void SDL_SetLcdMode(int mode)
{
    lcd_mode=mode;
}

int SDL_GetLcdMode(void)
{
    return lcd_mode;
}

int SDL_GetGphBoard(unsigned char * board_num)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(fb_fd>=0)
        ioctl(fb_fd, FBIO_GET_BOARD_NUMBER, board_num);
#else
    ioctl(SDL_videofd, FBIO_GET_BOARD_NUMBER, board_num);
#endif
    return 0;
}

// jhkang 080722 [[
int SDL_FBRGBControl(FB_RGBSET * rgbset)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && rgbset)
    {
	if(rgbset->Mlc_dev==PRI_MLC)
	{
      	    if(rgbset->Layer==0)
            {
                /*MLCADDRESS0=rgbset->Addrs;*/
                MLCADDRESS0=this->fb1_realaddr[0];
                MLCLEFTRIGHT0=(rgbset->Left<<16)|(rgbset->Right-1);
                MLCTOPBOTTOM0=(rgbset->Top<<16)|(rgbset->Bottom-1);

                if(rgbset->enable3D)
                {
                    MLCCONTROL0|=0x100;
                    MLCHSTRIDE0=rgbset->HStride;
                    MLCVSTRIDE0=rgbset->VStride;
                }
                else
                {
                    MLCCONTROL0&=~0x100;
                    MLCHSTRIDE0=2;
                    MLCVSTRIDE0=2*(rgbset->Right-rgbset->Left);
                }

                MLCCONTROL0|=0x44328000;
                MLCCONTROL0&=~0x7006;
                Wiz_DirtyLayer0();
            }
            else if(rgbset->Layer==1)
            {
                /*MLCADDRESS1=rgbset->Addrs;*/
                MLCADDRESS1=this->fb1_realaddr[0];
                MLCLEFTRIGHT1=(rgbset->Left<<16)|(rgbset->Right-1);
                MLCTOPBOTTOM1=(rgbset->Top<<16)|(rgbset->Bottom-1);

                if(rgbset->enable3D)
                {
                    MLCCONTROL1|=0x100;
                    MLCHSTRIDE1=rgbset->HStride;
                    MLCVSTRIDE1=rgbset->VStride;
                }
                else
                {
                    MLCCONTROL1&=~0x100;
                    MLCHSTRIDE1=2;
                    MLCVSTRIDE1=2*(rgbset->Right-rgbset->Left);
                }

                MLCCONTROL1|=0x44328000;
                MLCCONTROL1&=~0x7006;
                Wiz_DirtyLayer1();
            }

            MLCBGCOLOR=(rgbset->Bakcol & 0x00FFFFFF);
            Wiz_DirtyMLC();
        }
	if((rgbset->Mlc_dev==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
      	    if(rgbset->Layer==0)
            {
                /*MLCADDRESS0_TV=rgbset->Addrs;*/
                MLCADDRESS0_TV=this->fb1_realaddr[0];
                MLCLEFTRIGHT0_TV=(rgbset->Left<<16)|(rgbset->Right-1);
                MLCTOPBOTTOM0_TV=(rgbset->Top<<16)|(rgbset->Bottom-1);

                if(rgbset->enable3D)
                {
                    MLCCONTROL0_TV|=0x100;
                    MLCHSTRIDE0_TV=rgbset->HStride;
                    MLCVSTRIDE0_TV=rgbset->VStride;
                }
                else
                {
                    MLCCONTROL0_TV&=~0x100;
                    MLCHSTRIDE0_TV=2;
                    MLCVSTRIDE0_TV=2*(rgbset->Right-rgbset->Left);
                }

                MLCCONTROL0_TV|=0x44328000;
                MLCCONTROL0_TV&=~0x7006;
                Wiz_DirtyLayer0_TV();
            }
            else if(rgbset->Layer==1)
            {
                /*MLCADDRESS1_TV=rgbset->Addrs;*/
                MLCADDRESS1_TV=this->fb1_realaddr[0];
                MLCLEFTRIGHT1_TV=(rgbset->Left<<16)|(rgbset->Right-1);
                MLCTOPBOTTOM1_TV=(rgbset->Top<<16)|(rgbset->Bottom-1);

                if(rgbset->enable3D)
                {
                    MLCCONTROL1_TV|=0x100;
                    MLCHSTRIDE1_TV=rgbset->HStride;
                    MLCVSTRIDE1_TV=rgbset->VStride;
                }
                else
                {
                    MLCCONTROL1_TV&=~0x100;
                    MLCHSTRIDE1_TV=2;
                    MLCVSTRIDE1_TV=2*(rgbset->Right-rgbset->Left);
                }

                MLCCONTROL1_TV|=0x44328000;
                MLCCONTROL1_TV&=~0x7006;
                Wiz_DirtyLayer1_TV();
            }

            MLCBGCOLOR_TV=(rgbset->Bakcol & 0x00FFFFFF);
            Wiz_DirtyMLC_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_RGB_CONTROL, rgbset);
#endif
	return 0;
}

int SDL_FBVideoPriority(unsigned int * vpriority)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vpriority)
    {
        if(*(vpriority+1)!=SEC_MLC)
        {
            MLCCONTROLT=(MLCCONTROLT&(~0x300))|((*vpriority)<<8);
            Wiz_DirtyMLC();
        }
        if((*(vpriority+1)==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCCONTROLT_TV=(MLCCONTROLT_TV&(~0x300))|((*vpriority)<<8);
            Wiz_DirtyMLC_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_PRIORITY, vpriority);
#endif
	return 0;
}

int SDL_FBLayerTPColor(unsigned int  * tpcolor)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && tpcolor)
    {
        Uint8 r=(Uint8)(((*(tpcolor+1))>>16)&0xFF);
        Uint8 g=(Uint8)(((*(tpcolor+1))>>8)&0xFF);
        Uint8 b=(Uint8)(((*(tpcolor+1))>>0)&0xFF);
        Uint16 rgb565=((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3);

        Uint8 r5=(rgb565>>11) & 0x1F;
        Uint8 g6=(rgb565>>5) & 0x3F;
        Uint8 b5=(rgb565>>0) & 0x1F;
        r=((r5<<3)&0xF8)|((r5>>2)&0x7);
        g=((g6<<2)&0xFC)|((g6>>4)&0x3);
        b=((b5<<3)&0xF8)|((b5>>2)&0x7);
        (*(tpcolor+1))=(r<<16)|(g<<8)|(b);

        if(*(tpcolor+3)==PRI_MLC)
        {
            if((*tpcolor)==0)
            {
                if(*(tpcolor+2))
                    MLCCONTROL0|=0x1;
                else
                    MLCCONTROL0&=~0x1;

                MLCTPCOLOR0=(MLCTPCOLOR0&0xFF000000)|((*(tpcolor+1))&0x00FFFFFF);
                Wiz_DirtyLayer0();
            }
            else if((*tpcolor)==1)
            {
                if(*(tpcolor+2))
                    MLCCONTROL1|=0x1;
                else
                    MLCCONTROL1&=~0x1;

                MLCTPCOLOR1=(MLCTPCOLOR1&0xFF000000)|((*(tpcolor+1))&0x00FFFFFF);
                Wiz_DirtyLayer1();
            }
        }
        if((*(tpcolor+3)==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            if((*tpcolor)==0)
            {
                if(*(tpcolor+2))
                    MLCCONTROL0_TV|=0x1;
                else
                    MLCCONTROL0_TV&=~0x1;

                MLCTPCOLOR0_TV=(MLCTPCOLOR0_TV&0xFF000000)|((*(tpcolor+1))&0x00FFFFFF);
                Wiz_DirtyLayer0_TV();
            }
            else if((*tpcolor)==1)
            {
                if(*(tpcolor+2))
                    MLCCONTROL1_TV|=0x1;
                else
                    MLCCONTROL1_TV&=~0x1;

                MLCTPCOLOR1_TV=(MLCTPCOLOR1_TV&0xFF000000)|((*(tpcolor+1))&0x00FFFFFF);
                Wiz_DirtyLayer1_TV();
            }
        }
    }
#else
    ioctl(SDL_videofd, FBIO_LAYER_TPCOLOR, tpcolor);
#endif
	return 0;
}

int SDL_FBLayerAlphaBLD(unsigned int * alphabld)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && alphabld)
    {
        if((*(alphabld+1))<0) (*(alphabld+1))=0; 
        if((*(alphabld+1))>15) (*(alphabld+1))=15; 

        if(*(alphabld+3)==PRI_MLC)
        {
            if((*alphabld)==0)
            {
                if(*(alphabld+2))
                    MLCCONTROL0|=0x4;
                else
                    MLCCONTROL0&=~0x4;

                MLCTPCOLOR0=(MLCTPCOLOR0&0x00FFFFFF) | (*(alphabld+1)) << 28;
                Wiz_DirtyLayer0();
            }
            else if((*alphabld)==1)
            {
                if(*(alphabld+2))
                    MLCCONTROL1|=0x4;
                else
                    MLCCONTROL1&=~0x4;

                MLCTPCOLOR1=(MLCTPCOLOR1&0x00FFFFFF) | (*(alphabld+1)) << 28;
                Wiz_DirtyLayer1();
            }
        }
        if((*(alphabld+3)==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            if((*alphabld)==0)
            {
                if(*(alphabld+2))
                    MLCCONTROL0_TV|=0x4;
                else
                    MLCCONTROL0_TV&=~0x4;

                MLCTPCOLOR0_TV=(MLCTPCOLOR0_TV&0x00FFFFFF) | (*(alphabld+1)) << 28;
                Wiz_DirtyLayer0_TV();
            }
            else if((*alphabld)==1)
            {
                if(*(alphabld+2))
                    MLCCONTROL1_TV|=0x4;
                else
                    MLCCONTROL1_TV&=~0x4;

                MLCTPCOLOR1_TV=(MLCTPCOLOR1_TV&0x00FFFFFF) | (*(alphabld+1)) << 28;
                Wiz_DirtyLayer1_TV();
            }
        }
    }
#else
    ioctl(SDL_videofd, FBIO_LAYER_ALPHABLD, alphabld);
#endif
	return 0;
}

int SDL_FBDeviceEnable(unsigned int * devenable)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && devenable)
    {
        if(*(devenable+2)==PRI_MLC)
        {
            if(*devenable==0)
            {
                if(*(devenable+1))
                    MLCCONTROL0|=0x20;
                else
                    MLCCONTROL0&=~0x20;

                Wiz_DirtyLayer0();
            }
            else if(*devenable==1)
            {
                if(*(devenable+1))
                    MLCCONTROL1|=0x20;
                else
                    MLCCONTROL1&=~0x20;

                Wiz_DirtyLayer1();
            }
            Wiz_DirtyMLC();
        }
        if((*(devenable+2)==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            if(*devenable==0)
            {
                if(*(devenable+1))
                    MLCCONTROL0_TV|=0x20;
                else
                    MLCCONTROL0_TV&=~0x20;

                Wiz_DirtyLayer0_TV();
            }
            else if(*devenable==1)
            {
                if(*(devenable+1))
                    MLCCONTROL1_TV|=0x20;
                else
                    MLCCONTROL1_TV&=~0x20;

                Wiz_DirtyLayer1_TV();
            }
            Wiz_DirtyMLC_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_DEVICE_ENABLE, devenable);
#endif
	return 0;
}

int SDL_FBVideoInit(FB_VIDEO_CONF * vconf)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vconf)
    {
        int srcw=vconf->SrcWidth,srch=vconf->SrcHeight;
        int dstw,dsth;
        unsigned int hscale,vscale;
        unsigned int bw=0,bh=0;
        unsigned int colorkey=vconf->ColorKey;
        int screenwidth=320,screenheight=240;

#ifndef GP2XWIZ_MPLAYER
        if((vconf->Flags & SET_TPCOLOR)&&(SDL_VideoSurface->format->BitsPerPixel==16))
        {
            Uint8 r5=(colorkey>>11) & 0x1F;
            Uint8 g6=(colorkey>>5) & 0x3F;
            Uint8 b5=(colorkey>>0) & 0x1F;
            Uint8 r8=((r5<<3)&0xF8)|((r5>>2)&0x7);
            Uint8 g8=((g6<<2)&0xFC)|((g6>>4)&0x3);
            Uint8 b8=((b5<<3)&0xF8)|((b5>>2)&0x7);

            colorkey=(r8<<16)|(g8<<8)|(b8);
        }
#endif

        dstw=!(vconf->Right-vconf->Left)?2:(vconf->Right-vconf->Left);
        dsth=!(vconf->Bottom-vconf->Top)?2:(vconf->Bottom-vconf->Top);

        if(dstw>srcw)
        {
            srcw--;
            dstw--;
            bw=0x10000000;
        }
        if(dsth>srch)
        {
            srch--;
            dsth--;
            bh=0x10000000;
        }

        hscale=(srcw<<11)/dstw;
        vscale=(srch<<11)/dsth;

        if(vconf->VideoDev==PRI_MLC)
        {
            MLCLEFTRIGHT2=(vconf->Left<<16)|(vconf->Right-1);
            MLCTOPBOTTOM2=(vconf->Top<<16)|(vconf->Bottom-1);

            MLCHSCALE=bw|(hscale&0x7FFFFF);
            MLCVSCALE=bh|(vscale&0x7FFFFF);

#ifndef GP2XWIZ_MPLAYER
            MLCCONTROL1|=0x1;
            MLCTPCOLOR1=(MLCTPCOLOR1&0xFF000000)|(colorkey&0x00FFFFFF);
            Wiz_DirtyLayer1();

            if(this->rotate_screen)
            {
                screenwidth=240;
                screenheight=320;
            }

            MLCSCREENSIZE=((screenheight-1)<<16) | (screenwidth-1);
            MLCBGCOLOR=(colorkey&0x00FFFFFF);
	    Wiz_DirtyMLC();
#endif

            if(MLCCONTROL2 & 0x20) 
                MLCCONTROL2=0xD020;
            else MLCCONTROL2=0xD000;

            Wiz_DirtyLayer2();
        }
        else
        {
            screenwidth=720;
            screenheight=480;
        }

        if((vconf->VideoDev==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCLEFTRIGHT2_TV=(vconf->Left<<16)|(vconf->Right-1);
            MLCTOPBOTTOM2_TV=(vconf->Top<<16)|(vconf->Bottom-1);

            MLCHSCALE_TV=bw|(hscale&0x7FFFFF);
            MLCVSCALE_TV=bh|(vscale&0x7FFFFF);

#ifndef GP2XWIZ_MPLAYER
            MLCCONTROL1_TV|=0x1;
            MLCTPCOLOR1_TV=(MLCTPCOLOR1_TV&0xFF000000)|(colorkey&0x00FFFFFF);
            Wiz_DirtyLayer1_TV();

            MLCSCREENSIZE_TV=((screenheight-1)<<16) | (screenwidth-1);
            MLCBGCOLOR_TV=(colorkey&0x00FFFFFF);
	    Wiz_DirtyMLC_TV();
#endif

            if(MLCCONTROL2_TV & 0x20) 
                MLCCONTROL2_TV=0xD020;
            else MLCCONTROL2_TV=0xD000;

            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_INIT, vconf);
#endif
	return 0;
}

int SDL_FBVideoUpdate(FB_VIDEO_CONF * vconf)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vconf)
    {
        int srcw=vconf->SrcWidth,srch=vconf->SrcHeight;
        int dstw=vconf->DstWidth,dsth=vconf->DstHeight;
        unsigned int bw=0,bh=0;
        unsigned int leftright=(vconf->Left<<16)|(vconf->Right-1);
        unsigned int topbottom=(vconf->Top<<16)|(vconf->Bottom-1);
        unsigned int hscale,vscale;

        if(dstw==0) dstw=1;
        if(dsth==0) dsth=1;

        if(dstw>srcw)
        {
            srcw--;
            dstw--;
            bw=0x10000000;
        }
        if(dsth>srch)
        {
            srch--;
            dsth--;
            bh=0x10000000;
        }

        hscale=bw|(((srcw<<11)/dstw)&0x7FFFFF);
        vscale=bh|(((srch<<11)/dsth)&0x7FFFFF);

        MLCLEFTRIGHT2=leftright;
        MLCTOPBOTTOM2=topbottom;
        Wiz_DirtyLayer2();

        MLCHSCALE=hscale;
        MLCVSCALE=vscale;
        Wiz_DirtyLayer2();

        if((vconf->VideoDev==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCLEFTRIGHT2_TV=leftright;
            MLCTOPBOTTOM2_TV=topbottom;
            Wiz_DirtyLayer2_TV();

            MLCHSCALE_TV=hscale;
            MLCVSCALE_TV=vscale;
            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_UPDATE, vconf);
#endif
	return 0;
}

int SDL_FBVideoStart(FB_VMEMINFO * vmem)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vmem)
    {
        MLCVSTRIDE2=vmem->LuStride;
        MLCVSTRIDECB=vmem->CbStride;
        MLCVSTRIDECR=vmem->CrStride;
        MLCADDRESS2=vmem->LuOffset;
        MLCADDRESSCB=vmem->CbOffset;
        MLCADDRESSCR=vmem->CrOffset;
        Wiz_DirtyLayer2();

        MLCCONTROL2|=0x20;
        Wiz_DirtyLayer2();

        if((vmem->VideoDev==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCVSTRIDE2_TV=vmem->LuStride;
            MLCVSTRIDECB_TV=vmem->CbStride;
            MLCVSTRIDECR_TV=vmem->CrStride;
            MLCADDRESS2_TV=vmem->LuOffset;
            MLCADDRESSCB_TV=vmem->CbOffset;
            MLCADDRESSCR_TV=vmem->CrOffset;
            Wiz_DirtyLayer2_TV();

            MLCCONTROL2_TV|=0x20;
            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_START, vmem);
#endif
	return 0;
}

int SDL_FBVideoStop(unsigned int  * vstop)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vstop)
    {
        MLCCONTROL2&=~0x20;
        Wiz_DirtyLayer2();

        if((*vstop==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCCONTROL2_TV&=~0x20;
            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_STOP, vstop);
#endif
	return 0;
}

int SDL_FBVideoMemoryUpdate(FB_VMEMINFO * vmem)
{    
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && vmem)
    {
        MLCVSTRIDE2=vmem->LuStride;
        MLCVSTRIDECB=vmem->CbStride;
        MLCVSTRIDECR=vmem->CrStride;
        MLCADDRESS2=vmem->LuOffset;
        MLCADDRESSCB=vmem->CbOffset;
        MLCADDRESSCR=vmem->CrOffset;
        Wiz_DirtyLayer2();

        if((vmem->VideoDev==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCVSTRIDE2_TV=vmem->LuStride;
            MLCVSTRIDECB_TV=vmem->CbStride;
            MLCVSTRIDECR_TV=vmem->CrStride;
            MLCADDRESS2_TV=vmem->LuOffset;
            MLCADDRESSCB_TV=vmem->CbOffset;
            MLCADDRESSCR_TV=vmem->CrOffset;
            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_MEMORY_UPDATE, vmem);
#endif
	return 0;
}

int SDL_FBLuminance(unsigned int * value)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
    SDL_VideoDevice *this=current_video;
    if(memregs32 && value)
    {
        int brightness=*((int*)value);
        int contrast=*((int*)(value+1));

        if(contrast>8) contrast=7;
        if(contrast<=0) contrast=0;

        if(brightness>128) brightness=127;
        if(brightness<=-128) brightness=-128;

        if(*(value+2)==PRI_MLC)
        {
            MLCLUENH=(((unsigned int)brightness & 0xFFUL)<<8) | ((unsigned int)contrast);
            Wiz_DirtyLayer2();
        }
        if((*(value+2)==SEC_MLC)||(this->tvout_mode & MLC_SEC_ENABLED))
        {
            MLCLUENH_TV=(((unsigned int)brightness & 0xFFUL)<<8) | ((unsigned int)contrast);
            Wiz_DirtyLayer2_TV();
        }
    }
#else
    ioctl(SDL_videofd, FBIO_VIDEO_LUMINAENHANCE, value);
#endif
}
// jhkang ]]

/* 2010.5.27 ikari */
int SDL_WizRotateYUV(unsigned char *src,unsigned char *dst,SDL_Rect *rect,int srcstride)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
	unsigned int w=rect->w;
	unsigned int h=rect->h;
	unsigned int mod,ww,hh;

	if((w<4)||(h<4)||((srcstride&4)>0))
	{
		unsigned char *s=src;
		unsigned char *d=dst;
		hh=h;
		do
		{
			do
			{
				*d++=*s;
				s+=srcstride;
			}
			while(--hh);
			s=++src;
			dst-=4096;
			d=dst;
			hh=h;
		}
		while(--w);
		return 0;
	}

	mod=((unsigned int)src)&3;
	if(mod>0)
	{
		unsigned char *srcpixel=src;
		unsigned char *dstpixel=dst;
		unsigned char *s=srcpixel;
		unsigned char *d=dstpixel;

		mod=4-mod;
		ww=mod;
		hh=h;
		do
		{
			do
			{
				*d++=*s;
				s+=srcstride;
			}
			while(--hh);
			s=++srcpixel;
			dstpixel-=4096;
			d=dstpixel;
			hh=h;
		}
		while(--ww);
		w-=mod;
		src+=mod;
		dst-=(mod*4096);
	}

	mod=((unsigned int)dst)&3;
	if(mod>0)
	{
		unsigned char *srcpixel=src;
		unsigned char *dstpixel=dst;
		unsigned char *s=srcpixel;
		unsigned char *d=dstpixel;
		
		mod=4-mod;
		ww=w;
		hh=mod;
		do
		{
			do
			{
				*d++=*s;
				s+=srcstride;
			}
			while(--hh);
			s=++srcpixel;
			dstpixel-=4096;
			d=dstpixel;
			hh=mod;
		}
		while(--ww);
		h-=mod;
		src+=(mod*srcstride);
		dst+=mod;
	}

	mod=w&3;
	if(mod>0)
	{
		unsigned int pos=w-mod;
		unsigned char *srcpixel=src+pos;
		unsigned char *dstpixel=dst-pos*4096;
		unsigned char *s=srcpixel;
		unsigned char *d=dstpixel;

		ww=mod;
		hh=h;
		do
		{
			do
			{
				*d++=*s;
				s+=srcstride;
			}
			while(--hh);
			s=++srcpixel;
			dstpixel-=4096;
			d=dstpixel;
			hh=h;
		}
		while(--ww);
		w-=mod;
	}

	if(w==0) return 0;

	mod=h&3;
	if(mod>0)
	{
		unsigned int pos=h-mod;
		unsigned char *srcpixel=src+srcstride*pos;
		unsigned char *dstpixel=dst+pos;
		unsigned char *s=srcpixel;
		unsigned char *d=dstpixel;

		ww=w;
		hh=mod;
		do
		{
			do
			{
				*d++=*s;
				s+=srcstride;
			}
			while(--hh);
			s=++srcpixel;
			dstpixel-=4096;
			d=dstpixel;
			hh=mod;
		}
		while(--ww);
		h-=mod;
	}

	if(h==0) return 0;

	{
		unsigned int *s=(unsigned int *)src;
		unsigned int *d=(unsigned int *)dst;
		unsigned int srcskip=srcstride>>2;

		w>>=2; h>>=2; hh=h;
		do
		{
			do
			{
				unsigned int s1=*(s+srcskip*3);
				unsigned int s2=*(s+srcskip*2);
				unsigned int s3=*(s+srcskip);
				unsigned int s4=*s;

				*d=(s1<<24)|((s2&0xFF)<<16)|((s3&0xFF)<<8)|(s4&0xFF);
				*(d-1024)=((s1&0xFF00)<<16)|((s2&0xFF00)<<8)|(s3&0xFF00)|((s4&0xFF00)>>8);
				*(d-2048)=((s1&0xFF0000)<<8)|(s2&0xFF0000)|((s3&0xFF0000)>>8)|((s4&0xFF0000)>>16);
				*(d-3072)=(s1&0xFF000000)|((s2&0xFF000000)>>8)|((s3&0xFF000000)>>16)|(s4>>24);

				s+=srcstride;
				d++;
			}
			while(--hh);
			src+=4;
			s=(unsigned int *)src;
			dst-=(4096*4);
			d=(unsigned int *)dst;
			hh=h;
		}
		while(--w);
	}

	return 0;

#else
	return 0;
#endif
}

int SDL_WizRotateBlitSurface(SDL_Surface *src, SDL_Surface *dst, SDL_Rect *rect)
{
#if SDL_VIDEO_DRIVER_GP2XWIZ
#ifdef GP2XWIZ_MPLAYER
	SDL_Rect srect,drect;
	Uint32 bpp=src->format->BytesPerPixel;
	Uint8 *srcpixel, *dstpixel;

	srect=*rect;
	drect.x=srect.y;
	drect.y=319-srect.x;
	drect.w=srect.h;
	drect.h=srect.w;

	if(SDL_MUSTLOCK(src))
	{
		if(SDL_LockSurface(src)<0)
			return -1;
	}

	srcpixel=(Uint8*)src->pixels+srect.y*src->pitch+srect.x*bpp;
	dstpixel=(Uint8*)dst->pixels+drect.y*dst->pitch+drect.x*bpp;
	wiz_rotateblitsurface16((unsigned short*)srcpixel,(unsigned short*)dstpixel,&srect);

	if(SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);
	
	return 0;
#else
	return 0;
#endif
#else
	return 0;
#endif
}
/* ********************************************************************************** */

char *SDL_VideoDriverName(char *namebuf, int maxlen)
{
	if ( current_video != NULL ) {
		SDL_strlcpy(namebuf, current_video->name, maxlen);
		return(namebuf);
	}
	return(NULL);
}

/*
 * Get the current display surface
 */
SDL_Surface *SDL_GetVideoSurface(void)
{
	SDL_Surface *visible;

	visible = NULL;
	if ( current_video ) {
		visible = current_video->visible;
	}
	return(visible);
}

/*
 * Get the current information about the video hardware
 */
const SDL_VideoInfo *SDL_GetVideoInfo(void)
{
	const SDL_VideoInfo *info;

	info = NULL;
	if ( current_video ) {
		info = &current_video->info;
	}
	return(info);
}

/*
 * Return a pointer to an array of available screen dimensions for the
 * given format, sorted largest to smallest.  Returns NULL if there are
 * no dimensions available for a particular format, or (SDL_Rect **)-1
 * if any dimension is okay for the given format.  If 'format' is NULL,
 * the mode list will be for the format given by SDL_GetVideoInfo()->vfmt
 */
SDL_Rect ** SDL_ListModes (SDL_PixelFormat *format, Uint32 flags)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;
	SDL_Rect **modes;

	modes = NULL;
	if ( SDL_VideoSurface ) {
		if ( format == NULL ) {
			format = SDL_VideoSurface->format;
		}
		modes = video->ListModes(this, format, flags);
	}
	return(modes);
}

/*
 * Check to see if a particular video mode is supported.
 * It returns 0 if the requested mode is not supported under any bit depth,
 * or returns the bits-per-pixel of the closest available mode with the
 * given width and height.  If this bits-per-pixel is different from the
 * one used when setting the video mode, SDL_SetVideoMode() will succeed,
 * but will emulate the requested bits-per-pixel with a shadow surface.
 */
static Uint8 SDL_closest_depths[4][8] = {
	/* 8 bit closest depth ordering */
	{ 0, 8, 16, 15, 32, 24, 0, 0 },
	/* 15,16 bit closest depth ordering */
	{ 0, 16, 15, 32, 24, 8, 0, 0 },
	/* 24 bit closest depth ordering */
	{ 0, 24, 32, 16, 15, 8, 0, 0 },
	/* 32 bit closest depth ordering */
	{ 0, 32, 16, 15, 24, 8, 0, 0 }
};


#ifdef __MACOS__ /* MPW optimization bug? */
#define NEGATIVE_ONE 0xFFFFFFFF
#else
#define NEGATIVE_ONE -1
#endif

int SDL_VideoModeOK (int width, int height, int bpp, Uint32 flags)
{
	int table, b, i;
	int supported;
	SDL_PixelFormat format;
	SDL_Rect **sizes;

	/* Currently 1 and 4 bpp are not supported */
	if ( bpp < 8 || bpp > 32 ) {
		return(0);
	}
	if ( (width <= 0) || (height <= 0) ) {
		return(0);
	}

	/* Search through the list valid of modes */
	SDL_memset(&format, 0, sizeof(format));
	supported = 0;
	table = ((bpp+7)/8)-1;
	SDL_closest_depths[table][0] = bpp;
	SDL_closest_depths[table][7] = 0;
	for ( b = 0; !supported && SDL_closest_depths[table][b]; ++b ) {
		format.BitsPerPixel = SDL_closest_depths[table][b];
		sizes = SDL_ListModes(&format, flags);
		if ( sizes == (SDL_Rect **)0 ) {
			/* No sizes supported at this bit-depth */
			continue;
		} else 
		if (sizes == (SDL_Rect **)NEGATIVE_ONE) {
			/* Any size supported at this bit-depth */
			supported = 1;
			continue;
		} else if (current_video->handles_any_size) {
			/* Driver can center a smaller surface to simulate fullscreen */
			for ( i=0; sizes[i]; ++i ) {
				if ((sizes[i]->w >= width) && (sizes[i]->h >= height)) {
					supported = 1; /* this mode can fit the centered window. */
					break;
				}
			}
		} else
		for ( i=0; sizes[i]; ++i ) {
			if ((sizes[i]->w == width) && (sizes[i]->h == height)) {
				supported = 1;
				break;
			}
		}
	}
	if ( supported ) {
		--b;
		return(SDL_closest_depths[table][b]);
	} else {
		return(0);
	}
}

/*
 * Get the closest non-emulated video mode to the one requested
 */
static int SDL_GetVideoMode (int *w, int *h, int *BitsPerPixel, Uint32 flags)
{
	int table, b, i;
	int supported;
	int native_bpp;
	SDL_PixelFormat format;
	SDL_Rect **sizes;

	/* Check parameters */
	if ( *BitsPerPixel < 8 || *BitsPerPixel > 32 ) {
		SDL_SetError("Invalid bits per pixel (range is {8...32})");
		return(0);
	}
	if ((*w <= 0) || (*h <= 0)) {
		SDL_SetError("Invalid width or height");
		return(0);
	}

	/* Try the original video mode, get the closest depth */
	native_bpp = SDL_VideoModeOK(*w, *h, *BitsPerPixel, flags);
	if ( native_bpp == *BitsPerPixel ) {
		return(1);
	}
	if ( native_bpp > 0 ) {
		*BitsPerPixel = native_bpp;
		return(1);
	}

	/* No exact size match at any depth, look for closest match */
	SDL_memset(&format, 0, sizeof(format));
	supported = 0;
	table = ((*BitsPerPixel+7)/8)-1;
	SDL_closest_depths[table][0] = *BitsPerPixel;
	SDL_closest_depths[table][7] = SDL_VideoSurface->format->BitsPerPixel;
	for ( b = 0; !supported && SDL_closest_depths[table][b]; ++b ) {
		int best;

		format.BitsPerPixel = SDL_closest_depths[table][b];
		sizes = SDL_ListModes(&format, flags);
		if ( sizes == (SDL_Rect **)0 ) {
			/* No sizes supported at this bit-depth */
			continue;
		}
		best=0;
		for ( i=0; sizes[i]; ++i ) {
			/* Mode with both dimensions bigger or equal than asked ? */
			if ((sizes[i]->w >= *w) && (sizes[i]->h >= *h)) {
				/* Mode with any dimension smaller or equal than current best ? */
				if ((sizes[i]->w <= sizes[best]->w) || (sizes[i]->h <= sizes[best]->h)) {
					/* Now choose the mode that has less pixels */
					if ((sizes[i]->w * sizes[i]->h) <= (sizes[best]->w * sizes[best]->h)) {
						best=i;
						supported = 1;
					}
				}
			}
		}
		if (supported) {
			*w=sizes[best]->w;
			*h=sizes[best]->h;
			*BitsPerPixel = SDL_closest_depths[table][b];
		}
	}
	if ( ! supported ) {
		SDL_SetError("No video mode large enough for %dx%d", *w, *h);
	}
	return(supported);
}

/* This should probably go somewhere else -- like SDL_surface.c */
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.8 */
static void SDL_ClearSurface(SDL_Surface *surface)
{
	Uint32 black;

	black = SDL_MapRGB(surface->format, 0, 0, 0);
	SDL_FillRect(surface, NULL, black);
	if ((surface->flags&SDL_HWSURFACE) && (surface->flags&SDL_DOUBLEBUF)) {
		SDL_Flip(surface);
		SDL_FillRect(surface, NULL, black);
	}
	SDL_Flip(surface);
}
#endif

/*
 * Create a shadow surface suitable for fooling the app. :-)
 */
static void SDL_CreateShadowSurface(int depth)
{
	Uint32 Rmask, Gmask, Bmask;
#if SDL_VIDEO_DRIVER_GP2XWIZ 
	/* modified by ikari 2010.2.8 */
	SDL_VideoDevice* video=current_video;
	Uint32 Amask=0;
#endif

	/* Allocate the shadow surface */
	if ( depth == (SDL_VideoSurface->format)->BitsPerPixel ) {
		Rmask = (SDL_VideoSurface->format)->Rmask;
		Gmask = (SDL_VideoSurface->format)->Gmask;
		Bmask = (SDL_VideoSurface->format)->Bmask;
#if SDL_VIDEO_DRIVER_GP2XWIZ 
		Amask = (SDL_VideoSurface->format)->Amask;
#endif
	} else {
		Rmask = Gmask = Bmask = 0;
	}

#if SDL_VIDEO_DRIVER_GP2XWIZ 
	/* modified by ikari 2010.2.8 */
	if(!video->rotate_screen)
	{
		if((SDL_VideoSurface->flags & WIZ_UPPERSURFACE)!=WIZ_UPPERSURFACE)
		{
			SDL_ShadowSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
						SDL_VideoSurface->w, SDL_VideoSurface->h,
								depth, Rmask, Gmask, Bmask, Amask);
		}
		else
		{
			SDL_ShadowSurface = SDL_CreateRGBSurfaceFrom((void*)video->shadow_pixels,
						SDL_VideoSurface->w, SDL_VideoSurface->h,
								depth, video->shadow_pitch, Rmask, Gmask, Bmask, Amask);
		}
	}
	else
	{
		Uint8* pixels;
		Uint32 bpp=SDL_VideoSurface->format->BytesPerPixel;

		if((SDL_VideoSurface->flags & WIZ_UPPERSURFACE)!=WIZ_UPPERSURFACE)
		{
			video->shadow_pixels=malloc(0x12C00*bpp);
			if(!video->shadow_pixels) return;
			shadow_malloc=1;
		}

		pixels=(Uint8*)video->shadow_pixels+video->offset_x*video->shadow_pitch+
			(video->offset_y-video->width_odd)*bpp;

		SDL_ShadowSurface = SDL_CreateRGBSurfaceFrom((void*)pixels,
					SDL_VideoSurface->h, SDL_VideoSurface->w, 
						depth, video->shadow_pitch, Rmask, Gmask, Bmask, Amask);
	}

	/* modified by ikari 2010.2.7 */
	if ( SDL_ShadowSurface == NULL ) {
		if(video->shadow_pixels && shadow_malloc)
		{
			free(video->shadow_pixels);
			video->shadow_pixels=NULL;
			shadow_malloc=0;
		}
		return;
	}
#else
	SDL_ShadowSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				SDL_VideoSurface->w, SDL_VideoSurface->h,
						depth, Rmask, Gmask, Bmask, 0);
	if ( SDL_ShadowSurface == NULL ) {
		return;
	}
#endif
	/* 8-bit shadow surfaces report that they have exclusive palette */
	if ( SDL_ShadowSurface->format->palette ) {
		SDL_ShadowSurface->flags |= SDL_HWPALETTE;
		if ( depth == (SDL_VideoSurface->format)->BitsPerPixel ) {
			SDL_memcpy(SDL_ShadowSurface->format->palette->colors,
				SDL_VideoSurface->format->palette->colors,
				SDL_VideoSurface->format->palette->ncolors*
							sizeof(SDL_Color));
		} else {
			SDL_DitherColors(
			SDL_ShadowSurface->format->palette->colors, depth);
		}
	}

	/* If the video surface is resizable, the shadow should say so */
	if ( (SDL_VideoSurface->flags & SDL_RESIZABLE) == SDL_RESIZABLE ) {
		SDL_ShadowSurface->flags |= SDL_RESIZABLE;
	}
	/* If the video surface has no frame, the shadow should say so */
	if ( (SDL_VideoSurface->flags & SDL_NOFRAME) == SDL_NOFRAME ) {
		SDL_ShadowSurface->flags |= SDL_NOFRAME;
	}
	/* If the video surface is fullscreen, the shadow should say so */
	if ( (SDL_VideoSurface->flags & SDL_FULLSCREEN) == SDL_FULLSCREEN ) {
		SDL_ShadowSurface->flags |= SDL_FULLSCREEN;
	}
	/* If the video surface is flippable, the shadow should say so */
	if ( (SDL_VideoSurface->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF ) {
		SDL_ShadowSurface->flags |= SDL_DOUBLEBUF;
	}
#if SDL_VIDEO_DRIVER_GP2XWIZ 
	/* modified by ikari 2010.2.20 */
	if ( (SDL_VideoSurface->flags & WIZ_UPPERSURFACE) == WIZ_UPPERSURFACE ) {
		SDL_ShadowSurface->flags |= WIZ_UPPERSURFACE;
	}
	if ( (SDL_VideoSurface->flags & WIZ_TEARING) == WIZ_TEARING ) {
		SDL_ShadowSurface->flags |= WIZ_TEARING;
	}
	if ( (SDL_VideoSurface->flags & WIZ_240X320) == WIZ_240X320 ) {
		SDL_ShadowSurface->flags |= WIZ_240X320;
	}
	if ( (SDL_VideoSurface->flags & WIZ_VSYNCOFF) == WIZ_VSYNCOFF ) {
		SDL_ShadowSurface->flags |= WIZ_VSYNCOFF;
	}
#endif
	return;
}

#ifdef __QNXNTO__
    #include <sys/neutrino.h>
#endif /* __QNXNTO__ */

/*
 * Set the requested video mode, allocating a shadow buffer if necessary.
 */
SDL_Surface * SDL_SetVideoMode (int width, int height, int bpp, Uint32 flags)
{
	SDL_VideoDevice *video, *this;
	SDL_Surface *prev_mode, *mode;
	int video_w;
	int video_h;
	int video_bpp;
	int is_opengl;
	SDL_GrabMode saved_grab;

	/* Start up the video driver, if necessary..
	   WARNING: This is the only function protected this way!
	 */
	if ( ! current_video ) {
		if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
			return(NULL);
		}
	}
	this = video = current_video;

#if SDL_VIDEO_DRIVER_GP2XWIZ 
#if SDL_VIDEO_OPENGL /* modified by ikari 2010.2.12 */
	if(( flags & SDL_OPENGL ) == SDL_OPENGL )
		SDL_ShowCursor(SDL_DISABLE);
#endif
#endif

	/* Default to the current width and height */
	if ( width == 0 ) {
#if SDL_VIDEO_DRIVER_GP2XWIZ 
		/* modified by ikari 2010.2.8 */
		width = 320;
#else
		width = video->info.current_w;
#endif
	}
	if ( height == 0 ) {
#if SDL_VIDEO_DRIVER_GP2XWIZ
		/* modified by ikari 2010.2.8 */
		height = 240;
#else
		height = video->info.current_h; 
#endif
	}
	/* Default to the current video bpp */
	if ( bpp == 0 ) {
		flags |= SDL_ANYFORMAT;
		bpp = SDL_VideoSurface->format->BitsPerPixel;
	}

#if SDL_VIDEO_DRIVER_GP2XWIZ 
	/* modified by ikari 2010.1.23 */
	if(width>240)
		flags &= ~WIZ_240X320;
	if(height>240)
		flags |= WIZ_240X320;

	if((flags & WIZ_240X320)==WIZ_240X320)
		flags &= ~WIZ_TEARING;
	else
	{
		if(((flags & SDL_OPENGL)!=SDL_OPENGL)&&((flags & WIZ_TEARING)!=WIZ_TEARING))
			video->rotate_screen=1;
	}

	/* modified by ikari 2010.2.8 */
	if((bpp!=8)&&(bpp!=16)&&(bpp!=32))
	{
		if((flags&WIZ_240X320)!=WIZ_240X320)
		{
			flags |= WIZ_TEARING;
			video->rotate_screen=0;
		}
	}

	/* Determine TV-out mode */
	this->tvout_mode=0;
	if(DPCCTRL0 & 0x8000) 
		this->tvout_mode |= MLC_PRI_ENABLED;
	if(DPCCTRL0_TV & 0x8000) 
	{
		this->tvout_mode |= MLC_SEC_ENABLED;
		if((flags & WIZ_240X320)==WIZ_240X320)
		{
			SDL_SetError("Not supported screen mode on TV-out.");
			return(NULL);
		}
		else if(video->rotate_screen)
		{
			flags |= WIZ_TEARING;
			video->rotate_screen=0;
		}
	}
#endif

	/* Get a good video mode, the closest one possible */
#if SDL_VIDEO_DRIVER_GP2XWIZ 
	/* modified by ikari 2010.1.23 */
	if((( flags & SDL_OPENGL ) == SDL_OPENGL )||!video->rotate_screen)
	{
		video_w = width;
		video_h = height;
		SDL_BlitShadowToScreen=Wiz_BlitShadowToScreen;
	}
	else
	{
		video_w = height;
		video_h = width;
		SDL_BlitShadowToScreen=Wiz_RotateBlitShadowToScreen;
	}
#else
	video_w = width;
	video_h = height;
#endif
	video_bpp = bpp;
	if ( ! SDL_GetVideoMode(&video_w, &video_h, &video_bpp, flags) ) {
		return(NULL);
	}
	//printf("[2]SDL_SetVideoMode(width=%d, height=%d, bpp=%d)\n", width, height, bpp);
	/* Check the requested flags */
	/* There's no palette in > 8 bits-per-pixel mode */
	if ( video_bpp > 8 ) {
		flags &= ~SDL_HWPALETTE;
	}
#if 0
	if ( (flags&SDL_FULLSCREEN) != SDL_FULLSCREEN ) {
		/* There's no windowed double-buffering */
		flags &= ~SDL_DOUBLEBUF;
	}
#endif
#if 0 /* modified by ikari 2010.2.28 */
	if ( (flags&SDL_DOUBLEBUF) == SDL_DOUBLEBUF ) {
		/* Use hardware surfaces when double-buffering */
		flags |= SDL_HWSURFACE;
	}
#endif
	if(video->rotate_screen) {
		/* No SDL_HWSURFACE on 320x240 rotation mode */
		flags &= ~SDL_HWSURFACE;
	}

	is_opengl = ( ( flags & SDL_OPENGL ) == SDL_OPENGL );
	if ( is_opengl ) {
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.21 */
		flags &= ~(SDL_HWSURFACE|WIZ_UPPERSURFACE|SDL_DOUBLEBUF);
#else
		/* These flags are for 2D video modes only */
		flags &= ~(SDL_HWSURFACE|SDL_DOUBLEBUF);
#endif
	}

	/* Reset the keyboard here so event callbacks can run */
	SDL_ResetKeyboard();
	SDL_ResetMouse();
	SDL_cursorstate &= ~CURSOR_USINGSW;

	/* Clean up any previous video mode */
	/* modified by ikari 2010.1.23 */
#if SDL_VIDEO_DRIVER_GP2XWIZ
#ifdef GP2XWIZ_ROTATIONBUFFER
	if(video->rotation_buffer!=NULL)
	{
		SDL_FreeSurface(video->rotation_buffer);
		video->rotation_buffer=NULL;
	}
#endif
	if(video->shadow_pixels && shadow_malloc)
	{
		free(video->shadow_pixels);
		video->shadow_pixels=NULL;
		shadow_malloc=0;
	}
#endif
	if ( SDL_PublicSurface != NULL ) {
		SDL_PublicSurface = NULL;
	}
	if ( SDL_ShadowSurface != NULL ) {
		SDL_Surface *ready_to_go;
		ready_to_go = SDL_ShadowSurface;
		SDL_ShadowSurface = NULL;
		SDL_FreeSurface(ready_to_go);
	}
	if ( video->physpal ) {
		SDL_free(video->physpal->colors);
		SDL_free(video->physpal);
		video->physpal = NULL;
	}
	if( video->gammacols) {
		SDL_free(video->gammacols);
		video->gammacols = NULL;
	}

	/* Save the previous grab state and turn off grab for mode switch */
	saved_grab = SDL_WM_GrabInputOff();

	/* Try to set the video mode, along with offset and clipping */
	prev_mode = SDL_VideoSurface;
	SDL_LockCursor();
	SDL_VideoSurface = NULL;	/* In case it's freed by driver */
#if SDL_VIDEO_DRIVER_GP2XWIZ
#if SDL_VIDEO_OPENGL /* modified by ikari 2010.2.12 */
	if(is_opengl)
		mode = video->SetVideoMode(this, prev_mode,video_w,video_h,video_bpp,flags);
	else
#endif
		mode = video->SetVideoMode(this, prev_mode,width,height,video_bpp,flags);
#else
	mode = video->SetVideoMode(this, prev_mode,video_w,video_h,video_bpp,flags);
#endif

	if ( mode ) { /* Prevent resize events from mode change */
          /* But not on OS/2 */
#ifndef __OS2__
	    SDL_PrivateResize(mode->w, mode->h);
#endif

	    /* Sam - If we asked for OpenGL mode, and didn't get it, fail */
	    if ( is_opengl && !(mode->flags & SDL_OPENGL) ) {
		mode = NULL;
		SDL_SetError("OpenGL not available");
	    }
	}
	/*
	 * rcg11292000
	 * If you try to set an SDL_OPENGL surface, and fail to find a
	 * matching  visual, then the next call to SDL_SetVideoMode()
	 * will segfault, since  we no longer point to a dummy surface,
	 * but rather NULL.
	 * Sam 11/29/00
	 * WARNING, we need to make sure that the previous mode hasn't
	 * already been freed by the video driver.  What do we do in
	 * that case?  Should we call SDL_VideoInit() again?
	 */
	SDL_VideoSurface = (mode != NULL) ? mode : prev_mode;
	
	if ( (mode != NULL) && (!is_opengl) ) {
		/* Sanity check */
#if SDL_VIDEO_DRIVER_GP2XWIZ
		/* modified by ikari 2010.1.23 */
		if(video->rotate_screen)
		{
			if ( (mode->w < height) || (mode->h < width) ) {
				SDL_SetError("Video mode smaller than requested");
				return(NULL);
			}
		}
		else
#endif
		{
			if ( (mode->w < width) || (mode->h < height) ) {
				SDL_SetError("Video mode smaller than requested");
				return(NULL);
			}
		}


		/* If we have a palettized surface, create a default palette */
		if ( mode->format->palette ) {
			SDL_PixelFormat *vf = mode->format;
			SDL_DitherColors(vf->palette->colors, vf->BitsPerPixel);
			video->SetColors(this, 0, vf->palette->ncolors,
			                           vf->palette->colors);
		}

		/* Clear the surface to black */
		video->offset_x = 0;
		video->offset_y = 0;
		mode->offset = 0;
		SDL_SetClipRect(mode, NULL);

		/* Now adjust the offsets to match the desired mode */
		/* modified by ikari 2010.1.23 */
#if SDL_VIDEO_DRIVER_GP2XWIZ
		if(video->rotate_screen)
		{
			int diff_x=mode->h-width;
			video->width_odd=((diff_x&1)==0)?0:1;
			video->offset_x = (mode->w-height)/2;
			video->offset_y = diff_x/2+video->width_odd;
			mode->offset = video->offset_y*mode->pitch +
					video->offset_x*mode->format->BytesPerPixel;
		}
		else
#else
		SDL_ClearSurface(mode);
#endif
		{
			video->offset_x = (mode->w-width)/2;
			video->offset_y = (mode->h-height)/2;
			mode->offset = video->offset_y*mode->pitch +
					video->offset_x*mode->format->BytesPerPixel;
		}

#ifdef DEBUG_VIDEO
	  fprintf(stderr,
		"Requested mode: %dx%dx%d, obtained mode %dx%dx%d (offset %d)\n",
			width, height, bpp,
			mode->w, mode->h, mode->format->BitsPerPixel, mode->offset);
#endif
#if SDL_VIDEO_DRIVER_GP2XWIZ
		/* modified by ikari 2010.1.23 */
		if(video->rotate_screen)
		{
			mode->w = height;
			mode->h = width;
		}
		else
#endif
		{
			mode->w = width;
			mode->h = height;
		}
		SDL_SetClipRect(mode, NULL);
	}

	SDL_ResetCursor();
	SDL_UnlockCursor();

	/* If we failed setting a video mode, return NULL... (Uh Oh!) */
	if ( mode == NULL ) {
		return(NULL);
	}

	/* If there is no window manager, set the SDL_NOFRAME flag */
	if ( ! video->info.wm_available ) {
		mode->flags |= SDL_NOFRAME;
	}

	/* Reset the mouse cursor and grab for new video mode */
	SDL_SetCursor(NULL);
	if ( video->UpdateMouse ) {
		video->UpdateMouse(this);
	}
	SDL_WM_GrabInput(saved_grab);
	SDL_GetRelativeMouseState(NULL, NULL); /* Clear first large delta */

#if SDL_VIDEO_OPENGL
	/* Load GL symbols (before MakeCurrent, where we need glGetString). */
	if ( flags & (SDL_OPENGL | SDL_OPENGLBLIT) ) {

#if defined(__QNXNTO__) && (_NTO_VERSION < 630)
#define __SDL_NOGETPROCADDR__
#elif defined(__MINT__)
#define __SDL_NOGETPROCADDR__
#endif
#ifdef __SDL_NOGETPROCADDR__
    #define SDL_PROC(ret,func,params) video->func=func;
#else
    #define SDL_PROC(ret,func,params) \
    do { \
        video->func = SDL_GL_GetProcAddress(#func); \
        if ( ! video->func ) { \
            SDL_SetError("Couldn't load GL function %s: %s\n", #func, SDL_GetError()); \
        return(NULL); \
        } \
    } while ( 0 );

#endif /* __SDL_NOGETPROCADDR__ */

#if SDL_VIDEO_DRIVER_GP2XWIZ
#include "SDL_glesfuncs.h"
#else
#include "SDL_glfuncs.h"
#endif
#undef SDL_PROC	
	}
#endif /* SDL_VIDEO_OPENGL */

	/* If we're running OpenGL, make the context current */
	if ( (video->screen->flags & SDL_OPENGL) &&
	      video->GL_MakeCurrent ) {
		if ( video->GL_MakeCurrent(this) < 0 ) {
			return(NULL);
		}
	}

	/* Set up a fake SDL surface for OpenGL "blitting" */
	if ( (flags & SDL_OPENGLBLIT) == SDL_OPENGLBLIT ) {
		/* Load GL functions for performing the texture updates */
#if SDL_VIDEO_OPENGL
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		/* Create a software surface for blitting */
		if (bpp == 16) 
		{
			video->is_32bit = 0;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height,  
				16,
				31 << 11,
				63 << 5,
				31,
				0
				);
		}
		else
		{
			video->is_32bit = 1;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height, 
				32, 
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000
				);
		}
		if ( ! SDL_VideoSurface ) {
			return(NULL);
		}
		SDL_VideoSurface->flags = mode->flags | SDL_OPENGLBLIT;

		/* Free the original video mode surface (is this safe?) */
		SDL_FreeSurface(mode);

		/* Set the surface completely opaque & white by default */
		SDL_memset( SDL_VideoSurface->pixels, 255, SDL_VideoSurface->h * SDL_VideoSurface->pitch );
		video->glGenTextures( 1, &video->texture );
		video->glBindTexture( GL_TEXTURE_2D, video->texture );
		video->glTexImage2D(
			GL_TEXTURE_2D,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
			256,
			256,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
			video->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
			NULL);
#else
		/* Create a software surface for blitting */
#ifdef GL_VERSION_1_2
		/* If the implementation either supports the packed pixels
		   extension, or implements the core OpenGL 1.2 API, it will
		   support the GL_UNSIGNED_SHORT_5_6_5 texture format.
		 */
		if ( (bpp == 16) &&
		     (SDL_strstr((const char *)video->glGetString(GL_EXTENSIONS), "GL_EXT_packed_pixels") ||
		     (SDL_atof((const char *)video->glGetString(GL_VERSION)) >= 1.2f))
		   ) {
			video->is_32bit = 0;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height,  
				16,
				31 << 11,
				63 << 5,
				31,
				0
				);
		}
		else
#endif /* OpenGL 1.2 */
		{
			video->is_32bit = 1;
			SDL_VideoSurface = SDL_CreateRGBSurface(
				flags, 
				width, 
				height, 
				32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000
#else
				0xFF000000,
				0x00FF0000,
				0x0000FF00,
				0x000000FF
#endif
				);
		}
		if ( ! SDL_VideoSurface ) {
			return(NULL);
		}
		SDL_VideoSurface->flags = mode->flags | SDL_OPENGLBLIT;

		/* Free the original video mode surface (is this safe?) */
		SDL_FreeSurface(mode);

		/* Set the surface completely opaque & white by default */
		SDL_memset( SDL_VideoSurface->pixels, 255, SDL_VideoSurface->h * SDL_VideoSurface->pitch );
		video->glGenTextures( 1, &video->texture );
		video->glBindTexture( GL_TEXTURE_2D, video->texture );
		video->glTexImage2D(
			GL_TEXTURE_2D,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
			256,
			256,
			0,
			video->is_32bit ? GL_RGBA : GL_RGB,
#ifdef GL_VERSION_1_2
			video->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
#else
			GL_UNSIGNED_BYTE,
#endif
			NULL);
#endif

		video->UpdateRects = SDL_GL_UpdateRectsLock;
#else
		SDL_SetError("Somebody forgot to #define SDL_VIDEO_OPENGL");
		return(NULL);
#endif
	}

	/* Create a shadow surface if necessary */
	/* There are three conditions under which we create a shadow surface:
		1.  We need a particular bits-per-pixel that we didn't get.
		2.  We need a hardware palette and didn't get one.
		3.  We need a software surface and got a hardware surface.
	*/
	if ( !(SDL_VideoSurface->flags & SDL_OPENGL) &&
	     (
	     (  !(flags&SDL_ANYFORMAT) &&
			(SDL_VideoSurface->format->BitsPerPixel != bpp)) ||
	     (   (flags&SDL_HWPALETTE) && 
				!(SDL_VideoSurface->flags&SDL_HWPALETTE)) ||
		/* If the surface is in hardware, video writes are visible
		   as soon as they are performed, so we need to buffer them
		 */
	     (   ((flags&SDL_HWSURFACE) == SDL_SWSURFACE) &&
				(SDL_VideoSurface->flags&SDL_HWSURFACE)) ||
	     (   (flags&SDL_DOUBLEBUF) &&
				(SDL_VideoSurface->flags&SDL_HWSURFACE) &&
				!(SDL_VideoSurface->flags&SDL_DOUBLEBUF))
	     ) ) {
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
#ifdef GP2XWIZ_ROTATIONBUFFER
		if(video->rotate_screen)
		{
			SDL_PixelFormat* format=SDL_VideoSurface->format;
			video->rotation_buffer=SDL_CreateRGBSurface(SDL_VideoSurface->flags & WIZ_UPPERSURFACE
								,240,320,bpp,format->Rmask,format->Gmask,format->Bmask,0);

			if(!video->rotation_buffer)
			{
				SDL_SetError("Couldn't create Wiz rotation buffer");
				return(NULL);
			}
		}
#endif
#endif
		SDL_CreateShadowSurface(bpp);
		if ( SDL_ShadowSurface == NULL ) {
			SDL_SetError("Couldn't create shadow surface");
			return(NULL);
		}
		SDL_PublicSurface = SDL_ShadowSurface;
	} else {
		SDL_PublicSurface = SDL_VideoSurface;
	}

	video->info.vfmt = SDL_VideoSurface->format;
	video->info.current_w = SDL_VideoSurface->w;
	video->info.current_h = SDL_VideoSurface->h;
	//printf("[3]SDL_SetVideoMode(width=%d, height=%d, bpp=%d)\n", width, height, bpp);
	/* We're done! */
	return(SDL_PublicSurface);
}

/* 
 * Convert a surface into the video pixel format.
 */
SDL_Surface * SDL_DisplayFormat (SDL_Surface *surface)
{
	Uint32 flags;

	if ( ! SDL_PublicSurface ) {
		SDL_SetError("No video mode has been set");
		return(NULL);
	}
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.20 */
	/* Set the flags appropriate for copying to display surface */
	if ((SDL_PublicSurface->flags&WIZ_UPPERSURFACE) == WIZ_UPPERSURFACE)
		flags = WIZ_UPPERSURFACE;
#else
	/* Set the flags appropriate for copying to display surface */
	if (((SDL_PublicSurface->flags&SDL_HWSURFACE) == SDL_HWSURFACE) && current_video->info.blit_hw)
		flags = SDL_HWSURFACE;
#endif
	else 
		flags = SDL_SWSURFACE;
#ifdef AUTORLE_DISPLAYFORMAT
	flags |= (surface->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA));
	flags |= SDL_RLEACCELOK;
#else
	flags |= surface->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCELOK);
#endif
	return(SDL_ConvertSurface(surface, SDL_PublicSurface->format, flags));
}

/*
 * Convert a surface into a format that's suitable for blitting to
 * the screen, but including an alpha channel.
 */
SDL_Surface *SDL_DisplayFormatAlpha(SDL_Surface *surface)
{
	SDL_PixelFormat *vf;
	SDL_PixelFormat *format;
	SDL_Surface *converted;
	Uint32 flags;
	/* default to ARGB8888 */
	Uint32 amask = 0xff000000;
	Uint32 rmask = 0x00ff0000;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x000000ff;

	if ( ! SDL_PublicSurface ) {
		SDL_SetError("No video mode has been set");
		return(NULL);
	}
	vf = SDL_PublicSurface->format;

	switch(vf->BytesPerPixel) {
	    case 2:
		/* For XGY5[56]5, use, AXGY8888, where {X, Y} = {R, B}.
		   For anything else (like ARGB4444) it doesn't matter
		   since we have no special code for it anyway */
		if ( (vf->Rmask == 0x1f) &&
		     (vf->Bmask == 0xf800 || vf->Bmask == 0x7c00)) {
			rmask = 0xff;
			bmask = 0xff0000;
		}
		break;

	    case 3:
	    case 4:
		/* Keep the video format, as long as the high 8 bits are
		   unused or alpha */
		if ( (vf->Rmask == 0xff) && (vf->Bmask == 0xff0000) ) {
			rmask = 0xff;
			bmask = 0xff0000;
		}
		break;

	    default:
		/* We have no other optimised formats right now. When/if a new
		   optimised alpha format is written, add the converter here */
		break;
	}
	format = SDL_AllocFormat(32, rmask, gmask, bmask, amask);
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.21 */
	if((SDL_PublicSurface->flags & WIZ_UPPERSURFACE)==WIZ_UPPERSURFACE)
		flags = SDL_PublicSurface->flags & WIZ_UPPERSURFACE;
	else
#endif
	flags = SDL_PublicSurface->flags & SDL_HWSURFACE;
	flags |= surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
	converted = SDL_ConvertSurface(surface, format, flags);
	SDL_FreeFormat(format);
	return(converted);
}

/*
 * Update a specific portion of the physical screen
 */
void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
	if ( screen ) {
		SDL_Rect rect;

		/* Perform some checking */
		if ( w == 0 )
			w = screen->w;
		if ( h == 0 )
			h = screen->h;
		if ( (int)(x+w) > screen->w )
			return;
		if ( (int)(y+h) > screen->h )
			return;

		/* Fill the rectangle */
		rect.x = (Sint16)x;
		rect.y = (Sint16)y;
		rect.w = (Uint16)w;
		rect.h = (Uint16)h;
		SDL_UpdateRects(screen, 1, &rect);
	}
}

#if SDL_VIDEO_DRIVER_GP2XWIZ
/* Modified/appended for GP2X Wiz */
/* GP2X Wiz용으로 추가/수정된 코드임 : 2010.1.23 */
/* Appended code : Wiz_BlitShadowToScreen() */
/* 플리핑 : SW적으로 더블버퍼링된 ShadowSurface의 내용을 최종적으로 Wiz 화면에 그린다. */
static int Wiz_RotateBlitShadowToScreen(SDL_Surface* src,SDL_Surface* dst,SDL_Rect* rect)
{
	SDL_Rect srect,drect;
	unsigned int bpp=src->format->BytesPerPixel;
	SDL_VideoDevice *video=current_video;
#ifdef GP2XWIZ_ROTATIONBUFFER
	SDL_Surface* rotation_buffer=video->rotation_buffer;
#endif

	/* Convert destrect */
	srect=*rect;

	srect.x+=(video->offset_y-video->width_odd);
	srect.y+=video->offset_x;

	drect.x=srect.y;
	drect.y=319-srect.x;
	drect.w=srect.h;
	drect.h=srect.w;

	if(SDL_MUSTLOCK(src))
	{
		if(SDL_LockSurface(src)<0)
			return -1;
	}

	/* Rotate 270 degree & blit shadow surface to video surface */
	/* Wiz 액정모드(세로)에 맞춰 갱신할 내용을을 변환된 좌표에 270도(좌로 90도) 회전하여 그린다. */
	if((dst->offset>0) || (srect.x!=0) || (srect.y!=0) || (srect.w!=320) || (srect.h!=240)) /* dirty */
	{
#ifdef GP2XWIZ_ROTATIONBUFFER
		unsigned int srcskip=src->pitch;
		unsigned int dstskip=rotation_buffer->pitch;
		unsigned char* srcpixel=(unsigned char*)video->shadow_pixels+srect.y*srcskip+srect.x*bpp;
		unsigned char* dstpixel=(unsigned char*)rotation_buffer->pixels+drect.y*dstskip+drect.x*bpp;
		unsigned int w,h;

		w=srect.w;
		h=srect.h*bpp;

		if(bpp==1)
			wiz_rotateblitsurface8(srcpixel,dstpixel,&srect);
		else
		{
			if(bpp==2)
				wiz_rotateblitsurface16((unsigned short*)srcpixel,(unsigned short*)dstpixel,&srect);
			else
				wiz_rotateblitsurface32((unsigned int*)srcpixel,(unsigned int*)dstpixel,&srect);
		}

		drect.y=320-srect.x-srect.w;
		srcskip=dstskip;
		dstskip=dst->pitch;
		srcpixel=(unsigned char*)rotation_buffer->pixels+drect.y*srcskip+drect.x*bpp;
		dstpixel=(unsigned char*)dst->pixels+drect.y*dstskip+drect.x*bpp;

		if(!video->mmuhack_type)
		{
			while(w--)
			{
				gp2x_memcpy(dstpixel,srcpixel,h);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
		else
		{
			while(w--)
			{
				memcpy(dstpixel,srcpixel,h);
				srcpixel+=srcskip;
				dstpixel+=dstskip;
			}
		}
#else
		unsigned char* srcpixel=(unsigned char*)video->shadow_pixels+srect.y*src->pitch+srect.x*bpp;
		unsigned char* dstpixel=(unsigned char*)dst->pixels+drect.y*dst->pitch+drect.x*bpp;

		if(bpp==1)
			wiz_rotateblitsurface8(srcpixel,dstpixel,&srect);
		else
		{
			if(bpp==2)
				wiz_rotateblitsurface16((unsigned short*)srcpixel,(unsigned short*)dstpixel,&srect);
			else
				wiz_rotateblitsurface32((unsigned int*)srcpixel,(unsigned int*)dstpixel,&srect);
		}
#endif
	}
	else
	{
#ifdef GP2XWIZ_ROTATIONBUFFER
		unsigned char* buffer=(unsigned char*)rotation_buffer->pixels+319*rotation_buffer->pitch;

		if(bpp==1)
			rotateblitfast8(src->pixels,buffer,320,240);
		else
		{
			if(bpp==2)
				rotateblitfast16_4x4(src->pixels,buffer,320,240);
			else
				rotateblitfast32_4x1(src->pixels,buffer,320,240);
		}

		if(!video->mmuhack_type)
			gp2x_memcpy(dst->pixels,rotation_buffer->pixels,320*240*bpp);
		else
			memcpy(dst->pixels,rotation_buffer->pixels,320*240*bpp);
#else
		unsigned char* dstpixel=(unsigned char*)dst->pixels+319*dst->pitch;

		if(bpp==1)
			rotateblitfast8(src->pixels,dstpixel,320,240);
		else 
		{
			if(bpp==2)
				rotateblitfast16_4x4(src->pixels,dstpixel,320,240);
			else
				rotateblitfast32_4x1(src->pixels,dstpixel,320,240);
		}
#endif
	}

	if(SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);
	
	return 0;
}

static int Wiz_BlitShadowToScreen(SDL_Surface* src,SDL_Surface* dst,SDL_Rect* rect)
{
	SDL_VideoDevice *video=current_video;

	if(SDL_MUSTLOCK(src))
	{
		if(SDL_LockSurface(src)<0)
			return -1;
	}

	if((dst->offset>0) || (rect->x!=0) || (rect->y!=0) || (rect->w!=dst->w) || (rect->h!=dst->h))
	{
		unsigned int bpp=src->format->BytesPerPixel;
		unsigned int srcskip=src->pitch,dstskip=dst->pitch;
		unsigned int w=rect->w*bpp,h=rect->h;
		unsigned char* srcpixel=(unsigned char*)src->pixels+rect->y*srcskip+rect->x*bpp;
		unsigned char* dstpixel=(unsigned char*)dst->pixels+dst->offset+rect->y*dstskip+rect->x*bpp;

		if(!video->mmuhack_type)
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
		if(!video->mmuhack_type)
			gp2x_memcpy(dst->pixels,src->pixels,dst->pitch*rect->h);
		else
			memcpy(dst->pixels,src->pixels,dst->pitch*rect->h);
	}

	if(SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);
	
	return 0;
}
#endif

void SDL_UpdateRects (SDL_Surface *screen, int numrects, SDL_Rect *rects)
{
	int i;
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;

	if ( (screen->flags & (SDL_OPENGL | SDL_OPENGLBLIT)) == SDL_OPENGL ) {
		SDL_SetError("OpenGL active, use SDL_GL_SwapBuffers()");
		return;
	}
	//printf("SDL_UpdateRects() BitsPerPixel=%d\n", SDL_ShadowSurface->format->BitsPerPixel);
	if ( screen == SDL_ShadowSurface ) {
		/* Blit the shadow surface using saved mapping */
		SDL_Palette *pal = screen->format->palette;
		SDL_Color *saved_colors = NULL;
		if ( pal && !(SDL_VideoSurface->flags & SDL_HWPALETTE) ) {
			/* simulated 8bpp, use correct physical palette */
			saved_colors = pal->colors;
			if ( video->gammacols ) {
				/* gamma-corrected palette */
				pal->colors = video->gammacols;
			} else if ( video->physpal ) {
				/* physical palette different from logical */
				pal->colors = video->physpal->colors;
			}
		}
		if ( SHOULD_DRAWCURSOR(SDL_cursorstate) ) {
			SDL_LockCursor();
			SDL_DrawCursor(SDL_ShadowSurface);
			/* modified by ikari 2010.1.23 */
#if SDL_VIDEO_DRIVER_GP2XWIZ
			for ( i=0; i<numrects; ++i ) {
				SDL_BlitShadowToScreen(SDL_ShadowSurface, 
						SDL_VideoSurface, &rects[i]);
			}
#else
			for ( i=0; i<numrects; ++i ) {
				SDL_LowerBlit(SDL_ShadowSurface, &rects[i], 
						SDL_VideoSurface, &rects[i]);
			}
#endif
			SDL_EraseCursor(SDL_ShadowSurface);
			SDL_UnlockCursor();
		} else {
#if SDL_VIDEO_DRIVER_GP2XWIZ
			/* modified by ikari 2010.1.23 */
			for ( i=0; i<numrects; ++i ) {
				SDL_BlitShadowToScreen(SDL_ShadowSurface, 
							SDL_VideoSurface, &rects[i]);
			}
#else
			for ( i=0; i<numrects; ++i ) {
				SDL_LowerBlit(SDL_ShadowSurface, &rects[i], 
						SDL_VideoSurface, &rects[i]);
			}
#endif
		}
		if ( saved_colors ) {
			pal->colors = saved_colors;
		}

		/* Fall through to video surface update */
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		screen = SDL_VideoSurface;
#endif
	}

	if ( screen == SDL_VideoSurface ) {
		/* Update the video surface */
		if ( screen->offset ) {
			for ( i=0; i<numrects; ++i ) {
				rects[i].x += video->offset_x;
				rects[i].y += video->offset_y;
			}
			video->UpdateRects(this, numrects, rects);
			for ( i=0; i<numrects; ++i ) {
				rects[i].x -= video->offset_x;
				rects[i].y -= video->offset_y;
			}
		} else {
			video->UpdateRects(this, numrects, rects);
		}
	}
}

/*
 * Performs hardware double buffering, if possible, or a full update if not.
 */
/* modified by ikari 2010.1.23 */
int SDL_Flip(SDL_Surface *screen)
{
	SDL_VideoDevice *video = current_video;
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
	unsigned int bpp=screen->format->BytesPerPixel;
	SDL_VideoDevice *this  = current_video;
#endif
	/* Copy the shadow surface to the video surface */

	if( screen == SDL_ShadowSurface )
	{
		SDL_Rect rect;
		SDL_Palette *pal = screen->format->palette;
		SDL_Color *saved_colors = NULL;
		if ( pal && !(SDL_VideoSurface->flags & SDL_HWPALETTE) ) {
			/* simulated 8bpp, use correct physical palette */
			saved_colors = pal->colors;
			if ( video->gammacols ) {
				/* gamma-corrected palette */
				pal->colors = video->gammacols;
			} else if ( video->physpal ) {
				/* physical palette different from logical */
				pal->colors = video->physpal->colors;
			}
		}

		rect.x = 0;
		rect.y = 0;
		rect.w = screen->w;
		rect.h = screen->h;
		if ( SHOULD_DRAWCURSOR(SDL_cursorstate) ) {
			SDL_LockCursor();
			SDL_DrawCursor(SDL_ShadowSurface);
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
			if((SDL_VideoSurface->flags & SDL_DOUBLEBUF) != SDL_DOUBLEBUF)
				SDL_BlitShadowToScreen(SDL_ShadowSurface,SDL_VideoSurface,&rect);
			else
				video->FlipHWSurface(this, SDL_VideoSurface);
#else
			SDL_LowerBlit(SDL_ShadowSurface, &rect, 
					SDL_VideoSurface, &rect);
#endif
			SDL_EraseCursor(SDL_ShadowSurface);
			SDL_UnlockCursor();
		} else {
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
			if((SDL_VideoSurface->flags & SDL_DOUBLEBUF) != SDL_DOUBLEBUF)
				SDL_BlitShadowToScreen(SDL_ShadowSurface,SDL_VideoSurface,&rect);
			else
				video->FlipHWSurface(this, SDL_VideoSurface);
#else
			SDL_LowerBlit(SDL_ShadowSurface, &rect, 
					SDL_VideoSurface, &rect);
#endif
		}
		if ( saved_colors ) {
			pal->colors = saved_colors;
		}

#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		/* Fall through to video surface update */
		screen = SDL_VideoSurface;
#else
		return(0);
#endif
	}

	if ( (screen->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF ) {
		SDL_VideoDevice *this  = current_video;
		return(video->FlipHWSurface(this, SDL_VideoSurface));
	} else {
		SDL_UpdateRect(screen, 0, 0, 0, 0);
	}

	return(0);
}

static void SetPalette_logical(SDL_Surface *screen, SDL_Color *colors,
			       int firstcolor, int ncolors)
{
	SDL_Palette *pal = screen->format->palette;
	SDL_Palette *vidpal;

	if ( colors != (pal->colors + firstcolor) ) {
		SDL_memcpy(pal->colors + firstcolor, colors,
		       ncolors * sizeof(*colors));
	}

	if ( current_video && SDL_VideoSurface ) {
		vidpal = SDL_VideoSurface->format->palette;
		if ( (screen == SDL_ShadowSurface) && vidpal ) {
			/*
			 * This is a shadow surface, and the physical
			 * framebuffer is also indexed. Propagate the
			 * changes to its logical palette so that
			 * updates are always identity blits
			 */
			SDL_memcpy(vidpal->colors + firstcolor, colors,
			       ncolors * sizeof(*colors));
		}
	}
	SDL_FormatChanged(screen);
}

static int SetPalette_physical(SDL_Surface *screen,
                               SDL_Color *colors, int firstcolor, int ncolors)
{
	SDL_VideoDevice *video = current_video;
	int gotall = 1;

	if ( video->physpal ) {
		/* We need to copy the new colors, since we haven't
		 * already done the copy in the logical set above.
		 */
		SDL_memcpy(video->physpal->colors + firstcolor,
		       colors, ncolors * sizeof(*colors));
	}
	if ( screen == SDL_ShadowSurface ) {
		if ( SDL_VideoSurface->flags & SDL_HWPALETTE ) {
			/*
			 * The real screen is also indexed - set its physical
			 * palette. The physical palette does not include the
			 * gamma modification, we apply it directly instead,
			 * but this only happens if we have hardware palette.
			 */
			screen = SDL_VideoSurface;
		} else {
			/*
			 * The video surface is not indexed - invalidate any
			 * active shadow-to-video blit mappings.
			 */
			if ( screen->map->dst == SDL_VideoSurface ) {
				SDL_InvalidateMap(screen->map);
			}
			if ( video->gamma ) {
				if( ! video->gammacols ) {
					SDL_Palette *pp = video->physpal;
					if(!pp)
						pp = screen->format->palette;
					video->gammacols = SDL_malloc(pp->ncolors
							  * sizeof(SDL_Color));
					SDL_ApplyGamma(video->gamma,
						       pp->colors,
						       video->gammacols,
						       pp->ncolors);
				} else {
					SDL_ApplyGamma(video->gamma, colors,
						       video->gammacols
						       + firstcolor,
						       ncolors);
				}
			}
			SDL_UpdateRect(screen, 0, 0, 0, 0);
		}
	}

	if ( screen == SDL_VideoSurface ) {
		SDL_Color gcolors[256];

		if ( video->gamma ) {
			SDL_ApplyGamma(video->gamma, colors, gcolors, ncolors);
			colors = gcolors;
		}
		gotall = video->SetColors(video, firstcolor, ncolors, colors);
		if ( ! gotall ) {
			/* The video flags shouldn't have SDL_HWPALETTE, and
			   the video driver is responsible for copying back the
			   correct colors into the video surface palette.
			*/
			;
		}
		SDL_CursorPaletteChanged();
	}
	return gotall;
}

/*
 * Set the physical and/or logical colormap of a surface:
 * Only the screen has a physical colormap. It determines what is actually
 * sent to the display.
 * The logical colormap is used to map blits to/from the surface.
 * 'which' is one or both of SDL_LOGPAL, SDL_PHYSPAL
 *
 * Return nonzero if all colours were set as requested, or 0 otherwise.
 */
int SDL_SetPalette(SDL_Surface *screen, int which,
		   SDL_Color *colors, int firstcolor, int ncolors)
{
	SDL_Palette *pal;
	int gotall;
	int palsize;

	if ( !screen ) {
		return 0;
	}
	if ( !current_video || screen != SDL_PublicSurface ) {
		/* only screens have physical palettes */
		which &= ~SDL_PHYSPAL;
	} else if ( (screen->flags & SDL_HWPALETTE) != SDL_HWPALETTE ) {
		/* hardware palettes required for split colormaps */
		which |= SDL_PHYSPAL | SDL_LOGPAL;
	}

	/* Verify the parameters */
	pal = screen->format->palette;
	if( !pal ) {
		return 0;	/* not a palettized surface */
	}
	gotall = 1;
	palsize = 1 << screen->format->BitsPerPixel;
	if ( ncolors > (palsize - firstcolor) ) {
		ncolors = (palsize - firstcolor);
		gotall = 0;
	}

	if ( which & SDL_LOGPAL ) {
		/*
		 * Logical palette change: The actual screen isn't affected,
		 * but the internal colormap is altered so that the
		 * interpretation of the pixel values (for blits etc) is
		 * changed.
		 */
		SetPalette_logical(screen, colors, firstcolor, ncolors);
	}
	if ( which & SDL_PHYSPAL ) {
		SDL_VideoDevice *video = current_video;
		/*
		 * Physical palette change: This doesn't affect the
		 * program's idea of what the screen looks like, but changes
		 * its actual appearance.
		 */
		if ( !video->physpal && !(which & SDL_LOGPAL) ) {
			/* Lazy physical palette allocation */
			int size;
			SDL_Palette *pp = SDL_malloc(sizeof(*pp));
			if ( !pp ) {
				return 0;
			}
			video->physpal = pp;
			pp->ncolors = pal->ncolors;
			size = pp->ncolors * sizeof(SDL_Color);
			pp->colors = SDL_malloc(size);
			if ( !pp->colors ) {
				return 0;
			}
			SDL_memcpy(pp->colors, pal->colors, size);
		}
		if ( ! SetPalette_physical(screen,
		                           colors, firstcolor, ncolors) ) {
			gotall = 0;
		}
	}
	return gotall;
}

int SDL_SetColors(SDL_Surface *screen, SDL_Color *colors, int firstcolor,
		  int ncolors)
{
	return SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL,
			      colors, firstcolor, ncolors);
}

/*
 * Clean up the video subsystem
 */
void SDL_VideoQuit (void)
{
	SDL_Surface *ready_to_go;

	if ( current_video ) {
		SDL_VideoDevice *video = current_video;
		SDL_VideoDevice *this  = current_video;

		/* Halt event processing before doing anything else */
		SDL_StopEventLoop();

		/* Clean up allocated window manager items */
		if ( SDL_PublicSurface ) {
			SDL_PublicSurface = NULL;
		}
		SDL_CursorQuit();

		/* Just in case... */
		SDL_WM_GrabInputOff();

#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		/* modified by ikari 2010.1.23 */
#ifdef GP2XWIZ_ROTATIONBUFFER
		if(video->rotation_buffer) 
		{
			SDL_FreeSurface(video->rotation_buffer);
			video->rotation_buffer=NULL;
		}
#endif
		if(video->shadow_pixels && shadow_malloc)
		{
			free(video->shadow_pixels);
			video->shadow_pixels=NULL;
			shadow_malloc=0;
		}

#if SDL_VIDEO_OPENGL
		if((SDL_VideoSurface->flags&SDL_OPENGLBLIT)==SDL_OPENGLBLIT)
			video->glDeleteTextures( 1, &video->texture );
#endif
#endif

		/* Clean up the system video */
		video->VideoQuit(this);

		/* Free any lingering surfaces */
		ready_to_go = SDL_ShadowSurface;
		SDL_ShadowSurface = NULL;
		SDL_FreeSurface(ready_to_go);
		if ( SDL_VideoSurface != NULL ) {
			ready_to_go = SDL_VideoSurface;
			SDL_VideoSurface = NULL;
			SDL_FreeSurface(ready_to_go);
		}
		SDL_PublicSurface = NULL;

		/* Clean up miscellaneous memory */
		if ( video->physpal ) {
			SDL_free(video->physpal->colors);
			SDL_free(video->physpal);
			video->physpal = NULL;
		}
		if ( video->gammacols ) {
			SDL_free(video->gammacols);
			video->gammacols = NULL;
		}
		if ( video->gamma ) {
			SDL_free(video->gamma);
			video->gamma = NULL;
		}
		if ( video->wm_title != NULL ) {
			SDL_free(video->wm_title);
			video->wm_title = NULL;
		}
		if ( video->wm_icon != NULL ) {
			SDL_free(video->wm_icon);
			video->wm_icon = NULL;
		}

		/* Finish cleaning up video subsystem */
		video->free(this);

		current_video = NULL;
	}

	return;
}

/* Load the GL driver library */
int SDL_GL_LoadLibrary(const char *path)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;
	int retval;

	retval = -1;
	if ( video == NULL ) {
		SDL_SetError("Video subsystem has not been initialized");
	} else {
		if ( video->GL_LoadLibrary ) {
			retval = video->GL_LoadLibrary(this, path);
		} else {
			SDL_SetError("No dynamic GL support in video driver");
		}
	}
	return(retval);
}

void *SDL_GL_GetProcAddress(const char* proc)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;
	void *func;

	func = NULL;
	if ( video->GL_GetProcAddress ) {
		if ( video->gl_config.driver_loaded ) {
			func = video->GL_GetProcAddress(this, proc);
		} else {
			SDL_SetError("No GL driver has been loaded");
		}
	} else {
		SDL_SetError("No dynamic GL support in video driver");
	}
	return func;
}

/* Set the specified GL attribute for setting up a GL video mode */
int SDL_GL_SetAttribute( SDL_GLattr attr, int value )
{
	int retval;
	SDL_VideoDevice *video = current_video;

	retval = 0;
	switch (attr) {
		case SDL_GL_RED_SIZE:
			video->gl_config.red_size = value;
			break;
		case SDL_GL_GREEN_SIZE:
			video->gl_config.green_size = value;
			break;
		case SDL_GL_BLUE_SIZE:
			video->gl_config.blue_size = value;
			break;
		case SDL_GL_ALPHA_SIZE:
			video->gl_config.alpha_size = value;
			break;
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		case SDL_GL_DOUBLEBUFFER:
			video->gl_config.double_buffer = value;
			break;
#endif
		case SDL_GL_BUFFER_SIZE:
			video->gl_config.buffer_size = value;
			break;
		case SDL_GL_DEPTH_SIZE:
			video->gl_config.depth_size = value;
			break;
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		case SDL_GL_STENCIL_SIZE:
			video->gl_config.stencil_size = value;
			break;
		case SDL_GL_ACCUM_RED_SIZE:
			video->gl_config.accum_red_size = value;
			break;
		case SDL_GL_ACCUM_GREEN_SIZE:
			video->gl_config.accum_green_size = value;
			break;
		case SDL_GL_ACCUM_BLUE_SIZE:
			video->gl_config.accum_blue_size = value;
			break;
		case SDL_GL_ACCUM_ALPHA_SIZE:
			video->gl_config.accum_alpha_size = value;
			break;
		case SDL_GL_STEREO:
			video->gl_config.stereo = value;
			break;
		case SDL_GL_MULTISAMPLEBUFFERS:
			video->gl_config.multisamplebuffers = value;
			break;
		case SDL_GL_MULTISAMPLESAMPLES:
			video->gl_config.multisamplesamples = value;
			break;
		case SDL_GL_ACCELERATED_VISUAL:
			video->gl_config.accelerated = value;
			break;
#endif
		case SDL_GL_SWAP_CONTROL:
			video->gl_config.swap_control = value;
			break;
		default:
			SDL_SetError("Unknown OpenGL attribute");
			retval = -1;
			break;
	}
	return(retval);
}

/* Retrieve an attribute value from the windowing system. */
int SDL_GL_GetAttribute(SDL_GLattr attr, int* value)
{
	int retval = -1;
	SDL_VideoDevice* video = current_video;
	SDL_VideoDevice* this = current_video;

	if ( video->GL_GetAttribute ) {
		retval = this->GL_GetAttribute(this, attr, value);
	} else {
		*value = 0;
		SDL_SetError("GL_GetAttribute not supported");
	}
	return retval;
}

/* Perform a GL buffer swap on the current GL context */
void SDL_GL_SwapBuffers(void)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this = current_video;

	if ( video->screen->flags & SDL_OPENGL ) {
		video->GL_SwapBuffers(this);
	} else {
		SDL_SetError("OpenGL video mode has not been set");
	}
}

/* Update rects with locking */
void SDL_GL_UpdateRectsLock(SDL_VideoDevice* this, int numrects, SDL_Rect *rects)
{
	SDL_GL_Lock();
 	SDL_GL_UpdateRects(numrects, rects);
	SDL_GL_Unlock();
}

/* Update rects without state setting and changing (the caller is responsible for it) */
void SDL_GL_UpdateRects(int numrects, SDL_Rect *rects)
{
#if SDL_VIDEO_OPENGL
	SDL_VideoDevice *this = current_video;
	SDL_Rect update, tmp;
	int x, y, i;
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
	GLfloat texcoord[8],vertices[8];
#endif

	for ( i = 0; i < numrects; i++ )
	{
		tmp.y = rects[i].y;
		tmp.h = rects[i].h;
		for ( y = 0; y <= rects[i].h / 256; y++ )
		{
			tmp.x = rects[i].x;
			tmp.w = rects[i].w;
			for ( x = 0; x <= rects[i].w / 256; x++ )
			{
				update.x = tmp.x;
				update.y = tmp.y;
				update.w = tmp.w;
				update.h = tmp.h;

				if ( update.w > 256 )
					update.w = 256;

				if ( update.h > 256 )
					update.h = 256;
			
				this->glFlush();
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
				this->glTexSubImage2D( 
					GL_TEXTURE_2D, 
					0, 
					0, 
					0, 
					update.w, 
					update.h, 
					this->is_32bit? GL_RGBA : GL_RGB,
					this->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
					(Uint8 *)this->screen->pixels + 
						this->screen->format->BytesPerPixel * update.x + 
						update.y * this->screen->pitch );
	
				this->glFlush();

				vertices[0]=(GLfloat)update.x;
				vertices[1]=(GLfloat)update.y;
				vertices[2]=(GLfloat)(update.x + update.w);
				vertices[3]=(GLfloat)update.y;
				vertices[4]=(GLfloat)update.x;
				vertices[5]=(GLfloat)(update.y + update.h);
				vertices[6]=(GLfloat)(update.x + update.w);
				vertices[7]=(GLfloat)(update.y + update.h);

				texcoord[0]=(GLfloat)0.0;
				texcoord[1]=(GLfloat)0.0;
				texcoord[2]=(GLfloat)(update.w / 256.0);
				texcoord[3]=(GLfloat)0.0;
				texcoord[4]=(GLfloat)0.0;
				texcoord[5]=(GLfloat)(update.h / 256.0);
				texcoord[6]=(GLfloat)(update.w / 256.0);
				texcoord[7]=(GLfloat)(update.h / 256.0);

				this->glVertexPointer(2,GL_FLOAT,0,vertices);
				this->glTexCoordPointer(2,GL_FLOAT,0,texcoord);
				this->glDrawArrays(GL_TRIANGLE_STRIP,0,4);
#else
				this->glTexSubImage2D( 
					GL_TEXTURE_2D, 
					0, 
					0, 
					0, 
					update.w, 
					update.h, 
					this->is_32bit? GL_RGBA : GL_RGB,
#ifdef GL_VERSION_1_2
					this->is_32bit ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5,
#else
					GL_UNSIGNED_BYTE,
#endif

					(Uint8 *)this->screen->pixels + 
						this->screen->format->BytesPerPixel * update.x + 
						update.y * this->screen->pitch );
	
				this->glFlush();
				/*
				* Note the parens around the function name:
				* This is because some OpenGL implementations define glTexCoord etc 
				* as macros, and we don't want them expanded here.
				*/
				this->glBegin(GL_TRIANGLE_STRIP);
					(this->glTexCoord2f)( 0.0, 0.0 );	
					(this->glVertex2i)( update.x, update.y );
					(this->glTexCoord2f)( (float)(update.w / 256.0), 0.0 );	
					(this->glVertex2i)( update.x + update.w, update.y );
					(this->glTexCoord2f)( 0.0, (float)(update.h / 256.0) );
					(this->glVertex2i)( update.x, update.y + update.h );
					(this->glTexCoord2f)( (float)(update.w / 256.0), (float)(update.h / 256.0) );	
					(this->glVertex2i)( update.x + update.w	, update.y + update.h );
				this->glEnd();
#endif

				tmp.x += 256;
				tmp.w -= 256;
			}
			tmp.y += 256;
			tmp.h -= 256;
		}
	}
#endif
}

/* Lock == save current state */
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
#define _INT_TO_FIXED(i) ((GLfixed)((i)<<16))
#endif

void SDL_GL_Lock()
{
#if SDL_VIDEO_OPENGL
	lock_count--;
	if (lock_count==-1)
	{
		SDL_VideoDevice *this = current_video;

#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		this->glPushAttrib( GL_ALL_ATTRIB_BITS );	/* TODO: narrow range of what is saved */
#ifdef GL_CLIENT_PIXEL_STORE_BIT
		this->glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
#endif
#endif

		this->glEnable(GL_TEXTURE_2D);
		this->glEnable(GL_BLEND);
		this->glDisable(GL_FOG);
		this->glDisable(GL_ALPHA_TEST);
		this->glDisable(GL_DEPTH_TEST);
		this->glDisable(GL_SCISSOR_TEST);
#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		this->glDisable(GL_STENCIL_TEST);
#endif
		this->glDisable(GL_CULL_FACE);

#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		this->glEnableClientState(GL_VERTEX_ARRAY);
		this->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

		this->glBindTexture( GL_TEXTURE_2D, this->texture );
		this->glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		this->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		this->glPixelStorei( 0, this->screen->pitch / this->screen->format->BytesPerPixel );
		this->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		(this->glColor4x)(_INT_TO_FIXED(1), _INT_TO_FIXED(1), _INT_TO_FIXED(1), _INT_TO_FIXED(1));
#else
		(this->glColor4f)(1.0, 1.0, 1.0, 1.0);		/* Solaris workaround */
#endif

		this->glViewport(0, 0, this->screen->w, this->screen->h);
		this->glMatrixMode(GL_PROJECTION);
		this->glPushMatrix();
		this->glLoadIdentity();

#if SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		this->glOrthox(_INT_TO_FIXED(0), _INT_TO_FIXED(this->screen->w), _INT_TO_FIXED(this->screen->h), 
				_INT_TO_FIXED(0), _INT_TO_FIXED(0), _INT_TO_FIXED(1));
#else
		this->glOrtho(0.0, (GLdouble) this->screen->w, (GLdouble) this->screen->h, 0.0, 0.0, 1.0);
#endif

		this->glMatrixMode(GL_MODELVIEW);
		this->glPushMatrix();
		this->glLoadIdentity();
	}
#endif
}

/* Unlock == restore saved state */
void SDL_GL_Unlock()
{
#if SDL_VIDEO_OPENGL
	lock_count++;
	if (lock_count==0)
	{
		SDL_VideoDevice *this = current_video;

		this->glPopMatrix();
		this->glMatrixMode(GL_PROJECTION);
		this->glPopMatrix();

#if !SDL_VIDEO_DRIVER_GP2XWIZ /* modified by ikari 2010.2.15 */
		this->glPopClientAttrib();
		this->glPopAttrib();
#endif
	}
#endif
}

/*
 * Sets/Gets the title and icon text of the display window, if any.
 */
void SDL_WM_SetCaption (const char *title, const char *icon)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;

	if ( video ) {
		if ( title ) {
			if ( video->wm_title ) {
				SDL_free(video->wm_title);
			}
			video->wm_title = SDL_strdup(title);
		}
		if ( icon ) {
			if ( video->wm_icon ) {
				SDL_free(video->wm_icon);
			}
			video->wm_icon = SDL_strdup(icon);
		}
		if ( (title || icon) && (video->SetCaption != NULL) ) {
			video->SetCaption(this, video->wm_title,video->wm_icon);
		}
	}
}
void SDL_WM_GetCaption (char **title, char **icon)
{
	SDL_VideoDevice *video = current_video;

	if ( video ) {
		if ( title ) {
			*title = video->wm_title;
		}
		if ( icon ) {
			*icon = video->wm_icon;
		}
	}
}

/* Utility function used by SDL_WM_SetIcon();
 * flags & 1 for color key, flags & 2 for alpha channel. */
static void CreateMaskFromColorKeyOrAlpha(SDL_Surface *icon, Uint8 *mask, int flags)
{
	int x, y;
	Uint32 colorkey;
#define SET_MASKBIT(icon, x, y, mask) \
	mask[(y*((icon->w+7)/8))+(x/8)] &= ~(0x01<<(7-(x%8)))

	colorkey = icon->format->colorkey;
	switch (icon->format->BytesPerPixel) {
		case 1: { Uint8 *pixels;
			for ( y=0; y<icon->h; ++y ) {
				pixels = (Uint8 *)icon->pixels + y*icon->pitch;
				for ( x=0; x<icon->w; ++x ) {
					if ( *pixels++ == colorkey ) {
						SET_MASKBIT(icon, x, y, mask);
					}
				}
			}
		}
		break;

		case 2: { Uint16 *pixels;
			for ( y=0; y<icon->h; ++y ) {
				pixels = (Uint16 *)icon->pixels +
				                   y*icon->pitch/2;
				for ( x=0; x<icon->w; ++x ) {
					if ( (flags & 1) && *pixels == colorkey ) {
						SET_MASKBIT(icon, x, y, mask);
					} else if((flags & 2) && (*pixels & icon->format->Amask) == 0) {
						SET_MASKBIT(icon, x, y, mask);
					}
					pixels++;
				}
			}
		}
		break;

		case 4: { Uint32 *pixels;
			for ( y=0; y<icon->h; ++y ) {
				pixels = (Uint32 *)icon->pixels +
				                   y*icon->pitch/4;
				for ( x=0; x<icon->w; ++x ) {
					if ( (flags & 1) && *pixels == colorkey ) {
						SET_MASKBIT(icon, x, y, mask);
					} else if((flags & 2) && (*pixels & icon->format->Amask) == 0) {
						SET_MASKBIT(icon, x, y, mask);
					}
					pixels++;
				}
			}
		}
		break;
	}
}

/*
 * Sets the window manager icon for the display window.
 */
void SDL_WM_SetIcon (SDL_Surface *icon, Uint8 *mask)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;

	if ( icon && video->SetIcon ) {
		/* Generate a mask if necessary, and create the icon! */
		if ( mask == NULL ) {
			int mask_len = icon->h*(icon->w+7)/8;
			int flags = 0;
			mask = (Uint8 *)SDL_malloc(mask_len);
			if ( mask == NULL ) {
				return;
			}
			SDL_memset(mask, ~0, mask_len);
			if ( icon->flags & SDL_SRCCOLORKEY ) flags |= 1;
			if ( icon->flags & SDL_SRCALPHA ) flags |= 2;
			if( flags ) {
				CreateMaskFromColorKeyOrAlpha(icon, mask, flags);
			}
			video->SetIcon(video, icon, mask);
			SDL_free(mask);
		} else {
			video->SetIcon(this, icon, mask);
		}
	}
}

/*
 * Grab or ungrab the keyboard and mouse input.
 * This function returns the final grab mode after calling the
 * driver dependent function.
 */
static SDL_GrabMode SDL_WM_GrabInputRaw(SDL_GrabMode mode)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;

	/* Only do something if we have support for grabs */
	if ( video->GrabInput == NULL ) {
		return(video->input_grab);
	}

	/* If the final grab mode if off, only then do we actually grab */
#ifdef DEBUG_GRAB
  printf("SDL_WM_GrabInputRaw(%d) ... ", mode);
#endif
	if ( mode == SDL_GRAB_OFF ) {
		if ( video->input_grab != SDL_GRAB_OFF ) {
			mode = video->GrabInput(this, mode);
		}
	} else {
		if ( video->input_grab == SDL_GRAB_OFF ) {
			mode = video->GrabInput(this, mode);
		}
	}
	if ( mode != video->input_grab ) {
		video->input_grab = mode;
		if ( video->CheckMouseMode ) {
			video->CheckMouseMode(this);
		}
	}
#ifdef DEBUG_GRAB
  printf("Final mode %d\n", video->input_grab);
#endif

	/* Return the final grab state */
	if ( mode >= SDL_GRAB_FULLSCREEN ) {
		mode -= SDL_GRAB_FULLSCREEN;
	}
	return(mode);
}
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
{
	SDL_VideoDevice *video = current_video;

	/* If the video isn't initialized yet, we can't do anything */
	if ( ! video ) {
		return SDL_GRAB_OFF;
	}

	/* Return the current mode on query */
	if ( mode == SDL_GRAB_QUERY ) {
		mode = video->input_grab;
		if ( mode >= SDL_GRAB_FULLSCREEN ) {
			mode -= SDL_GRAB_FULLSCREEN;
		}
		return(mode);
	}

#ifdef DEBUG_GRAB
  printf("SDL_WM_GrabInput(%d) ... ", mode);
#endif
	/* If the video surface is fullscreen, we always grab */
	if ( mode >= SDL_GRAB_FULLSCREEN ) {
		mode -= SDL_GRAB_FULLSCREEN;
	}
	if ( SDL_VideoSurface && (SDL_VideoSurface->flags & SDL_FULLSCREEN) ) {
		mode += SDL_GRAB_FULLSCREEN;
	}
	return(SDL_WM_GrabInputRaw(mode));
}
static SDL_GrabMode SDL_WM_GrabInputOff(void)
{
	SDL_GrabMode mode;

	/* First query the current grab state */
	mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);

	/* Now explicitly turn off input grab */
	SDL_WM_GrabInputRaw(SDL_GRAB_OFF);

	/* Return the old state */
	return(mode);
}

/*
 * Iconify the window in window managed environments.
 * A successful iconification will result in an SDL_APPACTIVE loss event.
 */
int SDL_WM_IconifyWindow(void)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;
	int retval;

	retval = 0;
	if ( video->IconifyWindow ) {
		retval = video->IconifyWindow(this);
	}
	return(retval);
}

/*
 * Toggle fullscreen mode
 */
int SDL_WM_ToggleFullScreen(SDL_Surface *surface)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;
	int toggled;

	toggled = 0;
	if ( SDL_PublicSurface && (surface == SDL_PublicSurface) &&
	     video->ToggleFullScreen ) {
		if ( surface->flags & SDL_FULLSCREEN ) {
			toggled = video->ToggleFullScreen(this, 0);
			if ( toggled ) {
				SDL_VideoSurface->flags &= ~SDL_FULLSCREEN;
				SDL_PublicSurface->flags &= ~SDL_FULLSCREEN;
			}
		} else {
			toggled = video->ToggleFullScreen(this, 1);
			if ( toggled ) {
				SDL_VideoSurface->flags |= SDL_FULLSCREEN;
				SDL_PublicSurface->flags |= SDL_FULLSCREEN;
			}
		}
		/* Double-check the grab state inside SDL_WM_GrabInput() */
		if ( toggled ) {
			SDL_WM_GrabInput(video->input_grab);
		}
	}
	return(toggled);
}

/*
 * Get some platform dependent window manager information
 */
int SDL_GetWMInfo (SDL_SysWMinfo *info)
{
	SDL_VideoDevice *video = current_video;
	SDL_VideoDevice *this  = current_video;

	if ( video && video->GetWMInfo ) {
		return(video->GetWMInfo(this, info));
	} else {
		return(0);
	}
}

