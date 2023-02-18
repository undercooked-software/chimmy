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

/* OpenGL ES Lite support for GP2X Wiz
   2010.2.10 ikari */
#include "SDL_config.h"

#if SDL_VIDEO_OPENGL
#include "SDL_loadso.h"
#include "../SDL_sysvideo.h"
#include <GLES/egl.h>

struct SDL_PrivateGLData {
    int gl_active; /* to stop switching drivers while we have a valid context */

    /* OpenGL ES 1.1 data */
    NativeWindowType hwnd;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLSurface egl_surface;
    EGLContext egl_context;

    /* OpenGL ES 1.1 Functions */
    void * (*eglGetProcAddress) (const char *procName);

    EGLDisplay (*eglGetDisplay) (NativeDisplayType native_display);

    EGLBoolean (*eglInitialize) (EGLDisplay display, EGLint * major, EGLint * minor);

    EGLBoolean (*eglTerminate) (EGLDisplay display);

    EGLBoolean (*eglChooseConfig) (EGLDisplay display, EGLint const * attrib_list, 
				EGLConfig * configs, EGLint config_size, EGLint * num_config);

    EGLSurface (*eglCreateWindowSurface) (EGLDisplay display, EGLConfig config, 
					NativeWindowType native_window, EGLint const * attrib_list);

    EGLBoolean (*eglDestroySurface) (EGLDisplay display, EGLSurface surface);

    EGLContext (*eglCreateContext) (EGLDisplay display, EGLConfig config,
				EGLContext share_context, EGLint const * attrib_list);

    EGLBoolean (*eglDestroyContext) (EGLDisplay display, EGLContext context);

    EGLBoolean (*eglMakeCurrent) (EGLDisplay display, EGLSurface draw,
				EGLSurface read, EGLContext context);

    EGLBoolean (*eglSwapBuffers) (EGLDisplay display, EGLSurface surface);

    EGLBoolean (*eglGetConfigAttrib) (EGLDisplay display, EGLConfig config, 
	   			EGLint attribute, EGLint * value);

    EGLBoolean (*eglSwapInterval) (EGLDisplay display, EGLint interval);

    EGLBoolean (*eglWaitGL) (void);
    void (*glFinish) (void);
};

/* Old variable names */
#define gl_active		(this->gl_data->gl_active)
#define hwnd			(this->gl_data->hwnd)
#define egl_display		(this->gl_data->egl_display)
#define egl_config		(this->gl_data->egl_config)
#define egl_surface		(this->gl_data->egl_surface)
#define egl_context		(this->gl_data->egl_context)

/* OpenGL functions */
extern int GP2XWIZ_GL_CreateContext(_THIS);
extern void GP2XWIZ_GL_Shutdown(_THIS);
extern int GP2XWIZ_GL_MakeCurrent(_THIS);
extern int GP2XWIZ_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value);
extern void GP2XWIZ_GL_SwapBuffers(_THIS);
extern int GP2XWIZ_GL_LoadLibrary(_THIS, const char* path);
extern void *GP2XWIZ_GL_GetProcAddress(_THIS, const char* proc);
extern void GP2XWIZ_GL_UnloadLibrary(_THIS);
#endif

