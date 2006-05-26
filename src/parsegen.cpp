/*
Copyright (C) 2005-2006 Nach ( http://nsrt.edgeemu.com )

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
Config file handler creator by Nach (C) 2005-2006
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

#ifdef _MSC_VER //MSVC
typedef int ssize_t;
#define strcasecmp stricmp
#define __WIN32__
#endif

#if defined(__MSDOS__) || defined(__WIN32__)
#define SLASH_STR "\\"
#else
#define SLASH_STR "/"
#endif

#ifdef _MSC_VER //MSVC
#define COMPILE_EXE(exe, c) "cl /nologo /Fe"exe " "c
#define COMPILE_OBJ(obj, c) "cl /nologo /Fo"obj " "c
#else
#define COMPILE_EXE(exe, c) "gcc -o "exe " "c " -s"
#define COMPILE_OBJ(obj, c) "gcc -o "obj " -c "c
#endif

#define LINE_LENGTH 2048*10
char line[LINE_LENGTH];


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
string hex_convert(string str)
{
  size_t dollar_pos;
  while ((dollar_pos = str.find("$")) != string::npos)
  {
    str.replace(dollar_pos, 1, "0x");
  }

  while (tolower(str[0]) == 'h')
  {
    str.erase(0, 1);
  }

  size_t h_pos;
  while ((h_pos = str.find_first_of("hH")) != string::npos)
  {
    size_t h_len = 1;
    while ((h_pos-h_len) && isxdigit(str[h_pos-h_len]))
    {
      h_len++;
    }
    str.erase(h_pos, 1);
    str.insert(h_pos-h_len+1, "0x");
  }

  for (size_t i = 0; i < str.size(); i++)
  {
    if ((!i || !isxdigit(str[i-1])) && (str[i] == '0') && (str[i+1] != 'x'))
    {
      str.erase(i, 1);
      i--;
    }
  }

  return(str);
}

//Ascii numbers to integer, with support for mathematics in the string
ssize_t enhanced_atoi(const char *str)
{
  ssize_t num = 0;

  //Make sure result file doesn't exist
  if (remove("eatio.res") && (errno != ENOENT))
  {
    cerr << "Error: Can not get accurate value information (eatio.res)." << endl;
  }

  //Biggest cheat of all time
  ofstream out_stream("eatio.c");
  if (out_stream)
  {
    out_stream << "#include <stdio.h>\n"
               << "int main()\n"
               << "{\n"
               << "  FILE *fp = fopen(\"eatio.res\", \"w\");\n"
               << "  if (fp)\n"
               << "  {\n"
               << "    fprintf(fp, \"%d\", " << hex_convert(str) << ");\n"
               << "    fclose(fp);\n"
               << "  }\n"
               << "  return(0);\n"
               << "}\n\n";
    out_stream.close();

    system(COMPILE_EXE("eatio.exe", "eatio.c"));
    system("."SLASH_STR"eatio.exe");

    remove("eatio.c");
    remove("eatio.exe");
    remove("eatio.obj"); //Needed for stupid MSVCs which leave object files lying around

    ifstream in_stream("eatio.res");
    if (in_stream)
    {
      in_stream >> num;
      in_stream.close();
    }
    else
    {
      cerr << "Error: Can not get accurate value information (eatio.res)." << endl;
    }

    remove("eatio.res");
  }

  return(num);
}

//Standard atoi(), but shows error if string isn't all a number
ssize_t safe_atoi(string& str)
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

string family_name = "cfg";

set<string> defines;
stack<bool> ifs;

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

  enum storage_format { none, single, quoted, mult, mult_packed };

  struct config_data_element
  {
    string name;
    storage_format format;
    ctype type;
    size_t length;
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
      config_data_element new_element = { "", none, NT, 0, comment };
      data_array.push_back(new_element);
    }

    void add_var_single(string& name, ctype type, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, single, type, 0, comment };
        data_array.push_back(new_element);
      }
    }
    void add_var_single(string& name, const char *type, string comment = "")
    {
      add_var_single(name, GetCType(type), comment);
    }

    void add_var_quoted(string& name, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, quoted, NT, 0, comment };
        data_array.push_back(new_element);
      }
    }

    void add_var_mult(string& name, ctype type, size_t length, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, mult, type, length, comment };
        data_array.push_back(new_element);
      }
    }
    void add_var_mult(string& name, const char *type, size_t length, string comment = "")
    {
      add_var_mult(name, GetCType(type), length, comment);
    }

    void add_var_packed(string& name, size_t length, string comment = "")
    {
      if (!duplicate_name(name))
      {
        config_data_element new_element = { name, mult_packed, NT, length, comment };
        data_array.push_back(new_element);
      }
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
  else
  {
    current_location.error("Not a valid type");
  }

  if (var_type && !unsigned_var)
  {
    var_type += strlen("unsigned ");
  }

  return(var_type);
}

void output_parser_start(ostream& c_stream)
{
  c_stream << "/*\n"
           << "Config file handler generated by Nach's Config file handler creator.\n"
           << "*/\n"
           << "\n"
           << "#include <stdio.h>\n"
           << "#include <stdlib.h>\n"
           << "#include <ctype.h>\n"
           << "#include <string.h>\n"
           << "\n"
           << "\n"
           << "#define LINE_LENGTH " << LINE_LENGTH << "\n"
           << "static char line[LINE_LENGTH];\n"
           << "static char packed[LINE_LENGTH];\n"
           << "\n"
           << "\n"
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
           << "}\n"
           << "\n"
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
           << "static char *char_array_pack(const char *str, size_t len)\n"
           << "{\n"
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
           << "  return(packed);\n"
           << "}\n"
           << "\n"
           << "static char *char_array_unpack(char *str)\n"
           << "{\n"
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
           << "  return(packed);\n"
           << "}\n"
           << "\n"
           << "\n";
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
                 << "unsigned char write_" << family_name << "_vars(const char *);\n"
                 << "\n";
}

