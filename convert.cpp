#include "convert.h"

void NV12_T_RGB(unsigned int width, unsigned int height, unsigned char *yuyv, unsigned char *rgb)
{
    const unsigned int nv_start = width * height ;
    unsigned int  i, j, index = 0, rgb_index = 0;
    unsigned char y, u, v;
    unsigned int nv_index = 0;
    int r, g, b;


    for(i = 0; i <  height ; i++)
    {
        for(j = 0; j < width; j ++){
            //nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
            nv_index = i / 2  * width + j - j % 2;

            y = yuyv[rgb_index];
            v = yuyv[nv_start + nv_index ];
            u = yuyv[nv_start + nv_index + 1];


            r = y + (140 * (v-128))/100;  //r
            g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
            b = y + (177 * (u-128))/100; //b

            if(r > 255)   r = 255;
            if(g > 255)   g = 255;
            if(b > 255)   b = 255;
            if(r < 0)     r = 0;
            if(g < 0)     g = 0;
            if(b < 0)     b = 0;

            index = rgb_index % width + (height - i - 1) * width;
            rgb[index * 3+0] = (unsigned char)b;
            rgb[index * 3+1] = (unsigned char)g;
            rgb[index * 3+2] = (unsigned char)r;
            rgb_index++;
        }
    }
    return;
}

void BGGR2GRBG(unsigned int width, unsigned int height, unsigned char *bggr, unsigned char *grbg)
{
    unsigned int i;
    for (i = 0; i<height; i++)
    {
        memcpy(grbg+i*width, bggr+(height-1-i)*width, width);
    }
}

