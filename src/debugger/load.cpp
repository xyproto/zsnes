#include <QApplication>
#include <QThread>
#include "load.h"
#include "ui.h"

static QApplication *app = 0;

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

    DebuggerDialog::showDebuggerDialog(0);

#ifdef Q_OS_WIN
    ZSNESThread zthread;
    zthread.start();
	app->exec();
	zthread.terminate();
	exit(0);
#endif

    atexit(debug_exit);
  }
}

void debug_run()
{
  if (app)
  {
    QApplication::processEvents();
  }
}

void debug_exit()
{
}
