#include "common/Typedefs.h"
#include <span>

namespace Compression
{
    //Todo: Rewrite this function to handle errors and provide more input options
    //Simple and temporary inflate function. Expects output buffer to be perfectly sized and does no error checking
    void Inflate(std::span<u8> input, std::span<u8> output);
}