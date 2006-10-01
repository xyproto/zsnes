#ifndef ARGV_H
#define ARGV_H

static char *decode_string(char *str)
{
  size_t str_len = strlen(str), i = 0;
  char *dest = str;

  if ((str_len > 1) && ((*str == '\"') || (*str == '\'')) && (str[str_len-1] == *str))
  {
    memmove(str, str+1, str_len-2);
    str[str_len-2] = 0;
  }

  while (*str)
  {
    if (*str == '\\')
    {
      str++;
    }
    dest[i++] = *str++;
  }
  dest[i] = 0;
  return(dest);
}

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

static char *get_param(char *str)
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
    while (*pos == ' ') { pos++; }
    if (*pos)
    {
      token = pos;

      //Skip non-delimiters
      while (*pos && (*pos != ' '))
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
        //Skip escaped spaces
        if (*pos == '\\') { pos++; }
        pos++;
      }
      if (*pos) { *pos++ = '\0'; }
    }
  }
  return(token);
}

static size_t count_param(char *str)
{
  size_t i = 0;

  while (*str)
  {
    //Skip delimiters
    while (*str == ' ') { str++; }
    //Skip non-delimiters
    while (*str && (*str != ' '))
    {
      //Skip quoted characters
      if ((*str == '\"') || (*str == '\''))
      {
        char *match_str = 0;
        if ((match_str = find_next_match(str+1, *str)))
        {
          str = match_str;
        }
      }
      //Skip escaped spaces
      if (*str == '\\') { str++; }
      str++;
    }
    i++;
  }
  return(i);
}

static char **build_argv(char *str)
{
  size_t argc = count_param(str);
  char **argv = (char **)malloc(sizeof(char *)*(argc+1));

  if (argv)
  {
    char *p, **argp = argv;
    for (p = get_param(str); p; p = get_param(0), argp++)
    {
      *argp = decode_string(p);
    }
    *argp = 0;
    return(argv);
  }
  return(0);
}

/*
static void argv_print(char **argv)
{
  char **argp = argv;
  while (*argp)
  {
    printf("argv[%u]: %s\n", argp-argv, *argp);
    argp++;
  }
  printf("argv[%u]: NULL\n", argp-argv);
}
*/

#endif
