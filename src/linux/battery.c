/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

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

#include "../gblhdr.h"




/*
Functions for battery probing on Linux by Nach
I believe Linux 2.4.x+ is needed for ACPI support
but it'll compile fine for older versions too

Special thanks David Lee Lambert for most of the code here
*/

#ifdef linux
#include <dirent.h>
#include <math.h>

int CheckBattery()
{
  int battery = -1; //No battery / Can't get info
  const char *ac = "/proc/acpi/ac_adapter/";

  //Check ac adapter
  DIR *ac_dir = opendir(ac);
  if (ac_dir)
  {
    char fnbuf[40]; // longer than len(ac)+len(HEXDIGIT*4)+len({state|info})
    FILE *fp;
    const char *pattern = " %39[^:]: %39[ -~]"; // for sscanf
    char line[80], key[40], arg[40];

    struct dirent *ent;
    while ((ent = readdir(ac_dir)))
    {
      if (ent->d_name[0] == '.') { continue; }

      snprintf(fnbuf, 40, "%s%s/state", ac, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        while (fgets(line, 80, fp) && sscanf(line, pattern, key, arg) == 2)
        {
          if (!strcmp(key, "state"))
          {
            if (!strcmp(arg, "on-line"))
            {
              battery = 0;
            }
            else if (!strcmp(arg, "off-line"))
            {
              battery = 1;
              break;
            }
          }
        }
        fclose(fp);
      }
    }
    closedir(ac_dir);
  }
  return(battery);
}

static int BatteryLifeTime;
static int BatteryLifePercent;

static void update_battery_info()
{
  const char *batt = "/proc/acpi/battery/";

  //Check batteries
  DIR *batt_dir = opendir(batt);
  if (batt_dir)
  {
    char fnbuf[40]; // longer than len(ac)+len(HEXDIGIT*4)+len({state|info})
    FILE *fp;
    const char *pattern = " %39[^:]: %39[ -~]"; // for sscanf
    char line[80], key[40], arg[40];

    float x, design_capacity = 0.0f, remaining_capacity = 0.0f, present_rate = 0.0f, full_capacity = 0.0f;

    struct dirent *ent;
    while ((ent = readdir(batt_dir)))
    {
      if (ent->d_name[0] == '.') { continue; }
      snprintf(fnbuf, 40, "%s%s/info", batt, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        while (fgets(line, 80, fp) && sscanf(line, pattern, key, arg) == 2)
        {
          if (!strcmp(key, "design capacity") && sscanf(arg, "%g", &x) == 1)
          {
            design_capacity += x;
          }
          else if (!strcmp(key, "last full capacity") && sscanf(arg, "%g", &x) == 1)
          {
            full_capacity += x;
          }
        }
        fclose(fp);
      }
      snprintf(fnbuf, 40, "%s%s/state", batt, ent->d_name);
      fp = fopen(fnbuf, "r");
      if (fp)
      {
        int charging = 0;
        while (fgets(line, 80, fp) && sscanf(line, pattern, key, arg) == 2)
        {
          if (!strcmp(key, "charging state"))
          {
            if (!strcmp(arg, "discharging"))
            {
              charging = -1;
            }
            else if (!strcmp(arg, "charging"))
            {
              charging = 1;
            }
          }
          else if (!strcmp(key, "present rate") && sscanf(arg, "%g", &x) == 1)
          {
            present_rate += charging * x;
            charging = 0;
          }
          else if (!strcmp(key, "remaining capacity") && sscanf(arg, "%g:", &x) == 1)
          {
            remaining_capacity += x;
            charging = 0;
          }
        }
        fclose(fp);
      }
    }
    if (design_capacity > 0.0f)
    {
      BatteryLifePercent = (int)floorf(remaining_capacity / ((full_capacity > 0.0f) ? full_capacity : design_capacity) * 100.0);
      if (BatteryLifePercent > 100) { BatteryLifePercent = 100; }
      if (present_rate < 0.0f)
      {
        // Linux specifies rates in mWh or mAh
        BatteryLifeTime = (int)floorf(remaining_capacity / (-present_rate) * 3600.0);
      }
    }
    closedir(batt_dir);
  }
}

