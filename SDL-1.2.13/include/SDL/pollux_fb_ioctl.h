/*
 * include/asm/arch/fb_ioctl.h
 * 	Copyright (c) MagicEyes co.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    POLLUX Frame Buffer Driver
 *
 * ChangeLog
 *
 * 2007-06-16 : Jonathan <mesdigital.com>
 *	- First version
 */

/* 
 * IOCTL code
 */

#ifndef _POLLUX_FB_IOCTL_H
#define _POLLUX_FB_IOCTL_H
 
#include "pollux_fb_cfg.h"

#define FBIO_MAGIC	'D'

/* Video layer IO Control code */
#define FBIO_INFO					_IOR(FBIO_MAGIC, 52, struct _FB_INFORMATION)
#define FBIO_VIDEO_INIT				_IOW(FBIO_MAGIC, 53, struct _FB_VIDEO_CONF)
#define FBIO_VIDEO_START			_IOW(FBIO_MAGIC, 54, struct _FB_VMEMINFO)
#define FBIO_VIDEO_UPDATE			_IOW(FBIO_MAGIC, 55, struct _FB_VIDEO_CONF)
#define FBIO_VIDEO_STOP				_IOW(FBIO_MAGIC, 56, unsigned int)
			
#define FBIO_VIDEO_MEMORY_UPDATE	_IOW(FBIO_MAGIC, 59, struct _FB_VMEMINFO)
                                                                               
	/* Video layer priority code */
#define FBIO_VIDEO_PRIORITY			_IOW(FBIO_MAGIC, 69, unsigned int[2])
                                        
	/* Color control Io Control code */
#define FBIO_VIDEO_LUMINAENHANCE	_IOW(FBIO_MAGIC, 70, unsigned int[3])
#define FBIO_VIDEO_CHROMAENHANCE	_IOW(FBIO_MAGIC, 71, unsigned int[5])
                                        
	/* Each layer transparency, alpha blend */
#define FBIO_LAYER_TPCOLOR			_IOW(FBIO_MAGIC, 72, unsigned int[4])
#define FBIO_LAYER_INVCOLOR			_IOW(FBIO_MAGIC, 73, unsigned int[4])
#define FBIO_LAYER_ALPHABLD			_IOW(FBIO_MAGIC, 74, unsigned int[4])
                               	         
	/* Each layer power Io Control code */
#define FBIO_DEVICE_ENABLE			_IOW(FBIO_MAGIC, 77, unsigned int[3])
   
/* Each RGB layer properties Io Control */
#define FBIO_RGB_CONTROL			_IOW(FBIO_MAGIC, 78, struct _FB_RGBSET)
#define RGB_LAYER_POWER        		_IOW(FBIO_MAGIC, 79, unsigned int[3])
#define FBIO_DIRTFLAG           	_IOW(FBIO_MAGIC, 80, unsigned int[4])

#define FBIO_LCD_CHANGE_CONTROL     _IOW(FBIO_MAGIC, 90, unsigned int[2])
#define FBIO_GET_BOARD_NUMBER       _IOR(FBIO_MAGIC, 91, unsigned char*)
#define FBIO_SET_TVCFG_MODE         _IOW(FBIO_MAGIC, 92, struct _FB_TVCONF)
#define FBIO_SET_BOARD_NUMBER       _IOW(FBIO_MAGIC, 93, unsigned char)
#define FBIO_IDCT_RUN               _IOW(FBIO_MAGIC, 95, struct _FB_IDCT)
#define FBIO_GET_3D_NUMBER          _IOR(FBIO_MAGIC, 96, unsigned char*)
#define FBIO_STOP_BOOTANI          	_IOR(FBIO_MAGIC, 97, unsigned int)

#endif
