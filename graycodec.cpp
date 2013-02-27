#include "graycodec.h"

GrayCodec::GrayCodec(int width, int height)
{
    this->width=width;
    this->height=height;
    this->size=width*height;
}

void GrayCodec::grayStripesToBinary(unsigned char *frames, int nBits, unsigned char *frameBK, unsigned char *frameWT, int offset, int *code)
{
    for(int i=0;i<size;i++){
        code[i]=0;
    }
    for(int i=0;i<nBits;i++){
        for(int j=0;j<size;j++){
            if( (frames[i*size+j]-frameBK[j]) < (frameWT[j]-frames[i*size+j]) )
                code[j]=(code[j]<<1) | 0; //Black
            else
                code[j]=(code[j]<<1) | 1; //White
        }
    }
    for(int i=0;i<size;i++){
        code[i]=grayToBinary(code[i])-offset; //Subtract the offset
    }
}

unsigned int GrayCodec::grayToBinary(unsigned int num)
{   //Convert gray code to binary code
    unsigned int numBits = 8*sizeof(num);
    for (unsigned int shift = 1; shift < numBits; shift = 2*shift)
    {
        num = num ^ (num >> shift);
    }
    return num;
}

unsigned int GrayCodec::binaryToGray(unsigned int num)
{   //Convert binary code to gray code
    return (num >> 1) ^ num;
}
