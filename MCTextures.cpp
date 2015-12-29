#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "MCTextures.h"

GLuint Texture[3];

int LoadBMP(char *FileName, textureImage *Texture)
{
    FILE *file;
    unsigned short int bfType;
    long int bfOffBits;
    short int biPlanes;
    short int biBitCount;
    long int biSizeImage;
    int i;
    unsigned char temp;
    /* make sure the file is there and open it read-only (binary) */
    if ((file = fopen(FileName, "rb")) == NULL)
    {
        printf("File not found : %s\n", FileName);
        return 0;
    }
    if(!fread(&bfType, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    /* check if file is a bitmap */
    if (bfType != 19778)
    {
        printf("Not a Bitmap-File!\n");
        return 0;
    }        
    /* get the file size */
    /* skip file size and reserved fields of bitmap file header */
    fseek(file, 8, SEEK_CUR);
    /* get the position of the actual bitmap Data */
    if (!fread(&bfOffBits, sizeof(long int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    //printf("Data at Offset: %ld\n", bfOffBits);
    /* skip size of bitmap info header */
    fseek(file, 4, SEEK_CUR);
    /* get the Width of the bitmap */
    fread(&Texture->Width, sizeof(int), 1, file);
    //printf("Width of Bitmap: %d\n", Texture->Width);
    /* get the Height of the bitmap */
    fread(&Texture->Height, sizeof(int), 1, file);
    //printf("Height of Bitmap: %d\n", Texture->Height);
    /* get the number of planes (must be set to 1) */
    fread(&biPlanes, sizeof(short int), 1, file);
    if (biPlanes != 1)
    {
        printf("Error: number of Planes not 1!\n");
        return 0;
    }
    /* get the number of bits per pixel */
    if (!fread(&biBitCount, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    //printf("Bits per Pixel: %d\n", biBitCount);
    if (biBitCount != 24)
    {
        printf("Bits per Pixel not 24\n");
        return 0;
    }
    /* calculate the size of the image in bytes */
    biSizeImage = Texture->Width * Texture->Height * 3;
    //printf("Size of the image Data: %ld\n", biSizeImage);
    Texture->Data = (unsigned char*)malloc(biSizeImage);
    /* seek to the actual Data */
    fseek(file, bfOffBits, SEEK_SET);
    if (!fread(Texture->Data, biSizeImage, 1, file))
    {
        printf("Error loading file!\n");
        return 0;
    }
    /* swap red and blue (bgr -> rgb) */
    for (i = 0; i < biSizeImage; i += 3)
    {
        temp = Texture->Data[i];
        Texture->Data[i] = Texture->Data[i + 2];
        Texture->Data[i + 2] = temp;
    }
    return 1;
}

bool loadGLTextures()   /* Load Bitmaps And Convert To Textures */
{
    bool status;
    textureImage *texti;
    
    status = false;
    texti = (textureImage*)malloc(sizeof(textureImage));
    if (LoadBMP("blkmarble.bmp", texti))
    {
        status = true;
        glGenTextures(1, &Texture[0]);   /* create the Texture */
        glBindTexture(GL_TEXTURE_2D, Texture[0]);
        /* actually generate the Texture */
        glTexImage2D(GL_TEXTURE_2D, 0, 3, texti->Width, texti->Height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, texti->Data);
        /* enable linear filtering */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    /* free the ram we used in our Texture generation process */
    if (texti)
    {
        if (texti->Data)
            free(texti->Data);
        free(texti);
    }    
    texti = (textureImage*)malloc(sizeof(textureImage));
    if (LoadBMP("whtmarble.bmp", texti))
    {
        status = true;
        glGenTextures(1, &Texture[1]);   /* create the Texture */
        glBindTexture(GL_TEXTURE_2D, Texture[1]);
        /* actually generate the Texture */
        glTexImage2D(GL_TEXTURE_2D, 0, 3, texti->Width, texti->Height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, texti->Data);
        /* enable linear filtering */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    /* free the ram we used in our Texture generation process */
    if (texti)
    {
        if (texti->Data)
            free(texti->Data);
        free(texti);
    } 
    texti = (textureImage*)malloc(sizeof(textureImage));
    if (LoadBMP("wood.bmp", texti))
    {
        status = true;
        glGenTextures(1, &Texture[2]);   /* create the Texture */
        glBindTexture(GL_TEXTURE_2D, Texture[2]);
        /* actually generate the Texture */
        glTexImage2D(GL_TEXTURE_2D, 0, 3, texti->Width, texti->Height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, texti->Data);
        /* enable linear filtering */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    /* free the ram we used in our Texture generation process */
    if (texti)
    {
        if (texti->Data)
            free(texti->Data);
        free(texti);
    }    
   return status;
}