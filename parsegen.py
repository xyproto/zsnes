#!/usr/bin/env python3

import sys
import os
import re
import subprocess
import zlib
from enum import Enum
from typing import List, Set, Dict, Optional, TextIO, Tuple
from dataclasses import dataclass

LINE_LENGTH = 262144

gcc = "gcc"
cflags = ""
family_name = "cfg"

class CType(Enum):
    NT = 0
    UC = 1
    US = 2
    UD = 3
    SC = 4
    SS = 5
    SD = 6

@dataclass
class TypeInfo:
    space: str
    underscore: str
    format_char: str
    signed: bool

TYPE_INFO = {
    CType.NT: TypeInfo("", "", "", False),
    CType.UC: TypeInfo("unsigned char", "unsigned_char", "u", False),
    CType.US: TypeInfo("unsigned short", "unsigned_short", "u", False),
    CType.UD: TypeInfo("unsigned int", "unsigned_int", "u", False),
    CType.SC: TypeInfo("signed char", "signed_char", "d", True),
    CType.SS: TypeInfo("short", "short", "d", True),
    CType.SD: TypeInfo("int", "int", "d", True)
}

class StorageFormat(Enum):
    NONE = 0
    SINGLE = 1
    QUOTED = 2
    MULT = 3
    MULT_PACKED = 4
    PTR = 5

@dataclass
class ConfigDataElement:
    name: str = ""
    format: StorageFormat = StorageFormat.NONE
    type: CType = CType.NT
    length: int = 0
    dependancy: str = ""
    comment: str = ""

class ConfigData:
    def __init__(self):
        self.data_array: List[ConfigDataElement] = []
        self.duplicate_names: Set[str] = set()

    def duplicate_name(self, name: str) -> bool:
        if name in self.duplicate_names:
            print(f"Duplicate definition of \"{name}\" found on line {current_location.line_number}.", file=sys.stderr)
            return True
        self.duplicate_names.add(name)
        return False

    def add_comment(self, comment: str):
        self.data_array.append(ConfigDataElement(comment=comment))

    def add_var_single(self, name: str, type_val, dependancy: str, comment: str = ""):
        if not self.duplicate_name(name):
            if isinstance(type_val, str):
                type_val = get_c_type(type_val)
            self.data_array.append(ConfigDataElement(name, StorageFormat.SINGLE, type_val, 0, dependancy, comment))

    def add_var_quoted(self, name: str, dependancy: str, comment: str = ""):
        if not self.duplicate_name(name):
            self.data_array.append(ConfigDataElement(name, StorageFormat.QUOTED, CType.NT, 0, dependancy, comment))

    def add_var_mult(self, name: str, type_val, length: int, dependancy: str, comment: str = ""):
        if not self.duplicate_name(name):
            if isinstance(type_val, str):
                type_val = get_c_type(type_val)
            self.data_array.append(ConfigDataElement(name, StorageFormat.MULT, type_val, length, dependancy, comment))

    def add_var_packed(self, name: str, length: int, dependancy: str, comment: str = ""):
        if not self.duplicate_name(name):
            self.data_array.append(ConfigDataElement(name, StorageFormat.MULT_PACKED, CType.NT, length, dependancy, comment))

    def add_var_ptr(self, name: str, type_val, length: int, dependancy: str, comment: str = ""):
        if not self.duplicate_name(name):
            if isinstance(type_val, str):
                type_val = get_c_type(type_val)
            self.data_array.append(ConfigDataElement(name, StorageFormat.PTR, type_val, length, dependancy, comment))

    def ctype_mult_used(self, type_val: CType) -> bool:
        return any(elem.format == StorageFormat.MULT and elem.type == type_val for elem in self.data_array)

    def ctype_ptr_used(self, type_val: CType) -> bool:
        return any(elem.format == StorageFormat.PTR and elem.type == type_val for elem in self.data_array)

    def packed_used(self) -> bool:
        return any(elem.format == StorageFormat.MULT_PACKED for elem in self.data_array)

    def quoted_used(self) -> bool:
        return any(elem.format == StorageFormat.QUOTED for elem in self.data_array)

    def unsigned_used(self) -> bool:
        return any(not TYPE_INFO[elem.type].signed for elem in self.data_array)

class LocationTracker:
    def __init__(self):
        self.line_number = 0
        self.column_number = 0

    def error(self, msg: str):
        print(f"Error: parse problem occured at {self.line_number}:{self.column_number}. {msg}.", file=sys.stderr)

defines: Set[str] = set()
dependancies: Set[str] = set()
ifs: List[bool] = []
memsets: List[str] = []
config_data = ConfigData()
current_location = LocationTracker()

def get_c_type(type_str: str) -> CType:
    for ctype, info in TYPE_INFO.items():
        if info.space == type_str:
            return ctype
    print(f"Invalid C type \"{type_str}\" when parsing line {current_location.line_number}.", file=sys.stderr)
    return CType.NT

def find_next_match(s: str, match_char: str) -> int:
    pos = -1
    i = 0
    while i < len(s):
        if s[i] == match_char:
            pos = i
            break
        if s[i] == '\\' and i + 1 < len(s):
            i += 1
        i += 1
    return pos

