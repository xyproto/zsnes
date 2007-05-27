#include <QApplication>
#include <QThread>
#include "load.h"
#include "ui.h"

static QApplication *app = 0;
static QWidget *widget = 0;

void debug_main()
{
  if (!app)
  {
    int argc = 1;
    char *argv[] = { "debug" };
    app = new QApplication(argc, argv);
    widget = new QWidget();
    DebuggerDialog::showDebuggerDialog(widget);
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
  if (widget)
  {
    delete widget;
    widget = 0;
  }
}
