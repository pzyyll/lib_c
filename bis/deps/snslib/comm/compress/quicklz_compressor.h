#ifndef PETLIB_QUICKLZ_COMPRESSOR_H_
#define PETLIB_QUICKLZ_COMPRESSOR_H_

#include "quicklz.h"

//#include "compress_factory.h"
#include "base_compressor.h"

namespace snslib
{
class QuickLZCompressor : public BaseCompressor
{
public:
  QuickLZCompressor();

  virtual ~QuickLZCompressor();

  virtual int Compress(const StaticBuffer &input, DynamicBuffer &output);

  virtual int Decompress(const StaticBuffer &input, DynamicBuffer &output);

  virtual int GetAlgorithm() { return QUICKLZ; }

  virtual const char *GetErrMsg() { return m_err_msg; }

private: 
  char m_err_msg[4096];
  char m_quicklz_scratch[QLZ_SCRATCH_DECOMPRESS > QLZ_SCRATCH_COMPRESS ?
    QLZ_SCRATCH_DECOMPRESS : QLZ_SCRATCH_COMPRESS];
};
}

#endif