def find_chr(s: str, match_char: str) -> int:
    pos = -1
    i = 0
    while i < len(s):
        if s[i] == match_char:
            pos = i
            break
        if s[i] in ['"', "'"]:
            match_pos = find_next_match(s[i+1:], s[i])
            if match_pos >= 0:
                i += match_pos + 1
        i += 1
    return pos

def asm2c_hex_convert(s: str) -> str:
    s = re.sub(r'\$([0-9A-Fa-f]+)', r'0x\1', s)
    s = re.sub(r'([0-9][0-9A-Fa-f]*)[hH]', r'0x\1', s)
    return s

def c_hex_convert(s: str) -> str:
    def hex_to_dec(match):
        hex_val = match.group(1)
        return str(int(hex_val, 16))
    return re.sub(r'0x([0-9A-Fa-f]+)', hex_to_dec, s)

def enhanced_atoi(s: str) -> int:
    try:
        s = asm2c_hex_convert(s)
        s = c_hex_convert(s)
        return int(eval(s))
    except:
        current_location.error("Invalid number expression")
        return 0

def safe_atoi(s: str) -> int:
    if not s:
        s = "X"

    test_s = s[1:] if s.startswith('-') else s
    if not test_s.isdigit():
        current_location.error("Not a number")
        return 0

    return int(s)

def all_spaces(s: str) -> bool:
    return s.strip() == ""

def encode_string(s: str, quotes: bool = True) -> str:
    result = ""
    if quotes:
        result += '"'
    for char in s:
        if char in ['\\', '"', "'", '\n', '\t']:
            result += '\\'
        result += char
    if quotes:
        result += '"'
    return result

def all_true(lst: List[bool]) -> bool:
    return all(lst) if lst else True

def get_token(line: str, delimiters: str) -> List[str]:
    tokens = []
    current_token = ""
    in_quote = False
    quote_char = None
    i = 0

    while i < len(line):
        char = line[i]
        if not in_quote and char in delimiters:
            if current_token:
                tokens.append(current_token)
                current_token = ""
        elif char in ['"', "'"]:
            if not in_quote:
                in_quote = True
                quote_char = char
            elif char == quote_char:
                in_quote = False
                quote_char = None
            current_token += char
        else:
            current_token += char
        i += 1

    if current_token:
        tokens.append(current_token)

    return tokens

def convert_asm_type(type_str: str, unsigned_var: bool = True) -> Optional[str]:
    type_map = {
        "dd": "unsigned int",
        "dw": "unsigned short",
        "db": "unsigned char",
        "sd": "int",
        "sw": "short",
        "sb": "signed char"
    }

    var_type = type_map.get(type_str.lower())
    if not var_type:
        current_location.error("Not a valid type")
        return None

    if var_type.startswith("unsigned ") and not unsigned_var:
        var_type = var_type[9:]

    return var_type

