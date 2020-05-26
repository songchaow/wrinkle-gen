#include "imageio.hpp"
#include <string>
#include "ext/stb_image.write.h"
#include "tiff.h"
#include "tiffio.h"
#include <cstring>

void WritePNGfromChar(const std::string& filename, char* data, uint32_t width, uint32_t height, uint16_t numChannel) {
    stbi_write_png(filename.c_str(), width, height, numChannel, data, width * numChannel);
}

void WriteTIFFfromFloat(const std::string& filename, float* data, uint32_t width, uint32_t height, uint16_t numChannel, uint16_t numBitChannel) {
    TIFF* tifFile = TIFFOpen(filename.c_str(), "w");
    TIFFSetField(tifFile, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tifFile, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tifFile, TIFFTAG_SAMPLESPERPIXEL, numChannel);
    TIFFSetField(tifFile, TIFFTAG_BITSPERSAMPLE, numBitChannel);
    TIFFSetField(tifFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tifFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tifFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tifFile, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);

    tsize_t linebytes = numChannel * width * numBitChannel / 8;
    tsize_t lineFloats = numChannel * width;
    float* lineBuffer = nullptr;
    if (TIFFScanlineSize(tifFile) >= linebytes)
        lineBuffer = (float*)_TIFFmalloc(linebytes);
    else
        lineBuffer = (float*)_TIFFmalloc(TIFFScanlineSize(tifFile));

    for (uint32_t row = 0; row < height; row++) {
        float* startPos = data + row * lineFloats;
        std::memcpy(lineBuffer, startPos, linebytes);
        if (TIFFWriteScanline(tifFile, lineBuffer, row, 0) < 0)
            break;
    }
    TIFFClose(tifFile);
    _TIFFfree(lineBuffer);
}
