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
    void *Pixel;
    int Width;
    int Height; 
};

loaded_bitmap LoadImage(char *Filename)
{
    loaded_bitmap Bitmap = {};

    int N = 0;
    Bitmap.Pixel = stbi_load(Filename, &Bitmap.Width, &Bitmap.Height, &N, 4);
    
    return Bitmap; 
}

#define LOAD_IMAGE_H
#endif
