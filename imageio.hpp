#pragma once
#include <memory>
#include <string>

class Image {

};

void WritePNGfromChar(const std::string& filename, char* data, uint32_t width, uint32_t height, uint16_t numChannel);

void WriteTIFFfromFloat(const std::string& filename, float* data, uint32_t width, uint32_t height, uint16_t numChannel, uint16_t numBitChannel);