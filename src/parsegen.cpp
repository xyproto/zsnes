/*
Copyright (C) 2005-2007 Nach ( http://nsrt.edgeemu.com )

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
Config file handler creator by Nach (C) 2005-2007
*/

#if !defined(__GNUC__) && !defined(_MSC_VER)
#error You are using an unsupported compiler
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <set>
#include <stack>
using namespace std;

#include <errno.h>
#include <zlib.h>

#ifdef _MSC_VER //MSVC
typedef int ssize_t;
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define __WIN32__
#endif

#if defined(__MSDOS__) || defined(__WIN32__)
#define SLASH_STR "\\"
#else
#define SLASH_STR "/"
#endif

string gcc = "gcc";
string cflags;

#ifdef _MSC_VER //MSVC
static inline string COMPILE_OBJ(string obj, string c)
{
  return(string(string("cl /nologo /Fo")+obj+string(" ")+c));
}
#else
static inline string COMPILE_OBJ(string obj, string c)
{
  return(string(gcc+(" ")+cflags+(" -o ")+obj+string(" -c ")+c));
}
#endif

#define LINE_LENGTH 2048*4
char line[LINE_LENGTH];

string family_name = "cfg";

/*

Line Tracking and Error Control

*/

static struct
{
  size_t line_number;
  size_t column_number;

  void error(const char *str)
  {
    cerr << "Error: parse problem occured at " << line_number << ":" << column_number << ". " << str << "." << endl;
  }
} current_location;

/*

String Functions for various parsing

*/

//Find next matching character which is not escaped
char *find_next_match(char *str, char match_char)
{
  char *pos = 0;

  while (*str)
  {
    if (*str == match_char)
    {
      pos = str;
      break;
    }
    if (*str == '\\')
    {
      if (str[1])
      {
        str++;
      }
      else
      {
        break;
      }
    }
    str++;
  }
  return(pos);
}

//This is like strtok(), except this understands quoted characters and updates error locations
char *get_token(char *str, char *delim)
{
  static char *pos = 0;
  char *token = 0;

  if (str) //Start a new string?
  {
    pos = str;
  }

  if (pos)
  {
    //Skip delimiters
    while (*pos && strchr(delim, *pos))
    {
      pos++;
    }
    if (*pos)
    {
      token = pos;

      //Skip non-delimiters
      while (*pos && !strchr(delim, *pos))
      {
        //Skip quoted characters
        if ((*pos == '\"') || (*pos == '\''))
        {
          char *match_pos = 0;
          if ((match_pos = find_next_match(pos+1, *pos)))
          {
            pos = match_pos;
          }
        }
        pos++;
      }
      if (*pos)
      {
        *pos++ = '\0';
      }
    }
  }

  if (token) { current_location.column_number = token - line; }
  return(token);
}

//Like strchr() but understands quoted characters
char *find_chr(char *str, char match_char)
{
  char *pos = 0;

  while (*str)
  {
    if (*str == match_char)
    {
      pos = str;
      break;
    }
    //Skip quoted characters
    if ((*str == '\"') || (*str == '\''))
    {
      char *match_pos = 0;
      if ((match_pos = find_next_match(str+1, *str)))
      {
        str = match_pos;
      }
    }
    str++;
  }
  return(pos);
}

//Convert $AB12 and 0AB12h style hex to 0xAB12 hex
string asm2c_hex_convert(string str)
{
  size_t dollar_pos;
  int start = -1;
  while ((dollar_pos = str.find("$", start+1)) != string::npos)
  {
    if ((str.length()-dollar_pos > 1) && isxdigit(str[dollar_pos+1]))
    {
      str.replace(dollar_pos, 1, "0x");
      dollar_pos++;
    }
    start = dollar_pos;
  }

  start = -1;
  int h_pos;
  while ((string::size_type)(h_pos = str.find_first_of("hH", start+1)) != string::npos)
  {
    int h_len = 1;
    while ((h_pos-h_len > start) && isxdigit(str[h_pos-h_len]))
    {
      h_len++;
    }
    h_len--;

    if (isdigit(str[h_pos-h_len]))
    {
      str.erase(h_pos, 1);
      str.insert(h_pos-h_len, "0x");
      h_pos++;
    }
    start = h_pos;
  }

  return(str);
}

//Convert 2EF to 751
string c_hex_convert(string str)
{
  size_t hex_pos;
  while ((hex_pos = str.find("0x")) != string::npos)
  {
    size_t len = 2, total = 0;
    while (isxdigit(str[hex_pos+len]) && !(str[hex_pos+len] == '0' && str[hex_pos+len+1] == 'x'))
    {
      total *= 16;
      total += isdigit(str[hex_pos+len]) ? str[hex_pos+len]-'0' : (toupper(str[hex_pos+len])-'A')+10;
      len++;
    }

    ostringstream converter;
    converter << total;
    str.replace(hex_pos, len, converter.str());
  }
  return(str);
}

//Ascii numbers to integer, with support for mathematics in the string
ssize_t enhanced_atoi(char *&s, int level = 0)
{
  const int max_level = 6;
  if (level == max_level)
  {
    if (*s == '(')
    {
      ssize_t res = enhanced_atoi(++s);
      if (*s != ')') { current_location.error("Missing ) in expression"); }
      s++;
      return(res);
    }
    else if (isdigit(*s))
    {
      int numc, t;
      sscanf(s, "%d%n", &t, &numc);
      s += numc;
      return((ssize_t)t);
    }
    else if (*s == '-')
    {
      return(-enhanced_atoi(++s, max_level));
    }
    else if (*s == '~')
    {
      return(~enhanced_atoi(++s, max_level));
    }
  }
  ssize_t val = enhanced_atoi(s, level+1);
  while (*s)
  {
    const char *org = s;
    switch (level)
    {
      case 0: if (*s != '|') { return(val); } break;
      case 1: if (*s != '^') { return(val); } break;
      case 2: if (*s != '&') { return(val); } break;
      case 3: if (!((s[0] == '<' && s[1] == '<') || (s[0] == '>' && s[1] == '>'))) { return(val); } else { ++s; } break;
      case 4: if (!(*s == '+' || *s == '-')) { return(val); } break;
      case 5: if (!(*s == '*' || *s == '/' || *s == '%')) { return(val); } break;
    }
    ssize_t res = enhanced_atoi(++s, level+1);
    switch (*org)
    {
      case '|': val |= res; break;
      case '^': val ^= res; break;
      case '&': val &= res; break;
      case '<': val <<= res; break;
      case '>': val >>= res; break;
      case '+': val += res; break;
      case '-': val -= res; break;
      case '*': val *= res; break;
      case '/': val /= res; break;
      case '%': val %= res; break;
    }
  }
  return(val);
}

//Standard atoi(), but shows error if string isn't all a number
ssize_t safe_atoi(string str)
{
  if (!str.length()) { str = "X"; } //Force error

  const char *p = str.c_str();
  for (p = ((*p == '-') ? p+1 : p); *p; p++)
  {
    if (!isdigit(*p))
    {
      current_location.error("Not a number");
    }
  }

  return(atoi(str.c_str()));
}

