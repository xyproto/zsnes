#ifndef __COMPRESSION_BITCODER_H
#define __COMPRESSION_BITCODER_H

#include "rngcoder.h"

namespace NCompression {
namespace NArithmetic {

const int kNumBitModelTotalBits  = 11;
const UINT32 kBitModelTotal = (1 << kNumBitModelTotalBits);

const int kNumMoveReducingBits = 2;

template <int aNumMoveBits>
class CBitDecoder: public CBitModel<aNumMoveBits>
{
public:
  UINT32 Decode(CRangeDecoder *aRangeDecoder)
  {
    UINT32 aNewBound = (aRangeDecoder->m_Range >> kNumBitModelTotalBits) * CBitModel<aNumMoveBits>::m_Probability;
    if (aRangeDecoder->m_Code < aNewBound)
    {
      aRangeDecoder->m_Range = aNewBound;
      CBitModel<aNumMoveBits>::m_Probability += (kBitModelTotal - CBitModel<aNumMoveBits>::m_Probability) >> aNumMoveBits;
      if (aRangeDecoder->m_Range < kTopValue)
      {
        aRangeDecoder->m_Code = (aRangeDecoder->m_Code << 8) | aRangeDecoder->m_Stream.ReadByte();
        aRangeDecoder->m_Range <<= 8;
      }
      return 0;
    }
    else
    {
      aRangeDecoder->m_Range -= aNewBound;
      aRangeDecoder->m_Code -= aNewBound;
      CBitModel<aNumMoveBits>::m_Probability -= (CBitModel<aNumMoveBits>::m_Probability) >> aNumMoveBits;
      if (aRangeDecoder->m_Range < kTopValue)
      {
        aRangeDecoder->m_Code = (aRangeDecoder->m_Code << 8) | aRangeDecoder->m_Stream.ReadByte();
        aRangeDecoder->m_Range <<= 8;
      }
      return 1;
    }
  }
};

}}


#endif
