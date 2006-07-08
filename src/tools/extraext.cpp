/*
Copyright (C) 2005-2006 Nach, grinvader ( http://www.zsnes.com )

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

#include "fileutil.h"

#define LINE_LENGTH 500

set<string> ignore_include_file;

struct macro_var
{
  string macro_name;
  unsigned int param_num;
  string suffix;

  bool operator>(const macro_var &op2) const;
  bool operator<(const macro_var &op2) const;
};

bool macro_var::operator>(const macro_var &op2) const
{
  if (macro_name > op2.macro_name) { return(true); }
  if ((macro_name == op2.macro_name))
  {
    if (param_num > op2.param_num) { return(true); }
    if (param_num == op2.param_num)
    {
      if (suffix > op2.suffix) { return(true); }
    }
  }
  return(false);
}

bool macro_var::operator<(const macro_var &op2) const
{
  if (macro_name < op2.macro_name) { return(true); }
  if ((macro_name == op2.macro_name))
  {
    if (param_num < op2.param_num) { return(true); }
    if (param_num == op2.param_num)
    {
      if (suffix < op2.suffix) { return(true); }
    }
  }
  return(false);
}

bool macro_match(const macro_var macro, const string name)
{
  return(macro.macro_name == name);
}

void handle_file(const char *filename)
{
  queue<string> included_files;
  set<string> extsyms;
  set<string> used_vars;
  set<macro_var> macro_vars;
  list<string> not_used_extsyms;
  string current_macro;

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

          string inc_fname(*(tokens.end()-1));
          if (inc_fname != "macros.mac")
          {
            size_t last_slash = fname.find_last_of("/");
            if (last_slash != string::npos)
            {
              inc_fname.insert(0, fname, 0, last_slash+1);
            }
            included_files.push(inc_fname);
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
          if (atoi(tokens[1].c_str()))
          {
            current_macro = tokens[0];
          }
        }
        else if (!strncasecmp(p, "%endmacro", strlen("%endmacro")))
        {
          current_macro.clear();
        }
        else if (*p && (*p != ';'))
        {
          Tokenize(p, tokens, ";");
          if (tokens.size())
          {
            string not_commented = tokens[0];
            tokens.clear();
            Tokenize(not_commented, tokens, ", :[]+-*/\t");

            set<macro_var>::iterator macro = find_if(macro_vars.begin(), macro_vars.end(), bind2nd(ptr_fun(macro_match), tokens[0]));
            if (macro != macro_vars.end())
            {
              tokens.clear();
              Tokenize(not_commented, tokens, ", []\t"); //Retokenize properly for macros
              for (size_t i = 1; i < tokens.size(); i++)
              {
                for (set<macro_var>::iterator j = macro; (j != macro_vars.end()) && (j->macro_name == tokens[0]); j++)
                {
                  if (j->param_num == i)
                  {
                    if (!isdigit(tokens[i][0]) && (tokens[i][0] != '$'))
                    {
                      used_vars.insert(tokens[i]+j->suffix);
                    }
                  }
                }
              }
            }
            else
            {
              for (vector<string>::iterator i = tokens.begin()+1; i != tokens.end(); i++)
              {
                if (current_macro.size() && (i[0][0] == '%') && isdigit(i[0][1]))
                {
                  char buff[50];
                  *buff = 0;
                  macro_var var;
                  var.macro_name = current_macro;
                  sscanf(i->c_str(), "%%%u%s", &var.param_num, buff);
                  var.suffix = buff;
                  macro_vars.insert(var);
                }
                else if (!isdigit(i[0][0]) && (i[0][0] != '$') && !((i[0][0] == '%') && (i[0][1] == '%')))
                {
                  used_vars.insert(*i);
                }
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