bool all_spaces(const char *str)
{
  while (*str)
  {
    if (!isspace(*str)) { return(false); }
    str++;
  }
  return(true);
}

string encode_string(string& str, bool quotes = true)
{
  string newstr("");
  if (quotes) { newstr += '\"'; }

  for (size_t i = 0; i < str.length(); i++)
  {
    if ((str[i] == '\\') ||
        (str[i] == '\"') ||
        (str[i] == '\'') ||
        (str[i] == '\n') ||
        (str[i] == '\t'))
    {
      newstr += '\\';
    }
    newstr += str[i];
  }

  if (quotes) { newstr += '\"'; }
  return(newstr);
}

/*

Structures used to store config data

*/

template <typename T>
class nstack
{
  public:
  nstack() {}
  ~nstack() {}

  void push(T data) { this->data.push_back(data); }
  bool empty() { return(data.empty()); }
  T top() { return(data.back()); }
  void pop() { data.pop_back(); }
  size_t size() { return(data.size()); }

  bool all_true()
  {
    for (typename vector<T>::iterator i = data.begin(); i != data.end(); i++)
    {
      if (!*i)
      {
        return(false);
      }
    }
    return(true);
  }

  private:
  vector<T> data;
};

set<string> defines, dependancies;
nstack<bool> ifs;

typedef vector<string> str_array;

str_array memsets;

namespace variable
{
  enum ctype { NT, UC, US, UD, SC, SS, SD, LT };

  static struct
  {
    const char *CTypeSpace;
    const char *CTypeUnderscore;
    char FormatChar;
    bool Signed;
  } info[] = {
              { "", "", 0, false },
              { "unsigned char", "unsigned_char", 'u', false },
              { "unsigned short", "unsigned_short", 'u', false },
              { "unsigned int", "unsigned_int", 'u', false },
              { "char", "char", 'd', true },
              { "short", "short", 'd', true },
              { "int", "int", 'd', true }
             };

  ctype GetCType(const char *str)
  {
    int i = NT;
    for ( ; i < LT; i++)
    {
      if (!strcmp(info[i].CTypeSpace, str)) { break; }
    }
    i %= LT;

    if (i == NT)
    {
      cerr << "Invalid C type \"" << str << "\" when parsing line " << current_location.line_number << "." << endl;
    }

    return((ctype)i);
  }

  enum storage_format { none, single, quoted, mult, mult_packed, ptr };

  struct config_data_element
  {
    string name;
    storage_format format;
    ctype type;
    size_t length;
    string dependancy;
    string comment;

    bool operator==(const string& name) const { return(this->name == name); }
  };

  typedef vector<config_data_element> config_data_array;

  static class
  {
    private:
    config_data_array data_array;

    bool duplicate_name(string& name)
    {
      if (find(data_array.begin(), data_array.end(), name) != data_array.end())
      {
        cerr << "Duplicate definition of \"" << name << "\" found on line " << current_location.line_number << "." << endl;
        return(true);
      }
      return(false);
    }

    public:
    void add_comment(string comment)
    {
      config_data_element new_element = { "", none, NT, 0, "", comment };
      data_array.push_back(new_element);
    }

    void add_var_single(string& name, ctype type, string dependancy, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, single, type, 0, dependancy, comment };
        data_array.push_back(new_element);
      }
    }
    void add_var_single(string& name, const char *type, string dependancy, string comment = "")
    {
      add_var_single(name, GetCType(type), dependancy, comment);
    }

    void add_var_quoted(string& name, string dependancy, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, quoted, NT, 0, dependancy, comment };
        data_array.push_back(new_element);
      }
    }

    void add_var_mult(string& name, ctype type, size_t length, string dependancy, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, mult, type, length, dependancy, comment };
        data_array.push_back(new_element);
      }
    }
    void add_var_mult(string& name, const char *type, size_t length, string dependancy, string comment = "")
    {
      add_var_mult(name, GetCType(type), length, dependancy, comment);
    }

    void add_var_packed(string& name, size_t length, string dependancy, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, mult_packed, NT, length, dependancy, comment };
        data_array.push_back(new_element);
      }
    }

    void add_var_ptr(string& name, ctype type, size_t length, string dependancy, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, ptr, type, length, dependancy, comment };
        data_array.push_back(new_element);
      }
    }
    void add_var_ptr(string& name, const char *type, size_t length, string dependancy, string comment = "")
    {
      add_var_ptr(name, GetCType(type), length, dependancy, comment);
    }


    bool ctype_mult_used(ctype type)
    {
      for (config_data_array::iterator i = data_array.begin(); i != data_array.end(); i++)
      {
        if ((i->format == mult) && (i->type == type))
        {
          return(true);
        }
      }
      return(false);
    }

    bool ctype_ptr_used(ctype type)
    {
      for (config_data_array::iterator i = data_array.begin(); i != data_array.end(); i++)
      {
        if ((i->format == ptr) && (i->type == type))
        {
          return(true);
        }
      }
      return(false);
    }

    bool packed_used()
    {
      for (config_data_array::iterator i = data_array.begin(); i != data_array.end(); i++)
      {
        if (i->format == mult_packed)
        {
          return(true);
        }
      }
      return(false);
    }

    bool quoted_used()
    {
      for (config_data_array::iterator i = data_array.begin(); i != data_array.end(); i++)
      {
        if (i->format == quoted)
        {
          return(true);
        }
      }
      return(false);
    }

    bool unsigned_used()
    {
      for (config_data_array::iterator i = data_array.begin(); i != data_array.end(); i++)
      {
        if (!info[i->type].Signed)
        {
          return(true);
        }
      }
      return(false);
    }

    config_data_array::iterator begin() { return(data_array.begin()); }
    config_data_array::iterator end() { return(data_array.end()); }
  } config_data;
}

/*

Compiler with it's helper functions

*/

#define var_type_is_char(var_type)  !strcmp(var_type+strlen(var_type)-strlen("char"), "char")
#define var_type_is_short(var_type) !strcmp(var_type+strlen(var_type)-strlen("short"), "short")
#define var_type_is_int(var_type)   !strcmp(var_type+strlen(var_type)-strlen("int"), "int")

#define short_scale "*sizeof(short)"
#define int_scale "*sizeof(int)"

//Convert asm types to C types
char *convert_asm_type(const char *str, bool unsigned_var = true)
{
  char *var_type = 0;
  if (!strcasecmp(str, "dd"))
  {
    var_type = "unsigned int";
  }
  else if (!strcasecmp(str, "dw"))
  {
    var_type = "unsigned short";
  }
  else if (!strcasecmp(str, "db"))
  {
    var_type = "unsigned char";
  }
  else if (!strcasecmp(str, "sd"))
  {
    var_type = "int";
  }
  else if (!strcasecmp(str, "sw"))
  {
    var_type = "short";
  }
  else if (!strcasecmp(str, "sb"))
  {
    var_type = "char";
  }
  else
  {
    current_location.error("Not a valid type");
  }

  if (var_type && !strncmp(var_type, "unsigned ", strlen("unsigned ")) && !unsigned_var)
  {
    var_type += strlen("unsigned ");
  }

  return(var_type);
}

