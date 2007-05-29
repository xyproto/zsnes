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

#include <QApplication>
#include "load.h"
#include "ui.h"
#include "zthread.h"

static bool debugger_running = false;
static int app_exit_num = 0;
static ZSNESThread zthread;

unsigned char debugger_quit = false;

void debug_main()
{
  if (!debugger_running)
  {
    debugger_running = true;

    int argc = 1;
    char *argv[] = { "debug" };
    QApplication app(argc, argv);
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &zthread, SLOT(prepare_close()));

    QtDebugger::showQtDebugger(0);

    zthread.start();
    app.exec();
    zthread.wait();
    QtDebugger::destroyQtDebugger();
    exit(app_exit_num);
  }
}


void debug_exit(int exit_num)
{
  if (debugger_running)
  {
    app_exit_num = exit_num;
    qApp->quit();
    zthread.done();
  }
  else
  {
    exit(exit_num);
  }
}
