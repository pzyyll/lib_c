#ifndef PETLIB_ZLIB_COMPRESSOR_H_
#define PETLIB_ZLIB_COMPRESSOR_H_

#include <zlib.h>

#include "base_compressor.h"

namespace snslib
{
class ZlibCompressor : public BaseCompressor
{
public:
  ZlibCompressor(int level = Z_DEFAULT_COMPRESSION);

  virtual ~ZlibCompressor();

  virtual int Compress(const StaticBuffer &input, DynamicBuffer &output);

  virtual int Decompress(const StaticBuffer &input, DynamicBuffer &output);

  virtual int GetAlgorithm() { return ZLIB; }

  virtual const char *GetErrMsg() { return m_err_msg; }

private: 
  z_stream  m_inflate_stream;
  z_stream  m_deflate_stream;
  int       m_level;

  char m_err_msg[4096];
};
}

#endif