void output_parser_start(ostream& c_stream, string& cheader_file)
{
  c_stream << "/*\n"
           << "Config file handler generated by Nach's Config file handler creator.\n"
           << "*/\n"
           << "\n"
           << "#include <stdio.h>\n"
           << "#include <stdlib.h>\n"
           << "#include <ctype.h>\n"
           << "#include <string.h>\n";
  if (defines.find("PSR_COMPRESSED") != defines.end())
  {
    c_stream << "#include <zlib.h>\n";
  }
  if (cheader_file.length())
  {
    c_stream << "#include \"" << cheader_file << "\"\n";
  }
  c_stream << "\n"
           << "\n"
           << "#define LINE_LENGTH " << LINE_LENGTH << "\n"
           << "static char line[LINE_LENGTH];\n"
           << "\n";
  if (variable::config_data.quoted_used() || variable::config_data.packed_used())
  {
    c_stream << "\n"
             << "static char *encode_string(const char *str)\n"
             << "{\n"
             << "  size_t i = 0;\n"
             << "  line[i++] = '\\\"';\n"
             << "  while (*str)\n"
             << "  {\n"
             << "    if ((*str == '\\\\') ||\n"
             << "        (*str == '\\\"') ||\n"
             << "        (*str == '\\\'') ||\n"
             << "        (*str == '\\n') ||\n"
             << "        (*str == '\\t'))\n"
             << "    {\n"
             << "      line[i++] = '\\\\';\n"
             << "    }\n"
             << "    line[i++] = *str++;\n"
             << "  }\n"
             << "  line[i++] = '\\\"';\n"
             << "  line[i] = 0;\n"
             << "  return(line);\n"
             << "}\n"
             << "\n"
             << "static char *decode_string(char *str)\n"
             << "{\n"
             << "  size_t str_len = strlen(str), i = 0;\n"
             << "  char *dest = str;\n"
             << "\n"
             << "  if ((str_len > 1) && (*str == '\\\"') && (str[str_len-1] == '\\\"'))\n"
             << "  {\n"
             << "    memmove(str, str+1, str_len-2);\n"
             << "    str[str_len-2] = 0;\n"
             << "\n"
             << "    while (*str)\n"
             << "    {\n"
             << "      if (*str == '\\\\')\n"
             << "      {\n"
             << "        str++;\n"
             << "      }\n"
             << "      dest[i++] = *str++;\n"
             << "    }\n"
             << "  }\n"
             << "  dest[i] = 0;\n"
             << "  return(dest);\n"
             << "}\n";
  }
  c_stream << "\n"
           << "static char *find_next_match(char *str, char match_char)\n"
           << "{\n"
           << "  char *pos = 0;\n"
           << "\n"
           << "  while (*str)\n"
           << "  {\n"
           << "    if (*str == match_char)\n"
           << "    {\n"
           << "      pos = str;\n"
           << "      break;\n"
           << "    }\n"
           << "    if (*str == '\\\\')\n"
           << "    {\n"
           << "      if (str[1])\n"
           << "      {\n"
           << "        str++;\n"
           << "      }\n"
           << "      else\n"
           << "      {\n"
           << "        break;\n"
           << "      }\n"
           << "    }\n"
           << "    str++;\n"
           << "  }\n"
           << "  return(pos);\n"
           << "}\n"
           << "\n"
           << "static char *find_str(char *str, char *match_str)\n"
           << "{\n"
           << "  char *pos = 0;\n"
           << "\n"
           << "  while (*str)\n"
           << "  {\n"
           << "    if (strchr(match_str, *str))\n"
           << "    {\n"
           << "      pos = str;\n"
           << "      break;\n"
           << "    }\n"
           << "    if ((*str == '\\\"') || (*str == '\\\''))\n"
           << "    {\n"
           << "      char *match_pos = 0;\n"
           << "      if ((match_pos = find_next_match(str+1, *str)))\n"
           << "      {\n"
           << "        str = match_pos;\n"
           << "      }\n"
           << "    }\n"
           << "    str++;\n"
           << "  }\n"
           << "  return(pos);\n"
           << "}\n"
           << "\n";
  if (variable::config_data.unsigned_used())
  {
    c_stream << "\n"
             << "static int atoui(const char *nptr)\n"
             << "{\n"
             << "  return(strtoul(nptr, 0, 10));\n"
             << "}\n";
  }
  c_stream << "\n";
}

void output_cheader_start(ostream& cheader_stream)
{
  cheader_stream << "/*\n"
                 << "Config file handler header generated by Nach's Config file handler creator.\n"
                 << "*/\n"
                 << "\n"
                 << "#ifdef __cplusplus\n"
                 << "  extern \"C\" {\n"
                 << "#endif\n"
                 << "\n"
                 << "unsigned char read_" << family_name << "_vars(const char *);\n"
                 << "unsigned char write_" << family_name << "_vars(const char *);\n";
  if (defines.find("PSR_COMPRESSED") != defines.end())
  {
    cheader_stream << "unsigned char read_" << family_name << "_vars_compressed(const char *);\n"
                   << "unsigned char write_" << family_name << "_vars_compressed(const char *);\n";
  }
  if (defines.find("PSR_MEMCPY") != defines.end())
  {
    cheader_stream << "void read_" << family_name << "_vars_memory(unsigned char *);\n"
                   << "void write_" << family_name << "_vars_memory(unsigned char *);\n"
                   << "unsigned int size_" << family_name << "_vars_memory();\n";
  }
  cheader_stream << "\n";
}

void output_cheader_end(ostream& cheader_stream)
{
  cheader_stream << "\n"
                 << "#ifdef __cplusplus\n"
                 << "             }\n"
                 << "#endif\n"
                 << "\n";
}


void output_extsym_dependancies(ostream& c_stream)
{
  c_stream << "\n";
  for (set<string>::iterator i = dependancies.begin(); i != dependancies.end(); i++)
  {
    c_stream << "extern unsigned char " << *i << ";\n";
  }
}

void output_init_var(ostream& c_stream)
{
  c_stream << "\n"
           << "static unsigned char psr_init_done = 0;\n"
           << "static void init_" << family_name << "_vars()\n"
           << "{\n"
           << "  if (!psr_init_done)\n"
           << "  {\n"
           << "    psr_init_done = 1;\n"
           << "\n";
  for (str_array::iterator i = memsets.begin(); i != memsets.end(); i++)
  {
    c_stream << "    " << *i << "\n";
  }
  c_stream << "  }\n"
           << "}\n";
}

