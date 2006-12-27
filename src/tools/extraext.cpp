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

This program tells us if an assembly file has extra EXTSYMs.
*/

#include <iostream>
#include <fstream>
#include <set>
#include <list>
#include <queue>
#include "strutil.h"
using namespace std;

#include <unistd.h>

#include "fileutil.h"

#define LINE_LENGTH 500

set<string> ignore_include_file;

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
  return(replaced);
}

void parse_line(string line, set<string> &used_vars, set<macro_var> &macro_vars)
{
  vector<string> tokens;

  Tokenize(line, tokens, ", \t");

  set<macro_var>::iterator macro = macro_vars.find(macro_var(tokens[0], tokens.size()-1));
  if (macro != macro_vars.end())
  {
    for (vector<string>::const_iterator i = macro->lines.begin(); i != macro->lines.end(); i++)
    {
      parse_line(replace_params(*i, tokens), used_vars, macro_vars);
    }
  }
  else
  {
    tokens.clear();
    Tokenize(line, tokens, ", :[]+-*/\t");

    for (vector<string>::iterator i = tokens.begin()+1; i != tokens.end(); i++)
    {
      if (!isdigit(i[0][0]) && (i[0][0] != '$') && !((i[0][0] == '%') && (i[0][1] == '%')))
      {
        used_vars.insert(*i);
      }
    }
  }
}

void process_file(string filename, set<string> &extsyms, set<string> &used_vars, set<macro_var> &macro_vars)
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
          process_file(inc_fname, extsyms, used_vars, macro_vars);
        }
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
            if (tokens[0].size() && !((tokens[0][0] == '%') && !isdigit(tokens[0][1])))
            {
              lines.push_back(tokens[0]);
            }
          }
        }
        macro_vars.insert(macro_var(macro_name, param_count, lines));
      }
      else if (*p && (*p != ';'))
      {
        Tokenize(p, tokens, ";");
        if (tokens.size())
        {
          parse_line(tokens[0], used_vars, macro_vars);
        }
        tokens.clear();
      }
    }
  }
  else if (ignore_include_file.find(filename) == ignore_include_file.end())
  {
    cout << "Error opening: " << filename << endl;
  }
}

void handle_file(const char *filename)
{
  set<string> extsyms;
  set<string> used_vars;
  set<macro_var> macro_vars;
  list<string> not_used_extsyms;

  process_file(filename, extsyms, used_vars, macro_vars);

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
