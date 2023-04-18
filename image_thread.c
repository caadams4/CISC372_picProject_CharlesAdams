#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "image_thread.h"
#include <pthread.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int thread_count = 0;

//An array of kernel matrices to be used for image convolution.  
//The indexes of these match the enumeration from the header file. ie. algorithms[BLUR] returns the kernel corresponding to a box blur.
Matrix algorithms[]={
    {{0,-1,0},{-1,4,-1},{0,-1,0}},
    {{0,-1,0},{-1,5,-1},{0,-1,0}},
    {{1/9.0,1/9.0,1/9.0},{1/9.0,1/9.0,1/9.0},{1/9.0,1/9.0,1/9.0}},
    {{1.0/16,1.0/8,1.0/16},{1.0/8,1.0/4,1.0/8},{1.0/16,1.0/8,1.0/16}},
    {{-2,-1,0},{-1,1,1},{0,1,2}},
    {{0,0,0},{0,1,0},{0,0,0}}
};

Thing thread_struct;
Image srcImage,destImage,bwImage;
//getPixelValue - Computes the value of a specific pixel on a specific channel using the selected convolution kernel
//Paramters: srcImage:  An Image struct populated with the image being convoluted
//           x: The x coordinate of the pixel
//          y: The y coordinate of the pixel
//          bit: The color channel being manipulated
//          algorithm: The 3x3 kernel matrix to use for the convolution
//Returns: The new value for this x,y pixel and bit channel
uint8_t getPixelValue(int x,int y,int bit,Matrix algorithm){
    int px,mx,py,my,i,span;
    span=srcImage.width*srcImage.bpp;
    // for the edge pixes, just reuse the edge pixel
    px=x+1; py=y+1; mx=x-1; my=y-1;
    if (mx<0) mx=0;
    if (my<0) my=0;
    if (px>=srcImage.width) px=srcImage.width-1;
    if (py>=srcImage.height) py=srcImage.height-1;
    uint8_t result=
        algorithm[0][0]*srcImage.data[Index(mx,my,srcImage.width,bit,srcImage.bpp)]+
        algorithm[0][1]*srcImage.data[Index(x,my,srcImage.width,bit,srcImage.bpp)]+
        algorithm[0][2]*srcImage.data[Index(px,my,srcImage.width,bit,srcImage.bpp)]+
        algorithm[1][0]*srcImage.data[Index(mx,y,srcImage.width,bit,srcImage.bpp)]+
        algorithm[1][1]*srcImage.data[Index(x,y,srcImage.width,bit,srcImage.bpp)]+
        algorithm[1][2]*srcImage.data[Index(px,y,srcImage.width,bit,srcImage.bpp)]+
        algorithm[2][0]*srcImage.data[Index(mx,py,srcImage.width,bit,srcImage.bpp)]+
        algorithm[2][1]*srcImage.data[Index(x,py,srcImage.width,bit,srcImage.bpp)]+
        algorithm[2][2]*srcImage.data[Index(px,py,srcImage.width,bit,srcImage.bpp)];
    return result;
}