void output_packed_write(ostream& c_stream)
{
  if (variable::config_data.packed_used())
  {
    c_stream << "\n"
             << "static char *base94_encode(size_t size)\n"
             << "{\n"
             << "  unsigned int i;\n"
             << "  static char buffer[] = { 0, 0, 0, 0, 0, 0};\n"
             << "  for (i = 0; i < 5; i++)\n"
             << "  {\n"
             << "    buffer[i] = ' ' + (char)(size % 94);\n"
             << "    size /= 94;\n"
             << "  }\n"
             << "  return(buffer);\n"
             << "}\n"
             << "\n"
             << "static char *char_array_pack(const char *str, size_t len)\n"
             << "{\n"
             << "  char packed[LINE_LENGTH];\n"
             << "  char *p = packed;\n"
             << "  while (len)\n"
             << "  {\n"
             << "    if (*str)\n"
             << "    {\n"
             << "      size_t length = strlen(str);\n"
             << "      strcpy(p, encode_string(str));\n"
             << "      str += length;\n"
             << "      len -= length;\n"
             << "      p += strlen(p);\n"
             << "    }\n"
             << "    else\n"
             << "    {\n"
             << "      size_t i = 0;\n"
             << "      while (!*str && len)\n"
             << "      {\n"
             << "        i++;\n"
             << "        str++;\n"
             << "        len--;\n"
             << "      }\n"
             << "\n"
             << "      sprintf(p, \"0%s\", encode_string(base94_encode(i)));\n"
             << "      p += strlen(p);\n"
             << "    }\n"
             << "    *p++ = '\\\\';\n"
             << "  }\n"
             << "  p[-1] = 0;\n"
             << "  strcpy(line, packed);"
             << "  return(line);\n"
             << "}\n";
  }
}

void output_array_write(ostream& c_stream, variable::ctype type)
{
  if (variable::config_data.ctype_mult_used(type) || variable::config_data.ctype_ptr_used(type))
  {
    c_stream << "\n"
             << "static void write_" << variable::info[type].CTypeUnderscore << "_array(int (*outf)(void *, const char *, ...), void *fp, const char *var_name, " << variable::info[type].CTypeSpace << " *var, size_t size, const char *comment)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  outf(fp, \"%s=%" << variable::info[type].FormatChar << "\", var_name, (int)*var);\n"
             << "  for (i = 1; i < size; i++)\n"
             << "  {\n"
             << "    outf(fp, \",%" << variable::info[type].FormatChar << "\", (int)(var[i]));\n"
             << "  }\n"
             << "  if (comment)\n"
             << "  {\n"
             << "    outf(fp, \" ;%s\", comment);\n"
             << "  }\n"
             << "  outf(fp, \"\\n\");\n"
             << "}\n";
  }
}

void output_write_var(ostream& c_stream)
{
  output_packed_write(c_stream);
  output_array_write(c_stream, variable::UC);
  output_array_write(c_stream, variable::US);
  output_array_write(c_stream, variable::UD);
  output_array_write(c_stream, variable::SC);
  output_array_write(c_stream, variable::SS);
  output_array_write(c_stream, variable::SD);

  c_stream << "\n"
           << "static void write_" << family_name << "_vars_internal(void *fp, int (*outf)(void *, const char *, ...))\n"
           << "{\n";
  for (variable::config_data_array::iterator i = variable::config_data.begin(); i != variable::config_data.end(); i++)
  {
    string dependancy_prefix, dependancy_suffix;
    if (i->dependancy != "")
    {
      dependancy_prefix = string("if (") + string(i->dependancy, 0, i->dependancy.length()-1) + string(") { ");
      dependancy_suffix = " }";
    }

    if (i->format == variable::none)
    {
      if (i->comment != "")
      {
        c_stream << "  outf(fp, \";%s\\n\", " << encode_string(i->comment) << ");\n";
      }
      else
      {
        c_stream << "  outf(fp, \"\\n\");\n";
      }
    }
    else if ((i->format == variable::mult) || (i->format == variable::ptr))
    {
      c_stream << "  " << dependancy_prefix << "write_" << variable::info[i->type].CTypeUnderscore
               << "_array(outf, fp, \"" << i->dependancy << i->name << "\", " << i->name << ", " << i->length << ", " << ((i->comment != "") ? encode_string(i->comment) : "0") << ");" << dependancy_suffix << "\n";
    }
    else
    {
      string config_comment = (i->comment != "") ? (string(" ;") + encode_string(i->comment, false)) : "";
      c_stream << "  " << dependancy_prefix << "outf(fp, \"" << i->dependancy << i->name << "=";
      if (i->format == variable::single)
      {
        c_stream << "%" << variable::info[i->type].FormatChar << config_comment << "\\n\", " << i->name;
      }
      else if (i->format == variable::quoted)
      {
       c_stream << "%s" << config_comment << "\\n\", encode_string(" << i->name << ")";
      }
      else if (i->format == variable::mult_packed)
      {
        c_stream << "%s" << config_comment << "\\n\", char_array_pack((char *)" << i->name << ", " << i->length << ")";
      }
      c_stream << ");" << dependancy_suffix << "\n";
    }
  }
  if (defines.find("PSR_HASH") != defines.end())
  {
    c_stream << "  outf(fp, \"\\n\\n\\n;Do not modify the following, for internal use only.\\n\");\n"
             << "  outf(fp, \"PSR_HASH" << "=%u\\n\", PSR_HASH);\n";
  }
  c_stream << "}\n"
           << "\n"
           << "unsigned char write_" << family_name << "_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n";
  if (defines.find("PSR_EXTERN") == defines.end())
  {
    c_stream << "  init_" << family_name << "_vars();\n"
             << "\n";
  }
  c_stream << "  if ((fp = fopen(file, \"w\")))\n"
           << "  {\n"
           << "    write_" << family_name << "_vars_internal(fp, (int (*)(void *, const char *, ...))fprintf);\n"
           << "    fclose(fp);\n"
           << "\n"
           << "    return(1);\n"
           << "  }\n"
           << "  return(0);\n"
           << "}\n";

  if (defines.find("PSR_COMPRESSED") != defines.end())
  {
    c_stream << "\n"
             << "unsigned char write_" << family_name << "_vars_compressed(const char *file)\n"
             << "{\n"
             << "  gzFile gzfp;\n"
             << "\n";
    if (defines.find("PSR_EXTERN") == defines.end())
    {
      c_stream << "  init_" << family_name << "_vars();\n"
               << "\n";
    }
    c_stream << "  if ((gzfp = gzopen(file, \"wb9\")))\n"
             << "  {\n"
             << "    write_" << family_name << "_vars_internal(gzfp, gzprintf);\n"
             << "    gzclose(gzfp);\n"
             << "\n"
             << "    return(1);\n"
             << "  }\n"
             << "\n"
             << "  return(0);\n"
             << "}\n";
  }

  if (defines.find("PSR_MEMCPY") != defines.end())
  {
    c_stream << "\n"
             << "static unsigned int " << family_name << "_vars_memory(unsigned char *buffer, void *(*cpy)(void *, void *, size_t))\n"
             << "{\n"
             << "  unsigned char *p = buffer;\n";
    for (variable::config_data_array::iterator i = variable::config_data.begin(); i != variable::config_data.end(); i++)
    {
      string dependancy_prefix, dependancy_suffix;
      if (i->dependancy != "")
      {
        dependancy_prefix = string("if (") + string(i->dependancy, 0, i->dependancy.length()-1) + string(") { ");
        dependancy_suffix = " }";
      }

      if (i->format == variable::ptr)
      {
        c_stream << "  " << dependancy_prefix << "cpy(p, " << i->name << ", sizeof(" <<variable::info[i->type].CTypeSpace << ")*" << i->length << "); p += sizeof(" << variable::info[i->type].CTypeSpace << ")*" << i->length << ";" << dependancy_suffix << "\n";
      }
      else if (i->format != variable::none)
      {
        c_stream << "  " << dependancy_prefix << "cpy(p, " << ((i->format == variable::single) ? "&" : "") << i->name << ", sizeof(" << i->name << ")); p += sizeof(" << i->name << ");" << dependancy_suffix << "\n";
      }
    }
    c_stream << "  return(p-buffer);\n"
             << "}\n"
             << "\n"
             << "static void *cpynull(void *l, void *r, size_t len){ return(0); }\n"
             << "\n"
             << "unsigned int size_" << family_name << "_vars_memory()\n"
             << "{\n"
             << "  return(" << family_name << "_vars_memory(0, cpynull));\n"
             << "}\n"
             << "\n"
             << "void write_" << family_name << "_vars_memory(unsigned char *buffer)\n"
             << "{\n"
             << "  " << family_name << "_vars_memory(buffer, (void *(*)(void *, void *, size_t))memcpy);\n"
             << "}\n";
  }
}