int CheckBatteryTime()
{
  BatteryLifeTime = -1;
  update_battery_info();
  return(BatteryLifeTime);
}

int CheckBatteryPercent()
{
  BatteryLifePercent = -1;
  update_battery_info();
  return(BatteryLifePercent);
}

/*
Functions for battery on FreeBSD/DragonFly by Nach

If there's another FreeBSD based OS that doesn't
define one of these three, please let me know.
*/
#elif (defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__))
#include <sys/types.h>
#include <sys/sysctl.h>

int CheckBattery()
{
  int state;
  size_t state_len = sizeof(state);
  if (!sysctlbyname("hw.acpi.battery.state", &state, &state_len, 0, 0))
  {
    if ((state > -1) && (state < 7)) //7 == failure
    {
      if (!state || state&2)
      {
        return(0); //Plugged in
      }
      return(1); //Running off of battery
    }
  }
  return(-1);
}

//Note that I have not yet gotten anyone to test if this function has correct info returned
int CheckBatteryTime()
{
  int batt_time;
  size_t batt_time_len = sizeof(batt_time);
  if (!sysctlbyname("hw.acpi.battery.time", &batt_time, &batt_time_len, 0, 0))
  {
    if (batt_time > -1)
    {
      return(batt_time * 60);
    }
  }
  return(-1);
}

int CheckBatteryPercent()
{
  int life = -1;
  size_t life_len = sizeof(life);
  sysctlbyname("hw.acpi.battery.life", &life, &life_len, 0, 0);
  return(life);
}

/*
Functions for battery on NetBSD/OpenBSD by Nach

If there's another NetBSD based OS that uses
the same API, please let me know.

Note this was the least tested section for all
the battery specific code.
*/

#elif (defined(__NetBSD__) || defined(__OpenBSD__))
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <machine/apmvar.h>

#ifndef APM_BATT_ABSENT
#define APM_BATT_ABSENT APM_BATTERY_ABSENT
#endif

int CheckBattery()
{
  int fd = open("/dev/apm", O_RDONLY);
  if (fd != -1)
  {
    struct apm_power_info info;
    if (!ioctl(fd, APM_IOC_GETPOWER, &info) &&
        (info.battery_state != APM_BATT_UNKNOWN) && (info.battery_state != APM_BATT_ABSENT))
    {
      close(fd);
      if ((info.battery_state == APM_BATT_CHARGING) || (info.ac_state == APM_AC_ON)) { return(0); } //Plugged in
      return(1); //Running off of battery
    }
    close(fd);
  }
  return(-1);
}

int CheckBatteryTime()
{
  int fd = open("/dev/apm", O_RDONLY);
  if (fd != -1)
  {
    struct apm_power_info info;
    if (!ioctl(fd, APM_IOC_GETPOWER, &info) && (info.minutes_left > 0) && (info.minutes_left < 0xFFFF))
    {
      close(fd);
      return(info.minutes_left*60);
    }
    close(fd);
  }
  return(-1);
}

int CheckBatteryPercent()
{
  int fd = open("/dev/apm", O_RDONLY);
  if (fd != -1)
  {
    struct apm_power_info info;
    if (!ioctl(fd, APM_IOC_GETPOWER, &info))
    {
      close(fd);
      return((info.battery_life == 255) ? 100 : info.battery_life);
    }
    close(fd);
  }
  return(-1);
}

/*
Functions for battery on Mac OS X by drizztbsd, Nach

If you have issues, please report.
*/

#elif defined(__APPLE__)
#include <sys/types.h>
#include <math.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSkeys.h>

