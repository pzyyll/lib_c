#ifndef SNSLIB_COMPRESS_FACTORY_H_
#define SNSLIB_COMPRESS_FACTORY_H_
#include <string>
#include "base_compressor.h"
#include "quicklz_compressor.h"
#include "zlib_compressor.h"

namespace snslib
{
class StaticBuffer;
class DynamicBuffer;

class CompressFactory
{
public:
  enum Algorithm
  {
    UNKNOWN = 0,
    ZLIB = 1,
    QUICKLZ = 2,
  };

  CompressFactory() {}

  virtual ~CompressFactory() {}

//  virtual int Compress(const StaticBuffer &input, DynamicBuffer &output) = 0;
//  virtual int Decompress(const StaticBuffer &input, DynamicBuffer &output) = 0;
//
//  virtual int GetAlgorithm() = 0;
//
//  virtual const char *GetErrMsg() = 0;

  static int ParseAlgorithm(const char *algorithm){
	  if(strcmp(algorithm, "quicklz")==0){
		  return QUICKLZ;
	  }else if(strcmp(algorithm, "zlib")==0){
		  return ZLIB;
	  }else{
		  return UNKNOWN;
	  }
  }

  static BaseCompressor* CreateCompressor(const std::string &algorithm){
	  return CreateCompressor(algorithm.c_str());
  }

  static BaseCompressor* CreateCompressor(const char * algorithm){
	  switch (ParseAlgorithm(algorithm)){
	  case QUICKLZ:
	  {
		  BaseCompressor *p_quicklz_compressor;
		  p_quicklz_compressor = new QuickLZCompressor();
		  return p_quicklz_compressor;
	  }
	  case ZLIB:
	  {
		  BaseCompressor *p_zlib_compressor;
		  p_zlib_compressor = new ZlibCompressor();
		  return p_zlib_compressor;
	  }
	  default:
		  return NULL;
	  }
  }
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