void output_packed_read(ostream& c_stream)
{
  if (variable::config_data.packed_used())
  {
    c_stream << "\n"
             << "static size_t base94_decode(const char *buffer)\n"
             << "{\n"
             << "  size_t size = 0;\n"
             << "  int i;\n"
             << "  for (i = 4; i >= 0; i--)\n"
             << "  {\n"
             << "    size *= 94;\n"
             << "    size += (size_t)(buffer[i]-' ');\n"
             << "  }\n"
             << "  return(size);\n"
             << "}\n"
             << "\n"
             << "static char *get_token(char *str, char *delim)\n"
             << "{\n"
             << "  static char *pos = 0;\n"
             << "  char *token = 0;\n"
             << "\n"
             << "  if (str) //Start a new string?\n"
             << "  {\n"
             << "    pos = str;\n"
             << "  }\n"
             << "\n"
             << "  if (pos)\n"
             << "  {\n"
             << "    //Skip delimiters\n"
             << "    while (*pos && strchr(delim, *pos))\n"
             << "    {\n"
             << "      pos++;\n"
             << "    }\n"
             << "    if (*pos)\n"
             << "    {\n"
             << "      token = pos;\n"
             << "\n"
             << "      //Skip non-delimiters\n"
             << "      while (*pos && !strchr(delim, *pos))\n"
             << "      {\n"
             << "        //Skip quoted characters\n"
             << "        if ((*pos == '\\\"') || (*pos == '\\''))\n"
             << "        {\n"
             << "          char *match_pos = 0;\n"
             << "          if ((match_pos = find_next_match(pos+1, *pos)))\n"
             << "          {\n"
             << "            pos = match_pos;\n"
             << "          }\n"
             << "        }\n"
             << "        pos++;\n"
             << "      }\n"
             << "      if (*pos)\n"
             << "      {\n"
             << "        *pos++ = '\\0';\n"
             << "      }\n"
             << "    }\n"
             << "  }\n"
             << "  return(token);\n"
             << "}\n"
             << "\n"
             << "static char *char_array_unpack(char *str)\n"
             << "{\n"
             << "  char packed[LINE_LENGTH];\n"
             << "  char *p = packed, *token;\n"
             << "  size_t len = 0;\n"
             << "  memset(packed, 0, sizeof(packed));\n"
             << "  for (token = get_token(str, \"\\\\\"); token; token = get_token(0, \"\\\\\"))\n"
             << "  {\n"
             << "    if (*token == '0')\n"
             << "    {\n"
             << "      size_t i = base94_decode(decode_string(token+1));\n"
             << "      len += i;\n"
             << "      if (len > sizeof(packed)) { break; }\n"
             << "      memset(p, 0, i);\n"
             << "      p += i;\n"
             << "    }\n"
             << "    else\n"
             << "    {\n"
             << "      char *decoded = decode_string(token);\n"
             << "      size_t decoded_length = strlen(decoded);\n"
             << "      len += decoded_length;\n"
             << "      if (len > sizeof(packed))\n"
             << "      {\n"
             << "        memcpy(p, decoded, sizeof(packed)-(len-decoded_length));\n"
             << "        break;\n"
             << "      }\n"
             << "      memcpy(p, decoded, decoded_length);\n"
             << "      p += decoded_length;\n"
             << "    }\n"
             << "  }\n"
             << "  memcpy(line, packed, sizeof(packed));"
             << "  return(line);\n"
             << "}\n";
  }
}

void output_array_read(ostream& c_stream, variable::ctype type)
{
  if (variable::config_data.ctype_mult_used(type) || variable::config_data.ctype_ptr_used(type))
  {
    c_stream << "\n"
             << "static void read_" << variable::info[type].CTypeUnderscore << "_array(char *line, " << variable::info[type].CTypeSpace << " *var, size_t size)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  char *token;\n"
             << "  *var = (" << variable::info[type].CTypeSpace << ")" << (variable::info[type].Signed ? "atoi" : "atoui") << "(strtok(line, \", \\t\\r\\n\"));\n"
             << "  for (i = 1; (i < size) && (token = strtok(0, \", \\t\\r\\n\")); i++)\n"
             << "  {\n"
             << "    var[i] = (" << variable::info[type].CTypeSpace << ")" << (variable::info[type].Signed ? "atoi" : "atoui") << "(token);\n"
             << "  }\n"
             << "}\n";
  }
}

