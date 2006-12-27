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

This program prefixes and suffixes variables in files.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
using namespace std;

#include "fileutil.h"
#include "strutil.h"

#define LINE_LENGTH 2048

char line[LINE_LENGTH];

const char *variable_prefix, *variable_suffix;

vector<string> variables;

void save_file(const char *filename, vector<string>& file_buffer, const char *error_message)
{
  ofstream file(filename);
  if (file)
  {
    for (vector<string>::iterator i = file_buffer.begin(); i != file_buffer.end(); i++)
    {
      file.write(i->data(), i->length());
      file << "\n";
    }
  }
  else
  {
    cerr << filename << error_message << endl;
  }
}

bool is_cidentifier(char c)
{
  return(isalnum(c) || (c == '_') || (c == '$'));
}

void handle_c_file(const char *filename)
{
  bool file_modified = false;
  vector<string> file_buffer;

  ifstream file(filename);
  if (file)
  {
    while (file.getline(line, LINE_LENGTH))
    {
      for (vector<string>::iterator i = variables.begin(); i != variables.end(); i++)
      {
        char *start = line, *p;
        while ((p = strstr(start, i->c_str())))
        {
          if (((p == line) || !is_cidentifier(p[-1])) && (!is_cidentifier(p[i->length()])))
          {
            char *end_ident = p+i->length();
            if (*variable_prefix)
            {
              size_t len = strlen(variable_prefix);
              memmove(p+len, p, strlen(p)+1);
              memcpy(p, variable_prefix, len);
              end_ident += len;
            }
            if (*variable_suffix)
            {
              size_t len = strlen(variable_suffix);
              memmove(end_ident+len, end_ident, strlen(end_ident)+1);
              memcpy(end_ident, variable_suffix, len);
              end_ident += len;
            }
            start = end_ident;
            file_modified = true;
          }
          else
          {
            start += i->length();
          }
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
    save_file(filename, file_buffer, " has variables to be replaced, but a replaced version can't be saved.");
  }
}

void variable_replace(const char *filename, struct stat& stat_buffer)
{
  if (is_c_file(filename) ||
      is_cpp_file(filename))
  {
    handle_c_file(filename);
  }
}

bool process_vars(const char *fname)
{
  ifstream stream(fname);
  if (stream)
  {
    vector<string> tokens;
    while (stream.getline(line, sizeof(line)))
    {
      tokens.clear();
      Tokenize(line, tokens, ";, :[]+-*%^&|<>!/\t\n\r\\");
      variables.push_back(tokens[0]);
    }
    stream.close();
    return(true);
  }
  cerr << "Could not open " << fname << "." << endl;
  return(false);
}

int main(size_t argc, const char *const *const argv)
{
  if (argc >= 4)
  {
    if (process_vars(argv[1]))
    {
      variable_prefix = argv[2];
      variable_suffix = argv[3];

      if (argc > 4)
      {
        for (const char *const *argp = argv+4; *argp; argp++)
        {
          parse_path(*argp, variable_replace);
        }
      }
      else
      {
        parse_dir(".", variable_replace);
      }
    }
  }
  else
  {
    cout << "Usage: varrep <variable file> <var prefix> <var suffix>\n" << endl;
  }

  return(0);
}
