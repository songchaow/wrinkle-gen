#include "imageio.hpp"
#include <string>
#include "ext/stb_image.write.h"

void WritePNGfromChar(const std::string& filename, char* data, uint32_t width, uint32_t height, uint16_t numChannel) {
    stbi_write_png(filename.c_str(), width, height, numChannel, data, width * numChannel);
}