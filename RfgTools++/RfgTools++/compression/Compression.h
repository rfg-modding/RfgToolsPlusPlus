#include "common/Typedefs.h"
#include <span>

namespace Compression
{
    struct DeflateResult
    {
        //Deflate data buffer
        u8* Buffer;
        //Total size of the buffer
        u64 BufferSize;
        //Size of the compressed data in the buffer, may be smaller than BufferSize
        u64 DataSize;
    };

    //Todo: Rewrite this function to handle errors and provide more input options
    //Simple and temporary inflate function. Expects output buffer to be perfectly sized and does no error checking
    void Inflate(std::span<u8> input, std::span<u8> output);
    //Deflates the input buffer and returns the result. Caller must free deflated memory
    DeflateResult Deflate(std::span<u8> input);
}