class CodeGenerator:
    def __init__(self, c_stream: TextIO, cheader_file: str = ""):
        self.c_stream = c_stream
        self.cheader_file = cheader_file

    def write_includes(self):
        self.c_stream.write("""/*
Config file handler generated by Nach's Config file handler creator.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
""")

        if "PSR_COMPRESSED" in defines:
            self.c_stream.write("#include <zlib.h>\n")

        if self.cheader_file:
            self.c_stream.write(f'#include "{self.cheader_file}"\n')

        self.c_stream.write(f"""

#define LINE_LENGTH {LINE_LENGTH}
static char line[LINE_LENGTH];

""")

    def write_string_functions(self):
        if config_data.quoted_used() or config_data.packed_used():
            self.c_stream.write("""
static char *encode_string(const char *str)
{
  size_t i = 0;
  line[i++] = '\\"';
  while (*str)
  {
    if ((*str == '\\\\') ||
        (*str == '\\"') ||
        (*str == '\\'') ||
        (*str == '\\n') ||
        (*str == '\\t'))
    {
      line[i++] = '\\\\';
    }
    line[i++] = *str++;
  }
  line[i++] = '\\"';
  line[i] = 0;
  return(line);
}

static char *decode_string(char *str)
{
  size_t str_len = strlen(str), i = 0;
  char *dest = str;

  if ((str_len > 1) && (*str == '\\"') && (str[str_len-1] == '\\"'))
  {
    memmove(str, str+1, str_len-2);
    str[str_len-2] = 0;

    while (*str)
    {
      if (*str == '\\\\')
      {
        str++;
      }
      dest[i++] = *str++;
    }
  }
  dest[i] = 0;
  return(dest);
}
""")

    def write_utility_functions(self):
        self.c_stream.write("""
static char *find_next_match(char *str, char match_char)
{
  char *pos = 0;

  while (*str)
  {
    if (*str == match_char)
    {
      pos = str;
      break;
    }
    if (*str == '\\\\')
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

static char *find_str(char *str, char *match_str)
{
  char *pos = 0;

  while (*str)
  {
    if (strchr(match_str, *str))
    {
      pos = str;
      break;
    }
    if ((*str == '\\"') || (*str == '\\''))
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

""")

        if config_data.unsigned_used():
            self.c_stream.write("""
static int atoui(const char *nptr)
{
  return(strtoul(nptr, 0, 10));
}
""")

    def write_dependancies(self):
        self.c_stream.write("\n")
        for dep in dependancies:
            self.c_stream.write(f"extern unsigned char {dep};\n")

    def write_init_function(self):
        self.c_stream.write(f"""
static unsigned char psr_init_done = 0;
static void init_{family_name}_vars()
{{
  if (!psr_init_done)
  {{
    psr_init_done = 1;

""")
        for memset in memsets:
            self.c_stream.write(f"    {memset}\n")
        self.c_stream.write("  }\n}\n")

    def write_array_function(self, ctype: CType, operation: str):
        if not (config_data.ctype_mult_used(ctype) or config_data.ctype_ptr_used(ctype)):
            return

        info = TYPE_INFO[ctype]

        if operation == "write":
            self.c_stream.write(f"""
static void write_{info.underscore}_array(int (*outf)(void *, const char *, ...), void *fp, const char *var_name, {info.space} *var, size_t size, const char *comment)
{{
  size_t i;
  outf(fp, "%s=%{info.format_char}", var_name, (int)*var);
  for (i = 1; i < size; i++)
  {{
    outf(fp, ",%{info.format_char}", (int)(var[i]));
  }}
  if (comment)
  {{
    outf(fp, " ;%s", comment);
  }}
  outf(fp, "\\n");
}}
""")
        elif operation == "read":
            atoi_func = "atoi" if info.signed else "atoui"
            self.c_stream.write(f"""
static void read_{info.underscore}_array(char *line, {info.space} *var, size_t size)
{{
  size_t i;
  char *token;
  *var = ({info.space}){atoi_func}(strtok(line, ", \\t\\r\\n"));
  for (i = 1; (i < size) && (token = strtok(0, ", \\t\\r\\n")); i++)
  {{
    var[i] = ({info.space}){atoi_func}(token);
  }}
}}
""")

    def write_packed_functions(self):
        if not config_data.packed_used():
            return

        # Write functions
        self.c_stream.write("""
static char *base94_encode(size_t size)
{
  unsigned int i;
  static char buffer[] = { 0, 0, 0, 0, 0, 0};
  for (i = 0; i < 5; i++)
  {
    buffer[i] = ' ' + (char)(size % 94);
    size /= 94;
  }
  return(buffer);
}

static char *char_array_pack(const char *str, size_t len)
{
  char packed[LINE_LENGTH];
  char *p = packed;
  while (len)
  {
    if (*str)
    {
      size_t length = strlen(str);
      strcpy(p, encode_string(str));
      str += length;
      len -= length;
      p += strlen(p);
    }
    else
    {
      size_t i = 0;
      while (len && !*str)
      {
        i++;
        str++;
        len--;
      }

      sprintf(p, "0%s", encode_string(base94_encode(i)));
      p += strlen(p);
    }
    *p++ = '\\\\';
  }
  p[-1] = 0;
  strcpy(line, packed);
  return(line);
}

static size_t base94_decode(const char *buffer)
{
  size_t size = 0;
  int i;
  for (i = 4; i >= 0; i--)
  {
    size *= 94;
    size += (size_t)(buffer[i]-' ');
  }
  return(size);
}

static char *get_token(char *str, char *delim)
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
        if ((*pos == '\\"') || (*pos == '\\''))
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
        *pos++ = '\\0';
      }
    }
  }
  return(token);
}

static char *char_array_unpack(char *str)
{
  char packed[LINE_LENGTH];
  char *p = packed, *token;
  size_t len = 0;
  memset(packed, 0, sizeof(packed));
  for (token = get_token(str, "\\\\"); token; token = get_token(0, "\\\\"))
  {
    if (*token == '0')
    {
      size_t i = base94_decode(decode_string(token+1));
      len += i;
      if (len > sizeof(packed)) { break; }
      memset(p, 0, i);
      p += i;
    }
    else
    {
      char *decoded = decode_string(token);
      size_t decoded_length = strlen(decoded);
      len += decoded_length;
      if (len > sizeof(packed))
      {
        memcpy(p, decoded, sizeof(packed)-(len-decoded_length));
        break;
      }
      memcpy(p, decoded, decoded_length);
      p += decoded_length;
    }
  }
  memcpy(line, packed, sizeof(packed));
  return(line);
}
""")

    def write_io_functions(self, operation: str):
        if operation == "write":
            self.write_write_functions()
        elif operation == "read":
            self.write_read_functions()

    def write_write_functions(self):
        self.write_packed_functions()

        for ctype in [CType.UC, CType.US, CType.UD, CType.SC, CType.SS, CType.SD]:
            self.write_array_function(ctype, "write")

        self.c_stream.write(f"""
static void write_{family_name}_vars_internal(void *fp, int (*outf)(void *, const char *, ...))
{{
""")

        for elem in config_data.data_array:
            self._write_element(elem)

        if "PSR_HASH" in defines:
            self.c_stream.write(f"""  outf(fp, "\\n\\n\\n;Do not modify the following, for internal use only.\\n");
  outf(fp, "PSR_HASH=%u\\n", PSR_HASH);
""")

        self.c_stream.write(f"""}}

unsigned char write_{family_name}_vars(const char *file)
{{
  FILE *fp = 0;

""")

        if "PSR_EXTERN" not in defines:
            self.c_stream.write(f"  init_{family_name}_vars();\n\n")

        self._write_file_io("write", "w", "fprintf")

        if "PSR_COMPRESSED" in defines:
            self._write_compressed_io("write")

        if "PSR_MEMCPY" in defines:
            self._write_memcpy_functions()

    def write_read_functions(self):
        for ctype in [CType.UC, CType.US, CType.UD, CType.SC, CType.SS, CType.SD]:
            self.write_array_function(ctype, "read")

        self.c_stream.write(f"""
static void read_{family_name}_vars_internal(void *fp, char *(*fin)(char *, int, void *), int (*fend)(void *))
{{
  while (!fend(fp))
  {{
    char *p, *var, *value;

    fin(line, LINE_LENGTH, fp);
    if ((p = find_str(line, ";"))) {{ *p = 0; }}
    if ((p = strchr(line, '=')))
    {{
      *p = 0;
      var = line;
      value = p+1;
      while (isspace(*var)) {{ var++; }}
      while (isspace(*value)) {{ value++; }}
      if ((p = find_str(var, " \\t\\r\\n"))) {{ *p = 0; }}
      if ((p = find_str(value, " \\t\\r\\n"))) {{ *p = 0; }}
      if (!*var || !*value) {{ continue; }}
""")

        if dependancies:
            self._write_dependancy_checks()

        self.c_stream.write("""    }
    else
    {
      continue;
    }

""")

        for elem in config_data.data_array:
            self._read_element(elem)

        if "PSR_HASH" in defines:
            self._write_hash_check()

        self.c_stream.write("  }\n")

        if "PSR_HASH" in defines:
            self._write_hash_validation()

        self.c_stream.write(f"""}}

unsigned char read_{family_name}_vars(const char *file)
{{
  FILE *fp = 0;

""")

        if "PSR_EXTERN" not in defines:
            self.c_stream.write(f"  init_{family_name}_vars();\n\n")

        self._write_file_io("read", "r", "fgets", "feof")

        if "PSR_COMPRESSED" in defines:
            self._write_compressed_io("read")

    def _write_element(self, elem: ConfigDataElement):
        dependancy_prefix = dependancy_suffix = ""
        if elem.dependancy:
            dependancy_prefix = f"if ({elem.dependancy[:-1]}) {{ "
            dependancy_suffix = " }"

        if elem.format == StorageFormat.NONE:
            if elem.comment:
                self.c_stream.write(f'  outf(fp, ";%s\\n", {encode_string(elem.comment)});\n')
            else:
                self.c_stream.write('  outf(fp, "\\n");\n')
        elif elem.format in [StorageFormat.MULT, StorageFormat.PTR]:
            info = TYPE_INFO[elem.type]
            comment_str = encode_string(elem.comment) if elem.comment else "0"
            self.c_stream.write(f'  {dependancy_prefix}write_{info.underscore}_array(outf, fp, "{elem.dependancy}{elem.name}", {elem.name}, {elem.length}, {comment_str});{dependancy_suffix}\n')
        else:
            config_comment = f" ;{encode_string(elem.comment, False)}" if elem.comment else ""
            self.c_stream.write(f'  {dependancy_prefix}outf(fp, "{elem.dependancy}{elem.name}=')
            if elem.format == StorageFormat.SINGLE:
                info = TYPE_INFO[elem.type]
                self.c_stream.write(f'%{info.format_char}{config_comment}\\n", {elem.name}')
            elif elem.format == StorageFormat.QUOTED:
                self.c_stream.write(f'%s{config_comment}\\n", encode_string({elem.name})')
            elif elem.format == StorageFormat.MULT_PACKED:
                self.c_stream.write(f'%s{config_comment}\\n", char_array_pack((char *){elem.name}, {elem.length})')
            self.c_stream.write(f');{dependancy_suffix}\n')

    def _read_element(self, elem: ConfigDataElement):
        if elem.format == StorageFormat.NONE:
            return

        self.c_stream.write(f'    if (!strcmp(var, "{elem.dependancy}{elem.name}")) {{ ')
        if elem.format == StorageFormat.SINGLE:
            info = TYPE_INFO[elem.type]
            atoi_func = "atoi" if info.signed else "atoui"
            self.c_stream.write(f'{elem.name} = ({info.space}){atoi_func}(value);')
        elif elem.format in [StorageFormat.MULT, StorageFormat.PTR]:
            info = TYPE_INFO[elem.type]
            self.c_stream.write(f'read_{info.underscore}_array(value, {elem.name}, {elem.length});')
        elif elem.format == StorageFormat.QUOTED:
            self.c_stream.write(f'*{elem.name} = 0; strncat({elem.name}, decode_string(value), sizeof({elem.name})-1);')
        elif elem.format == StorageFormat.MULT_PACKED:
            self.c_stream.write(f'memcpy({elem.name}, char_array_unpack(value), {elem.length});')
        self.c_stream.write(' continue; }\n')

    def _write_dependancy_checks(self):
        self.c_stream.write("""      if ((p = strchr(var, ':')))
      {
        if (!strlen(p+1)) { continue; }
""")
        deps = list(dependancies)
        self.c_stream.write(f'        if (!strncmp(var, "{deps[0]}:", (p-var)+1)) {{ if (!{deps[0]}) {{ continue; }} }}\n')
        for dep in deps[1:]:
            self.c_stream.write(f'        else if (!strncmp(var, "{dep}:", (p-var)+1)) {{ if (!{dep}) {{ continue; }} }}\n')
        self.c_stream.write("""        else { continue; }
      }
""")

    def _write_hash_check(self):
        self.c_stream.write("""    if (!strcmp(var, "PSR_HASH"))
    {
       if ((unsigned int)atoui(value) == PSR_HASH)
       {
         psr_init_done = 2;
         continue;
       }
       break;
    }
""")

    def _write_hash_validation(self):
        self.c_stream.write(f"""  if (psr_init_done == 2)
  {{
    psr_init_done = 1;
  }}
  else
  {{
    psr_init_done = 0;
    init_{family_name}_vars();
  }}
""")

    def _write_file_io(self, operation: str, mode: str, func: str, end_func: str = None):
        self.c_stream.write(f"""  if ((fp = fopen(file, "{mode}")))
  {{
    {operation}_{family_name}_vars_internal(fp, (""")

        if operation == "write":
            self.c_stream.write(f"int (*)(void *, const char *, ...)){func}")
        else:
            self.c_stream.write(f"char *(*)(char *, int, void *)){func}, (int (*)(void *)){end_func}")

        self.c_stream.write(""");
    fclose(fp);
""")

        if "PSR_NOUPDATE" not in defines and operation == "read":
            self.c_stream.write(f"    write_{family_name}_vars(file);\n")

        self.c_stream.write("""    return(1);
  }

""")

        if "PSR_NOUPDATE" not in defines and operation == "read":
            self.c_stream.write(f"  write_{family_name}_vars(file);\n")

        self.c_stream.write("  return(0);\n}\n")

    def _write_compressed_io(self, operation: str):
        self.c_stream.write(f"""
{"static char *gzgets_fix(char *buf, int len, void *file)\n{\n  return(gzgets(file, buf, len));\n}\n" if operation == "read" else ""}
unsigned char {operation}_{family_name}_vars_compressed(const char *file)
{{
  gzFile gzfp;

""")
        if "PSR_EXTERN" not in defines:
            self.c_stream.write(f"  init_{family_name}_vars();\n\n")

        mode = "rb" if operation == "read" else "wb9"
        func = "gzgets_fix, gzeof" if operation == "read" else "gzprintf"

        self.c_stream.write(f"""  if ((gzfp = gzopen(file, "{mode}")))
  {{
    {operation}_{family_name}_vars_internal(gzfp, {func});
    gzclose(gzfp);
""")
        if "PSR_NOUPDATE" not in defines and operation == "read":
            self.c_stream.write(f"    write_{family_name}_vars_compressed(file);\n")
        self.c_stream.write("""    return(1);
  }

""")
        if "PSR_NOUPDATE" not in defines and operation == "read":
            self.c_stream.write(f"  write_{family_name}_vars_compressed(file);\n")
        self.c_stream.write("  return(0);\n}\n")

    def _write_memcpy_functions(self):
        self.c_stream.write(f"""
static unsigned int {family_name}_vars_memory(unsigned char *buffer, void *(*cpy)(void *, void *, size_t))
{{
  unsigned char *p = buffer;
""")
        for elem in config_data.data_array:
            dependancy_prefix = dependancy_suffix = ""
            if elem.dependancy:
                dependancy_prefix = f"if ({elem.dependancy[:-1]}) {{ "
                dependancy_suffix = " }"

            if elem.format == StorageFormat.PTR:
                info = TYPE_INFO[elem.type]
                self.c_stream.write(f'  {dependancy_prefix}cpy(p, {elem.name}, sizeof({info.space})*{elem.length}); p += sizeof({info.space})*{elem.length};{dependancy_suffix}\n')
            elif elem.format != StorageFormat.NONE:
                prefix = "&" if elem.format == StorageFormat.SINGLE else ""
                self.c_stream.write(f'  {dependancy_prefix}cpy(p, {prefix}{elem.name}, sizeof({elem.name})); p += sizeof({elem.name});{dependancy_suffix}\n')

        self.c_stream.write(f"""  return(p-buffer);
}}

static void *cpynull(void *l, void *r, size_t len){{ return(0); }}

unsigned int size_{family_name}_vars_memory()
{{
  return({family_name}_vars_memory(0, cpynull));
}}

void write_{family_name}_vars_memory(unsigned char *buffer)
{{
  {family_name}_vars_memory(buffer, (void *(*)(void *, void *, size_t))memcpy);
}}

static void *cpyright(void *src, void *dest, size_t len)
{{
  memcpy(dest, src, len);
  return(0);
}}

void read_{family_name}_vars_memory(unsigned char *buffer)
{{
  {family_name}_vars_memory(buffer, cpyright);
}}
""")

