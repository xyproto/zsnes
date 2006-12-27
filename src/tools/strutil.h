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

#ifndef STRUTIL_H
#define STRUTIL_H

#include <string>
#include <vector>
#include <cctype>

struct ci_char_traits : public std::char_traits<char>
{
  static bool eq(char c1, char c2) { return tolower(c1) == tolower(c2); }
  static bool ne(char c1, char c2) { return tolower(c1) != tolower(c2); }
  static bool lt(char c1, char c2) { return tolower(c1) < tolower(c2); }
  static int compare(const char* s1, const char* s2, size_t n) { return strncasecmp( s1, s2, n ); }

  static const char* find( const char* s, int n, char a )
  {
    while(n-- > 0 && tolower(*s) != tolower(a))
    {
      ++s;
    }
    return n >= 0 ? s : 0;
  }
};

typedef std::basic_string<char, ci_char_traits> string_ci;

void Tokenize(const std::string&, std::vector<std::string>&, const std::string&);
void Tokenize(const string_ci&, std::vector<string_ci>&, const string_ci&);
bool all_whitespace(const char *);

#endif
