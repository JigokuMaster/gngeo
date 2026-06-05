
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "../emu.h"
#include "../screen.h"
#include "../video.h"
#include "../effect.h"
#include "../conf.h"

#include "SDL_opengl.h"

typedef struct SGL_Rect SGL_Rect;
struct SGL_Rect
{
    GLfloat x, y, w, h;
};

typedef struct SGL_Surface SGL_Surface;
struct SGL_Surface
{
    int w, h, owner_w, owner_h;
    GLuint texture;
    GLfloat texcoord_w, texcoord_h;
    SDL_Surface *teximage;
    int teximage_w, teximage_h;
    GLenum teximage_format;
};



#define glOrtho glOrthof

static int power_of_two(int i)
{
    int r;
    for (r = 1; r < i; r *= 2) {}
    return r;
}


void SGL_BlitSurfaceScaled(SGL_Surface* surface, SGL_Rect* src_rect, SGL_Rect* dst_rect)
{
    GLfloat r = 1.0f;
    GLfloat g = 1.0f;
    GLfloat b = 1.0f;
    GLfloat a = 1.0f;

    int w = surface->w;
    int h = surface->h;
    GLfloat x = src_rect->x;
    GLfloat y = src_rect->y;

        /*int s_w = surface->owner_w;*/
        int s_h = surface->owner_h;
        
	GLfloat texcoord_w = surface->texcoord_w;
        GLfloat texcoord_h = surface->texcoord_h;

	// GL coordinates start at bottom-left corner, which is counter-intuitive for sprite graphics, so we have to flip Y coordinate
	GLfloat texture_coord[] = {
	    0.0f, 0.0f,
	    0.0f, texcoord_h,
	    texcoord_w, texcoord_h,
	    texcoord_w, 0.0f
	};


	GLfloat vertices[] = { 
	    x, s_h - y,
	    x, s_h - (y + h),
	    x + w, s_h - (y + h),
	    x + w, s_h - y 
	};

	glColor4f(r, g, b, a);
	glBindTexture(GL_TEXTURE_2D, surface->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, surface->teximage_format, GL_UNSIGNED_SHORT_5_6_5, surface->teximage->pixels);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texture_coord);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); 
}
//#endif

#if 0
void SGL_BlitSurfaceScaled(SGL_Surface* surface, SGL_Rect* src_rect, SGL_Rect* dst_rect) {
    GLfloat r = 1.0f;
    GLfloat g = 1.0f;
    GLfloat b = 1.0f;
    GLfloat a = 1.0f;

    if(surface->texture == 0) {
//fprintf(stderr, "SGL_BlitSurfaceScaled(surface->texture=0)\n");
        return;
    }

    GLfloat x1 = dst_rect->x;
    GLfloat y1 = dst_rect->y;
    GLfloat x2 = dst_rect->x + dst_rect->w;
    GLfloat y2 = dst_rect->y + dst_rect->h;

    GLfloat s_h = surface->owner_h;
    GLfloat texcoord_w = surface->texcoord_w;
    GLfloat texcoord_h = surface->texcoord_h;

    GLfloat s_x = src_rect->x / surface->w;
    GLfloat s_y = src_rect->y / surface->h;
    GLfloat s_w = src_rect->w / surface->w;
    GLfloat s_h_tex = src_rect->h / surface->h;

    GLfloat texture_coord[] = {
        s_x, s_y,
        s_x, s_y + s_h_tex,
        s_x + s_w, s_y + s_h_tex,
        s_x + s_w, s_y
    };

    GLfloat vertices[] = {
        x1, s_h - y1,
        x1, s_h - y2,
        x2, s_h - y2,
        x2, s_h - y1
    };

    glColor4f(r, g, b, a);
    glBindTexture(GL_TEXTURE_2D, surface->texture);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texture_coord);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
#endif

