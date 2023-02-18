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
#include <unistd.h>
#include "SDL_gp2xwizvideo.h"
#include "SDL_gp2xwizgles_c.h"


int GP2XWIZ_GL_CreateContext(_THIS)
{
	EGLint config_attrib[15];
	int config;
	struct SDL_PrivateGLData* gl_data=this->gl_data;

	if(gl_active) GP2XWIZ_GL_Shutdown(this);

	/* Load OpenGL ES Lite library dynamically */
	if (!this->gl_config.driver_loaded) {
		if (GP2XWIZ_GL_LoadLibrary(this, NULL)<0) {
			return -1;
		}
	}

	/* Alloc OpenGL ES lite surface for GP2X Wiz */
	hwnd=(NativeWindowType)malloc(16*1024);
	if(!hwnd)
	{
		SDL_SetError("Failed to create OpenGL ES surface");
		return -1;
	}

	/* Initialize OpenGL ES lite */
	egl_display=gl_data->eglGetDisplay((NativeDisplayType)0);
	if(egl_display==EGL_NO_DISPLAY)
	{
		SDL_SetError("Failed to get OpenGL ES display");
		free(hwnd);
		return -1;
	}

	if(gl_data->eglInitialize(egl_display, NULL, NULL)!=EGL_TRUE)
	{
		SDL_SetError("Failed to init OpenGL ES");
		free(hwnd);
		return -1;
	}

	/* Config OpenGL ES Lite */
	config_attrib[0]=EGL_SURFACE_TYPE;
	config_attrib[1]=EGL_WINDOW_BIT;
	config_attrib[2]=EGL_RED_SIZE;
	config_attrib[3]=this->gl_config.red_size;
	config_attrib[4]=EGL_GREEN_SIZE;
	config_attrib[5]=this->gl_config.green_size;
	config_attrib[6]=EGL_BLUE_SIZE;
	config_attrib[7]=this->gl_config.blue_size;
	config_attrib[8]=EGL_ALPHA_SIZE;
	config_attrib[9]=this->gl_config.alpha_size;
	config_attrib[10]=EGL_BUFFER_SIZE;
	config_attrib[11]=this->gl_config.buffer_size;
	config_attrib[12]=EGL_DEPTH_SIZE;
	config_attrib[13]=this->gl_config.depth_size;
	config_attrib[14]=EGL_NONE;

	if((gl_data->eglChooseConfig(egl_display, config_attrib, &egl_config, 1, &config)!=EGL_TRUE)||
		(config==0))
	{
		/* If configuration failed, use "default settings" */ 
		const EGLint default_config_attrib[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_NONE};
		if((gl_data->eglChooseConfig(egl_display, default_config_attrib, &egl_config, 1, &config)!=EGL_TRUE)
			||(config==0))
		{
			SDL_SetError("Failed to config OpenGL ES");
			gl_data->eglTerminate(egl_display);
			free(hwnd);
			return -1;
		}
	}

	/* Create EGL context */
	egl_context=gl_data->eglCreateContext(egl_display, egl_config,
					EGL_NO_CONTEXT, NULL);
	if(egl_context==EGL_NO_CONTEXT)
	{
		SDL_SetError("Failed to create OpenGL ES context");
		gl_data->eglTerminate(egl_display);
		free(hwnd);
		return -1;
	}

	/* Create OpenGL ES surface */
	egl_surface=gl_data->eglCreateWindowSurface(egl_display, egl_config, hwnd, NULL);
	if(egl_surface==0)
	{
		SDL_SetError("Failed to create OpenGL ES surface");
		gl_data->eglDestroyContext(egl_display, egl_context);
		gl_data->eglTerminate(egl_display);
		free(hwnd);
		return -1;
	}
	
	/* Make current */
	if(gl_data->eglMakeCurrent(egl_display, egl_surface, egl_surface, 
					egl_context)!=EGL_TRUE)
	{
		gl_data->eglDestroySurface(egl_display, egl_surface);
		gl_data->eglDestroyContext(egl_display, egl_context);
		gl_data->eglTerminate(egl_display);
		free(hwnd);
		return -1;
	}

	/* Set swap interval (currently it doesn't works) */
	gl_data->eglSwapInterval(egl_display,(EGLint)this->gl_config.swap_control);
	gl_active=1;

	/* Copy OpenGL ES attributes for further init process */
	if((GP2XWIZ_GL_GetAttribute(this,SDL_GL_RED_SIZE,&this->gl_config.red_size)<0)||
		(GP2XWIZ_GL_GetAttribute(this,SDL_GL_GREEN_SIZE,&this->gl_config.green_size)<0)||
		(GP2XWIZ_GL_GetAttribute(this,SDL_GL_BLUE_SIZE,&this->gl_config.blue_size)<0)||
		(GP2XWIZ_GL_GetAttribute(this,SDL_GL_ALPHA_SIZE,&this->gl_config.alpha_size)<0)||
		(GP2XWIZ_GL_GetAttribute(this,SDL_GL_BUFFER_SIZE,&this->gl_config.buffer_size)<0)||
		(GP2XWIZ_GL_GetAttribute(this,SDL_GL_DEPTH_SIZE,&this->gl_config.depth_size)<0))
	{
		gl_active=0;
		gl_data->eglDestroySurface(egl_display, egl_surface);
		gl_data->eglDestroyContext(egl_display, egl_context);
		gl_data->eglTerminate(egl_display);
		free(hwnd);
		return -1;
	}

	return 0;
}

