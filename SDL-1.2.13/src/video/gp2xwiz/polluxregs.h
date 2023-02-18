/* Copyright (c) 2009 Adan Scotney

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

/* This file copied from libcastor 0.2 */

#ifndef __POLLUXREGS_H__
#define __POLLUXREGS_H__

/* Primary MLC */
#define	MLCCONTROLT		(memregs32[0x4000>>2])
#define	MLCSCREENSIZE		(memregs32[0x4004>>2])
#define	MLCBGCOLOR		(memregs32[0x4008>>2])

#define	MLCLEFTRIGHT0		(memregs32[0x400C>>2])
#define	MLCTOPBOTTOM0		(memregs32[0x4010>>2])
#define MLCLEFTRIGHT0_0		(memregs32[0x4014>>2])
#define	MLCTOPBOTTOM0_0		(memregs32[0x4018>>2])
#define MLCLEFTRIGHT0_1		(memregs32[0x401C>>2])
#define	MLCTOPBOTTOM0_1		(memregs32[0x4020>>2])
#define	MLCCONTROL0		(memregs32[0x4024>>2])
#define	MLCHSTRIDE0		(memregs32[0x4028>>2])
#define	MLCVSTRIDE0		(memregs32[0x402C>>2])
#define	MLCTPCOLOR0		(memregs32[0x4030>>2])
#define	MLCINVCOLOR0		(memregs32[0x4034>>2])
#define	MLCADDRESS0		(memregs32[0x4038>>2])
#define	MLCPALETTE0		(memregs32[0x403C>>2])

#define	MLCLEFTRIGHT1		(memregs32[0x4040>>2])
#define	MLCTOPBOTTOM1		(memregs32[0x4044>>2])
#define MLCLEFTRIGHT1_0		(memregs32[0x4048>>2])
#define	MLCTOPBOTTOM1_0		(memregs32[0x404C>>2])
#define MLCLEFTRIGHT1_1		(memregs32[0x4050>>2])
#define	MLCTOPBOTTOM1_1		(memregs32[0x4054>>2])
#define	MLCCONTROL1		(memregs32[0x4058>>2])
#define	MLCHSTRIDE1		(memregs32[0x405C>>2])
#define	MLCVSTRIDE1		(memregs32[0x4060>>2])
#define	MLCTPCOLOR1		(memregs32[0x4064>>2])
#define	MLCINVCOLOR1		(memregs32[0x4068>>2])
#define	MLCADDRESS1		(memregs32[0x406C>>2])
#define	MLCPALETTE1		(memregs32[0x4070>>2])

#define MLCLEFTRIGHT2		(memregs32[0x4074>>2])
#define MLCTOPBOTTOM2		(memregs32[0x4078>>2])
#define MLCCONTROL2		(memregs32[0x407C>>2])
#define MLCVSTRIDE2		(memregs32[0x4080>>2])
#define MLCTPCOLOR2		(memregs32[0x4084>>2])
#define MLCADDRESS2		(memregs32[0x408C>>2])
#define MLCADDRESSCB		(memregs32[0x4090>>2])
#define MLCADDRESSCR		(memregs32[0x4094>>2])
#define MLCVSTRIDECB		(memregs32[0x4098>>2])
#define MLCVSTRIDECR		(memregs32[0x409C>>2])
#define MLCHSCALE		(memregs32[0x40A0>>2])
#define MLCVSCALE		(memregs32[0x40A4>>2])
#define MLCLUENH		(memregs32[0x40A8>>2])
#define MLCCHENH0		(memregs32[0x40AC>>2])
#define MLCCHENH1		(memregs32[0x40B0>>2])
#define MLCCHENH2		(memregs32[0x40B4>>2])
#define MLCCHENH3		(memregs32[0x40B8>>2])

/* Secondary MLC */
#define	MLCCONTROLT_TV		(memregs32[0x4400>>2])
#define	MLCSCREENSIZE_TV	(memregs32[0x4404>>2])
#define	MLCBGCOLOR_TV		(memregs32[0x4408>>2])

