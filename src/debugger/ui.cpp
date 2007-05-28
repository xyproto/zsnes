#include "ui.h"

QtDebugger::QtDebugger(QWidget *parent) : QMainWindow(parent)
{
  ui.setupUi(this);
}

QtDebugger::~QtDebugger()
{

}

QtDebugger *QtDebugger::singleton = 0;

void QtDebugger::showQtDebugger(QWidget *parent)
{
  if (!singleton)
  {
    singleton = new QtDebugger(parent);
  }
  singleton->show();
}

void QtDebugger::destroyQtDebugger()
{
  if (singleton)
  {
    delete singleton;
    singleton = 0;
  }
}
