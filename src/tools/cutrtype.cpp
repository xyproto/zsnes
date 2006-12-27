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

This program removes redundant typecasting in reg<->memory transfers
*/

#include <iostream>
#include <fstream>
using namespace std;

#include "fileutil.h"
#include "strutil.h"

#define LINE_LENGTH 2048

static unsigned char getsize(const string_ci& token, const char mode)
{
  unsigned char val = 0;

  if (mode == 'r')
  {
    val = 0;

    if (token == "al" || token == "ah" || token == "bl" || token == "bh" ||
        token == "cl" || token == "ch" || token == "dl" || token == "dh" ||
        token == "di" || token == "si" || token == "bp")
    { val = 1; }
    else if (token == "ax" || token == "bx" || token == "cx" || token == "dx")
    { val = 2; }
    else if (token == "eax" || token == "ebx" || token == "ecx" ||
             token == "edx" || token == "edi" || token == "esi" ||
             token == "ebp")
    { val = 4; }
  }

  if (mode == 't')
  {
    val = 0xFF;

    if (token == "byte")  { val = 1; }
    if (token == "word")  { val = 2; }
    if (token == "dword") { val = 4; }
  }

  return (val);
}

static bool isredund(string_ci& cur_line, const vector<string_ci>& tokens, const char offset)
{
  if (getsize(tokens[offset], 't') == getsize(tokens[(offset+2)%3], 'r'))
  {
    size_t loc = cur_line.find(tokens[offset]);
    cur_line.erase(loc, cur_line.find(tokens[offset+1])-loc-1);
    return (true);
  }

  return (false);
}

void handle_file(const char *filename, size_t orig_fsize)
{
  bool modify_file = false;
  vector<string_ci> file_buffer;

  ifstream file(filename, ios::in);
  if (file)
  {
    char line[LINE_LENGTH];

    while (file.getline(line, LINE_LENGTH))
    {
      vector<string_ci> tokens;
      string_ci mline(line);
      char *p = line;
      while (isspace(*p)) { p++; }

      if (!strncasecmp(p, "mov ", strlen("mov ")))
      {
        p += strlen("mov ");
        while (isspace(*p)) { p++; }
        Tokenize(p, tokens, ";");
        string_ci not_commented = tokens[0];
        tokens.clear();
        Tokenize(not_commented, tokens, ", []");

        if (tokens.size()>2)
        {
          modify_file |= isredund(mline, tokens, 0) ||
                         isredund(mline, tokens, 1);
        }
      }

      file_buffer.push_back(mline);
    }

    file.close();
  }
  else
  {
    cerr << "Could not open " << filename << "." << endl;
  }

  if (modify_file)
  {
    ofstream file(filename, ios::out);
    if (file)
    {
      for (vector<string_ci>::iterator i = file_buffer.begin(); i != file_buffer.end(); i++)
      {
        file.write(i->data(), i->length());
        file << "\n";
      }
      size_t cur_fsize = file.tellp();
      file.close();
      cout << "Trimmed " << filename << " of " << orig_fsize-cur_fsize
           << " bytes." << endl;
    }
    else
    {
      cerr << filename
           << " has redundant typecasts, but a trimmed copy can't be saved."
           << endl;
    }
  }
}

void cut_redund(const char *filename, struct stat& stat_buffer)
{
  if (is_asm_file(filename))
  {
    handle_file(filename, stat_buffer.st_size);
  }
}

int main()
{
  parse_dir(".", cut_redund);
  return(0);
}
