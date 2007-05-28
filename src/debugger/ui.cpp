#include <QMessageBox>

#include "ui.h"

QtDebugger::QtDebugger(QWidget *parent) : QMainWindow(parent)
{
  ui.setupUi(this);
}

QtDebugger::~QtDebugger()
{

}

void QtDebugger::closeEvent(QCloseEvent *event)
{
  QMessageBox::information(this, tr("Wish to exit?"),
                           tr("If you'd like to exit, exit from within ZSNES itself."));
  event->ignore();
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