void GRBG2BGR(unsigned int width, unsigned int height, unsigned  char * grbg, unsigned char * bgrOutputDat)
{
    unsigned  char* newimagedata_start = bgrOutputDat;

    unsigned int currentTempIndex = 0;
    unsigned int nearestBluesAvg = 0;
    unsigned int nearestRedsAvg = 0;
    unsigned int nearestGreensAvg = 0;

    for(unsigned int j = 0; j < (height/2); j++)
    {
        for(unsigned int i = 0; i < width; i++) //G R G R G...
        {
            if(currentTempIndex % 2 == 0 /* even, green */)
            {
                //avg blue
                if(j == 0) //if in the first row, only take next blue
                {
                    nearestBluesAvg = *(grbg+currentTempIndex+width);
                }
                else
                {
                    nearestBluesAvg = (*(grbg + currentTempIndex + width) + *(grbg+currentTempIndex-width)) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestBluesAvg; //b
                bgrOutputDat++;
                *bgrOutputDat = *(grbg + currentTempIndex); //g
                bgrOutputDat++;
                //avg red
                if(i == 0) //if in first column, only take next red
                {
                    nearestRedsAvg = *(grbg+currentTempIndex+1);
                }
                else
                {
                    nearestRedsAvg = ( (*(grbg+currentTempIndex+1)) + (*(grbg+currentTempIndex-1)) ) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestRedsAvg; //r
                bgrOutputDat++;

                currentTempIndex++;
            }
            else /* odd, red*/
            {
                //avg blue
                if(i == (height/2 -1)) //if in last column, take just left-down blue pixel
                {
                    nearestBluesAvg = *(grbg+currentTempIndex-1+width);
                }
                else // else take both left-down and right-down
                {
                    nearestBluesAvg = (*(grbg+currentTempIndex+1+width) + *(grbg+currentTempIndex-1+width)) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestBluesAvg; //b
                bgrOutputDat++;
                //avg green
                nearestGreensAvg = (*(grbg+currentTempIndex-1) + *(grbg+currentTempIndex+width)) / 2;
                *bgrOutputDat = (unsigned char)nearestGreensAvg;  //g
                bgrOutputDat++;
                *bgrOutputDat = *(grbg + currentTempIndex); //r
                bgrOutputDat++;

                currentTempIndex++;
            }
        }
        for(unsigned int i = 0; i < width; i++)//B G B G B G B....
        {
            if(currentTempIndex % 2 == 0 /* even, blue */)
            {

                *bgrOutputDat = *(grbg + currentTempIndex); //b
                bgrOutputDat++;
                //avg green
                nearestGreensAvg = (*(grbg + currentTempIndex + 1) + *(grbg + currentTempIndex -width)) / 2;
                *bgrOutputDat = (unsigned char)nearestGreensAvg; //g
                bgrOutputDat++;
                //avg red
                if(i == 0) //if first column, take only right-up pixel
                {
                    nearestRedsAvg = *(grbg+currentTempIndex+1-width);
                }
                else //else take both left-up and right-up pixels
                {
                    nearestRedsAvg = (*(grbg+currentTempIndex-1-width) + *(grbg+currentTempIndex+1-width)) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestRedsAvg; //r
                bgrOutputDat++;

                currentTempIndex++;

            }
            else /* odd, green*/
            {
                //avg blue
                if(i == width) //if in last column, only take previous blue (next blue doesnt exist)
                {
                    nearestBluesAvg = *(grbg + currentTempIndex - 1);
                }
                else //else take both next and previous
                {
                    nearestBluesAvg = (*(grbg+currentTempIndex+1) + *(grbg+currentTempIndex-1)) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestBluesAvg; //b
                bgrOutputDat++;
                *bgrOutputDat = *(grbg + currentTempIndex); //g
                bgrOutputDat++;
                //avg red
                if(j == (height/2 -1)) //if in last row, only take previous red (next red doesn't exist)
                {
                    nearestRedsAvg = *(grbg+currentTempIndex-width);
                }
                else //else take both
                {
                    nearestRedsAvg = (*(grbg+currentTempIndex+width) + *(grbg+currentTempIndex-width)) / 2;
                }
                *bgrOutputDat = (unsigned char)nearestRedsAvg; //r
                bgrOutputDat++;

                currentTempIndex++;
            }
        }
    }


    bgrOutputDat = newimagedata_start;

    //printf("ConvertBayer8ToBGR succeed!\n");
}

void rgb_adjust(unsigned int width, unsigned int height, unsigned char *rgb, double n)
{
    unsigned int i;
    for (i = 1; i < width*height*3; i += 3)
    {
        if ((*(rgb + i)==255) && (*(rgb + i + 1)==255) && (*(rgb + i - 1)==255))
        {
            continue;
        }
        if ((*(rgb + i)>253))
        {
            *(rgb + i) = (unsigned char)pow(*(rgb+i), 0.957);
            *(rgb + i+1) = (unsigned char)pow(*(rgb+i), 0.985);
            *(rgb + i-1) = (unsigned char)pow(*(rgb+i), 0.98);
            continue;
        }
        *(rgb + i) = (unsigned char)pow(*(rgb+i), n);

    }
}

void SaveBmp1(const char *filename, unsigned char *rgb, int width, int height)
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    bf.bftype = 0x4d42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    /*其中这两项为何减去2，是因为结构体对齐问题，sizeof(BITMAPFILEHEADER)并不是14，而是16*/
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+width*height*3-2;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)-2;//0x36
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = (unsigned int)width;
    bi.biHeight = -height;//
    bi.biBitCount = 24;
    bi.biCompression = 0;
    bi.biSizeImage = width*height*3;
    bi.biXPelsPerMeter = 5000;
    bi.biYPelsPerMeter = 5000;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    FILE *file = fopen(filename,"wb");
    if(!file)
    {
        printf("file open failed\n");
    }

    fwrite(&bf.bftype,2,1,file);
    fwrite((&bf.bftype)+2,12,1,file);
    fwrite(&bi,sizeof(bi),1,file);
    fwrite(rgb,width*height*3,1,file);
    fclose(file);
    // printf("SaveBmp succeed!\n");
    return;
}
