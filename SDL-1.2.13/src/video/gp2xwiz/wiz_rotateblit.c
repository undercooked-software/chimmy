/* Screen rotation routine for GP2X Wiz(Wrapper) */
#include "wiz_rotateblit.h"

int wiz_rotateblitsurface8(unsigned char* srcpixel,unsigned char* dstpixel,SDL_Rect* rect)
{
	int x=rect->x,y=rect->y;
	unsigned int w=rect->w,h=rect->h;
	unsigned int x_mod=x&3,y_mod=y&3;
	unsigned int w_mod,h_mod;
	
	if((w<4) || (h<4))
	{
		rotateblitslow8(srcpixel,dstpixel,w,h);
		return 0;	
	}

	if(x_mod==1)
	{
		rotateblitslow8(srcpixel,dstpixel,3,h);
		srcpixel+=3;
		dstpixel-=720;
		w-=3;
	}
	else
	{
		if(x_mod==2)
		{
			rotateblitslow8(srcpixel,dstpixel,2,h);
			srcpixel+=2;
			dstpixel-=480;
			w-=2;
		}
		else
		{
			if(x_mod==3)
			{
				rotateblitslow8(srcpixel,dstpixel,1,h);
				srcpixel++;
				dstpixel-=240;
				w--;
			}
		}
	}

	w_mod=w&3;
	if(w_mod==1)
	{
		rotateblitslow8(srcpixel+w-1,dstpixel-((w-1)*240),1,h);
		w--;
	}
	else
	{
		if(w_mod==2)
		{
			rotateblitslow8(srcpixel+w-2,dstpixel-((w-2)*240),2,h);
			w-=2;
		}
		else
		{
			if(w_mod==3)
			{
				rotateblitslow8(srcpixel+w-3,dstpixel-((w-3)*240),3,h);
				w-=3;
			}
		}
	}

	if(w==0) return 0;

	if(y_mod==1)
	{
		rotateblitslow8(srcpixel,dstpixel,w,3);
		srcpixel+=960;
		dstpixel+=3;
		h-=3;
	}
	else
	{
		if(y_mod==2)
		{
			rotateblitslow8(srcpixel,dstpixel,w,2);
			srcpixel+=640;
			dstpixel+=2;
			h-=2;
		}
		else
		{
			if(y_mod==3)
			{
				rotateblitslow8(srcpixel,dstpixel,w,1);
				srcpixel+=320;
				dstpixel++;
				h--;
			}
		}
	}

	h_mod=h&3;
	if(h_mod==1)
	{
		rotateblitslow8(srcpixel+((h-1)*320),dstpixel+h-1,w,1);
		h--;
	}
	else
	{
		if(h_mod==2)
		{
			rotateblitslow8(srcpixel+((h-2)*320),dstpixel+h-2,w,2);
			h-=2;
		}
		else
		{
			if(h_mod==3)
			{
				rotateblitslow8(srcpixel+((h-3)*320),dstpixel+h-3,w,3);
				h-=3;
			}
		}
	}

	if(h==0) return 0;

	rotateblitfast8(srcpixel,dstpixel,w,h);

	return 0;
}

int wiz_rotateblitsurface16(unsigned short* srcpixel,unsigned short* dstpixel,SDL_Rect* rect)
{
	int x=rect->x,y=rect->y;
	unsigned int w=rect->w,h=rect->h;
	unsigned int w_mod,h_mod;

	if((w<2) || (h<2))
	{
		rotateblitslow16(srcpixel,dstpixel,w,h);
		return 0;	
	}

	if((x&1)!=0)
	{
		rotateblitslow16(srcpixel,dstpixel,1,h);
		srcpixel++;
		dstpixel-=240;
		w--;
	}

	if((w&1)!=0)
	{
		rotateblitslow16(srcpixel+w-1,dstpixel-((w-1)*240),1,h);
		w--;
	}

	if(w==0) return 0;

	if((y&1)!=0)
	{
		rotateblitslow16(srcpixel,dstpixel,w,1);
		srcpixel+=320;
		dstpixel++;
		h--;
	}

	if((h&1)!=0)
	{
		rotateblitslow16(srcpixel+((h-1)*320),dstpixel+h-1,w,1);
		h--;
	}

	if(h==0) return 0;

	w_mod=w&3;
	h_mod=h&3;
	if(w_mod==0)
	{
		if(h_mod==0)
			rotateblitfast16_4x4(srcpixel,dstpixel,w,h);
		else
		{
			rotateblitfast16_4x2(srcpixel,dstpixel,w,2);
			srcpixel+=640;
			dstpixel+=2;
			h-=2;

			if(h>0)
				rotateblitfast16_4x4(srcpixel,dstpixel,w,h);
		}
	}
	else
	{
		if(h_mod==0)
		{
			rotateblitfast16_2x4(srcpixel,dstpixel,2,h);
			srcpixel+=2;
			dstpixel-=480;
			w-=2;

			if(w>0)
				rotateblitfast16_4x4(srcpixel,dstpixel,w,h);
		}
		else
		{
			rotateblitfast16_2x2(srcpixel,dstpixel,2,h);
			srcpixel+=2;
			dstpixel-=480;
			w-=2;
			
			if(w>0)
			{
				rotateblitfast16_2x2(srcpixel,dstpixel,w,2);
				srcpixel+=640;
				dstpixel+=2;
				h-=2;
			}

			if((w>0)&&(h>0))
				rotateblitfast16_4x4(srcpixel,dstpixel,w,h);
		}
	}

	return 0;
}

int wiz_rotateblitsurface32(unsigned int* srcpixel,unsigned int* dstpixel,SDL_Rect* rect)
{
	unsigned int w=rect->w,h=rect->h;

	if(h<2)
	{
		rotateblitslow32(srcpixel,dstpixel,w,h);
		return 0;
	}

	if((h&1)!=0)
	{
		rotateblitslow32(srcpixel,dstpixel,w,1);
		srcpixel+=320;
		dstpixel++;
		h--;
	}

	if((h&2)!=0)
	{
		rotateblitfast32_2x1(srcpixel,dstpixel,w,2);
		srcpixel+=640;
		dstpixel+=2;
		h-=2;
	}

	if(h==0) return 0;

	rotateblitfast32_4x1(srcpixel,dstpixel,w,h);

	return 0;
}

