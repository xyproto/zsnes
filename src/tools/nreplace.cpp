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

This program is able to replace one list of data with another in multiple files.
Multiline replace is of course supported.
*/

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "fileutil.h"

string SearchText, ReplaceText;

void help()
{
  cout << "Usage: nreplace [-r] <search file> <replace file> <file 1 to modify> <file 2 to modify> ...\n" << endl;
  exit(1);
}

void readin_file(istream& stream, string& buffer)
{
  char byte;
  for (;;)
  {
    stream.get(byte);
    if (!stream.eof())
    {
      buffer += byte;
    }
    else
    {
      break;
    }
  }
}

void handle_file(const char *filename, struct stat& stat_buffer)
{
  fstream ModifyFile(filename, ios::in | ios::out);
  if (ModifyFile)
  {
    string ModifyText;
    readin_file(ModifyFile, ModifyText);

    bool changed = false;

    for (size_t start_pos = 0;;)
    {
      size_t match_point = ModifyText.find(SearchText, start_pos);
      if (match_point != string::npos)
      {
        ModifyText.replace(match_point, SearchText.size(), ReplaceText);
        start_pos += ReplaceText.size();
        changed = true;
      }
      else
      {
        break;
      }
    }

    if (changed)
    {
      ModifyFile.clear();
      ModifyFile.seekp(0, ios::beg);
      ModifyFile.write(ModifyText.data(), ModifyText.size());
      truncate(filename, ModifyText.size());
    }

    ModifyFile.close();
  }
  else
  {
    cout << "Could not open " << filename << endl;
  }
}

int main(size_t argc, const char **argv)
{
  bool subdir_scan = false;
  const char **argp = argv+1;

  if (*argp && !strcmp(*argp, "-r"))
  {
    if (argc < 5) { help(); }

    subdir_scan = true;
    argp++;
  }
  else if (argc < 4)
  {
    help();
  }


  ifstream SearchFile(*argp, ios::in);
  if (SearchFile)
  {
    argp++;
  }
  else
  {
    cout << "Could not open " << *argp << endl;
    return(2);
  }

  ifstream ReplaceFile(*argp, ios::in);
  if (ReplaceFile)
  {
    argp++;
  }
  else
  {
    cout << "Could not open " << *argp << endl;
    return(2);
  }

  readin_file(SearchFile, SearchText); SearchFile.close();
  readin_file(ReplaceFile, ReplaceText); ReplaceFile.close();

  if (subdir_scan)
  {
    for (; *argp; argp++)
    {
      if (!parse_path(*argp, handle_file))
      {
        cout << "Could not open " << *argp << endl;
      }
    }
  }
  else
  {
    struct stat stat_buffer; //Not used
    for (; *argp; argp++)
    {
      handle_file(*argp, stat_buffer);
    }
  }

  return(0);
}
