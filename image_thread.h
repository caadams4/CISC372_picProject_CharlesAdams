#ifndef ___IMAGE
#define ___IMAGE
#include <stdint.h>

#define Index(x,y,width,bit,bpp) y*width*bpp+bpp*x+bit

typedef struct{
    uint8_t* data;
    int width;
    int height;
    int bpp;
} Image;

typedef double Matrix[3][3];

enum KernelTypes{EDGE=0,SHARPEN=1,BLUR=2,GAUSE_BLUR=3,EMBOSS=4,IDENTITY=5};

typedef struct{
    int thread;
    Image *srcImage;
    Image *destImage;
    enum KernelTypes type;
} Thing;


uint8_t getPixelValue(int x,int y,int bit,Matrix algorithm);
void convolute(int i_am_thread);
int Usage();
enum KernelTypes GetKernelType(char* type);

#endif
