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

#include "outbyte.h"

namespace NStream {

COutByte::COutByte(UINT32 aBufferSize):
  m_BufferSize(aBufferSize)
{
  m_Buffer = new BYTE[m_BufferSize];
}

COutByte::~COutByte()
{
  delete []m_Buffer;
}

void COutByte::Init(ISequentialOutStream *aStream)
{
  m_Stream = aStream;
  m_ProcessedSize = 0;
  m_Pos = 0;
}

HRESULT COutByte::Flush()
{
  if (m_Pos == 0)
    return S_OK;
  UINT32 aProcessedSize;
  HRESULT aResult = m_Stream->Write(m_Buffer, m_Pos, &aProcessedSize);
  if (aResult != S_OK)
    return aResult;
  if (m_Pos != aProcessedSize)
    return E_FAIL;
  m_ProcessedSize += aProcessedSize;
  m_Pos = 0;
  return S_OK;
}

void COutByte::WriteBlock()
{
  HRESULT aResult = Flush();
  if (aResult != S_OK)
    throw aResult;
}

}
