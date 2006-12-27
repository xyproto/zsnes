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
This program can add a prefix to every symbol in an object file.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
using namespace std;

#include "fileutil.h"
#include "strutil.h"

string prefix;
string extension;

vector<string> files;
set<string> symbols;
set<string> sections;


const char *symbol_fname = "symbol.map";
const char *section_fname = "section.map";

bool create_symbol_file()
{
  ofstream stream(symbol_fname, ios::out);
  if (stream)
  {
    for (set<string>::iterator i = symbols.begin(); i != symbols.end(); i++)
    {
      stream << *i << "\n";
    }
    stream.close();
    return(true);
  }
  return(false);
}

bool create_section_file()
{
  ofstream stream(section_fname, ios::out);
  if (stream)
  {
    for (set<string>::iterator i = sections.begin(); i != sections.end(); i++)
    {
      stream << "--rename-section " << *i << " ";
    }
    stream.close();
    return(true);
  }
  return(false);
}

void handle_file(const char *filename)
{
  string command("nm ");
  command += filename;

  FILE *pp = popen(command.c_str(), "r");
  if (pp)
  {
    files.push_back(filename);
    char line[1024];
    while (fgets(line, sizeof(line), pp))
    {
      vector<string> tokens;
      Tokenize(line, tokens, " \t\r\n");
      vector<string>::iterator i = tokens.begin();
      if ((i->size() != 1) && isxdigit((*i)[0])) { i++; }
      if (*i != "U")
      {
        i++;
        if ((*i)[0] != '.')
        {
          symbols.insert(*i+" "+prefix+*i);
        }
        else
        {
          size_t p = i->find('$');
          if (p != string::npos)
          {
            string change(*i);
            i->insert(p+1, prefix);
            symbols.insert(change+" "+*i);
            sections.insert(change+"="+*i);
          }
        }
      }
    }
    pclose(pp);
  }
  else
  {
    cout << "Coult not execute " << command << "." << endl;
  }
}

void extra_check(const char *filename, struct stat& stat_buffer)
{
  if (extension_match(filename, extension.c_str()))
  {
    handle_file(filename);
  }
}

int main(size_t argc, char **argv)
{
  if (argc == 3)
  {
     prefix = argv[1];
     extension = argv[2];
     parse_dir(".", extra_check);

     if (create_symbol_file())
     {
       string objcopy("objcopy ");
       if (sections.size() && create_section_file())
       {
         objcopy += "@";
         objcopy += section_fname;
         objcopy += " ";
       }
       objcopy += "--redefine-syms ";
       objcopy += symbol_fname;
       objcopy += " ";
       for (vector<string>::iterator i = files.begin(); i != files.end(); i++)
       {
         string command(objcopy);
         command += *i;
         command += " ";
         command += prefix;
         command += *i;
         system(command.c_str());
       }
     }
     else
     {
       cout << "Error creating: " << symbol_fname << endl;
     }
  }
  return(0);
}