void GP2XWIZ_GL_Shutdown(_THIS)
{
	struct SDL_PrivateGLData* gl_data=this->gl_data;

	if(!gl_active) return;

	/* Terminate OpenGL ES mode and unload library */
	gl_data->eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	gl_data->eglDestroySurface(egl_display, egl_surface);
	gl_data->eglDestroyContext(egl_display, egl_context);;
	gl_data->eglTerminate(egl_display);
	if(hwnd) free(hwnd);

	GP2XWIZ_GL_UnloadLibrary(this);
	gl_active=0;
}

int GP2XWIZ_GL_MakeCurrent(_THIS)
{
	if(!gl_active) 
	{
		SDL_SetError("Not OpenGL ES mode");
		return -1;
	}

	if(this->gl_data->eglMakeCurrent(egl_display, egl_surface, egl_surface, 
					egl_context)!=EGL_TRUE)
	{
		SDL_SetError("Unable to make OpenGL ES context");
		return -1;
	}

	return 0;
}

int GP2XWIZ_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value)
{
	int retval = -1;
	int unsupported = 0;
	EGLint egl_attrib;

	if(!gl_active) 
	{
		SDL_SetError("Not OpenGL ES mode");
		return -1;
	}

	switch( attrib ) {
	    case SDL_GL_RED_SIZE:
		egl_attrib = EGL_RED_SIZE;
		break;
	    case SDL_GL_GREEN_SIZE:
		egl_attrib = EGL_GREEN_SIZE;
		break;
	    case SDL_GL_BLUE_SIZE:
		egl_attrib = EGL_BLUE_SIZE;
		break;
	    case SDL_GL_ALPHA_SIZE:
		egl_attrib = EGL_ALPHA_SIZE;
		break;
	    case SDL_GL_BUFFER_SIZE:
		egl_attrib = EGL_BUFFER_SIZE;
		break;
	    case SDL_GL_DEPTH_SIZE:
		egl_attrib = EGL_DEPTH_SIZE;
		break;
	    case SDL_GL_SWAP_CONTROL:
		*value=this->gl_config.swap_control;
		return 0;
	    default:
		unsupported = 1;
		break;
	}

	if (unsupported) {
		SDL_SetError("OpenGL ES attribute is unsupported on this system");
	} else {
		retval = this->gl_data->eglGetConfigAttrib(egl_display, egl_config, egl_attrib, (EGLint*)value);
	}
	return retval;
}

void GP2XWIZ_GL_SwapBuffers(_THIS)
{
	struct SDL_PrivateGLData* gl_data=this->gl_data;

	if(!gl_active) return;

	/* Wait until OpenGL ES rendering finished */
	gl_data->glFinish();
	gl_data->eglWaitGL();
	/* Now, Swap buffers */
	gl_data->eglSwapBuffers(egl_display, egl_surface);
}