def output_cheader_start(cheader_stream: TextIO):
    cheader_stream.write(f"""/*
Config file handler header generated by Nach's Config file handler creator.
*/

#ifdef __cplusplus
  extern "C" {{
#endif

unsigned char read_{family_name}_vars(const char *);
unsigned char write_{family_name}_vars(const char *);
""")

    if "PSR_COMPRESSED" in defines:
        cheader_stream.write(f"""unsigned char read_{family_name}_vars_compressed(const char *);
unsigned char write_{family_name}_vars_compressed(const char *);
""")

    if "PSR_MEMCPY" in defines:
        cheader_stream.write(f"""void read_{family_name}_vars_memory(unsigned char *);
void write_{family_name}_vars_memory(unsigned char *);
unsigned int size_{family_name}_vars_memory();
""")

    cheader_stream.write("\n")

def output_cheader_end(cheader_stream: TextIO):
    cheader_stream.write("""
#ifdef __cplusplus
             }
#endif

""")

def handle_directive(instruction: str, label: Optional[str]):
    global ifs

    if instruction.lower() == "define":
        if label:
            defines.add(label)
        else:
            current_location.error("Could not get define label")
    elif instruction.lower() == "undef":
        if label:
            defines.discard(label)
        else:
            current_location.error("Could not get undefine label")
    elif instruction.lower() == "ifdef":
        if label:
            ifs.append(label in defines)
        else:
            current_location.error("Could not get ifdef label")
    elif instruction.lower() == "ifndef":
        if label:
            ifs.append(label not in defines)
        else:
            current_location.error("Could not get ifndef label")
    elif instruction.lower() == "else":
        if label:
            current_location.error("Processor directive else does not accept labels")
        else:
            if not ifs:
                current_location.error("Processor directive else without ifdef")
            else:
                process = not ifs.pop()
                ifs.append(process)
    elif instruction.lower() in ["elifdef", "elseifdef"]:
        if label:
            if ifs and ifs[-1]:
                ifs.pop()
                ifs.append(False)
            elif label in defines:
                ifs.pop()
                ifs.append(True)
        else:
            current_location.error("Could not get elseifdef label")
    elif instruction.lower() == "endif":
        if label:
            current_location.error("Processor directive endif does not accept labels")
        else:
            if not ifs:
                current_location.error("Processor directive endif without ifdef")
            else:
                ifs.pop()
    else:
        current_location.error("Unknown processor directive")

