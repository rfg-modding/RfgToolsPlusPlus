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
		inflateStream.avail_in = (u32)input.size_bytes();
		inflateStream.next_in = input.data();
		inflateStream.avail_out = (u32)output.size_bytes();
		inflateStream.next_out = output.data();

		inflateInit(&inflateStream);
		inflate(&inflateStream, Z_NO_FLUSH);
		inflateEnd(&inflateStream);
    }

	DeflateResult Deflate(std::span<u8> input)
	{
		uLong deflateUpperBound = compressBound(input.size_bytes());
		uLongf destLen = deflateUpperBound;
		char* dest = new char[deflateUpperBound];

		compress2((Bytef*)dest, &destLen, (Bytef*)input.data(), input.size_bytes(), Z_BEST_SPEED);
		return DeflateResult
		{
			.Buffer = (u8*)dest,
			.BufferSize = deflateUpperBound,
			.DataSize = destLen
		};
	}
}