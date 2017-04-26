#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "comm/buffer/static_buffer.h"
#include "comm/buffer/dynamic_buffer.h"

#include "quicklz_compressor.h"

namespace snslib
{
QuickLZCompressor::QuickLZCompressor()
{
}

QuickLZCompressor::~QuickLZCompressor()
{
}

int QuickLZCompressor::Compress(const StaticBuffer &input, DynamicBuffer &output)
{
  if (input.Remain() == 0)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "compress empty input failed");
    return -1;
  }

  uint32_t avail_out = input.Remain() + 400;
  
  output.Reserve(avail_out);

  size_t len = qlz_compress(input.Str(), output.Borrow(1),
      input.Remain(), m_quicklz_scratch);

  output.Advance(len); 
  
  return 0;
}

int QuickLZCompressor::Decompress(const StaticBuffer &input, DynamicBuffer &output)
{
  size_t data_length = qlz_size_decompressed(input.Str());
  if (data_length <= 0)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "qlz size decompressed [%ld] failed", data_length);
    return -1;
  }

  if (data_length > 0x100000)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress input first 4 bytes %ld", data_length);
    return -1;
  }

  output.Reserve(data_length);

  size_t len = qlz_decompress(input.Str(), output.Borrow(1),
      m_quicklz_scratch);

  if (len != data_length)
  {
    snprintf(m_err_msg, sizeof(m_err_msg) - 1, "decompress result len %ld, data length %ld",
      len, data_length);
    return -2;
  }
  
  output.Advance(data_length);

  return 0;
}
}
