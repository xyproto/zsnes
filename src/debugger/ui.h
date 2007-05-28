#ifndef UI_H
#define UI_H

#include <QDialog>

#ifdef __UNIXSDL__ //I hate this hack -Nach
#include "debugger/ui_debugger.h"
#else
#include "ui_debugger.h"
#endif

class DebuggerDialog : public QDialog
{
  Q_OBJECT

  private:
  Ui::Debugger ui;

  static DebuggerDialog *singleton;

  DebuggerDialog(QWidget *parent);
  ~DebuggerDialog();

  private slots:

  public:
  static void showDebuggerDialog(QWidget *parent);
};

#endif
