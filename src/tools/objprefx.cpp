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

const char *symbol_fname = "symbol.map";

bool create_symbol_file()
{
  ofstream stream(symbol_fname, ios::out);
  if (stream)
  {
    for (set<string>::iterator i = symbols.begin(); i != symbols.end(); i++)
    {
      stream << *i << " " << prefix << *i << "\n";
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
          symbols.insert(*i);
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
       string objcopy("objcopy --redefine-syms ");
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