void output_read_var(ostream& c_stream)
{
  output_packed_read(c_stream);
  output_array_read(c_stream, variable::UC);
  output_array_read(c_stream, variable::US);
  output_array_read(c_stream, variable::UD);
  output_array_read(c_stream, variable::SC);
  output_array_read(c_stream, variable::SS);
  output_array_read(c_stream, variable::SD);

  c_stream << "\n"
           << "static void read_" << family_name << "_vars_internal(void *fp, char *(*fin)(char *, int, void *), int (*fend)(void *))\n"
           << "{\n"
           << "  while (!fend(fp))\n"
           << "  {\n"
           << "    char *p, *var, *value;\n"
           << "\n"
           << "    fin(line, LINE_LENGTH, fp);\n"
           << "    if ((p = find_str(line, \";\"))) { *p = 0; }\n"
           << "    if ((p = strchr(line, '=')))\n"
           << "    {\n"
           << "      *p = 0;\n"
           << "      var = line;\n"
           << "      value = p+1;\n"
           << "      while (isspace(*var)) { var++; }\n"
           << "      while (isspace(*value)) { value++; }\n"
           << "      if ((p = find_str(var, \" \\t\\r\\n\"))) { *p = 0; }\n"
           << "      if ((p = find_str(value, \" \\t\\r\\n\"))) { *p = 0; }\n"
           << "      if (!*var || !*value) { continue; }\n";
  if (dependancies.size())
  {
    c_stream << "      if ((p = strchr(var, ':')))\n"
             << "      {\n"
             << "        if (!strlen(p+1)) { continue; }\n";
    set<string>::iterator i = dependancies.begin();
    c_stream << "        if (!strncmp(var, \"" << *i << ":\", (p-var)+1)) { if (!" << *i << ") { continue; } }\n";
    for (i++; i != dependancies.end(); i++)
    {
      c_stream << "        else if (!strncmp(var, \"" << *i << ":\", (p-var)+1)) { if (!" << *i << ") { continue; } }\n";
    }
    c_stream << "        else { continue; }\n"
             << "      }\n";
  }
  c_stream << "    }\n"
           << "    else\n"
           << "    {\n"
           << "      continue;\n"
           << "    }\n"
           << "\n";
  for (variable::config_data_array::iterator i = variable::config_data.begin(); i != variable::config_data.end(); i++)
  {
    if (i->format != variable::none)
    {
      c_stream << "    if (!strcmp(var, \"" << i->dependancy << i->name << "\")) { ";
      if (i->format == variable::single)
      {
        c_stream << i->name << " = (" << variable::info[i->type].CTypeSpace << ")" << (variable::info[i->type].Signed ? "atoi" : "atoui") << "(value);";
      }
      else if ((i->format == variable::mult) || (i->format == variable::ptr))
      {
        c_stream << "read_" << variable::info[i->type].CTypeUnderscore
                 << "_array(value, " << i->name << ", " << i->length << ");";
      }
      else if (i->format == variable::quoted)
      {
        c_stream << "*" << i->name << " = 0; "
                 << "strncat(" << i->name << ", decode_string(value), sizeof(" << i->name << ")-1);";
      }
      else if (i->format == variable::mult_packed)
      {
        c_stream << "memcpy(" << i->name << ", char_array_unpack(value), " << i->length << ");";
      }
      c_stream << " continue; }\n";
    }
  }
  if (defines.find("PSR_HASH") != defines.end())
  {
    c_stream << "    if (!strcmp(var, \"PSR_HASH\"))\n"
             << "    {\n"
             << "       if ((unsigned int)atoui(value) == PSR_HASH)\n"
             << "       {\n"
             << "         psr_init_done = 2;\n"
             << "         continue;\n"
             << "       }\n"
             << "       break;\n"
             << "    }\n";
  }
  c_stream << "  }\n";
  if (defines.find("PSR_HASH") != defines.end())
  {
    c_stream << "  if (psr_init_done == 2)\n"
             << "  {\n"
             << "    psr_init_done = 1;\n"
             << "  }\n"
             << "  else\n"
             << "  {\n"
             << "    psr_init_done = 0;\n"
             << "    init_" << family_name << "_vars();\n"
             << "  }\n";
  }
  c_stream << "}\n"
           << "\n"
           << "unsigned char read_" << family_name << "_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n";
  if (defines.find("PSR_EXTERN") == defines.end())
  {
    c_stream << "  init_" << family_name << "_vars();\n"
             << "\n";
  }
  c_stream << "  if ((fp = fopen(file, \"r\")))\n"
           << "  {\n"
           << "    read_" << family_name << "_vars_internal(fp, (char *(*)(char *, int, void *))fgets, (int (*)(void *))feof);\n"
           << "    fclose(fp);\n";
  if (defines.find("PSR_NOUPDATE") == defines.end())
  {
    c_stream << "    write_" << family_name << "_vars(file);\n";
  }
  c_stream << "    return(1);\n"
           << "  }\n"
           << "\n";
  if (defines.find("PSR_NOUPDATE") == defines.end())
  {
    c_stream << "  write_" << family_name << "_vars(file);\n";
  }
  c_stream << "  return(0);\n"
           << "}\n";

  if (defines.find("PSR_COMPRESSED") != defines.end())
  {
    c_stream << "\n"
             << "static char *gzgets_fix(char *buf, int len, void *file)\n"
             << "{\n"
             << "  return(gzgets(file, buf, len));\n"
             << "}\n"
             << "\n"
             << "unsigned char read_" << family_name << "_vars_compressed(const char *file)\n"
             << "{\n"
             << "  gzFile gzfp;\n"
             << "\n";
    if (defines.find("PSR_EXTERN") == defines.end())
    {
      c_stream << "  init_" << family_name << "_vars();\n"
               << "\n";
    }
    c_stream << "  if ((gzfp = gzopen(file, \"rb\")))\n"
             << "  {\n"
             << "    read_" << family_name << "_vars_internal(gzfp, gzgets_fix, gzeof);\n"
             << "    gzclose(gzfp);\n";
    if (defines.find("PSR_NOUPDATE") == defines.end())
    {
      c_stream << "    write_" << family_name << "_vars_compressed(file);\n";
    }
    c_stream << "    return(1);\n"
             << "  }\n"
             << "\n";
    if (defines.find("PSR_NOUPDATE") == defines.end())
    {
      c_stream << "  write_" << family_name << "_vars_compressed(file);\n";
    }
    c_stream << "  return(0);\n"
             << "}\n";
  }

  if (defines.find("PSR_MEMCPY") != defines.end())
  {
    c_stream << "\n"
             << "static void *cpyright(void *src, void *dest, size_t len)\n"
             << "{\n"
             << "  memcpy(dest, src, len);\n"
             << "  return(0);\n"
             << "}\n"
             << "\n"
             << "void read_" << family_name << "_vars_memory(unsigned char *buffer)\n"
             << "{\n"
             << "  " << family_name << "_vars_memory(buffer, cpyright);\n"
             << "}\n";
  }
}

