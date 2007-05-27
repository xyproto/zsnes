#include <QApplication>
#include <QThread>
#include "load.h"
#include "ui.h"

QApplication *app = 0;

extern "C" { void zstart(); }

class ZSNESThread : public QThread
{
  public:
  void run()
  {
    zstart();
  }
};

void debug_main()
{
  if (!app)
  {
    int argc = 1;
    char *argv[] = { "debug" };
    app = new QApplication(argc, argv);

    //DebuggerThread *debuggerThread = new DebuggerThread;
    //debuggerThread->start();

    ZSNESThread zthread;
    zthread.start();

    QWidget widget;
    DebuggerDialog::showDebuggerDialog(&widget);
  }
}
