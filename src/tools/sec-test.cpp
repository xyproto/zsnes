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

This program tells us is a variable is declared in the wrong section.
*/

#include <iostream>
#include <fstream>
using namespace std;

#include "fileutil.h"

#define LINE_LENGTH 500

void handle_file(const char *filename)
{
  enum sections { sec_unknown, sec_bss, sec_data, sec_text };

  ifstream file(filename, ios::in);
  if (file)
  {
    char line[LINE_LENGTH];
    sections cur_section = sec_unknown;
    for (size_t i = 0; file.getline(line, LINE_LENGTH); i++)
    {
      if (!strcasecmp(line, "SECTION .BSS")) { cur_section = sec_bss; }
      if (!strcasecmp(line, "SECTION .DATA")) { cur_section = sec_data; }
      if (!strcasecmp(line, "SECTION .text")) { cur_section = sec_text; }

      if ((cur_section != sec_bss) &&
          (strstr(line, " resd ") || strstr(line, " resw ") || strstr(line, " resb ") ||
          (strstr(line, ",resd ") || strstr(line, ",resw ") || strstr(line, ",resb ")) ))
      {
        cout << filename << ": line " << i << ": Error, resx in non BSS section. \"" << line << "\"" << endl;
      }

      if ((cur_section != sec_data) &&
          (strstr(line, " dd ") || strstr(line, " dw ") || strstr(line, " db ") ||
          (strstr(line, ",dd ") || strstr(line, ",dw ") || strstr(line, ",db ")) ))
      {
        cout << filename << ": line " << i << ": Error, dx in non DATA section. \"" << line << "\"" << endl;
      }
    }
  }
  else
  {
    cout << "Error opening: " << filename << endl;
  }
}

void section_test(const char *filename, struct stat& stat_buffer)
{
  if (is_asm_file(filename))
  {
    handle_file(filename);
  }
}

int main()
{
  parse_dir(".", section_test);
  return(0);
}
