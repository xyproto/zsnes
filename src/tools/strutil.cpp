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
*/

#include "strutil.h"
using namespace std;

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
  //Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);

  //Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos)
  {
    //Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));

    //Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);

    //Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

//Remove this at some point with a template
void Tokenize(const string_ci& str, vector<string_ci>& tokens, const string_ci& delimiters)
{
  //Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);

  //Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos)
  {
    //Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));

    //Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);

    //Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

bool all_whitespace(const char *str)
{
  for (; *str; str++)
  {
    if (!isspace(*str))
    {
      return(false);
    }
  }
  return(true);
}