void handle_directive(const char *instruction, const char *label)
{
  if (!strcasecmp(instruction, "define"))
  {
    if (label)
    {
      defines.insert(label);
    }
    else
    {
      current_location.error("Could not get define label");
    }
  }
  else if (!strcasecmp(instruction, "undef"))
  {
    if (label)
    {
      defines.erase(label);
    }
    else
    {
      current_location.error("Could not get undefine label");
    }
  }
  else if (!strcasecmp(instruction, "ifdef"))
  {
    if (label)
    {
      if (defines.find(label) != defines.end())
      {
        ifs.push(true);
      }
      else
      {
        ifs.push(false);
      }
    }
    else
    {
      current_location.error("Could not get ifdef label");
    }
  }
  else if (!strcasecmp(instruction, "ifndef"))
  {
    if (label)
    {
      if (defines.find(label) == defines.end())
      {
        ifs.push(true);
      }
      else
      {
        ifs.push(false);
      }
    }
    else
    {
      current_location.error("Could not get ifndef label");
    }
  }
  else if (!strcasecmp(instruction, "else"))
  {
    if (label)
    {
      current_location.error("Processor directive else does not accept labels");
    }
    else
    {
      if (ifs.empty())
      {
        current_location.error("Processor directive else without ifdef");
      }
      else
      {
        bool process = !ifs.top();
        ifs.pop();
        ifs.push(process);
      }
    }
  }
  else if (!strcasecmp(instruction, "elifdef") || !strcasecmp(instruction, "elseifdef"))
  {
    if (label)
    {
      if (ifs.top())
      {
        ifs.pop();
        ifs.push(false);
      }
      else if (defines.find(label) != defines.end())
      {
        ifs.pop();
        ifs.push(true);
      }
    }
    else
    {
      current_location.error("Could not get elseifdef label");
    }

  }
  else if (!strcasecmp(instruction, "endif"))
  {
    if (label)
    {
      current_location.error("Processor directive endif does not accept labels");
    }
    else
    {
      if (ifs.empty())
      {
        current_location.error("Processor directive endif without ifdef");
      }
      else
      {
        ifs.pop();
      }
    }
  }
  else
  {
    current_location.error("Unknown processor directive");
  }
}

//Return the comment from global line variable
char *get_comment(char comment_seperator)
{
  char *comment = find_chr(line, comment_seperator);
  if (comment)
  {
    *comment = 0;
    comment++;
    if (isspace(comment[strlen(comment)-1]))
    {
      comment[strlen(comment)-1] = 0;
    }
  }
  return(comment);
}

void output_parser_comment(ostream& c_stream, const char *comment)
{
  if (comment)
  {
    c_stream << " //" << comment;
  }
  c_stream << "\n";
}

void output_header_conditional(ostream& cheader_stream, const char *instruction, const char *label)
{
  if ((!strcasecmp(instruction, "elifdef") || !strcasecmp(instruction, "elseifdef")) && label)
  {
    cheader_stream << "#elif defined(" << label << ")\n";
  }
  else
  {
    cheader_stream << "#" << instruction;
    if (label)
    {
      cheader_stream << " " << label;
    }
    cheader_stream << "\n";
  }
}


#define CONFIG_COMMENT (config_comment ? config_comment : "")

void parser_generate(istream& psr_stream, ostream& c_stream, ostream& cheader_stream, string cheader_file = "")
{
  current_location.line_number = current_location.column_number = 0;
  ostringstream cvars(""), hvars("");
  uLong psr_file_hash = crc32(0L, Z_NULL, 0);

  while (!psr_stream.eof())
  {
    char *token;
    char *parser_comment;
    char *config_comment;


    psr_stream.getline(line, LINE_LENGTH);
    current_location.line_number++;
    psr_file_hash = crc32(psr_file_hash, (const Bytef *)line, strlen(line));

    parser_comment = get_comment(';');

    if (all_spaces(line))
    {
      if (ifs.all_true())
      {
        output_parser_comment(cvars, parser_comment);
      }
      continue;
    }

    config_comment = get_comment('@');

    if (all_spaces(line) && config_comment)
    {
      if (ifs.all_true())
      {
        variable::config_data.add_comment(config_comment);
      }
      continue;
    }

    if ((token = get_token(line, " ")) &&
        (strcasecmp(token, "NEWSYM") || (token = get_token(0, " ,"))))
    {
      if ((*token == '#') || (*token == '%'))
      {
        char *next_token = get_token(0, " ");
        handle_directive(token+1, next_token);

        if (cheader_stream && (!next_token || strncasecmp(next_token, "PSR_", strlen("PSR_"))))
        {
          output_header_conditional(hvars, token+1, next_token);
        }
        continue;
      }

      string varname;
      string dependancy;
      char *d;

      if ((d = strchr(token, ':')))
      {
        varname = d+1;
        dependancy.assign(token, d-token);
        dependancies.insert(dependancy);
        dependancy += ':';
      }
      else
      {
        varname = token;
      }

      if ((token = get_token(0, " ,")))
      {
        size_t array = 0;
        bool is_array = !strcasecmp(token, "times");
        bool is_packed = !strcasecmp(token, "packed");
        bool is_ptr = !strcasecmp(token, "ptr");
        if ((!is_array && !is_packed && !is_ptr) ||
            ((token = get_token(0, " ")) && (array = enhanced_atoi(token)) && (token = get_token(0, " "))))
        {
          char *asm_type = token;
          char *var_type = convert_asm_type(asm_type);

          if (var_type)
          {
            string initial_value = get_token(0, " ,\n");
            ostringstream var_init("");

            if (((initial_value[0] == '\"') && (initial_value[initial_value.length()-1] == '\"')) ||
                ((initial_value[0] == '\'') && (initial_value[initial_value.length()-1] == '\'')))
            {
              //Make sure it's double quoted
              initial_value[0] = '\"';
              initial_value[initial_value.length()-1] = '\"';

              if (!array)
              {
                array = initial_value.length()-1; //Size minus quotes plus null
              }

              var_init << "char " << varname << "[" << array << "];";

              if (ifs.all_true())
              {
                ostringstream memset_line;

                if (initial_value.length()-2 < array)
                {
                  memset_line << "strcpy(" << varname << ", " << initial_value << ");";
                }
                else
                {
                  memset_line << "strncpy(" << varname << ", " << initial_value << ", " << (array-1) << "); "
                              << varname << "[" << array << "] = 0;";
                }
                memsets.push_back(memset_line.str());
                variable::config_data.add_var_quoted(varname, dependancy, CONFIG_COMMENT);
              }
            }
            else if (is_ptr)
            {
              var_init << var_type << " *" << varname << ";";
              if (ifs.all_true())
              {
                variable::config_data.add_var_ptr(varname, var_type, array, dependancy, CONFIG_COMMENT);
              }
            }
            else
            {
              ssize_t init_value_num = safe_atoi(c_hex_convert(asm2c_hex_convert(initial_value)));

              if ((init_value_num < 0) && !strncmp(var_type, "unsigned ", strlen("unsigned ")))
              {
                var_type += strlen("unsigned ");
              }

              var_init << var_type << " " << varname;
              if (array)
              {
                if (var_type_is_char(var_type) || !init_value_num)
                {
                  var_init << "[" << array << "]";

                  if (ifs.all_true())
                  {
                    ostringstream memset_line;
                    memset_line << "memset(" << varname << ", " << init_value_num << ", " << array;

                    if (var_type_is_short(var_type))
                    {
                      memset_line << short_scale;
                    }
                    else if (var_type_is_int(var_type))
                    {
                      memset_line << int_scale;
                    }

                    memset_line << ");";
                    memsets.push_back(memset_line.str());
                  }
                }
                else
                {
                  var_init << "[" << array << "] = {";
                  for (size_t i = array; i > 1; i--)
                  {
                    var_init << init_value_num << ",";
                  }
                  var_init << init_value_num << "}";
                }

                if (ifs.all_true())
                {
                  if (is_array)
                  {
                    variable::config_data.add_var_mult(varname, var_type, array, dependancy, CONFIG_COMMENT);
                  }
                  else if (is_packed)
                  {
                    variable::config_data.add_var_packed(varname, array, dependancy, CONFIG_COMMENT);
                  }
                }
              }
              else
              {
                if ((token = get_token(0, " ,\n")))
                {
                  array = 1;
                  var_init << "[] = {" << init_value_num;
                  do
                  {
                    var_init << "," << atoi(token);
                    array++;
                  } while((token = get_token(0, " ,\n")));
                  var_init << "}";

                  if (ifs.all_true())
                  {
                    variable::config_data.add_var_mult(varname, var_type, array, dependancy, CONFIG_COMMENT);
                  }
                }
                else
                {
                  var_init << " = " << init_value_num;

                  if (ifs.all_true())
                  {
                    variable::config_data.add_var_single(varname, var_type, dependancy, CONFIG_COMMENT);
                  }
                }
              }
              var_init << ";";
            }

            if (ifs.all_true())
            {
              cvars << var_init.str();
            }

            if (cheader_stream)
            {
              string header_data = var_init.str();
              size_t equal_pos;
              if ((equal_pos = header_data.find("=")) != string::npos)
              {
                header_data.erase(equal_pos-1);
                header_data.append(";");
              }
              hvars << "extern " << header_data << "\n";
            }
          }
          //Else already handled
        }
        else
        {
          current_location.error("Could not get array size");
        }
      }
      else
      {
        current_location.error("Could not get type");
      }
    }
    else
    {
      current_location.error("Could not get variable name");
    }

    if (ifs.all_true())
    {
      output_parser_comment(cvars, parser_comment);
    }
  }

  output_parser_start(c_stream, cheader_file);
  output_extsym_dependancies(c_stream);
  if (defines.find("PSR_EXTERN") == defines.end())
  {
    c_stream << cvars.str();
    output_init_var(c_stream);
  }
  else if (!cheader_file.length())
  {
    cerr << "Error: Requested PSR_EXTERN yet no header file specified." << endl;
  }

  if (defines.find("PSR_HASH") != defines.end())
  {
    c_stream << "static unsigned int PSR_HASH = 0x" << hex << psr_file_hash << dec << ";\n";
  }

  output_write_var(c_stream);
  output_read_var(c_stream);
  c_stream << "\n";


  if (cheader_stream)
  {
    output_cheader_start(cheader_stream);
    cheader_stream << hvars.str();
    output_cheader_end(cheader_stream);
  }

  if (!ifs.empty())
  {
    cerr << "Error: " << ifs.size() << " ifdef segments have no endif." << endl;
  }
}