void output_cheader_end(ostream& cheader_stream)
{
  cheader_stream << "\n"
                 << "#ifdef __cplusplus\n"
                 << "             }\n"
                 << "#endif\n"
                 << "\n";
}


void output_init_var(ostream& c_stream)
{
  c_stream << "\n"
           << "static void init_" << family_name << "_vars()\n"
           << "{\n"
           << "  static unsigned char init_done = 0;\n"
           << "  if (!init_done)\n"
           << "  {\n"
           << "    init_done = 1;\n"
           << "\n";
  for (str_array::iterator i = memsets.begin(); i != memsets.end(); i++)
  {
    c_stream << "    " << *i << "\n";
  }
  c_stream << "  }\n"
           << "}\n";
}

void output_array_write(ostream& c_stream, variable::ctype type)
{
  if (variable::config_data.ctype_mult_used(type))
  {
    c_stream << "\n"
             << "static void write_" << variable::info[type].CTypeUnderscore << "_array(FILE *fp, const char *var_name, " << variable::info[type].CTypeSpace << " *var, size_t size, const char *comment)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  fprintf(fp, \"%s=%" << variable::info[type].FormatChar << "\", var_name, (int)*var);\n"
             << "  for (i = 1; i < size; i++)\n"
             << "  {\n"
             << "    fprintf(fp, \",%" << variable::info[type].FormatChar << "\", (int)(var[i]));\n"
             << "  }\n"
             << "  if (comment)\n"
             << "  {\n"
             << "    fprintf(fp, \" ;%s\", comment);\n"
             << "  }\n"
             << "  fprintf(fp, \"\\n\");\n"
             << "}\n";
  }
}