def get_comment(line: str, comment_separator: str) -> Tuple[str, Optional[str]]:
    pos = find_chr(line, comment_separator)
    if pos >= 0:
        comment = line[pos+1:].strip()
        if comment and comment[-1].isspace():
            comment = comment.rstrip()
        line = line[:pos]
        return line, comment
    return line, None

def output_header_conditional(hvars_lines: List[str], instruction: str, label: Optional[str]):
    if instruction.lower() in ["elifdef", "elseifdef"] and label:
        hvars_lines.append(f"#elif defined({label})")
    else:
        line = f"#{instruction}"
        if label:
            line += f" {label}"
        hvars_lines.append(line)

def parser_generate(psr_stream: TextIO, c_stream: TextIO, cheader_stream: Optional[TextIO], cheader_file: str = ""):
    global current_location
    current_location.line_number = 0
    current_location.column_number = 0

    cvars_lines = []
    hvars_lines = []
    psr_file_hash = zlib.crc32(b"")

    for line in psr_stream:
        line = line.rstrip('\n\r')
        current_location.line_number += 1
        psr_file_hash = zlib.crc32(line.encode('utf-8'), psr_file_hash)

        line, parser_comment = get_comment(line, ';')

        if all_spaces(line):
            if all_true(ifs):
                comment_line = f" //{parser_comment}" if parser_comment else ""
                cvars_lines.append(comment_line)
            continue

        line, config_comment = get_comment(line, '@')

        if all_spaces(line) and config_comment:
            if all_true(ifs):
                config_data.add_comment(config_comment)
            continue

        tokens = get_token(line, " ,")
        if not tokens:
            continue

        token = tokens[0]
        if token.upper() == "NEWSYM" and len(tokens) > 1:
            token = tokens[1]
            tokens = tokens[1:]

        if token.startswith('#') or token.startswith('%'):
            next_token = tokens[1] if len(tokens) > 1 else None
            handle_directive(token[1:], next_token)

            if cheader_stream and (not next_token or not next_token.upper().startswith("PSR_")):
                output_header_conditional(hvars_lines, token[1:], next_token)
            continue

        varname = ""
        dependancy = ""

        if ':' in token:
            parts = token.split(':', 1)
            dependancy = parts[0]
            varname = parts[1]
            dependancies.add(dependancy)
            dependancy += ':'
        else:
            varname = token

        if len(tokens) < 2:
            current_location.error("Could not get type")
            continue

        var_init = parse_variable_declaration(tokens, varname, dependancy, config_comment)
        if not var_init:
            continue

        if all_true(ifs):
            comment_suffix = f" //{parser_comment}" if parser_comment else ""
            cvars_lines.append(var_init + comment_suffix)

        if cheader_stream:
            header_data = var_init
            if "=" in header_data:
                header_data = header_data.split("=")[0].strip() + ";"
            hvars_lines.append(f"extern {header_data}")

    # Generate output files
    generator = CodeGenerator(c_stream, cheader_file)
    generator.write_includes()
    generator.write_string_functions()
    generator.write_utility_functions()
    generator.write_dependancies()

    if "PSR_EXTERN" not in defines:
        for line in cvars_lines:
            c_stream.write(line + "\n")
        generator.write_init_function()
    elif not cheader_file:
        print("Error: Requested PSR_EXTERN yet no header file specified.", file=sys.stderr)

    if "PSR_HASH" in defines:
        c_stream.write(f"static unsigned int PSR_HASH = 0x{psr_file_hash & 0xffffffff:x};\n")

    generator.write_io_functions("write")
    generator.write_io_functions("read")
    c_stream.write("\n")

    if cheader_stream:
        output_cheader_start(cheader_stream)
        for line in hvars_lines:
            cheader_stream.write(line + "\n")
        output_cheader_end(cheader_stream)

    if ifs:
        print(f"Error: {len(ifs)} ifdef segments have no endif.", file=sys.stderr)