int GP2XWIZ_GL_LoadLibrary(_THIS, const char* path)
{
	void* handle = NULL;
	struct SDL_PrivateGLData* gl_data=this->gl_data;

	if ( gl_active ) {
		SDL_SetError("OpenGL context already created");
		return -1;
	}

	/* Load OpenGL ES Lite 1.1 library */
	if ( path == NULL ) {
		path = "libopengles_lite.so";
		if (access(path,F_OK)!=0) {
			path = "/lib/libopengles_lite.so";
		}
	}

	handle = SDL_LoadObject(path);
	if ( handle == NULL ) {
		SDL_SetError("Failed loading %s", path);
		return -1;
	}

	/* Unload the old driver and reset the pointers */
	GP2XWIZ_GL_UnloadLibrary(this);

	/* Load new function pointers */
	gl_data->eglGetProcAddress = (void *(*)(const char *procName))SDL_LoadFunction(handle, "eglGetProcAddress");
	gl_data->eglGetDisplay = (EGLDisplay (*)(NativeDisplayType))SDL_LoadFunction(handle, "eglGetDisplay");
	gl_data->eglInitialize = (EGLBoolean (*)(EGLDisplay, EGLint *, EGLint *))SDL_LoadFunction(handle, "eglInitialize");
	gl_data->eglTerminate = (EGLBoolean (*)(EGLDisplay))SDL_LoadFunction(handle, "eglTerminate");
	gl_data->eglChooseConfig = 
		(EGLBoolean (*)(EGLDisplay, EGLint const *, EGLConfig *, EGLint, EGLint *))SDL_LoadFunction(handle, "eglChooseConfig");
	gl_data->eglCreateWindowSurface = 
		(EGLSurface (*)(EGLDisplay, EGLConfig, NativeWindowType, EGLint const *))SDL_LoadFunction(handle, "eglCreateWindowSurface");
	gl_data->eglDestroySurface = (EGLBoolean (*)(EGLDisplay, EGLSurface))SDL_LoadFunction(handle, "eglDestroySurface");
	gl_data->eglCreateContext = 
		(EGLContext (*)(EGLDisplay, EGLConfig, EGLContext, EGLint const *))SDL_LoadFunction(handle, "eglCreateContext");
	gl_data->eglDestroyContext = (EGLBoolean (*)(EGLDisplay, EGLContext))SDL_LoadFunction(handle, "eglDestroyContext");
	gl_data->eglMakeCurrent = 
		(EGLBoolean (*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))SDL_LoadFunction(handle, "eglMakeCurrent");
	gl_data->eglSwapBuffers = (EGLBoolean (*)(EGLDisplay, EGLSurface))SDL_LoadFunction(handle, "eglSwapBuffers");
	gl_data->eglGetConfigAttrib = 
		(EGLBoolean (*)(EGLDisplay, EGLConfig, EGLint, EGLint *))SDL_LoadFunction(handle, "eglGetConfigAttrib");
	gl_data->eglSwapInterval = (EGLBoolean (*)(EGLDisplay, EGLint))SDL_LoadFunction(handle, "eglSwapInterval");
	gl_data->eglWaitGL = (EGLBoolean (*)(void))SDL_LoadFunction(handle, "eglWaitGL");
	gl_data->glFinish = (void (*)(void))SDL_LoadFunction(handle, "glFinish");
	
	if ( (gl_data->eglGetProcAddress == NULL) || 
	     (gl_data->eglGetDisplay == NULL) ||
	     (gl_data->eglInitialize == NULL) ||
	     (gl_data->eglTerminate == NULL) ||
	     (gl_data->eglChooseConfig == NULL) ||
	     (gl_data->eglCreateWindowSurface == NULL) ||
	     (gl_data->eglDestroySurface == NULL) ||
	     (gl_data->eglCreateContext == NULL) ||
	     (gl_data->eglDestroyContext == NULL) ||
	     (gl_data->eglMakeCurrent == NULL) ||
	     (gl_data->eglSwapBuffers == NULL) ||
	     (gl_data->eglGetConfigAttrib == NULL) ||
	     (gl_data->eglSwapInterval == NULL) ||
	     (gl_data->eglWaitGL == NULL) ||
	     (gl_data->glFinish == NULL)) {
		SDL_SetError("Could not retrieve OpenGL ES functions");
		return -1;
	}

	this->gl_config.dll_handle = handle;
	this->gl_config.driver_loaded = 1;
	if ( path ) {
		SDL_strlcpy(this->gl_config.driver_path, path,
			SDL_arraysize(this->gl_config.driver_path));
	} else {
		*this->gl_config.driver_path = '\0';
	}
	return 0;
}

void *GP2XWIZ_GL_GetProcAddress(_THIS, const char* proc)
{
	void *handle,*ret_value=NULL;
	
	handle = this->gl_config.dll_handle;
	if ( this->gl_data->eglGetProcAddress ) {
		ret_value=this->gl_data->eglGetProcAddress(proc);
	}
	if(!ret_value)
		return SDL_LoadFunction(handle, proc);
}

void GP2XWIZ_GL_UnloadLibrary(_THIS)
{
	/* Unload library */
	struct SDL_PrivateGLData* gl_data=this->gl_data;
	if(!this->gl_config.driver_loaded) return;

	SDL_UnloadObject(this->gl_config.dll_handle);

	gl_data->eglGetProcAddress = NULL;
	gl_data->eglGetDisplay = NULL;
	gl_data->eglInitialize = NULL;
	gl_data->eglTerminate = NULL;
	gl_data->eglChooseConfig = NULL;
	gl_data->eglCreateWindowSurface = NULL;
	gl_data->eglDestroySurface = NULL;
	gl_data->eglCreateContext = NULL;
	gl_data->eglDestroyContext = NULL;
	gl_data->eglMakeCurrent = NULL;
	gl_data->eglSwapBuffers = NULL;
	gl_data->eglGetConfigAttrib = NULL;
	gl_data->eglSwapInterval = NULL;
	gl_data->eglWaitGL = NULL;
	gl_data->glFinish = NULL;

	this->gl_config.dll_handle = NULL;
	this->gl_config.driver_loaded = 0;
}
#endif

