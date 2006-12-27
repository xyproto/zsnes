/*
Copyright (C) 2005-2007 Nach, grinvader ( http://www.zsnes.com )

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

/*
This is part of a toolkit used to assist in ZSNES development

This program tells us is a variable is declared in the wrong section.
Or if code is not in an executable section.
*/

#include <iostream>
#include <fstream>
#include "strutil.h"
using namespace std;

#include "fileutil.h"

#define LINE_LENGTH 500



bool contains_resx(const char *str)
{
  return(!strncmp(str, "resd ", strlen("resd ")) ||
         !strncmp(str, "resw ", strlen("resw ")) ||
         !strncmp(str, "resb ", strlen("resb ")) ||
         strstr(str, " resd ") ||
         strstr(str, " resw ") ||
         strstr(str, " resb ") ||
         strstr(str, ",resd ") ||
         strstr(str, ",resw ") ||
         strstr(str, ",resb "));
}

bool contains_dx(const char *str)
{
  return(!strncmp(str, "dd ", strlen("dd ")) ||
         !strncmp(str, "dw ", strlen("dw ")) ||
         !strncmp(str, "db ", strlen("db ")) ||
         strstr(str, " dd ") ||
         strstr(str, " dw ") ||
         strstr(str, " db ") ||
         strstr(str, ",dd ") ||
         strstr(str, ",dw ") ||
         strstr(str, ",db "));
}

bool label(const char *str)
{
  return(!strchr(str, ' ') && !strchr(str, '\t') && (str[strlen(str)-1] == ':'));
}

void handle_file(const char *filename)
{
  enum sections { sec_unknown, sec_bss, sec_data, sec_text, sec_macro };

  ifstream file(filename, ios::in);
  if (file)
  {
    char buffer[LINE_LENGTH];
    sections cur_section = sec_unknown, prev_section = sec_unknown;
    for (size_t i = 1; file.getline(buffer, LINE_LENGTH); i++)
    {
      char *line = buffer;

      char *comment_p = strchr(line, ';');
      if (comment_p) { *comment_p = 0; }

      if (all_whitespace(line)) { continue; }

      for (char *p = line+strlen(line)-1; isspace(*p); p--) { *p = 0; }
      while (isspace(*line)) { line++; }


      if (!strcasecmp(line, "SECTION .BSS"))
      {
        prev_section = cur_section;
        cur_section = sec_bss;
        continue;
      }

      if (!strcasecmp(line, "SECTION .DATA"))
      {
        prev_section = cur_section;
        cur_section = sec_data;
        continue;
      }

      if (!strcasecmp(line, "SECTION .text"))
      {
        prev_section = cur_section;
        cur_section = sec_text;
        continue;
      }

      if (!strncmp(line, "%macro", strlen("%macro")) ||
          !strncmp(line, "%imacro", strlen("%imacro")))
      {
        prev_section = cur_section;
        cur_section = sec_macro;
        continue;
      }

      if (!strncmp(line, "%endmacro", strlen("%endmacro")))
      {
        cur_section = prev_section;
        continue;
      }


      if ((cur_section != sec_bss) && contains_resx(line))
      {
        cout << filename << ": line " << i << ": Error, resx in non BSS section. \"" << line << "\"" << endl;
      }

      if ((cur_section != sec_data) && contains_dx(line))
      {
        cout << filename << ": line " << i << ": Error, dx in non DATA section. \"" << line << "\"" << endl;
      }

      if ((cur_section != sec_text) && (cur_section != sec_macro))
      {
        if (!contains_resx(line) && !contains_dx(line) && !label(line) && !strstr(line, "RMREGS") &&
            !strstr(line, "NEWSYM") && !strstr(line, "EXTSYM") && !strstr(line, " equ ") &&
            (*line != '%') && strncasecmp(line, "ALIGN", strlen("ALIGN")) &&
            strncasecmp(line, "bits ", strlen("bits ")))
        {
          cout << filename << ": line " << i << ": Error, code in non TEXT section. \"" << line << "\"" << endl;
        }
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