void output_write_var(ostream& c_stream)
{
  output_array_write(c_stream, variable::UC);
  output_array_write(c_stream, variable::US);
  output_array_write(c_stream, variable::UD);
  output_array_write(c_stream, variable::SC);
  output_array_write(c_stream, variable::SS);
  output_array_write(c_stream, variable::SD);

  c_stream << "\n"
           << "unsigned char write_" << family_name << "_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n"
           << "  init_" << family_name << "_vars();\n"
           << "\n"
           << "  if ((fp = fopen(file, \"w\")))\n"
           << "  {\n";
  for (variable::config_data_array::iterator i = variable::config_data.begin(); i != variable::config_data.end(); i++)
  {
    if (i->format == variable::none)
    {
      if (i->comment != "")
      {
        c_stream << "    fprintf(fp, \";%s\\n\", " << encode_string(i->comment) << ");\n";
      }
      else
      {
        c_stream << "    fprintf(fp, \"\\n\");\n";
      }
    }
    else if (i->format == variable::mult)
    {
      c_stream << "    write_" << variable::info[i->type].CTypeUnderscore
               << "_array(fp, \"" << i->name << "\", " << i->name << ", " << i->length << ", " << ((i->comment != "") ? encode_string(i->comment) : "0") << ");\n";
    }
    else
    {
      string config_comment = (i->comment != "") ? (string(" ;") + encode_string(i->comment, false)) : "";
      c_stream << "    fprintf(fp, \"" << i->name << "=";
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
      c_stream << ");\n";
    }
  }
  c_stream << "    fclose(fp);\n"
           << "\n"
           << "    return(1);\n"
           << "  }\n"
           << "  return(0);\n"
           << "}\n";
}

void output_array_read(ostream& c_stream, variable::ctype type)
{
  if (variable::config_data.ctype_mult_used(type))
  {
    c_stream << "\n"
             << "static void read_" << variable::info[type].CTypeUnderscore << "_array(char *line, " << variable::info[type].CTypeSpace << " *var, size_t size)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  char *token;\n"
             << "  *var = (" << variable::info[type].CTypeSpace << ")atoi(strtok(line, \", \\t\\r\\n\"));\n"
             << "  for (i = 1; (i < size) && (token = strtok(0, \", \\t\\r\\n\")); i++)\n"
             << "  {\n"
             << "    var[i] = (" << variable::info[type].CTypeSpace << ")atoi(token);\n"
             << "  }\n"
             << "}\n";
  }
}

