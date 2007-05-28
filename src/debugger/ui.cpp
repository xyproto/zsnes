#include "ui.h"

DebuggerDialog::DebuggerDialog(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
}

DebuggerDialog::~DebuggerDialog()
{

}

DebuggerDialog *DebuggerDialog::singleton = 0;

void DebuggerDialog::showDebuggerDialog(QWidget *parent)
{
  if (!singleton)
  {
    singleton = new DebuggerDialog(parent);
  }
  singleton->show();
}

void DebuggerDialog::destroyDebuggerDialog()
{
  if (singleton)
  {
    delete singleton;
    singleton = 0;
  }
}
