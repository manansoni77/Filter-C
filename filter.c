#include <stdint.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint16_t WORD;

typedef struct
{
    WORD   bfType;
    DWORD  bfSize;
    WORD   bfReserved1;
    WORD   bfReserved2;
    DWORD  bfOffBits;
} __attribute__((__packed__))
BITMAPFILEHEADER;

typedef struct
{
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} __attribute__((__packed__))
BITMAPINFOHEADER;

typedef struct
{
    BYTE  rgbtBlue;
    BYTE  rgbtGreen;
    BYTE  rgbtRed;
} __attribute__((__packed__))
RGBTRIPLE;

void grayscale(int height, int width, RGBTRIPLE image[height][width]);

void sepia(int height, int width, RGBTRIPLE image[height][width]);

void reflect(int height, int width, RGBTRIPLE image[height][width]);

void blur(int height, int width, RGBTRIPLE image[height][width]);

void edges(int height, int width, RGBTRIPLE image[height][width]);

int main()
{

    // Filters supported by programme
    char *filters = "bgrse";
    char in[25], out[25];
    char *infile, *outfile;
    char filter;
    printf("--------Filter--------\n\n");
    printf("Filters supported are -\nblur(b)\ngrayscale(g)\nreflect(r)\nsepia(s)\nedges(e)\n"); 
    printf("File to be applied filter to:\n");
    scanf("%s", in);
    printf("File to be saved as:\n");
    scanf("%s", out);
    printf("Filter to be applied?\n");
    scanf(" %c", &filter);

    infile = &in[0];
    outfile = &out[0];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 1;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 1;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Make sure that Bitmap Fileheader is a 24bit BMP 4.0 header
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 1;
    }

    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        fprintf(stderr, "Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 1;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over infile's scanlines
    for (int i = 0; i < height; i++)
    {
        // Read row into pixel array
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        // Skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // Filter image
    switch (filter)
    {
        // Blur
        case 'b':
            blur(height, width, image);
            break;

        // Grayscale
        case 'g':
            grayscale(height, width, image);
            break;

        // Reflection
        case 'r':
            reflect(height, width, image);
            break;

        // Sepia
        case 's':
            sepia(height, width, image);
            break;
        
        // Edges
        case 'e':
            edges(height, width, image);
            break;
            
        default:
            printf("Check Available Filters!\n");
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write new pixels to outfile
    for (int i = 0; i < height; i++)
    {
        // Write row to outfile
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

        // Write padding at end of row
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Free memory for image
    free(image);

    // Close infile
    fclose(inptr);

    // Close outfile
    fclose(outptr);

    return 0;
}

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE temp;
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            float t = (float)image[i][j].rgbtBlue + (float)image[i][j].rgbtGreen + (float)image[i][j].rgbtRed;
            t /= 3;
            t = round(t);
            temp.rgbtBlue = (BYTE) t;
            image[i][j].rgbtBlue = temp.rgbtBlue;
            image[i][j].rgbtRed = temp.rgbtBlue;
            image[i][j].rgbtGreen = temp.rgbtBlue;
        }
    }
    return;
}

// Convert image to sepia
void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            float ired = image[i][j].rgbtRed;
            float iblue = image[i][j].rgbtBlue;
            float igreen = image[i][j].rgbtGreen;
            float sred = .393 * ired + .769 * igreen + .189 * iblue;
            float sgreen = .349 * ired + .686 * igreen + .168 * iblue;
            float sblue = .272 * ired + .534 * igreen + .131 * iblue;
            sred = sred>255?255:sred;
            sblue = sblue>255?255:sblue;
            sgreen = sgreen>255?255:sgreen;
            sred = round(sred);
            sblue = round(sblue);
            sgreen = round(sgreen);
            image[i][j].rgbtRed = (BYTE) sred;
            image[i][j].rgbtBlue = (BYTE) sblue;
            image[i][j].rgbtGreen = (BYTE) sgreen;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE ref;
    for(int i=0;i<height;i++)
    {
        for(int j=0,w=width/2;j<w;j++)
        {
            ref = image[i][j];
            image[i][j] = image[i][width-j-1];
            image[i][width-j-1] = ref;
        }
    }
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            copy[i][j] = image[i][j];
        }
    }
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            float c = 0;
            float red=0,blue=0,green=0;
            for(int m=-1;m<2;m++)
            {
                for(int n=-1;n<2;n++)
                {
                    if(i+m < 0)
                    {
                        continue;
                    }
                    if(i+m >= height)
                    {
                        continue;
                    }if(j+n < 0)
                    {
                        continue;
                    }if(j+n >= width)
                    {
                        continue;
                    }
                    red = red + (float)copy[i+m][j+n].rgbtRed;
                    blue = blue + (float)copy[i+m][j+n].rgbtBlue;
                    green = green + (float)copy[i+m][j+n].rgbtGreen;
                    c++;
                }
            }
            red = red/c;
            red = round(red);
            blue = blue/c;
            blue = round(blue);
            green = green/c;
            green = round(green);
            image[i][j].rgbtRed = (BYTE) red;
            image[i][j].rgbtBlue = (BYTE) blue;
            image[i][j].rgbtGreen = (BYTE) green;
        }
    }
    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            copy[i][j] = image[i][j];
        }
    }
    float gxr,gxb,gxg,gyr,gyb,gyg,a,gr,gb,gg;
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            gxr=0,gxb=0,gxg=0,gyr=0,gyb=0,gyg=0,gr=0,gb=0,gg=0,a=0;
            for(int x=-1;x<2;x++)
            {
                for(int y=-1;y<2;y++)
                {
                    if(i+x<0 || i+x>=height)
                    {
                        continue;
                    }
                    if(j+y<0 || j+y>=width)
                    {
                        continue;
                    }
                    a=1;
                    if(x==0)
                    {
                        a=2;
                    }
                    gxr += (float)copy[i+x][j+y].rgbtRed*y*a;
                    gxb += (float)copy[i+x][j+y].rgbtBlue*y*a;
                    gxg += (float)copy[i+x][j+y].rgbtGreen*y*a;
                    a=1;
                    if(y==0)
                    {
                        a=2;
                    }
                    gyr += (float)copy[i+x][j+y].rgbtRed*x*a;
                    gyb += (float)copy[i+x][j+y].rgbtBlue*x*a;
                    gyg += (float)copy[i+x][j+y].rgbtGreen*x*a;
                }
            }
            gr = pow(gxr,2) + pow(gyr,2);
            gb = pow(gxb,2) + pow(gyb,2);
            gg = pow(gxg,2) + pow(gyg,2);
            gr = sqrt(gr);
            gr = gr>255?255:gr;
            gr = round(gr);
            gb = sqrt(gb);
            gb = gb>255?255:gb;
            gb = round(gb);
            gg = sqrt(gg);
            gg = gg>255?255:gg;
            gg = round(gg);
            image[i][j].rgbtRed = (BYTE) gr;
            image[i][j].rgbtBlue = (BYTE) gb;
            image[i][j].rgbtGreen = (BYTE) gg;
        }
    }
    return;
}
