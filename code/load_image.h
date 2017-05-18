#if !defined(LOAD_IMAGE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2017 by Barret Gaylor, Inc. All Rights Reserved. $
   ======================================================================== */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#include "stb_image.h"

struct loaded_bitmap
{
    unsigned int Handle; 
    void *Pixel;
    int Width;
    int Height; 
};

loaded_bitmap LoadImage(char *Filename)
{
    loaded_bitmap Bitmap = {};

    int N = 0;
    Bitmap.Pixel = stbi_load(Filename, &Bitmap.Width, &Bitmap.Height, &N, 4);

    unsigned int *SourceDest = (unsigned int *)Bitmap.Pixel;
    for(int Y = 0;
        Y < Bitmap.Height;
        ++Y)
    {
        for(int X = 0;
            X < Bitmap.Width;
            ++X)
        {
            unsigned int C = *SourceDest;

            float A = (float)((C & 0xFF000000) >> 24);
            float B = (float)((C & 0xFF0000) >> 16);
            float G = (float)((C & 0xFF00) >> 8);
            float R = (float)((C & 0xFF) >> 0);
            float AN = (A / 255.0f);


            R = R*AN;
            G = G*AN;
            B = B*AN;

            *SourceDest++ = (((unsigned char)(A + 0.5f) << 24) |
                             ((unsigned char)(R + 0.5f) << 16) |
                             ((unsigned char)(G + 0.5f) << 8) |
                             ((unsigned char)(B + 0.5f) << 0));
        }
    }
    
    return Bitmap; 
}

#define LOAD_IMAGE_H
#endif
