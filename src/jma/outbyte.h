/*
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )
Copyright (C) 2001-4 Igor Pavlov ( http://www.7-zip.org )

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __STREAM_OUTBYTE_H
#define __STREAM_OUTBYTE_H

#include "portable.h"
#include "iiostrm.h"

namespace NStream {

class COutByte
{
  BYTE *m_Buffer;
  UINT32 m_Pos;
  UINT32 m_BufferSize;
  ISequentialOutStream* m_Stream;
  UINT64 m_ProcessedSize;

  void WriteBlock();
public:
  COutByte(UINT32 aBufferSize = (1 << 20));
  ~COutByte();

  void Init(ISequentialOutStream *aStream);
  HRESULT Flush();

  void WriteByte(BYTE aByte)
  {
    m_Buffer[m_Pos++] = aByte;
    if(m_Pos >= m_BufferSize)
      WriteBlock();
  }
  void WriteBytes(const void *aBytes, UINT32 aSize)
  {
    for (UINT32 i = 0; i < aSize; i++)
      WriteByte(((const BYTE *)aBytes)[i]);
  }

  UINT64 GetProcessedSize() const { return m_ProcessedSize + m_Pos; }
};

}

#endif
