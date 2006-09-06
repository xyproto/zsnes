/*
Copyright (C) 2005-2006 pagefault, Nach, grinvader ( http://www.zsnes.com )

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
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "tools\fileutil.h"

#define MAXLINE 5000

string pline,fname,symbol;


void size_tally(const char *filename, struct stat& stat_buffer)
{
  string Filename = filename;
  if (string::npos != Filename.find(fname)) { cout << Filename << " " << symbol << endl; }
}

main()
{
   char line[MAXLINE]; // Max size per line
   int loc;
char *ptr;

   fstream file("gcc.txt",ios::in);
   while (!file.eof())
   {   
     file.getline(line, MAXLINE);
     pline = line;
     loc = pline.find(".obj");
     
     if (loc)
     {
        fname = pline.substr(0, loc+3);
        symbol = pline.substr(pline.find("`")+1,pline.find("'"));
        symbol = symbol.substr(0,symbol.size()-1);
     }

    parse_dir(".", size_tally);
   }
}