/*
Copyright (C) 2005 Nach, grinvader ( http://www.zsnes.com )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
This is part of a toolkit used to assist in ZSNES development

This program tells us how far our port progress is.
*/

#include <iostream>
#include <string>
using namespace std;

#include "fileutil.h"

size_t c_count = 0;
size_t cpp_count = 0;
size_t asm_count = 0;
size_t psr_count = 0;

void size_tally(const char *filename, struct stat& stat_buffer)
{
  if      (is_c_file(filename))   { c_count += stat_buffer.st_size;   }
  else if (is_asm_file(filename)) { asm_count += stat_buffer.st_size; }
  else if (is_cpp_file(filename)) { cpp_count += stat_buffer.st_size; }
  else if (is_psr_file(filename)) { psr_count += stat_buffer.st_size; }
}

int main()
{
  parse_dir(".", size_tally);
  
  cout << "Total C files use " << c_count << " bytes.\n"
       << "Total C++ files use " << cpp_count << " bytes.\n"
       << "Total Assembly files use " << asm_count << " bytes.\n"
       << "PSR file uses " << psr_count << " bytes.\n"
       << "\n"
       << "ASM ratio: " << (float)(asm_count*100)/(float)(asm_count+c_count+cpp_count+psr_count)
       << "%" << endl;
  return(0);
}
