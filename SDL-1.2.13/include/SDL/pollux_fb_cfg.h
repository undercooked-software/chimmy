/*
 * Copyright (c) MagicEyes co.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    pollux Frame Buffer Driver
 *
 * ChangeLog
 *
 * 2007-06-16 : Jonathan <mesdigital.com>
 *	- First version
 */


#ifndef _POLLUX_FB_CFG_H
#define _POLLUX_FB_CFG_H

/* ------------------------------------------------------------------------------
 * Enum type for vip driver & applictaion
 * ------------------------------------------------------------------------------
 */
enum __MES_CHIPID {
	MES_CHIPID_MP2530F = 1,
	MES_CHIPID_POLLUX  = 2
};

enum  __MES_MEM_MODE {
	FRAME_BUFFER = 0,	/* 1D memory region */
	VIDEO_BUFFER = 1	/* 2D memory region */
};

enum  __MES_MEM_ALIGN {
	MES_USERDEF = 0,
	MES_NORMAL  = 1,
	MES_MPEG    = 256,
	MES_H264    = 512
};

enum __MES_VID_DEV
{
	PRI_VIDEO   = 1,	/* Primary 	 MLC Video Layer */
	SEC_VIDEO 	= 2		/* Secondary MLC Video Layer */
};

enum __MES_MLC_DEV
{
	PRI_MLC 	= 1,	/* Primary 	 MLC */
	SEC_MLC 	= 2		/* Secondary MLC */
};

enum __LCD_OLED_LIGHT
{
    HIGH_LIGHT_LCD,
    MID_LIGHT_LCD,
    LESS_MID_LIGHT_LCD,
    LOW_LIGHT_LCD
};

enum __LCD_SUB_COMMAND
{
    LCD_POWER_DOWN_ON_CMD,
    LCD_POWER_DOWN_OFF_CMD,
    LCD_GAMMA_SET_CMD,
    LCD_LIGHT_SET_CMD,
    LCD_PANEL_ID_CMD,
    LCD_DIRECTION_ON_CMD,    /* frame buff 320x240 */
    LCD_DIRECTION_OFF_CMD    /* frame buff 240x320 */
};


enum __TV_COMMAND
{
    COMMAND_INDIVIDUALLY 	= 1,	/* Not test */
	COMMAND_COMMONVIEW 	    = 2,
	COMMAND_ONLY_TV         = 3,
	COMMAND_RETURN_LCD		= 4,
	COMMAND_SCREEN_POS      = 5,
	COMMAND_SCREEN_SCALE    = 6,
	COMMAND_CHANGE_MODE     = 7,
	COMMAND_SET_COLOR       = 8,
	COMMAND_SET_SYNC_TEST   = 9
};


typedef enum __MES_CHIPID		MES_CHIPID;
typedef enum __MES_MEM_MODE		MES_MEM_MODE;
typedef enum __MES_MEM_ALIGN	MES_MEM_ALIGN;
typedef enum __MES_VID_DEV		MES_VID_DEV;
typedef enum __MES_MLC_DEV		MES_MLC_DEV;
typedef enum __LCD_OLED_LIGHT	LCD_OLED_LIGHT;
typedef enum __TV_COMMAND	    TV_COMMAND;



#define SCH_PHASE               0x01
#define HUE_PHASE               0x02
#define CHROMA_SATURATION       0x04
#define LUMA_GAIN               0x08
#define LUMA_OFFSET             0x10


#define SCH_SHIFT               0
#define HUE_SHIFT               8
#define CHROMA_SHIFT            16
#define LUMA_GAIN_SHIFT         24
#define LUMA_OFFSET_SHIFT       0


#define MES_MPEG_X_ALIGN		512
#define MES_MPEG_Y_ALIGN		256
#define MES_H264_X_ALIGN		512
#define MES_H264_Y_ALIGN		512

/* ------------------------------------------------------------------------------
 * Display driver parameters
 *------------------------------------------------------------------------------
 */
/* Display(LCD) information struct */
typedef struct _FB_INFORMATION
{
	u_int32_t		ScreenWidth;		/* full screen width */
	u_int32_t		ScreenHeight;		/* full screen height */
	int32_t			Frequency;
	int32_t			ColorDepth;
	u_int32_t		ColorKey;
	MES_CHIPID		CHIPID;
	u_int32_t		FrameMemPBase;		/* 1D memory physical base. */
	u_int32_t		FrameMemVBase;		/* 1D memory Virtual  base. */
	u_int32_t		FrameMemSize;		/* 1D memory size. */
	u_int32_t		BlockMemPBase;		/* 2D memory physical base. */
	u_int32_t		BlockMemVBase;		/* 2D memory Virtual  base. */
	u_int32_t		BlockMemSize;		/* 2D memory size. */
	u_int32_t		BlockMemStride; 	/* 2D memory stride. */
	u_int32_t		IsDualDisplay;		/* define Dual display. */
	u_int32_t		IsPrimaryMain;		/* define main display device. */
	u_int32_t		PriScreenWidth;		/* primary screen width */
	u_int32_t		PriScreenHeight;	/* primary screen height */
	u_int32_t		SecScreenWidth;		/* primary screen width */
	u_int32_t		SecScreenHeight;	/* primary screen height */
} FB_INFO, *LPFB_INFO;

/* ------------------------------------------------------------------------------
 * Display driver Video memory allocate parameters
 *------------------------------------------------------------------------------
 */