int main(size_t argc, const char **argv)
{
  const char *cheader_file = 0;
  bool compile = false;

  size_t param_pos = 1;
  for (; param_pos < argc; param_pos++)
  {
    if (!strncmp(argv[param_pos], "-D", 2))
    {
      defines.insert(argv[param_pos]+2);
    }
    else if (!strcmp(argv[param_pos], "-cheader"))
    {
      param_pos++;
      cheader_file = argv[param_pos];
    }
    else if (!strcmp(argv[param_pos], "-compile"))
    {
      compile = true;
    }
    else if (!strcmp(argv[param_pos], "-flags"))
    {
      param_pos++;
      cflags = argv[param_pos];
    }
    else if (!strcmp(argv[param_pos], "-fname"))
    {
      param_pos++;
      family_name = argv[param_pos];
    }
    else if (!strcmp(argv[param_pos], "-gcc"))
    {
      param_pos++;
      gcc = argv[param_pos];
    }
    else
    {
      break;
    }
  }

  if ((argc-param_pos) != 2)
  {
    cout << "Config file handler creator by Nach (C) 2005-2007\n"
         << "\n"
         << "Usage:\n"
         << "parsegen [options] <output> <input>\n"
         << "\n"
         << "\n"
         << "Options:\n"
         << "\n"
         << "  -Ddefine   Define a processor director. Example: -D__MSDOS__\n"
         << "             Can specify multiple defines.\n"
         << "\n"
         << "  -cheader   Create a C/C++ header with the following name.\n"
         << "             Example: -cheader cfgvars.h\n"
         << "\n"
         << "  -fname     Use the following name for the main functions.\n"
         << "             Example: -fname math\n"
         << "             Would make init_cfg_vars become init_math_vars the\n"
         << "             happens to write_cfg_vars and read_cfg_vars.\n"
         << "\n"
         << "  -compile   Compiles output instead of outputting C file.\n"
         << "\n"
         << "  -gcc       Use with -compile. Parameter passed in the name of\n"
         << "             the C compiler to use, it should be GCC based.\n"
         << "             It will not work with MSVC based compilers.\n"
         << "\n"
         << "  -flags     Use with -compile. Flags passed as next parameter\n"
         << "             are passed to the C compiler.\n"
         << "             Example: -flags \"-O3 -march=pentium3 -ggdb3\"\n"
         << "\n"
         << endl;

    return(1);
  }

  string cname = family_name+string(".c");
  const char *psr_file = argv[param_pos+1], *c_file = compile ? cname.c_str() : argv[param_pos];
  const char *obj_file = compile ? argv[param_pos] : 0;
  int ret_val = 0;

  ifstream psr_stream(psr_file);
  if (psr_stream)
  {
    ofstream c_stream(c_file);
    if (c_stream)
    {
      ofstream cheader_stream;
      if (cheader_file)
      {
        cheader_stream.open(cheader_file);
        if (cheader_stream)
        {
          parser_generate(psr_stream, c_stream, cheader_stream, cheader_file);
        }
        else
        {
          cerr << "Error opening " << cheader_file << " for writing." << endl;
          ret_val |= 8;
        }

        cheader_stream.close();
      }
      else
      {
        parser_generate(psr_stream, c_stream, cheader_stream);
      }
      c_stream.close();
    }
    else
    {
      cerr << "Error opening " << c_file << " for writing." << endl;
      ret_val |= 2;
    }

    psr_stream.close();
  }
  else
  {
    cerr << "Error opening " << psr_file << " for reading." << endl;
    ret_val |= 4;
  }

  if (!ret_val && compile)
  {
    string command = COMPILE_OBJ(obj_file, cname);
    cout << "parsegen: " << command << "\n";
    system(command.c_str());
    remove(cname.c_str());
  }

  return(ret_val);
}