static int stringsAreEqual(CFStringRef a, CFStringRef b)
{
  if (!a || !b)
  {
    return(0);
  }
  return(CFStringCompare(a, b, 0) == kCFCompareEqualTo);
}

static int BatteryLifeTime;
static int BatteryLifePercent;
static int HasBattery;

static void update_battery_info()
{
  CFTypeRef    powerBlob = IOPSCopyPowerSourcesInfo();
  CFArrayRef   powerSourcesList = IOPSCopyPowerSourcesList(powerBlob);
  unsigned int count = CFArrayGetCount(powerSourcesList);
  unsigned int i;
  unsigned int tmp;
  char         ret;

  int totalCurrentCapacity = 0, totalMaxCapacity = 0;

  BatteryLifeTime = -1;
  BatteryLifePercent = -1;
  HasBattery = -1;

  for (i = 0; i < count; ++i)
  {
    CFTypeRef       powerSource;
    CFDictionaryRef description;

    powerSource = CFArrayGetValueAtIndex(powerSourcesList, i);
    description = IOPSGetPowerSourceDescription(powerBlob, powerSource);

    //continue if one battery is not present
    if (CFDictionaryGetValue(description, CFSTR(kIOPSIsPresentKey)) == kCFBooleanFalse)
    {
      continue;
    }

    if (stringsAreEqual(CFDictionaryGetValue(description, CFSTR(kIOPSTransportTypeKey)), CFSTR(kIOPSInternalType)))
    {
      int currentCapacity, maxCapacity;

      CFStringRef currentState = CFDictionaryGetValue(description, CFSTR(kIOPSPowerSourceStateKey));
      CFNumberRef timeToEmptyNum = CFDictionaryGetValue(description, CFSTR(kIOPSTimeToEmptyKey));

      if (CFEqual(currentState, CFSTR(kIOPSACPowerValue)) && (HasBattery != 1))
      {
        HasBattery = 0;
      }
      else if (CFEqual(currentState, CFSTR(kIOPSBatteryPowerValue)))
      {
        CFNumberRef timeToEmptyNum = CFDictionaryGetValue(description, CFSTR(kIOPSTimeToEmptyKey));
        if(CFNumberGetValue(timeToEmptyNum, kCFNumberIntType, &tmp))
        {
          if (BatteryLifeTime > -1)
          {
            BatteryLifeTime += tmp;
          }
          else
          {
            BatteryLifeTime = tmp;
          }
        }
        HasBattery = 1;
      }
      CFNumberRef currentCapacityNum = CFDictionaryGetValue(description, CFSTR(kIOPSCurrentCapacityKey));
      CFNumberRef maxCapacityNum = CFDictionaryGetValue(description, CFSTR(kIOPSMaxCapacityKey));

      if (CFNumberGetValue(currentCapacityNum, kCFNumberIntType, &currentCapacity) && CFNumberGetValue(maxCapacityNum, kCFNumberIntType, &maxCapacity))
      {
        totalCurrentCapacity += currentCapacity;
        totalMaxCapacity += maxCapacity;
      }
    }
  }

  CFRelease(powerSourcesList);
  CFRelease(powerBlob);

  if (totalCurrentCapacity && totalMaxCapacity)
  {
    BatteryLifePercent = (int)roundf((totalCurrentCapacity / (float)totalMaxCapacity) * 100.0f);
  }
}

int CheckBattery()
{
  update_battery_info();
  return(HasBattery);
}

int CheckBatteryTime()
{
  update_battery_info();
  if (BatteryLifeTime > -1)
  {
    return(BatteryLifeTime * 60);
  }
  return(-1);
}

int CheckBatteryPercent()
{
  update_battery_info();
  return(BatteryLifePercent);
}

#else //Not Linux, FreeBSD/DragonFlyBSD, NetBSD/OpenBSD, Mac OS X

int CheckBattery()
{
  return(-1);
}

int CheckBatteryTime()
{
  return(-1);
}

int CheckBatteryPercent()
{
  return(-1);
}

#endif
