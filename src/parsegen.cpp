/*
Copyright (C) 2005 Nach ( http://nsrt.edgeemu.com )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
Config file handler creator by Nach (C) 2005
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

#define LINE_LENGTH 2048*10
char line[LINE_LENGTH];

set<string> defines;
stack<bool> ifs;

typedef vector<string> str_array;

str_array memsets;


static struct
{
  size_t line_number;
  size_t column_number;

  void error(const char *str)
  {
    cerr << "Error: parse problem occured at " << line_number << ":" << column_number << ". " << str << "." << endl;
  }
} current_location;

enum cfg_value_type { single_value, quoted_value, parameterized_value };

class variable_type
{
  public:
  enum v_type { UC, US, UD, SC, SS, SD };
  variable_type(v_type type) : type(type) {}
  v_type getOType() { return(type); }
  const char *getCType(bool);
  char getFChar();
  bool is_signed();

  private:
  v_type type;
};

const char *variable_type::getCType(bool underscore)
{
  if (underscore)
  {
    switch (type)
    {
      case SC: return("char");
      case SS: return("short");
      case SD: return("int");
      case UC: return("unsigned_char");
      case US: return("unsigned_short");
      case UD: return("unsigned_int");
    }
  }
  else
  {
    switch (type)
    {
      case SC: return("char");
      case SS: return("short");
      case SD: return("int");
      case UC: return("unsigned char");
      case US: return("unsigned short");
      case UD: return("unsigned int");
    }
  }
  return("");
}

char variable_type::getFChar()
{
  return(is_signed() ? 'd' : 'u');
}

bool variable_type::is_signed()
{
  switch (type)
  {
    case SC: case SS: case SD: return(true);
    default: return(false);
  }
}

struct full_type
{
  variable_type::v_type object;
  cfg_value_type type;
  full_type(variable_type::v_type object, cfg_value_type type) : object(object), type(type) {}
  bool operator==(const full_type& ft) const { return((ft.object == object) && (ft.type == type)); }
};

struct cfg_var_struct
{
  string name;
  variable_type object;
  cfg_value_type type;
  size_t size;

  cfg_var_struct(string& name, variable_type::v_type object, cfg_value_type val, size_t size) :
  name(name), object(object), type(val), size(size)
  {
  }
  bool operator==(const string& name) const { return(this->name == name); }
  bool operator==(const full_type& ft) { return(ft == full_type(object.getOType(), type)); }
};

typedef vector<cfg_var_struct> cfg_var_array;

static class
{
  private:
  cfg_var_array array;


  public:
  void add(string& name, variable_type::v_type object, cfg_value_type val, size_t size)
  {
    cfg_var_struct var(name, object, val, size);
    if (find(array.begin(), array.end(), name) == array.end())
    {
      array.push_back(var);
    }
    else
    {
      cerr << "Duplicate definition of \"" << name << "\" found on line " << current_location.line_number << "." << endl;
    }
  }

  void add(string& name, const char *str, cfg_value_type val, size_t size)
  {
    if (!strcmp(str, "char")) { add(name, variable_type::SC, val, size); }
    else if (!strcmp(str, "short")) { add(name, variable_type::SS, val, size); }
    else if (!strcmp(str, "int")) { add(name, variable_type::SD, val, size); }
    else if (!strcmp(str, "unsigned char")) { add(name, variable_type::UC, val, size); }
    else if (!strcmp(str, "unsigned short")) { add(name, variable_type::US, val, size); }
    else if (!strcmp(str, "unsigned int")) { add(name, variable_type::UD, val, size); }
    else
    {
      cerr << "Invalid C type \"" << str << "\" when parsing line " << current_location.line_number << "." << endl;
    }
  }

  bool type_used_array(const full_type& ft)
  {
    return(find(array.begin(), array.end(), ft) != array.end());
  }

  cfg_var_array::iterator begin() { return(array.begin()); }
  cfg_var_array::iterator end() { return(array.end()); }

} cfg_var;

//Find next matching character which is not escaped
char *find_next_match(char *str, char match_char)
{
  char *pos = 0;

  while (*str)
  {
    if (*str == match_char)
    {
      pos = str;
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

#ifdef _MSC_VER //MSVC
    system("cl /nologo /Feeatio.exe eatio.c");
#else
    system("gcc -o eatio.exe eatio.c -s");
#endif

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

#define var_type_is_char(var_type)  !strcmp(var_type+strlen(var_type)-strlen("char"), "char")
#define var_type_is_short(var_type) !strcmp(var_type+strlen(var_type)-strlen("short"), "short")
#define var_type_is_int(var_type)   !strcmp(var_type+strlen(var_type)-strlen("int"), "int")

#define short_scale "*sizeof(short)"
#define int_scale "*sizeof(int)"

//Return the comment from global line variable
char *get_comment()
{
  char *comment = find_chr(line, ';');
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

void output_comment(ostream& c_stream, const char *comment)
{
  if (comment)
  {
    c_stream << " //" << comment;
  }
  c_stream << "\n";
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
           << "  \n"
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
           << "  return(pos);  \n"
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
                 << "unsigned char read_cfg_vars(const char *);\n"
                 << "unsigned char write_cfg_vars(const char *);\n"
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
           << "static void init_cfg_vars()\n"
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

void output_array_write(ostream& c_stream, variable_type::v_type v_type)
{
  if (cfg_var.type_used_array(full_type(v_type, parameterized_value)))
  {
    variable_type type(v_type);

    c_stream << "\n"
             << "static void write_" << type.getCType(true) << "_array(FILE *fp, const char *var_name, " << type.getCType(false) << " *var, size_t size)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  fprintf(fp, \"%s=%" << type.getFChar() << "\", var_name, (int)*var);\n"
             << "  for (i = 1; i < size; i++)\n"
             << "  {\n"
             << "    fprintf(fp, \",%" << type.getFChar() << "\", (int)(var[i]));\n"
             << "  }\n"
             << "  fprintf(fp, \"\\n\");\n"
             << "}\n";
  }
}

void output_write_var(ostream& c_stream)
{
  output_array_write(c_stream, variable_type::UC);
  output_array_write(c_stream, variable_type::US);
  output_array_write(c_stream, variable_type::UD);
  output_array_write(c_stream, variable_type::SC);
  output_array_write(c_stream, variable_type::SS);
  output_array_write(c_stream, variable_type::SD);

  c_stream << "\n"
           << "unsigned char write_cfg_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n"
           << "  init_cfg_vars();\n"
           << "\n"
           << "  if ((fp = fopen(file, \"w\")))\n"
           << "  {\n";
  for (cfg_var_array::iterator i = cfg_var.begin(); i != cfg_var.end(); i++)
  {
    if (i->type == parameterized_value)
    {
      c_stream << "    write_" << i->object.getCType(true)
               << "_array(fp, \"" << i->name << "\", " << i->name << ", " << i->size << ");\n";
    }
    else
    {
      c_stream << "    fprintf(fp, \"" << i->name << "=";
      if (i->type == single_value)
      {
      c_stream << "%" << i->object.getFChar() << "\\n\", " << i->name;
      }
      else
      {
       c_stream << "%s\\n\", encode_string(" << i->name << ")";
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

void output_array_read(ostream& c_stream, variable_type::v_type v_type)
{
  if (cfg_var.type_used_array(full_type(v_type, parameterized_value)))
  {
    variable_type type(v_type);

    c_stream << "\n"
             << "static void read_" << type.getCType(true) << "_array(char *line, " << type.getCType(false) << " *var, size_t size)\n"
             << "{\n"
             << "  size_t i;\n"
             << "  char *token;\n"
             << "  *var = (" << type.getCType(false) << ")atoi(strtok(line, \", \\t\\r\\n\"));\n"
             << "  for (i = 1; (i < size) && (token = strtok(0, \", \\t\\r\\n\")); i++)\n"
             << "  {\n"
             << "    var[i] = (" << type.getCType(false) << ")atoi(token);\n"
             << "  }\n"
             << "}\n";
  }
}

void output_read_var(ostream& c_stream)
{
  output_array_read(c_stream, variable_type::UC);
  output_array_read(c_stream, variable_type::US);
  output_array_read(c_stream, variable_type::UD);
  output_array_read(c_stream, variable_type::SC);
  output_array_read(c_stream, variable_type::SS);
  output_array_read(c_stream, variable_type::SD);

  c_stream << "\n"
           << "unsigned char read_cfg_vars(const char *file)\n"
           << "{\n"
           << "  FILE *fp = 0;\n"
           << "\n"
           << "  init_cfg_vars();\n"
           << "\n"
           << "  if (!(fp = fopen(file, \"r\")))\n"
           << "  {\n"
           << "    write_cfg_vars(file);\n"
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
  for (cfg_var_array::iterator i = cfg_var.begin(); i != cfg_var.end(); i++)
  {
    c_stream << "    if (!strcmp(var, \"" + i->name + "\")) { ";
    if (i->type == single_value)
    {
      c_stream << i->name << " = (" << i->object.getCType(false) << ")atoi(value);";
    }
    else if (i->type == parameterized_value)
    {
      c_stream << "read_" << i->object.getCType(true)
               << "_array(value, " << i->name << ", " << i->size << ");";
    }
    else
    {
      c_stream << "*" << i->name << " = 0; "
               << "strncat(" << i->name << ", decode_string(value), sizeof(" << i->name << ")-1);";
    }
    c_stream << " }\n";
  }
  c_stream << "  }\n"
           << "\n"
           << "  fclose(fp);\n"
           << "  write_cfg_vars(file);\n"
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
    char *comment;

    psr_stream.getline(line, LINE_LENGTH);
    current_location.line_number++;

    comment = get_comment();

    if (all_spaces(line))
    {
      output_comment(c_stream, comment);
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
        if (strcasecmp(token, "times") ||
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

              cfg_var.add(varname, variable_type::SC, quoted_value, 0);
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

                cfg_var.add(varname, var_type, parameterized_value, array);
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

                  cfg_var.add(varname, var_type, parameterized_value, array);
                }
                else
                {
                  var_init << " = " << init_value_num;

                  cfg_var.add(varname, var_type, single_value, 0);
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

    output_comment(c_stream, comment);
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
    else
    {
      break;
    }
  }

  if ((argc-param_pos) != 2)
  {
    cout << "Config file handler creator by Nach (C) 2005\n"
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
         << endl;

    return(1);
  }

  const char *psr_file = argv[param_pos+1], *c_file = argv[param_pos];
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

  return(0);
}

