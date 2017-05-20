#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "comm/buffer/static_buffer.h"
#include "comm/buffer/dynamic_buffer.h"

#include "zlib_compressor.h"

namespace snslib
{
ZlibCompressor::ZlibCompressor(int level) :
  m_level(level)
{
  memset(&m_deflate_stream, 0, sizeof(m_deflate_stream));
  m_deflate_stream.zalloc = Z_NULL;
  m_deflate_stream.zfree = Z_NULL;
  m_deflate_stream.opaque = Z_NULL;

  int ret = deflateInit(&m_deflate_stream, m_level);
  assert(ret == Z_OK);

  memset(&m_inflate_stream, 0, sizeof(m_inflate_stream));
  m_inflate_stream.zalloc = Z_NULL;
  m_inflate_stream.zfree = Z_NULL;
  m_inflate_stream.opaque = Z_NULL;
  m_inflate_stream.avail_in = 0;
  m_inflate_stream.next_in = Z_NULL;
  ret = inflateInit(&m_inflate_stream);
  assert(ret == Z_OK);
}

ZlibCompressor::~ZlibCompressor()
{
  deflateEnd(&m_deflate_stream);
  inflateEnd(&m_inflate_stream);
}

// compression result of "abcdefghijklmnopqrstuvwxyz" will have an trailing space
int ZlibCompressor::Compress(const StaticBuffer &input, DynamicBuffer &output)
{
  if (input.Remain() == 0)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "compress empty input failed");
    return -1;
  }

  ::deflateReset(&m_deflate_stream);

  uint32_t avail_out = input.Remain() + 6 + (((input.Remain() / 16000) + 1) * 5);

  output.Reserve(avail_out + 4);

  m_deflate_stream.avail_in = input.Remain();
  m_deflate_stream.next_in = (Byte *)input.Str();

  m_deflate_stream.avail_out = avail_out;
  char *pout = output.Borrow(4);
  *(int *)pout = input.Remain(); // TODO: write little-endian?
  m_deflate_stream.next_out = (Byte *)(pout + 4);

  int ret = ::deflate(&m_deflate_stream, Z_FINISH);
  if (ret != Z_STREAM_END)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "compress deflate failed, ret=%d",
      ret);
    return -2;
  }

  uint32_t zlen = avail_out - m_deflate_stream.avail_out;
  output.Advance(zlen + 4);

  return 0;
}

int ZlibCompressor::Decompress(const StaticBuffer &input, DynamicBuffer &output)
{
  if (input.Remain() < 4)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress input size %ld",
      input.Remain());
    return -1;
  }

  const char *ptr = input.Str(); 
  size_t remaining = input.Remain();

  size_t data_length =  *(const int *)ptr;

  if (data_length > 0x100000)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress input first 4 bytes %ld", data_length);
    return -1;
  }

  output.Reserve(data_length);

  ::inflateReset(&m_inflate_stream);

  m_inflate_stream.avail_in = remaining;
  m_inflate_stream.next_in = (Byte *)(ptr + 4);

  m_inflate_stream.avail_out = data_length;
  m_inflate_stream.next_out = (Byte *)output.Borrow(1);

  int ret = ::inflate(&m_inflate_stream, Z_NO_FLUSH);
  if (ret != Z_STREAM_END)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress inflate failed, ret=%d",
      ret);
    return -2;
  }
  
  if (m_inflate_stream.avail_out != 0)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress inflate stream avail out %d",
      m_inflate_stream.avail_out);
    return -3;
  }

  output.Advance(data_length);

  return 0;
}
}
