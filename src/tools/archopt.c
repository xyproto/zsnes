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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _M_X64
#define __x86_64__
#endif

#ifdef __GNUC__
#ifdef __x86_64__
#define cpuid(in, a, b, c, d) asm volatile("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));
#else
#define cpuid(in, a, b, c, d) asm volatile("\
  pushl %%ebx;\
  movl %%edi,%%ebx;\
  cpuid;\
  movl %%ebx,%%edi;\
  popl %%ebx": "=a" (a), "=D" (b), "=c" (c), "=d" (d) : "a" (in));
#endif
#else
char cpubuf[256];
int z_in, z_a, z_b, z_c, z_d;
void cpuid_run()
{
  _asm {
    mov eax,z_in
    cpuid
    mov z_a,eax
    mov z_b,ebx
    mov z_c,ecx
    mov z_d,edx
  };
}
#define cpuid(in, a, b, c, d) z_in = in; cpuid_run(); a = z_a; b = z_b; c = z_c; d = z_d;
#endif

char *x86_flags[] =
{      "fpu",        "vme",        "de",         "pse",        "tsc",    "msr",      "pae",    "mce",
       "cx8",       "apic",           0,         "sep",       "mtrr",    "pge",      "mca",   "cmov",
       "pat",      "pse36",        "pn",     "clflush",            0,    "dts",     "acpi",    "mmx",
      "fxsr",        "sse",      "sse2",          "ss",         "ht",     "tm",     "ia64",    "pbe",

           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,     "syscall",            0,        0,          0,        0,
           0,            0,           0,          "mp",         "nx",        0,   "mmxext",        0,
           0,   "fxsr_opt",    "rdtscp",             0,            0,     "lm", "3dnowext",  "3dnow",

  "recovery",    "longrun",           0,        "lrti",            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,

       "pni",            0,           0,     "monitor",     "ds_cpl",    "vmx",      "smx",    "est",
       "tm2",      "ssse3",       "cid",             0,            0,   "cx16",     "xtpr",        0,
           0,            0,       "dca",             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,

           0,            0,       "rng",      "rng_en",            0,        0,      "ace", "ace_en",
      "ace2",    "ace2_en",       "phe",      "phe_en",        "pmm", "pmm_en",          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,

   "lahf_lm", "cmp_legacy",       "svm",             0,  "cr8legacy",        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0,
           0,            0,           0,             0,            0,        0,          0,        0  };

void add_flags(char *flags, unsigned int reg, unsigned int offset)
{
  unsigned int i;
  for (i = 0; i < 32; i++)
  {
    if ((reg & (1 << i)) && x86_flags[i+offset])
    {
      strcat(flags, x86_flags[i+offset]);
      strcat(flags, " ");
    }
  }
}

int have_cpuid()
{
  int have = 0x200000;
  #ifndef __x86_64__
  #ifdef __GNUC__
  asm volatile
  (
  "  pushfl;"
  "  pop %%eax;"
  "  movl %%eax,%%edx;"
  "  xorl %%ecx,%%eax;"
  "  push %%eax;"
  "  popfl;"
  "  pushfl;"
  "  pop %%eax;"
  "  xorl %%edx,%%eax;"
    : "=a" (have)
    : "c" (have)
  );
  #else
  z_c = have;
  _asm {
    mov ecx,z_c
    pushfd
    pop eax
    mov edx,eax
    xor eax,ecx
    push eax
    popfd
    pushfd
    pop eax
    xor eax,edx
    mov z_a,eax
  };
  have = z_a;
  #endif
  #endif
  return(have);
}

