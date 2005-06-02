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

This program tells us if an assembly file has extra EXTSYMs.
*/

#include <iostream>
#include <fstream>
#include <set>
#include <list>
#include <queue>
#include "strutil.h"
using namespace std;

#include "fileutil.h"

#define LINE_LENGTH 500

set<string> ignore_include_file;

void handle_file(const char *filename)
{
  queue<string> included_files;
  set<string> extsyms;
  set<string> used_vars;
  list<string> not_used_extsyms;

  included_files.push(filename);
  do
  {
    string fname = included_files.front();
    included_files.pop();

    ifstream file(fname.c_str(), ios::in);
    if (file)
    {
      char line[LINE_LENGTH];

      //Build lists
      for (size_t i = 0; file.getline(line, LINE_LENGTH); i++)
      {
        vector<string> tokens;
        char *p = line;
        while (isspace(*p)) { p++; }

        if (!strncasecmp(p, "%include ", strlen("%include ")))
        {
          p += strlen("%include ");
          Tokenize(p, tokens, ";");
          string not_commented = tokens[0];
          tokens.clear();
          Tokenize(not_commented, tokens, "/\" \t");
          included_files.push(*(tokens.end()-1));
        }
        else if (!strncasecmp(p, "extsym ", strlen("extsym ")))
        {
          p += strlen("extsym ");
          Tokenize(p, tokens, ";");
          string not_commented = tokens[0];
          tokens.clear();
          Tokenize(not_commented, tokens, ", ");
          for (vector<string>::iterator i = tokens.begin(); i != tokens.end(); i++)
          {
            extsyms.insert(*i);
          }
        }
        else if (*p && (*p != ';'))
        {
          Tokenize(p, tokens, ";");
          if (tokens.size())
          {
            string not_commented = tokens[0];
            tokens.clear();
            Tokenize(not_commented, tokens, ", :[]+-*/\t");
            for (vector<string>::iterator i = tokens.begin()+1; i != tokens.end(); i++)
            {
              if (!isdigit(i[0][0]) && (i[0][0] != '$'))
              {
                used_vars.insert(*i);
              }
            }
          }
        }
      }
    }
    else if (ignore_include_file.find(fname) == ignore_include_file.end())
    {
      cout << "Error opening: " << fname << endl;
    }
  } while(!included_files.empty());

  set_difference(extsyms.begin(), extsyms.end(), used_vars.begin(), used_vars.end(), back_inserter(not_used_extsyms));

  if (not_used_extsyms.size())
  {
    cout << "Extra EXTSYMs found in " << filename << ":" << endl;
    for (list<string>::iterator i = not_used_extsyms.begin(); i != not_used_extsyms.end(); i++)
    {
      cout << "  " << *i << "\n";
    }
    cout << endl;
  }
}

void extra_check(const char *filename, struct stat& stat_buffer)
{
  if (extension_match(filename, ".asm"))
  {
    handle_file(filename);
  }
}

int main(size_t argc, char **argv)
{
  for (char **i = argv+1; *i; i++)
  {
    ignore_include_file.insert(*i);
  }

  parse_dir(".", extra_check);
  return(0);
}