SDL_bool SGL_BlitSurface(SGL_Surface* surface, GLfloat x, GLfloat y)
{

    SGL_Rect src_r = {0,0,0,0};
    SGL_Rect dst_r = {0,0,0,0};
    src_r.x = x;
    src_r.y = y;
    SGL_BlitSurfaceScaled(surface, &src_r, &dst_r);
    return SDL_TRUE;
}


SGL_Surface* SGL_CreateSurface(SDL_Surface *surface, int s_w, int s_h)
{
 
    GLenum glFormat = GL_RGB;
 
    SGL_Surface* gl_surface = malloc(sizeof(SGL_Surface));
    gl_surface->texture = 0;
    gl_surface->teximage = surface;
    gl_surface->owner_w = s_w;
    gl_surface->owner_h = s_h;
    gl_surface->w = surface->w;
    gl_surface->h = surface->h;
    gl_surface->teximage_format = glFormat;

    // All OpenGL textures must have size which is power of 2, such as 128, 256, 512 etc
    int upload_w = power_of_two(surface->w);
    int upload_h = power_of_two(surface->h);
    gl_surface->texcoord_w = (float)gl_surface->w / (float)upload_w;
    gl_surface->texcoord_h = (float)gl_surface->h / (float)upload_h;
    gl_surface->teximage_w = upload_w;
    gl_surface->teximage_h = upload_h;
 
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glGenTextures(1, &(gl_surface->texture));
    glBindTexture(GL_TEXTURE_2D, gl_surface->texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, upload_w, upload_h, 0, glFormat,GL_UNSIGNED_SHORT_5_6_5, NULL);
    return gl_surface;
}


void SGL_FreeSurface(SGL_Surface *surface)
{
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDeleteTextures(1, &(surface->texture));
    free(surface);
}


static SDL_Rect screen_rect =	{ 0,  0, 304, 224};
SGL_Surface* GLScreen = NULL;

SDL_bool blitter_opengl_init()
{
	Uint32 width = visible_area.w;
	Uint32 height = visible_area.h;

	Uint32 sdl_flags = SDL_OPENGL | SDL_FULLSCREEN;
	SDL_Rect** modes = SDL_ListModes(NULL, sdl_flags);
	width = modes[0]->w;
	height = modes[0]->h; 
	/*if(width < 320)
	{
	    width = 304;
	    visible_area.w = width;
	    visible_area.x = 24;
	    visible_area.y = 16;
	}    

	if(height < 224)
	{
	    height = 224;
	}

	if((width > 320) && (height > 320))
	{
	    width = 320;
	    height = 240;    
	}

	if (neffect != 0)
	{	    
	    width*=effect[neffect].x_ratio;
	    height*=effect[neffect].y_ratio;
	}*/
	

	//screen_rect.x = SDL_max(0, SDL_abs(width-visible_area.w)/2); 
	//screen_rect.y = SDL_max(0, SDL_abs(height-visible_area.h)/2); 
	//screen_rect.w = width;
	//screen_rect.h = height;
	printf("blitter_opengl_init\n"); 
	printf("screen resolution: %dx%d\n", width, height); 	
	printf("neogen screen xy: %d,%d\n", screen_rect.x , screen_rect.y); 
	
	SDL_GL_LoadLibrary(NULL);	
	screen = SDL_SetVideoMode(width, height, 16, sdl_flags);
	if(screen == NULL)
	{ 
	    return SDL_FALSE;
	}    

	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0.0f, screen->w, 0.0f, screen->h, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);

	return SDL_TRUE;
}

SDL_bool blitter_opengl_resize(int w,int h)
{}

void blitter_opengl_update()
{
    if (GLScreen == NULL)
    {
	GLScreen = SGL_CreateSurface(buffer, screen->w, screen->h);
    }

    SGL_BlitSurface(GLScreen, 0, 0);
    SDL_GL_SwapBuffers();
}

void blitter_opengl_close()
{
    if (GLScreen) SGL_FreeSurface(GLScreen);

}
	
void blitter_opengl_fullscreen()
{
}
	