/* struct for surface */
/* this struct has a memory space for a single surface. */
typedef struct _FB_MEMINFO
{
	u_int32_t			Address;	/* Allocated address. */
	MES_MEM_MODE		MemMode;	/* memory mode(1D/2D). */
	MES_MEM_ALIGN		Align;		/* Align type(mpeg, h264, 3D, nomal). */
	u_int32_t		    AlignX;		/* if Align type is MES_USERDEF, align y */
	u_int32_t		    AlignY;		/* if Align type is MES_USERDEF, align y */
	u_int32_t			FourCC;		/* FourCC, define allocate memory format */
	int32_t				Width;		/* Image width. */
	int32_t				Height;		/* Image height. */
	u_int32_t			PhyAddr;	/* 2D or 1D physical address for surface */
	u_int32_t			Stride;		/* 2D or 1D physical address for surface */
	u_int32_t			Offset;		/* 2D or 1D physical address for surface */
	MES_VID_DEV			VideoDev;	/* Priamry or Secondary Video device */
} FB_MEMINFO, * LPFB_MEMINFO;

/*
 * struct for video surface(LuCbCr format)
 * this struct has Lu, Cb, Cr memory spaces for video surface
 */
typedef struct _FB_VMEMINFO
{
	u_int32_t			Address;	/* Allocated address. */
	MES_MEM_MODE		MemMode;	/* memory mode(1D/2D). */
	MES_MEM_ALIGN		Align;		/* Align type(mpeg, h264, 3D, nomal). */
	u_int32_t		    AlignX;		/* if Align type is MES_USERDEF, align y */
	u_int32_t		    AlignY;		/* if Align type is MES_USERDEF, align y */
	u_int32_t			FourCC;		/* FourCC, define allocate memory format */
	int32_t				Width;		/* Image width. */
	int32_t				Height;		/* Image height. */
	u_int32_t			LuAddr;		/* 2D or 1D physical address for video surface(YUV) */
	u_int32_t			CbAddr;		/* 2D or 1D physical address for video surface(YUV) */
	u_int32_t			CrAddr;		/* 2D or 1D physical address for video surface(YUV) */
	u_int32_t			LuStride;	/* 2D or 1D stride value for video surface(YUV) */
	u_int32_t			CbStride;	/* 2D or 1D stride value for video surface(YUV) */
	u_int32_t			CrStride;	/* 2D or 1D stride value for video surface(YUV) */
	u_int32_t			LuOffset;	/* 2D format addr or 1D physical address. */
	u_int32_t			CbOffset;	/* 2D format addr or 1D physical address. */
	u_int32_t			CrOffset;	/* 2D format addr or 1D physical address. */
	MES_VID_DEV			VideoDev;	/* Priamry or Secondary Video device */
} FB_VMEMINFO, * LPFB_VMEMINFO;


typedef struct _FB_RGBSET
{
	u_int32_t	Layer;
	u_int32_t	Addrs;
	int32_t		Left;
	int32_t		Top;
	int32_t		Right;
	int32_t		Bottom;
	u_int32_t	HStride;
	u_int32_t	VStride;
	int32_t   enable3D;
	u_int32_t	Bakcol;
	u_int32_t   Mlc_dev;
} FB_RGBSET, * LPFB_RGBSET;

typedef struct _FB_IDCT
{
	u_int16_t	*Indat;
	u_int16_t	*QuantMatrix;
	u_int16_t	*Outdat;
} FB_IDCT, * LPFB_IDCT;


typedef struct _FB_TVCONF
{
	u_int8_t    command;
	u_int8_t	tv_mode;
	u_int32_t   SecScreenWidth;
	u_int32_t	SecScreenHeight;
} FB_TVCONF, * LPFB_TVCONF;

/* video surface configuration struct */
typedef struct _FB_VIDEO_CONF
{
	u_int32_t		Flags;		/* flags for video device set. */
	u_int32_t		FourCC;		/* FourCC, define video format. */
	u_int32_t		ColorKey;	/* Transparency Color Key of screen */
	int32_t	    	SrcWidth;	/* The source width of the overlay image. */
	int32_t	    	SrcHeight;	/* The source height of the overlay image. */
	int32_t	    	DstWidth;	/* The target width of the overlay image. */
	int32_t	    	DstHeight;	/* The target height of the overlay image. */
	/* RECTL			rVIDRECT;	/1* A RECT that defines the position. (Windows only) *1/ */
	int32_t			Left;		/* Position information - use integer instead of rectangle structure */
	int32_t			Top;
	int32_t			Right;
	int32_t			Bottom;
	MES_VID_DEV	VideoDev;	/* Priamry or Secondary Video device */
} __attribute__((packed)) FB_VIDEO_CONF, * LPFB_VIDEO_CONF;


/* ------------------------------------------------------------------------------
 * Display driver video flags
 * ------------------------------------------------------------------------------
 */
#define	SET_TPCOLOR 0x00000001

/* ------------------------------------------------------------------------------
 * Display 2D Memory configuration
 * ------------------------------------------------------------------------------
 */
#define	MEM_2D_X_BYTE_PER_BLOCK		64
#define	MEM_2D_Y_BYTE_PER_BLOCK		32
#define	MEM_2D_X_BLOCK_PER_SEGMEMT	64		/* 64 * 64 : 4096 */
#define	MEM_2D_Y_BLOCK_PER_SEGMEMT	128		/* 64 * 128: 4096 */
#define	MEM_2D_Y_COORD_OFFBIT		12
#define	MEM_2D_X_COORD_OFFBIT		0

#endif /* _MP2530F_FB_CFG_H */
