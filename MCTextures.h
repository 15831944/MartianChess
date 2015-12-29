#ifndef _MC_TEXTURES_H
#define _MC_TEXTURES_H

#include <gl/gl.h>
#include <gl/glu.h>

struct textureImage
{
	int Width;
	int Height;
	unsigned char *Data;
};

bool loadGLTextures();
int LoadBMP(char *FileName, textureImage *Texture);

#endif