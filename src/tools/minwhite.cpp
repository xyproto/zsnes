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

This program trims unneeded at end of line whitespace.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "fileutil.h"

#define LINE_LENGTH 2048

void handle_file(const char *filename, size_t orig_file_size)
{
  bool file_modified = false;
  vector<string> file_buffer;

  ifstream file(filename, ios::in);
  if (file)
  {
    char line[LINE_LENGTH];

    while (file.getline(line, LINE_LENGTH))
    {
      for (char *p = line+strlen(line)-1; p >= line; p--)
      {
        if (strchr(" \t\r", *p))
        {
          *p = 0;
          file_modified = true;
        }
        else
        {
          break;
        }
      }
      file_buffer.push_back(line);
    }

    file.close();
  }
  else
  {
    cerr << "Could not open " << filename << "." << endl;
  }

  if (file_modified)
  {
    ofstream file(filename, ios::out);
    if (file)
    {
      for (vector<string>::iterator i = file_buffer.begin(); i != file_buffer.end(); i++)
      {
        file.write(i->data(), i->length());
        file << "\n";
      }
      size_t file_size = file.tellp();
      file.close();
      cout << "Trimmed " << filename << " of " << orig_file_size-file_size << " bytes." << endl;
    }
    else
    {
      cerr << filename << " has extra whitespace, but a trimmed copy can't be saved." << endl;
    }
  }
}

void force_trim(const char *filename, struct stat& stat_buffer)
{
  handle_file(filename, stat_buffer.st_size);
}

void trim_whitespace(const char *filename, struct stat& stat_buffer)
{
  if (is_c_file(filename) ||
      is_cpp_file(filename) ||
      is_asm_file(filename) ||
      is_psr_file(filename))
  {
    handle_file(filename, stat_buffer.st_size);
  }
}

int main(size_t argc, char **argv)
{
  if (argc > 1)
  {
    for (char **i = argv+1; *i; i++)
    {
      parse_path(*i, force_trim);
    }
  }
  else
  {
    parse_dir(".", trim_whitespace);
  }
  return(0);
}
