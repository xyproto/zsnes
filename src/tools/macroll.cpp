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

This program prints out a macro with params passed on command line in expanded/unrolled form.
*/

#include <iostream>
#include <fstream>
#include <set>
#include "strutil.h"
using namespace std;

#include <unistd.h>

#include "fileutil.h"

#define LINE_LENGTH 500

struct macro_var
{
  string macro_name;
  unsigned int params;
  vector<string> lines;

  macro_var(string &macro_name, unsigned int params) : macro_name(macro_name), params(params) {}
  macro_var(string &macro_name, unsigned int params, vector<string> &lines) : macro_name(macro_name), params(params), lines(lines) {}
  bool operator<(const macro_var &op2) const;
};

bool macro_var::operator<(const macro_var &op2) const
{
  if (macro_name < op2.macro_name) { return(true); }
  if (macro_name == op2.macro_name)
  {
    return(params < op2.params);
  }
  return(false);
}

bool macro_match(const macro_var macro, const string name)
{
  return(macro.macro_name == name);
}

set<macro_var> macro_vars;
string prefix;

string replace_params(string line, vector<string> &params)
{
  string replaced;
  const char *p = line.c_str();
  while (*p)
  {
    if (*p == '%')
    {
      if (isdigit(p[1]))
      {
        size_t num = 0;
        p++;
        while (isdigit(*p))
        {
          num *= 10;
          num += *p-'0';
          p++;
        }
        replaced += params[num];
      }
      else
      {
        replaced += *p++;
        if (*p) { replaced += *p++; }
      }
    }
    else
    {
      replaced += *p;
      p++;
    }
  }
  if (replaced == line)
  {
    cout << prefix << replaced << "\n";
  }
  else
  {
    cout << prefix << replaced << " ; " << line << "\n";
  }
  return(replaced);
}

void parse_line(string line)
{
  vector<string> tokens;

  Tokenize(line, tokens, ", \t");

  set<macro_var>::iterator macro = macro_vars.find(macro_var(tokens[0], tokens.size()-1));
  if (macro != macro_vars.end())
  {
    cout << prefix << ";Begin %macro " << tokens[0] << " " << tokens.size()-1 << "\n";
    prefix += "  ";
    for (vector<string>::const_iterator i = macro->lines.begin(); i != macro->lines.end(); i++)
    {
      parse_line(replace_params(*i, tokens));
    }
    prefix.erase(0, 2);
    cout << prefix << ";End %macro " << tokens[0] << " " << tokens.size()-1 << "\n";
  }
}

void process_file(string filename)
{
  ifstream file(filename.c_str(), ios::in);
  if (file)
  {
    char line[LINE_LENGTH];

    //Parse file
    while (file.getline(line, LINE_LENGTH))
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
        Tokenize(not_commented, tokens, "\" \t");

        string inc_fname(*(tokens.end()-1));
        if (inc_fname != "macros.mac")
        {
          size_t last_slash = filename.find_last_of("/");
          if (last_slash != string::npos)
          {
            inc_fname.insert(0, filename, 0, last_slash+1);
            if (access(inc_fname.c_str(), F_OK))
            {
              inc_fname.insert(last_slash+1, "../");
            }
          }
          process_file(inc_fname);
        }
      }
      else if (!strncasecmp(p, "%macro ", strlen("%macro ")))
      {
        p += strlen("%macro ");
        Tokenize(p, tokens, ";");
        string not_commented = tokens[0];
        tokens.clear();
        Tokenize(not_commented, tokens, " ");

        string macro_name = tokens[0];
        unsigned int param_count = atoi(tokens[1].c_str());
        vector<string> lines;

        while (file.getline(line, LINE_LENGTH))
        {
          p = line;
          while (isspace(*p)) { p++; }

          if (*p && (*p != ';'))
          {
            tokens.clear();
            Tokenize(p, tokens, ";");
            if (tokens[0] == "%endmacro")
            {
              break;
            }
            if (tokens[0].size())
            {
              lines.push_back(tokens[0]);
            }
          }
        }
        macro_vars.insert(macro_var(macro_name, param_count, lines));
      }
    }
  }
}

void build_macro(const char *filename, struct stat& stat_buffer)
{
  if (extension_match(filename, ".asm"))
  {
    process_file(filename);
  }
}

int main(size_t argc, char **argv)
{
  if (argc > 1)
  {
    parse_dir(".", build_macro);

    string line;
    argv++;
    while (*argv)
    {
      line += *argv++;
      line += " ";
    }
    cout << line << "\n";
    parse_line(line);
    cout << endl;
  }
  return(0);
}