def parse_variable_declaration(tokens: List[str], varname: str, dependancy: str, config_comment: Optional[str]) -> Optional[str]:
    array = 0
    is_array = False
    is_packed = False
    is_ptr = False

    if tokens[1].upper() == "TIMES":
        is_array = True
        if len(tokens) < 4:
            current_location.error("Could not get array size")
            return None
        array = enhanced_atoi(tokens[2])
        asm_type = tokens[3]
        initial_value = tokens[4] if len(tokens) > 4 else "0"
    elif tokens[1].upper() == "PACKED":
        is_packed = True
        if len(tokens) < 4:
            current_location.error("Could not get array size")
            return None
        array = enhanced_atoi(tokens[2])
        asm_type = tokens[3]
        initial_value = tokens[4] if len(tokens) > 4 else "0"
    elif tokens[1].upper() == "PTR":
        is_ptr = True
        if len(tokens) < 4:
            current_location.error("Could not get array size")
            return None
        array = enhanced_atoi(tokens[2])
        asm_type = tokens[3]
        initial_value = tokens[4] if len(tokens) > 4 else "0"
    else:
        asm_type = tokens[1]
        initial_value = tokens[2] if len(tokens) > 2 else "0"

    var_type = convert_asm_type(asm_type)
    if not var_type:
        return None

    if is_string_literal(initial_value):
        return handle_string_variable(varname, dependancy, config_comment, initial_value, array)
    elif is_ptr:
        return handle_pointer_variable(varname, dependancy, config_comment, var_type, array)
    else:
        return handle_numeric_variable(tokens, varname, dependancy, config_comment, var_type,
                                     initial_value, array, is_array, is_packed)

