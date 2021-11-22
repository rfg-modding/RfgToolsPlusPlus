#include "Compression.h"
//Todo: Currently have manually placed copy of zconf.h in here because of zlib cmake scripts renaming it. Need to automate that fix
#include <zlib.h>

namespace Compression
{
    void Inflate(std::span<u8> input, std::span<u8> output)
    {
        z_stream inflateStream;
        inflateStream.zalloc = Z_NULL;
        inflateStream.zfree = Z_NULL;
        inflateStream.opaque = Z_NULL;
        inflateStream.avail_in = static_cast<u32>(input.size_bytes());
        inflateStream.next_in = input.data();
        inflateStream.avail_out = static_cast<u32>(output.size_bytes());
        inflateStream.next_out = output.data();

        inflateInit(&inflateStream);
        inflate(&inflateStream, Z_NO_FLUSH);
        inflateEnd(&inflateStream);
    }

    DeflateResult Deflate(std::span<u8> input)
    {
        uLong deflateUpperBound = compressBound(static_cast<uLong>(input.size_bytes()));
        uLongf destLen = deflateUpperBound;
        Bytef* dest = new Bytef[deflateUpperBound];

        compress2(dest, &destLen, static_cast<Bytef*>(input.data()), static_cast<uLong>(input.size_bytes()), Z_BEST_SPEED);
        return DeflateResult
        {
            .Buffer = static_cast<u8*>(dest),
            .BufferSize = deflateUpperBound,
            .DataSize = destLen
        };
    }
}