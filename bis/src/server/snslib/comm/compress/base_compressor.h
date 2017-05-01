#ifndef SNSLIB_BASE_COMPRESSOR_H_
#define SNSLIB_BASE_COMPRESSOR_H_

namespace snslib
{
class StaticBuffer;
class DynamicBuffer;

class BaseCompressor
{
public:
  enum Algorithm
  {
    UNKNOWN = 0,
    ZLIB = 1,
    QUICKLZ = 2,
  };

  BaseCompressor() {}

  virtual ~BaseCompressor() {}

  virtual int Compress(const StaticBuffer &input, DynamicBuffer &output) = 0;
  virtual int Decompress(const StaticBuffer &input, DynamicBuffer &output) = 0;
 
  virtual int GetAlgorithm() = 0;

  virtual const char *GetErrMsg() = 0;

/*
  static int ParseAlgorithm(const char *algorithm)
  {
    if (strcmp(algorithm, "quicklz") == 0)
      return QUICKLZ;
    if (strcmp(algorithm, "zlib") == 0)
      return ZLIB;

    return 0;
  }

  static BaseCompressor* CreateCompressor(const std::string &algorithm)
  { return CreateCompressor(algorithm.c_str()); }

  static BaseCompressor* CreateCompressor(const char *algorithm)
  {
    switch (ParseAlgorithm(algorithm))
    {
      case QUICKLZ:
        return new QuickLZCompressor();
      case ZLIB:
        return new ZlibCompressor();
      default:
        return NULL;
    }
  }
*/
};
}

#endif