def is_string_literal(value: str) -> bool:
    return ((value.startswith('"') and value.endswith('"')) or
            (value.startswith("'") and value.endswith("'")))

def handle_string_variable(varname: str, dependancy: str, config_comment: Optional[str],
                          initial_value: str, array: int) -> str:
    initial_value = f'"{initial_value[1:-1]}"'

    if not array:
        array = len(initial_value) - 1

    if all_true(ifs):
        if len(initial_value) - 2 < array:
            memset_line = f"strcpy({varname}, {initial_value});"
        else:
            memset_line = f"strncpy({varname}, {initial_value}, {array-1}); {varname}[{array}] = 0;"
        memsets.append(memset_line)
        config_data.add_var_quoted(varname, dependancy, config_comment or "")

    return f"char {varname}[{array}];"

def handle_pointer_variable(varname: str, dependancy: str, config_comment: Optional[str],
                           var_type: str, array: int) -> str:
    if all_true(ifs):
        config_data.add_var_ptr(varname, var_type, array, dependancy, config_comment or "")

    return f"{var_type} *{varname};"

def handle_numeric_variable(tokens: List[str], varname: str, dependancy: str, config_comment: Optional[str],
                           var_type: str, initial_value: str, array: int, is_array: bool, is_packed: bool) -> str:
    init_value_num = safe_atoi(c_hex_convert(asm2c_hex_convert(initial_value)))

    if init_value_num < 0 and var_type.startswith("unsigned "):
        var_type = var_type[9:]
        if var_type == "char":
            var_type = "signed char"

    var_init = f"{var_type} {varname}"

    if array:
        var_init += handle_array_initialization(varname, dependancy, config_comment, var_type,
                                               init_value_num, array, is_array, is_packed)
    else:
        var_init += handle_scalar_initialization(tokens, varname, dependancy, config_comment,
                                                var_type, init_value_num)

    return var_init + ";"