void output_read_var(ostream& c_stream)
{
  output_array_read(c_stream, variable::UC);
  output_array_read(c_stream, variable::US);
  output_array_read(c_stream, variable::UD);
  output_array_read(c_stream, variable::SC);
  output_array_read(c_stream, variable::SS);
  output_array_read(c_stream, variable::SD);

  c_stream << "\n"
           << "unsigned char read_" << family_name << "_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n"
           << "  init_" << family_name << "_vars();\n"
           << "\n"
           << "  if (!(fp = fopen(file, \"r\")))\n"
           << "  {\n"
           << "    write_" << family_name << "_vars(file);\n"
           << "    return(0);\n"
           << "  }\n"
           << "\n"
           << "  while (!feof(fp))\n"
           << "  {\n"
           << "    char *p, *var, *value;\n"
           << "\n"
           << "    fgets(line, LINE_LENGTH, fp);\n"
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
           << "      if (!*var || !*value) { continue; }\n"
           << "    }\n"
           << "    else\n"
           << "    {\n"
           << "      continue;\n"
           << "    }\n"
           << "\n";
  for (variable::config_data_array::iterator i = variable::config_data.begin(); i != variable::config_data.end(); i++)
  {
    if (i->format != variable::none)
    {
      c_stream << "    if (!strcmp(var, \"" + i->name + "\")) { ";
      if (i->format == variable::single)
      {
        c_stream << i->name << " = (" << variable::info[i->type].CTypeSpace << ")atoi(value);";
      }
      else if (i->format == variable::mult)
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
  c_stream << "  }\n"
           << "\n"
           << "  fclose(fp);\n"
           << "  write_" << family_name << "_vars(file);\n"
           << "  return(1);\n"
           << "}\n";
}

void handle_directive(char *instruction, char *label)
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

#define CONFIG_COMMENT (config_comment ? config_comment : "")

void parser_generate(istream& psr_stream, ostream& c_stream, ostream& cheader_stream)
{
  current_location.line_number = current_location.column_number = 0;

  output_parser_start(c_stream);

  if (cheader_stream)
  {
    output_cheader_start(cheader_stream);
  }

  while (!psr_stream.eof())
  {
    char *token;
    char *parser_comment;
    char *config_comment;

    psr_stream.getline(line, LINE_LENGTH);
    current_location.line_number++;

    parser_comment = get_comment(';');

    if (all_spaces(line))
    {
      output_parser_comment(c_stream, parser_comment);
      continue;
    }

    config_comment = get_comment('@');

    if (all_spaces(line) && config_comment)
    {
      if (ifs.empty() || ifs.top())
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
        handle_directive(token+1, get_token(0, " "));
        continue;
      }

      if (!ifs.empty() && !ifs.top())
      {
        continue;
      }

      string varname = token;

      if ((token = get_token(0, " ,")))
      {
        size_t array = 0;
        bool is_array = !strcasecmp(token, "times");
        bool is_packed = !strcasecmp(token, "packed");
        if ((!is_array && !is_packed) ||
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
              variable::config_data.add_var_quoted(varname, CONFIG_COMMENT);
            }
            else
            {
              ssize_t init_value_num = safe_atoi(initial_value);

              if (init_value_num < 0)
              {
                var_type += strlen("unsigned ");
              }

              var_init << var_type << " " << varname;
              if (array)
              {
                if (var_type_is_char(var_type) || !init_value_num)
                {
                  var_init << "[" << array << "]";

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
                else
                {
                  var_init << "[" << array << "] = {";
                  for (size_t i = array; i > 1; i--)
                  {
                    c_stream << init_value_num << ",";
                  }
                  var_init << init_value_num << "%d}";
                }

                if (is_array)
                {
                  variable::config_data.add_var_mult(varname, var_type, array, CONFIG_COMMENT);
                }
                else if (is_packed)
                {
                  variable::config_data.add_var_packed(varname, array, CONFIG_COMMENT);
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

                  variable::config_data.add_var_mult(varname, var_type, array, CONFIG_COMMENT);
                }
                else
                {
                  var_init << " = " << init_value_num;

                  variable::config_data.add_var_single(varname, var_type, CONFIG_COMMENT);
                }
              }
              var_init << ";";
            }

            c_stream << var_init.str();

            if (cheader_stream)
            {
              string header_data = var_init.str();
              size_t equal_pos;
              if ((equal_pos = header_data.find("=")) != string::npos)
              {
                header_data.erase(equal_pos-1);
                header_data.append(";");
              }
              cheader_stream << "extern " << header_data << "\n";
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

    output_parser_comment(c_stream, parser_comment);
  }

  output_init_var(c_stream);
  output_write_var(c_stream);
  output_read_var(c_stream);

  c_stream << "\n";

  if (cheader_stream)
  {
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
    else if (!strcmp(argv[param_pos], "-fname"))
    {
      param_pos++;
      family_name = argv[param_pos];
    }
    else
    {
      break;
    }
  }

  if ((argc-param_pos) != 2)
  {
    cout << "Config file handler creator by Nach (C) 2005-2006\n"
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
         << "             Example -cheader cfgvars.h\n"
         << "\n"
         << "  -fname     Use the following name for the main functions.\n"
         << "             Example -fname math\n"
         << "             Would make init_cfg_vars become init_math_vars the\n"
         << "             happens to write_cfg_vars and read_cfg_vars.\n"
         << "\n"
         << endl;

    return(1);
  }

  const char *psr_file = argv[param_pos+1], *c_file = compile ? "psrtemp.c" : argv[param_pos];
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
          parser_generate(psr_stream, c_stream, cheader_stream);
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
    cout << COMPILE_OBJ("psrtemp.obj", "psrtemp.c") << "\n";
    system(COMPILE_OBJ("psrtemp.obj", "psrtemp.c"));
    remove("psrtemp.c");
    cout << "Renaming psrtemp.obj to " << obj_file << endl;
    rename("psrtemp.obj", obj_file);
  }

  return(ret_val);
}

