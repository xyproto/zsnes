#include <QApplication>
#include <QThread>
#include "load.h"
#include "ui.h"

#include <setjmp.h>

extern "C" { void zstart(); }

class ZSNESThread : public QThread
{
  jmp_buf jump;
  public:
  void run()
  {
    if (!setjmp(jump))
    {
      zstart();
    }
  }

  void done()
  {
    longjmp(jump, 1);
  }
};

static bool debugger_running = false;
static int app_exit_num = 0;
static ZSNESThread zthread;

void debug_main()
{
  if (!debugger_running)
  {
    debugger_running = true;

    int argc = 1;
    char *argv[] = { "debug" };
    QApplication app(argc, argv);

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