def handle_array_initialization(varname: str, dependancy: str, config_comment: Optional[str],
                               var_type: str, init_value_num: int, array: int, is_array: bool, is_packed: bool) -> str:
    var_type_is_char = var_type.endswith("char")
    var_type_is_short = var_type.endswith("short")
    var_type_is_int = var_type.endswith("int")

    if var_type_is_char or init_value_num == 0:
        var_init = f"[{array}]"

        if all_true(ifs):
            scale = ""
            if var_type_is_short:
                scale = "*sizeof(short)"
            elif var_type_is_int:
                scale = "*sizeof(int)"

            memset_line = f"memset({varname}, {init_value_num}, {array}{scale});"
            memsets.append(memset_line)
    else:
        var_init = f"[{array}] = {{{', '.join([str(init_value_num)] * array)}}}"

    if all_true(ifs):
        if is_array:
            config_data.add_var_mult(varname, var_type, array, dependancy, config_comment or "")
        elif is_packed:
            config_data.add_var_packed(varname, array, dependancy, config_comment or "")

    return var_init

def handle_scalar_initialization(tokens: List[str], varname: str, dependancy: str, config_comment: Optional[str],
                                var_type: str, init_value_num: int) -> str:
    remaining_tokens = tokens[3:] if len(tokens) > 3 else []
    if remaining_tokens:
        array = 1 + len(remaining_tokens)
        values = [str(init_value_num)] + [str(safe_atoi(c_hex_convert(asm2c_hex_convert(t)))) for t in remaining_tokens]
        var_init = f"[] = {{{', '.join(values)}}}"

        if all_true(ifs):
            config_data.add_var_mult(varname, var_type, array, dependancy, config_comment or "")
    else:
        var_init = f" = {init_value_num}"

        if all_true(ifs):
            config_data.add_var_single(varname, var_type, dependancy, config_comment or "")

    return var_init

def main():
    global gcc, cflags, family_name

    cheader_file = None
    compile_flag = False
    param_pos = 1

    while param_pos < len(sys.argv):
        arg = sys.argv[param_pos]
        if arg.startswith("-D"):
            defines.add(arg[2:])
        elif arg == "-cheader":
            param_pos += 1
            if param_pos >= len(sys.argv):
                print("Error: -cheader requires an argument", file=sys.stderr)
                return 1
            cheader_file = sys.argv[param_pos]
        elif arg == "-compile":
            compile_flag = True
        elif arg == "-flags":
            param_pos += 1
            if param_pos >= len(sys.argv):
                print("Error: -flags requires an argument", file=sys.stderr)
                return 1
            cflags = sys.argv[param_pos]
        elif arg == "-fname":
            param_pos += 1
            if param_pos >= len(sys.argv):
                print("Error: -fname requires an argument", file=sys.stderr)
                return 1
            family_name = sys.argv[param_pos]
        elif arg == "-gcc":
            param_pos += 1
            if param_pos >= len(sys.argv):
                print("Error: -gcc requires an argument", file=sys.stderr)
                return 1
            gcc = sys.argv[param_pos]
        else:
            break
        param_pos += 1

    if len(sys.argv) - param_pos != 2:
        print("""Config file handler creator by Nach (C) 2005-2007

Usage:
parsegen [options] <output> <input>


Options:

  -Ddefine   Define a processor director. Example: -D__WIN32__
             Can specify multiple defines.

  -cheader   Create a C/C++ header with the following name.
             Example: -cheader cfgvars.h

  -fname     Use the following name for the main functions.
             Example: -fname math
             Would make init_cfg_vars become init_math_vars this
             also happens to write_cfg_vars and read_cfg_vars.

  -compile   Compiles output instead of outputting C file.

  -gcc       Use with -compile. Parameter passed in the name of
             the C compiler to use, it should be GCC based.
             It will not work with MSVC based compilers.

  -flags     Use with -compile. Flags passed as next parameter
             are passed to the C compiler.
             Example: -flags "-O3 -march=pentium3 -ggdb3"

""", file=sys.stderr)
        return 1

    cname = f"{family_name}.c"
    psr_file = sys.argv[param_pos + 1]
    c_file = cname if compile_flag else sys.argv[param_pos]
    obj_file = sys.argv[param_pos] if compile_flag else None
    ret_val = 0

    try:
        with open(psr_file, 'r') as psr_stream:
            with open(c_file, 'w') as c_stream:
                cheader_stream = None
                if cheader_file:
                    try:
                        cheader_stream = open(cheader_file, 'w')
                        parser_generate(psr_stream, c_stream, cheader_stream, cheader_file)
                    except IOError:
                        print(f"Error opening {cheader_file} for writing.", file=sys.stderr)
                        ret_val |= 8
                    finally:
                        if cheader_stream:
                            cheader_stream.close()
                else:
                    parser_generate(psr_stream, c_stream, cheader_stream)
    except IOError:
        if not os.path.exists(psr_file):
            print(f"Error opening {psr_file} for reading.", file=sys.stderr)
            ret_val |= 4
        else:
            print(f"Error opening {c_file} for writing.", file=sys.stderr)
            ret_val |= 2

    if not ret_val and compile_flag:
        command = f"{gcc} {cflags} -o {obj_file} -c {cname}"
        print(f"parsegen: {command}")
        subprocess.call(command, shell=True)
        try:
            os.remove(cname)
        except OSError:
            pass

    return ret_val

if __name__ == "__main__":
    sys.exit(main())