int main(int argc, const char *const *const argv)
{
  char model_name[216];
  char flags[216];
  char cpu_family[216];
  char vendor_id[216];
  char model[216];

  char *cpu = 0;

  FILE *fp;

  *model_name = 0;
  *cpu_family = 0;
  *vendor_id = 0;
  *model = 0;
  strcpy(flags, " ");

  if ((fp = fopen("/proc/cpuinfo", "r")))
  {
    char line[256], key[40], arg[216];
    const char *pattern = " %39[^:]: %215[ -~]"; // for sscanf

    while (fgets(line, sizeof(line), fp) && sscanf(line, pattern, key, arg) == 2)
    {
      if (!strncmp(key, "model name", strlen("model name")) && !*model_name)
      {
        strcpy(model_name, arg);
      }
      else if (!strncmp(key, "flags", strlen("flags")) && !flags[1])
      {
        strcat(flags, arg);
        strcat(flags, " ");
      }
      else if (!strncmp(key, "cpu family", strlen("cpu family")) && !*cpu_family)
      {
        strcpy(cpu_family, arg);
      }
      else if (!strncmp(key, "vendor_id", strlen("vendor_id")) && !*vendor_id)
      {
        strcpy(vendor_id, arg);
      }
      else if (!strncmp(key, "model", strlen("model")) && !*model)
      {
        strcpy(model, arg);
      }
    }
    fclose(fp);
  }
  else if (have_cpuid())
  {
    unsigned int maxei, eax, ebx, ecx, edx, unused, i;

    cpuid(0, unused, ebx, ecx, edx);
    strncat(vendor_id, (char *)&ebx, 4);
    strncat(vendor_id, (char *)&edx, 4);
    strncat(vendor_id, (char *)&ecx, 4);

    cpuid(1, eax, ebx, ecx, edx);
    sprintf(model, "%u", (eax >> 4) & 0xf);
    sprintf(cpu_family, "%u", (eax >> 8) & 0xf);
    add_flags(flags, edx, 0);
    add_flags(flags, ecx, 96);

    cpuid(0x80000000, maxei, unused, unused, unused);

    if (maxei >= 0x80000001)
    {
      cpuid(0x80000001, unused, unused, ecx, edx);
      add_flags(flags, edx, 32);
      add_flags(flags, ecx, 160);
    }

    //Transmeta
    cpuid(0x80860000, eax, unused, unused, unused);
    if (((eax & 0xffff0000) == 0x80860000) && (eax > 0x80860001))
    {
      cpuid(0x80860001, unused, unused, unused, edx);
      add_flags(flags, edx, 64);
    }

    //Centaur
    cpuid(0xC0000000, eax, unused, unused, unused);
    if (eax >= 0xC0000001)
    {
      cpuid(0xC0000001, unused, unused, unused, edx);
      add_flags(flags, edx, 128);
    }

    if (maxei >= 0x80000002)
    {
      for (i = 0x80000002; i <= 0x80000004; i++)
      {
        cpuid(i, eax, ebx, ecx, edx);
        strncat(model_name, (char *)&eax, 4);
        strncat(model_name, (char *)&ebx, 4);
        strncat(model_name, (char *)&ecx, 4);
        strncat(model_name, (char *)&edx, 4);
      }
    }
  }

  if (argc > 1)
  {
    printf("vendor_id: %s\n", vendor_id);
    printf("cpu family: %s\n", cpu_family);
    printf("model: %s\n", model);
    printf("model name: %s\n", model_name);
    printf("flags:%s\n", flags);
  }

#if __GNUC__ > 3
#if __GNUC__ > 4 || __GNUC_MINOR__ > 1
  cpu = "native";
#endif
#endif
  if (!cpu)
  {
    if (!cpu && *cpu_family && *vendor_id)
    {
      #ifdef __GNUC__
      if (!strcmp(vendor_id, "AuthenticAMD") || strstr(model_name, "AMD"))
      {
        if (strstr(flags, " mmx "))
        {
          #if __GNUC__ > 2
          if (strstr(flags, " 3dnow "))
          {
            if (strstr(flags, " 3dnowext ") && (atoi(cpu_family) > 5))
            {
              #if __GNUC__ > 3 || __GNUC_MINOR__ > 0
              if (strstr(flags, " sse "))
              {
                #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
                if (strstr(flags, " sse2 ") && strstr(flags, " lm ")) //Need two checks to protect Semprons
                {
                  if (strstr(model_name, "Opteron"))
                  {
                    cpu = "opteron";
                  }
                  else if (strstr(model_name, "Athlon(tm) 64")) //Also athlon-fx
                  {
                    cpu = "athlon64";
                  }
                  else
                  {
                    cpu = "k8";
                  }
                }
                #endif
                if (!cpu)
                {
                  if (strstr(model_name, "Athlon(tm) 4"))
                  {
                    cpu = "athlon-4";
                  }
                  else if (strstr(model_name, "Athlon(tm) MP"))
                  {
                    cpu = "athlon-mp";
                  }
                  else
                  {
                    cpu = "athlon-xp";
                  }
                }
              }

              if (!cpu && (atoi(model) > 3))
              {
                cpu = "athlon-tbird";
              }
              #endif

              if (!cpu)
              {
                cpu = "athlon";
              }
            }

            #if __GNUC__ > 3 || __GNUC_MINOR__ > 0
            if (!cpu)
            {
              int model_num = atoi(model);
              if ((model_num == 9) || (model_num >= 13))
              {
                cpu = "k6-3";
              }
              else
              {
                cpu = "k6-2";
              }
            }
            #endif
          }
          #endif

          if (!cpu)
          {
            cpu = "k6";
          }
        }
      }
      else if (!strcmp(vendor_id, "GenuineIntel") || strstr(model_name, "Intel"))
      {
        #if __GNUC__ > 2
        if (strstr(flags, " mmx "))
        {
          if (strstr(flags, " sse "))
          {
            if (strstr(flags, " sse2 "))
            {
              #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
              if (strstr(flags, " pni ") && strcmp(cpu_family, "6"))
              {
                if (strstr(flags, " lm "))
                {
                  cpu = "nocona";
                }
                else
                {
                  cpu = "prescott";
                }
              }
              #endif

              if (!cpu)
              {
                if (!strcmp(cpu_family, "6"))
                {
                  #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
                  cpu = "pentium-m";
                  #else
                  cpu = "pentium3";
                  #endif
                }
                else
                {
                  #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
                  if (strstr(model_name, "Mobile"))
                  {
                    cpu = "pentium4m";
                  }
                  #endif

                  if (!cpu)
                  {
                    cpu = "pentium4";
                  }
                }
              }
            }
            else
            {
              #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
              if (strstr(model_name, "Mobile"))
              {
                cpu = "pentium3m";
              }
              #endif

              if (!cpu)
              {
                cpu = "pentium3";
              }
            }
          }
          else
          {
            if (!strcmp(cpu_family, "6"))
            {
              cpu = "pentium2";
            }
            else
            {
              cpu = "pentium-mmx";
            }
          }
        }
        #endif

        if (!cpu)
        {
          int family = atoi(cpu_family);
          if (family > 5)
          {
            cpu = "pentiumpro";
          }
          else if (family == 5)
          {
            cpu = "pentium";
          }
        }
      }
      #if __GNUC__ > 2
      #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
      else if (!strcmp(vendor_id, "CentaurHauls") && strstr(flags, " mmx "))
      {
        if (strstr(flags, " 3dnow "))
        {
          if (atoi(cpu_family) > 5)
          {
            cpu = "c3";
          }
          else
          {
            cpu = "winchip2";
          }
        }
        #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
        else if (strstr(flags, " sse "))
        {
          cpu = "c3-2";
        }
        #endif

        if (!cpu)
        {
          cpu = "winchip-c6";
        }
      }
      #endif
      #endif

      if (!cpu)
      {
        int family = atoi(cpu_family);
        if (family > 5)
        {
          cpu = "i686";
        }
        else if (family == 5)
        {
          cpu = "i586";
        }
        else if (family == 4)
        {
          cpu = "i486";
        }
        else
        {
          cpu = "i386";
        }
      }
      #else //MSVC
      cpu = cpubuf;
      *cpu = 0;

      if (strstr(flags, " sse2 "))
      {
        strcat(cpu, " /arch:SSE2");
      }
      else if (strstr(flags, " sse "))
      {
        strcat(cpu, " /arch:SSE");
      }

      #ifdef __x86_64__
      if (strstr(flags, " lm ")) //64 bit
      {
        if (!strcmp(vendor_id, "AuthenticAMD") || strstr(model_name, "AMD"))
        {
          strcat(cpu, " /favor:AMD64");
        }
        else if (!strcmp(vendor_id, "GenuineIntel") || strstr(model_name, "Intel"))
        {
          strcat(cpu, " /favor:EM64T");
        }
      }
      #endif
      #endif
    }
    else
    {
      puts("Could not open /proc/cpuinfo, and CPUID instruction not available.");
      return(1);
    }
  }
  puts(cpu);

  return(0);
}
