#ifndef CONVERT_H
#define CONVERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
        /**** BMP file header structure ****/
{
    unsigned short bftype;
    unsigned int bfSize; /* Size of file */
    unsigned short bfReserved1; /* Reserved */
    unsigned short bfReserved2; /* ... */
    unsigned int bfOffBits;
    /* Offset to bitmap data */
} BITMAPFILEHEADER;

typedef struct /**** BMP file info structure ****/
{
    unsigned int biSize; /* Size of info header */
    int biWidth; /* Width of image */
    int biHeight; /* Height of image */
    unsigned short biPlanes; /* Number of color planes */
    unsigned short biBitCount; /* Number of bits per pixel */
    unsigned int biCompression; /* Type of compression to use */
    unsigned int biSizeImage; /* Size of image data */
    int biXPelsPerMeter; /* X pixels per meter */
    int biYPelsPerMeter; /* Y pixels per meter */
    unsigned int biClrUsed; /* Number of colors used */
    unsigned int biClrImportant; /* Number of important colors */
} BITMAPINFOHEADER;

void NV12_T_RGB(unsigned int width, unsigned int height, unsigned char *yuyv, unsigned char *rgb);
void BGGR2GRBG(unsigned int width, unsigned int height, unsigned char *bggr, unsigned char *grbg);
void GRBG2BGR(unsigned int width, unsigned int height, unsigned  char * grbg, unsigned char * bgrOutputDat);
void rgb_adjust(unsigned int width, unsigned int height, unsigned char *rgb, double n);
void SaveBmp(const char *filename, unsigned char *rgb, int width, int height);

#endif // CONVERT_H
