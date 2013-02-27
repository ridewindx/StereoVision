#ifndef GRAYCODEC_H
#define GRAYCODEC_H

class GrayCodec
{
public:
    GrayCodec(int width, int height);

    void grayStripesToBinary(unsigned char *frames, int nBits, unsigned char *frameBK, unsigned char *frameWT, int offset, int *code);

    unsigned int grayToBinary(unsigned int graycode);
    unsigned int binaryToGray(unsigned int num);

private:
    int width;
    int height;
    int size;
};

#endif // GRAYCODEC_H