//convolute:  Applies a kernel matrix to an image
//Parameters: srcImage: The image being convoluted
//            destImage: A pointer to a  pre-allocated (including space for the pixel array) structure to receive the convoluted image.  It should be the same size as srcImage
//            algorithm: The kernel matrix to use for the convolution
//Returns: Nothing
void convolute(int i_am_thread){
puts("GRANTED"); 
    printf("%p\n",__builtin_return_address(0));
   
int thread = i_am_thread;
    int row,pix,bit,span;
    enum KernelTypes type = thread_struct.type;
    printf("Thread num: %i\n",thread);
puts("4");
    span=srcImage.bpp*srcImage.bpp;
long count = 0;    
    // FOR EACH bit in each pixel in each row
    printf("Height: %i, thread_count: %i\n",srcImage.height,thread_count);

    int remainder = srcImage.height % (thread_count+1);
    int chunk = (srcImage.height-remainder) / (thread_count+1);
    int lower_lim = (chunk * thread) + remainder;
    int upper_lim = (chunk * (thread+1)) + remainder;
    printf("CONVOLUTE THREAD: %i, lower: %i, upper: %i, chunk: %i\n",thread,lower_lim,upper_lim,chunk);

    for (row=lower_lim;row<upper_lim;row++){
       for (pix=0;pix<srcImage.width;pix++){
for (bit=0;bit<srcImage.bpp;bit++){
	    

     destImage.data[Index(pix,row,srcImage.width,bit,srcImage.bpp)]=getPixelValue(pix,row,bit,algorithms[type]);
            //destImage->data[Index(pix,row,srcImage->width,bit,srcImage->bpp)]=getPixelValue(srcImage,pix,row,bit,algorithms[type]); 
	    }
        }
//printf("row: %i\n",row);
    
}
puts("11");

    printf("%p\n",__builtin_return_address(0));
return;
}

//Usage: Prints usage information for the program
//Returns: -1
int Usage(){
    printf("Usage: image <filename> <type>\n\twhere type is one of (edge,sharpen,blur,gauss,emboss,identity)\n");
    return -1;
}

//GetKernelType: Converts the string name of a convolution into a value from the KernelTypes enumeration
//Parameters: type: A string representation of the type
//Returns: an appropriate entry from the KernelTypes enumeration, defaults to IDENTITY, which does nothing but copy the image.
enum KernelTypes GetKernelType(char* type){
    if (!strcmp(type,"edge")) return EDGE;
    else if (!strcmp(type,"sharpen")) return SHARPEN;
    else if (!strcmp(type,"blur")) return BLUR;
    else if (!strcmp(type,"gauss")) return GAUSE_BLUR;
    else if (!strcmp(type,"emboss")) return EMBOSS;
    else return IDENTITY;
}

//main:
//argv is expected to take 2 arguments.  First is the source file name (can be jpg, png, bmp, tga).  Second is the lower case name of the algorithm.
int main(int argc,char** argv){
    pthread_t *thread_handles;
    thread_handles = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
	puts("0");	
    long t1,t2;
    t1=time(NULL);

    stbi_set_flip_vertically_on_load(0); 
    if (argc!=3) return Usage();
    char* fileName=argv[1];
    if (!strcmp(argv[1],"pic4.jpg")&&!strcmp(argv[2],"gauss")){
        printf("You have applied a gaussian filter to Gauss which has caused a tear in the time-space continum.\n");
    }
    enum KernelTypes type=GetKernelType(argv[2]);
puts("1");
    //Image srcImage,destImage,bwImage;   
    srcImage.data=stbi_load(fileName,&srcImage.width,&srcImage.height,&srcImage.bpp,0);
    if (!srcImage.data){
        printf("Error loading file %s.\n",fileName);
        return -1;
    }
    destImage.bpp=srcImage.bpp;
    destImage.height=srcImage.height;
    destImage.width=srcImage.width;
    destImage.data=malloc(sizeof(uint8_t)*destImage.width*destImage.bpp*destImage.height);
 puts("2");
     
  

	puts("3");
 
    for (int thread = 0; thread <= thread_count; thread++) {
	pthread_create(&thread_handles[thread],NULL,&convolute,(void*)thread);
}
	puts("THANK YOU");
    for (int thread2 = 0; thread2 <= thread_count; thread2++) {
puts("HOLDING");
	pthread_join(thread_handles[thread2],NULL);
puts("OSCAR MIKE!");    
}
    stbi_write_png("output.png",destImage.width,destImage.height,destImage.bpp,destImage.data,destImage.bpp*destImage.width);
    stbi_image_free(srcImage.data);
    
    free(destImage.data);
    t2=time(NULL);
    printf("Took %ld seconds\n",t2-t1);
    return 0;
} 
