/*
Copyright (C) 2004-2007 NSRT Team ( http://nsrt.edgeemu.com )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <vector>
using namespace std;

#include "zsnesjma.h"
#include "jma.h"

extern "C" {
extern unsigned char *romdata;
extern unsigned int curromspace;
extern unsigned int maxromspace;
}

void load_jma_file(const char *filename)
{
  try
  {
    JMA::jma_open JMAFile(filename);
    vector<JMA::jma_public_file_info> file_info = JMAFile.get_files_info();

    string our_file_name;
    size_t our_file_size = 0;

    for (vector<JMA::jma_public_file_info>::iterator i = file_info.begin(); i != file_info.end(); i++)
    {
      //Check for valid ROM based on size
      if ((i->size <= maxromspace+512) && (i->size > our_file_size))
      {
        our_file_name = i->name;
        our_file_size = i->size;
      }
    }

    if (!our_file_size)
    {
      return;
    }

    JMAFile.extract_file(our_file_name, romdata);

    curromspace = our_file_size;
  }
  catch (JMA::jma_errors jma_error)
  {
    //No need to do anything
  }
}

