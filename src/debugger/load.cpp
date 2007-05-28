#include <QApplication>
#include <QThread>
#include "load.h"
#include "ui.h"

static QApplication *app = 0;
static int app_exit_num = 0;

extern "C" { void zstart(); }

class ZSNESThread : public QThread
{
  public:
  void run()
  {
    zstart();
  }
} zthread;

void debug_main()
{
  if (!app)
  {
    int argc = 1;
    char *argv[] = { "debug" };
    app = new QApplication(argc, argv);

    DebuggerDialog::showDebuggerDialog(0);

    zthread.start();
    app->exec();
    zthread.exit();
    zthread.wait();
    DebuggerDialog::destroyDebuggerDialog();
    delete app;
    exit(app_exit_num);
  }
}


void debug_exit(int exit_num)
{
  if (app)
  {
    app_exit_num = exit_num;
    app->quit();
  }
  else
  {
    exit(exit_num);
  }
}