#define	MLCLEFTRIGHT0_TV	(memregs32[0x440C>>2])
#define	MLCTOPBOTTOM0_TV	(memregs32[0x4410>>2])
#define MLCLEFTRIGHT0_0_TV	(memregs32[0x4414>>2])
#define	MLCTOPBOTTOM0_0_TV	(memregs32[0x4418>>2])
#define MLCLEFTRIGHT0_1_TV	(memregs32[0x441C>>2])
#define	MLCTOPBOTTOM0_1_TV	(memregs32[0x4420>>2])
#define	MLCCONTROL0_TV		(memregs32[0x4424>>2])
#define	MLCHSTRIDE0_TV		(memregs32[0x4428>>2])
#define	MLCVSTRIDE0_TV		(memregs32[0x442C>>2])
#define	MLCTPCOLOR0_TV		(memregs32[0x4430>>2])
#define	MLCINVCOLOR0_TV		(memregs32[0x4434>>2])
#define	MLCADDRESS0_TV		(memregs32[0x4438>>2])
#define	MLCPALETTE0_TV		(memregs32[0x443C>>2])

#define	MLCLEFTRIGHT1_TV	(memregs32[0x4440>>2])
#define	MLCTOPBOTTOM1_TV	(memregs32[0x4444>>2])
#define MLCLEFTRIGHT1_0_TV	(memregs32[0x4448>>2])
#define	MLCTOPBOTTOM1_0_TV	(memregs32[0x444C>>2])
#define MLCLEFTRIGHT1_1_TV	(memregs32[0x4450>>2])
#define	MLCTOPBOTTOM1_1_TV	(memregs32[0x4454>>2])
#define	MLCCONTROL1_TV		(memregs32[0x4458>>2])
#define	MLCHSTRIDE1_TV		(memregs32[0x445C>>2])
#define	MLCVSTRIDE1_TV		(memregs32[0x4460>>2])
#define	MLCTPCOLOR1_TV		(memregs32[0x4464>>2])
#define	MLCINVCOLOR1_TV		(memregs32[0x4468>>2])
#define	MLCADDRESS1_TV		(memregs32[0x446C>>2])
#define	MLCPALETTE1_TV		(memregs32[0x4470>>2])

#define MLCLEFTRIGHT2_TV	(memregs32[0x4474>>2])
#define MLCTOPBOTTOM2_TV	(memregs32[0x4478>>2])
#define MLCCONTROL2_TV		(memregs32[0x447C>>2])
#define MLCVSTRIDE2_TV		(memregs32[0x4480>>2])
#define MLCTPCOLOR2_TV		(memregs32[0x4484>>2])
#define MLCADDRESS2_TV		(memregs32[0x448C>>2])
#define MLCADDRESSCB_TV		(memregs32[0x4490>>2])
#define MLCADDRESSCR_TV		(memregs32[0x4494>>2])
#define MLCVSTRIDECB_TV		(memregs32[0x4498>>2])
#define MLCVSTRIDECR_TV		(memregs32[0x449C>>2])
#define MLCHSCALE_TV		(memregs32[0x44A0>>2])
#define MLCVSCALE_TV		(memregs32[0x44A4>>2])
#define MLCLUENH_TV		(memregs32[0x44A8>>2])
#define MLCCHENH0_TV		(memregs32[0x44AC>>2])
#define MLCCHENH1_TV		(memregs32[0x44B0>>2])
#define MLCCHENH2_TV		(memregs32[0x44B4>>2])
#define MLCCHENH3_TV		(memregs32[0x44B8>>2])

/* Display controller */
#define DPCCTRL0		(memregs16[0x308C>>1])
#define DPCCTRL0_TV		(memregs16[0x348C>>1])

#define Wiz_DirtyMLC() (MLCCONTROLT|=0x8)
#define Wiz_DirtyMLC_TV() (MLCCONTROLT_TV|=0x8)

#define Wiz_DirtyLayer0() (MLCCONTROL0|=0x10)
#define Wiz_DirtyLayer0_TV() (MLCCONTROL0_TV|=0x10)
#define Wiz_DirtyLayer1() (MLCCONTROL1|=0x10)
#define Wiz_DirtyLayer1_TV() (MLCCONTROL1_TV|=0x10)
#define Wiz_DirtyLayer2() (MLCCONTROL2|=0x10)
#define Wiz_DirtyLayer2_TV() (MLCCONTROL2_TV|=0x10)

#endif
